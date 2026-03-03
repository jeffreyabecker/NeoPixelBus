#include <unity.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include <ArduinoFake.h>

#include "colors/Color.h"
#include "protocols/Ws2812xProtocol.h"
#include "transports/OneWireEncoding.h"
#include "transports/OneWireTiming.h"
#include "transports/OneWireWrapper.h"

namespace
{
    using namespace fakeit;

    struct TransportSpySettings
    {
        bool invert{false};
        uint32_t clockRateHz{0};
    };

    class TransportSpy : public lw::ITransport
    {
    public:
        using TransportSettingsType = TransportSpySettings;

        explicit TransportSpy(TransportSettingsType)
        {
        }

        void begin() override
        {
            ++beginCount;
            calls.emplace_back("begin");
        }

        void beginTransaction() override
        {
            ++beginTransactionCount;
            calls.emplace_back("beginTransaction");
        }

        void transmitBytes(lw::span<uint8_t> data) override
        {
            ++transmitCount;
            calls.emplace_back("transmit");
            lastTransmitted.assign(data.begin(), data.end());
            transmittedSizes.push_back(data.size());
        }

        void endTransaction() override
        {
            ++endTransactionCount;
            calls.emplace_back("endTransaction");
        }

        bool isReadyToUpdate() const override
        {
            return ready;
        }

        void clearCallLog()
        {
            calls.clear();
        }

        int beginCount{0};
        int beginTransactionCount{0};
        int transmitCount{0};
        int endTransactionCount{0};
        bool ready{true};
        std::vector<std::string> calls{};
        std::vector<uint8_t> lastTransmitted{};
        std::vector<size_t> transmittedSizes{};
    };

    using Wrapper = lw::OneWireWrapper<TransportSpy>;
    using WrapperNoReset = lw::OneWireWrapper<TransportSpy, 0, 0, false>;
    using WrapperIdleHigh = lw::OneWireWrapper<TransportSpy, 0, 0, true>;

    class WrapperTransportAdapter : public lw::ITransport
    {
    public:
        explicit WrapperTransportAdapter(Wrapper::TransportSettingsType cfg)
            : wrapper(cfg)
        {
        }

        void begin() override
        {
            wrapper.begin();
        }

        void beginTransaction() override
        {
            wrapper.TransportSpy::beginTransaction();
        }

        void transmitBytes(lw::span<uint8_t> data) override
        {
            wrapper.transmitBytes(data);
        }

        void endTransaction() override
        {
            wrapper.TransportSpy::endTransaction();
        }

        bool isReadyToUpdate() const override
        {
            return wrapper.isReadyToUpdate();
        }

        Wrapper wrapper;
    };

    static uint32_t gMicrosNow = 0;

    template <size_t N>
    void assert_bytes_equal(const std::vector<uint8_t> &actual, const std::array<uint8_t, N> &expected)
    {
        TEST_ASSERT_EQUAL_UINT32(expected.size(), static_cast<uint32_t>(actual.size()));
        for (size_t idx = 0; idx < N; ++idx)
        {
            TEST_ASSERT_EQUAL_UINT8(expected[idx], actual[idx]);
        }
    }

    void assert_bytes_equal(const std::vector<uint8_t> &actual, const std::vector<uint8_t> &expected)
    {
        TEST_ASSERT_EQUAL_UINT32(expected.size(), static_cast<uint32_t>(actual.size()));
        for (size_t idx = 0; idx < expected.size(); ++idx)
        {
            TEST_ASSERT_EQUAL_UINT8(expected[idx], actual[idx]);
        }
    }

    size_t expected_encoded_size(size_t sourceBytes,
                                 uint8_t encodedBitsPerDataBit)
    {
        return ((sourceBytes * 8U * encodedBitsPerDataBit) + 7U) / 8U;
    }

    std::vector<uint8_t> encode_reference_bits(const std::vector<uint8_t> &src,
                                               uint8_t encodedOne,
                                               uint8_t encodedZero,
                                               uint8_t encodedBitsPerDataBit)
    {
        std::vector<uint8_t> out;
        out.reserve(expected_encoded_size(src.size(), encodedBitsPerDataBit));

        uint32_t current = 0;
        uint8_t bitsInCurrent = 0;
        for (uint8_t input : src)
        {
            uint8_t value = input;
            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                const uint8_t encoded = (value & 0x80) ? encodedOne : encodedZero;
                value <<= 1;

                current = (current << encodedBitsPerDataBit) | encoded;
                bitsInCurrent += encodedBitsPerDataBit;

                while (bitsInCurrent >= 8)
                {
                    const uint8_t shift = static_cast<uint8_t>(bitsInCurrent - 8);
                    out.push_back(static_cast<uint8_t>((current >> shift) & 0xFFu));
                    bitsInCurrent = shift;
                    if (bitsInCurrent == 0)
                    {
                        current = 0;
                    }
                    else
                    {
                        current &= (static_cast<uint32_t>(1u) << bitsInCurrent) - 1u;
                    }
                }
            }
        }

        if (bitsInCurrent > 0)
        {
            out.push_back(static_cast<uint8_t>(current << static_cast<uint8_t>(8 - bitsInCurrent)));
        }

        return out;
    }

    template <typename TProtocol>
    std::vector<uint8_t> bind_protocol_buffer(TProtocol &protocol)
    {
        std::vector<uint8_t> buffer(protocol.requiredBufferSizeBytes(), 0);
        protocol.setBuffer(lw::span<uint8_t>{buffer.data(), buffer.size()});
        return buffer;
    }

    Wrapper::TransportSettingsType make_default_config(void)
    {
        Wrapper::TransportSettingsType cfg{};
        cfg.clockRateHz = 0;
        cfg.timing = lw::timing::Ws2812x;
        return cfg;
    }

    size_t compute_reset_bytes(const Wrapper::TransportSettingsType &cfg,
                               uint8_t resetMultiplier)
    {
        if (resetMultiplier == 0)
        {
            return 0;
        }

        Wrapper wrapper(cfg);
        wrapper.begin();
        std::array<uint8_t, 1> empty{};
        wrapper.transmitBytes(lw::span<uint8_t>{empty.data(), 0});
        return wrapper.lastTransmitted.size() * static_cast<size_t>(resetMultiplier);
    }

    size_t compute_encoded_payload_bytes(size_t srcSize,
                                         lw::EncodedClockDataBitPattern pattern)
    {
        if (srcSize == 0)
        {
            return 0;
        }

        std::vector<uint8_t> src(srcSize, 0xA5);
        std::vector<uint8_t> dest(srcSize * 4U, 0x00);

        if (pattern == lw::EncodedClockDataBitPattern::FourStep)
        {
            return Wrapper::encode4StepBytes(dest.data(), src.data(), src.size());
        }

        return Wrapper::encode3StepBytes(dest.data(), src.data(), src.size());
    }

    void test_1_1_1_construction_and_begin_initialization(void)
    {
        gMicrosNow = 1234;
        auto cfg = make_default_config();
        Wrapper wrapper(cfg);

        wrapper.begin();

        TEST_ASSERT_EQUAL_INT(1, wrapper.beginCount);
        TEST_ASSERT_TRUE(wrapper.isReadyToUpdate());
    }

    void test_1_1_2_3step_4step_encode_length(void)
    {
        const std::array<uint8_t, 5> src{0x00, 0x01, 0x7F, 0x80, 0xFF};
        std::array<uint8_t, 32> dest{};

        TEST_ASSERT_EQUAL_UINT32(0U, static_cast<uint32_t>(Wrapper::encode3StepBytes(dest.data(), src.data(), 0)));
        TEST_ASSERT_EQUAL_UINT32(expected_encoded_size(1U, 3U), static_cast<uint32_t>(Wrapper::encode3StepBytes(dest.data(), src.data(), 1)));
        TEST_ASSERT_EQUAL_UINT32(expected_encoded_size(src.size(), 3U), static_cast<uint32_t>(Wrapper::encode3StepBytes(dest.data(), src.data(), src.size())));

        TEST_ASSERT_EQUAL_UINT32(0U, static_cast<uint32_t>(Wrapper::encode4StepBytes(dest.data(), src.data(), 0)));
        TEST_ASSERT_EQUAL_UINT32(expected_encoded_size(1U, 4U), static_cast<uint32_t>(Wrapper::encode4StepBytes(dest.data(), src.data(), 1)));
        TEST_ASSERT_EQUAL_UINT32(expected_encoded_size(src.size(), 4U), static_cast<uint32_t>(Wrapper::encode4StepBytes(dest.data(), src.data(), src.size())));
    }

    void test_1_1_3_encode_golden_patterns(void)
    {
        std::array<uint8_t, 4> src{0x00, 0xFF, 0x80, 0x01};
        std::array<uint8_t, 32> dest{};

        const size_t out3 = Wrapper::encode3StepBytes(dest.data(), src.data(), src.size());
        const auto expected3 = encode_reference_bits(std::vector<uint8_t>{src.begin(), src.end()},
                                                     lw::OneWireEncoding::EncodedOne3Step,
                                                     lw::OneWireEncoding::EncodedZero3Step,
                                                     3);
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(expected3.size()), static_cast<uint32_t>(out3));
        assert_bytes_equal(std::vector<uint8_t>{dest.begin(), dest.begin() + out3}, expected3);

        const size_t out4 = Wrapper::encode4StepBytes(dest.data(), src.data(), src.size());
        const auto expected4 = encode_reference_bits(std::vector<uint8_t>{src.begin(), src.end()},
                                                     lw::OneWireEncoding::EncodedOne4Step,
                                                     lw::OneWireEncoding::EncodedZero4Step,
                                                     4);
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(expected4.size()), static_cast<uint32_t>(out4));
        assert_bytes_equal(std::vector<uint8_t>{dest.begin(), dest.begin() + out4}, expected4);
    }

    void test_1_1_4_wrapper_does_not_manage_transactions(void)
    {
        std::array<uint8_t, 2> payload{0x12, 0x34};

        auto cfg = make_default_config();
        Wrapper wrapper(cfg);
        wrapper.begin();
        wrapper.clearCallLog();

        wrapper.transmitBytes(lw::span<uint8_t>{payload.data(), payload.size()});

        TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(wrapper.calls.size()));
        TEST_ASSERT_EQUAL_STRING("transmit", wrapper.calls[0].c_str());
    }

    void test_1_1_4b_protocol_idle_high_prefix_and_inverts_encoding(void)
    {
        auto cfg = make_default_config();
        cfg.clockRateHz = 2400000;

        WrapperNoReset normal(cfg);
        WrapperIdleHigh idleHigh(cfg);

        normal.begin();
        idleHigh.begin();

        std::array<uint8_t, 1> payload{0xA5};
        normal.transmitBytes(lw::span<uint8_t>{payload.data(), payload.size()});
        idleHigh.transmitBytes(lw::span<uint8_t>{payload.data(), payload.size()});

        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(normal.lastTransmitted.size() + 1),
                                 static_cast<uint32_t>(idleHigh.lastTransmitted.size()));
        TEST_ASSERT_EQUAL_UINT8(0xFF, idleHigh.lastTransmitted[0]);

        for (size_t i = 0; i < normal.lastTransmitted.size(); ++i)
        {
            TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(~normal.lastTransmitted[i]), idleHigh.lastTransmitted[i + 1]);
        }
    }

    void test_1_1_5_timing_and_readiness_gate(void)
    {
        auto cfg = make_default_config();
        cfg.clockRateHz = 0;
        cfg.timing.resetNs = 300000;

        Wrapper wrapper(cfg);
        wrapper.ready = true;

        gMicrosNow = 1000;
        wrapper.begin();
        std::array<uint8_t, 1> payload{0xAA};
        wrapper.transmitBytes(lw::span<uint8_t>{payload.data(), payload.size()});

        gMicrosNow = 1299;
        TEST_ASSERT_TRUE(wrapper.isReadyToUpdate());

        gMicrosNow = 1300;
        TEST_ASSERT_TRUE(wrapper.isReadyToUpdate());

        wrapper.ready = false;
        TEST_ASSERT_FALSE(wrapper.isReadyToUpdate());
    }

    void test_1_1_6_bitrate_dependent_frame_duration(void)
    {
        std::array<uint8_t, 10> payload{};

        {
            auto cfg = make_default_config();
            cfg.clockRateHz = 0;
            cfg.timing.resetNs = 300000;
            Wrapper wrapper(cfg);
            wrapper.begin();

            gMicrosNow = 5000;
            wrapper.transmitBytes(lw::span<uint8_t>{payload.data(), payload.size()});

            gMicrosNow = 5299;
            TEST_ASSERT_TRUE(wrapper.isReadyToUpdate());

            gMicrosNow = 5300;
            TEST_ASSERT_TRUE(wrapper.isReadyToUpdate());
        }

        {
            auto cfg = make_default_config();
            cfg.clockRateHz = 100000;
            cfg.timing.resetNs = 300000;
            Wrapper wrapper(cfg);
            wrapper.begin();

            gMicrosNow = 7000;
            wrapper.transmitBytes(lw::span<uint8_t>{payload.data(), payload.size()});

            gMicrosNow = 9399;
            TEST_ASSERT_TRUE(wrapper.isReadyToUpdate());

            gMicrosNow = 9400;
            TEST_ASSERT_TRUE(wrapper.isReadyToUpdate());
        }
    }

    void test_1_1_7_protocol_integration_length_consistency_ws2812x(void)
    {
        const uint16_t pixelCount = 4;
        const std::array<lw::Rgbcw8Color, 4> colors{
            lw::Rgbcw8Color{1, 2, 3, 4, 5},
            lw::Rgbcw8Color{6, 7, 8, 9, 10},
            lw::Rgbcw8Color{11, 12, 13, 14, 15},
            lw::Rgbcw8Color{16, 17, 18, 19, 20}};

        auto run_case = [&](const char *channelOrder, size_t expectedChannels)
        {
            auto cfg = make_default_config();
            cfg.timing = lw::timing::Ws2812x;

            auto transport = std::make_unique<WrapperTransportAdapter>(cfg);
            auto *transportRaw = transport.get();

            lw::Ws2812xProtocol<lw::Rgbcw8Color> protocol(
                pixelCount,
                lw::Ws2812xProtocolSettings{transport.get(), channelOrder});

            auto protocolBuffer = bind_protocol_buffer(protocol);
            protocol.initialize();
            protocol.update(colors);

            const size_t protocolBytes = static_cast<size_t>(pixelCount) * expectedChannels;
                        const size_t protocolResetBytes = lw::OneWireEncoding::computeResetBytes(lw::timing::Ws2812x, 0, 1);
                        const size_t protocolFrameBytes = compute_encoded_payload_bytes(protocolBytes,
                                                                                                                                                        cfg.timing.bitPattern()) +
                                                                                            protocolResetBytes;
                        const size_t expectedLength = compute_encoded_payload_bytes(protocolFrameBytes,
                                                                                                                                                cfg.timing.bitPattern()) +
                                          compute_reset_bytes(cfg, 1);
            TEST_ASSERT_EQUAL_UINT32(expectedLength, static_cast<uint32_t>(transportRaw->wrapper.lastTransmitted.size()));
        };

        run_case(lw::ChannelOrder::GRB::value, 3);
        run_case(lw::ChannelOrder::GRBW::value, 4);
        run_case(lw::ChannelOrder::GRBCW::value, 5);
        run_case(nullptr, 3);
        run_case("", 3);
    }

    void test_1_1_8_p0_byte_boundary_carry_integrity(void)
    {
        const std::array<uint8_t, 2> src{0x80, 0x01};
        std::array<uint8_t, 16> dest{};

        const size_t out3 = Wrapper::encode3StepBytes(dest.data(), src.data(), src.size());
        const auto expected3 = encode_reference_bits(std::vector<uint8_t>{src.begin(), src.end()},
                                                     lw::OneWireEncoding::EncodedOne3Step,
                                                     lw::OneWireEncoding::EncodedZero3Step,
                                                     3);
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(expected3.size()), static_cast<uint32_t>(out3));
        assert_bytes_equal(std::vector<uint8_t>{dest.begin(), dest.begin() + out3}, expected3);

        const size_t out4 = Wrapper::encode4StepBytes(dest.data(), src.data(), src.size());
        const auto expected4 = encode_reference_bits(std::vector<uint8_t>{src.begin(), src.end()},
                                                     lw::OneWireEncoding::EncodedOne4Step,
                                                     lw::OneWireEncoding::EncodedZero4Step,
                                                     4);
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(expected4.size()), static_cast<uint32_t>(out4));
        assert_bytes_equal(std::vector<uint8_t>{dest.begin(), dest.begin() + out4}, expected4);
    }

    void test_1_1_9_p0_large_payload_resizing_stability(void)
    {
        auto cfg = make_default_config();
        cfg.timing = lw::timing::Ws2812x;
        Wrapper wrapper(cfg);
        wrapper.begin();

        const size_t suffixResetBytes = compute_reset_bytes(cfg, 1);

        const std::vector<size_t> sizes{256, 4096, 16384, 1024, 2048};
        for (size_t srcSize : sizes)
        {
            std::vector<uint8_t> payload(srcSize, 0xA5);
            wrapper.transmitBytes(lw::span<uint8_t>{payload.data(), payload.size()});
            const size_t expectedPayloadBytes = compute_encoded_payload_bytes(srcSize,
                                                                              cfg.timing.bitPattern());
            TEST_ASSERT_EQUAL_UINT32(expectedPayloadBytes + suffixResetBytes, static_cast<uint32_t>(wrapper.lastTransmitted.size()));
        }

        TEST_ASSERT_EQUAL_UINT32(sizes.size(), static_cast<uint32_t>(wrapper.transmittedSizes.size()));
    }

    void test_1_1_10_ws2812x_16bit_components_emit_both_bytes(void)
    {
        TransportSpy transport(TransportSpySettings{});

        lw::Ws2812xProtocol<lw::Rgb16Color> protocol(
            1,
            lw::Ws2812xProtocolSettings{&transport, lw::ChannelOrder::GRB::value});

        const std::array<lw::Rgb16Color, 1> colors{
            lw::Rgb16Color{0x1122, 0x3344, 0x5566}};

        auto protocolBuffer = bind_protocol_buffer(protocol);
        protocol.initialize();
        protocol.update(colors);

        TEST_ASSERT_EQUAL_INT(1, transport.beginCount);
        TEST_ASSERT_EQUAL_INT(1, transport.transmitCount);

        std::vector<uint8_t> raw{0x33, 0x44, 0x11, 0x22, 0x55, 0x66};
        const size_t encodedSize = lw::OneWireEncoding::expandedPayloadSizeBytes(raw.size(), lw::timing::Ws2812x.bitPattern());
        const size_t resetBytes = lw::OneWireEncoding::computeResetBytes(lw::timing::Ws2812x, 0, 1);
        std::vector<uint8_t> expected(encodedSize + resetBytes, 0x00);
        const size_t expectedSize = lw::OneWireEncoding::encodeWithResets(raw.data(),
                                                                           raw.size(),
                                                                           expected.data(),
                                                                           expected.size(),
                                                                           lw::timing::Ws2812x,
                                                                           0,
                                                                           0,
                                                                           1,
                                                                           false);
        expected.resize(expectedSize);
        assert_bytes_equal(transport.lastTransmitted, expected);
    }

    void test_1_1_11_edge_contract_cases(void)
    {
        {
            auto cfg = make_default_config();
            cfg.timing = lw::timing::Ws2812x;
            Wrapper wrapper(cfg);
            wrapper.begin();

            const size_t suffixResetBytes = compute_reset_bytes(cfg, 1);

            std::array<uint8_t, 1> payload{0xFF};
            wrapper.transmitBytes(lw::span<uint8_t>{payload.data(), payload.size()});
            const size_t expectedPayloadBytes = compute_encoded_payload_bytes(payload.size(),
                                                                              cfg.timing.bitPattern());
            TEST_ASSERT_EQUAL_UINT32(expectedPayloadBytes + suffixResetBytes, static_cast<uint32_t>(wrapper.lastTransmitted.size()));
        }

        {
            auto cfg = make_default_config();
            cfg.clockRateHz = 1;
            cfg.timing.resetNs = 10000;
            Wrapper wrapper(cfg);
            wrapper.begin();

            gMicrosNow = 200;
            std::array<uint8_t, 1> payload{0x01};
            wrapper.transmitBytes(lw::span<uint8_t>{payload.data(), payload.size()});

            gMicrosNow = 23999999;
            TEST_ASSERT_TRUE(wrapper.isReadyToUpdate());

            gMicrosNow = 24000200;
            TEST_ASSERT_TRUE(wrapper.isReadyToUpdate());
        }

        {
            auto cfg = make_default_config();
            Wrapper wrapper(cfg);
            wrapper.begin();

            const size_t suffixResetBytes = compute_reset_bytes(cfg, 1);

            wrapper.transmitBytes(lw::span<uint8_t>{});
            TEST_ASSERT_EQUAL_INT(1, wrapper.transmitCount);
            TEST_ASSERT_EQUAL_UINT32(suffixResetBytes, static_cast<uint32_t>(wrapper.lastTransmitted.size()));
        }

        {
            auto cfg = make_default_config();
            cfg.timing.resetNs = 300000;
            Wrapper wrapper(cfg);
            wrapper.begin();

            gMicrosNow = std::numeric_limits<uint32_t>::max() - 10U;
            std::array<uint8_t, 1> payload{0xAA};
            wrapper.transmitBytes(lw::span<uint8_t>{payload.data(), payload.size()});

            gMicrosNow = 100U;
            TEST_ASSERT_TRUE(wrapper.isReadyToUpdate());

            gMicrosNow = 289U;
            TEST_ASSERT_TRUE(wrapper.isReadyToUpdate());
        }

        {
            auto cfg = make_default_config();
            Wrapper wrapper(cfg);
            wrapper.begin();

            const size_t suffixResetBytes = compute_reset_bytes(cfg, 1);

            std::array<uint8_t, 3> payloadA{0xFF, 0xFF, 0xFF};
            wrapper.transmitBytes(lw::span<uint8_t>{payloadA.data(), payloadA.size()});
            const auto firstSize = wrapper.lastTransmitted.size();

            std::array<uint8_t, 1> payloadB{0x00};
            wrapper.transmitBytes(lw::span<uint8_t>{payloadB.data(), payloadB.size()});
            const auto secondSize = wrapper.lastTransmitted.size();

            TEST_ASSERT_EQUAL_UINT32(compute_encoded_payload_bytes(payloadA.size(), cfg.timing.bitPattern()) + suffixResetBytes,
                                     static_cast<uint32_t>(firstSize));
            TEST_ASSERT_EQUAL_UINT32(compute_encoded_payload_bytes(payloadB.size(), cfg.timing.bitPattern()) + suffixResetBytes,
                                     static_cast<uint32_t>(secondSize));
        }
    }

    void test_1_1_12_helper_parity_with_wrapper_encoding(void)
    {
        const std::vector<size_t> sizes{0, 1, 2, 5, 17, 64};

        for (const size_t srcSize : sizes)
        {
            std::vector<uint8_t> src(srcSize, 0);
            for (size_t index = 0; index < srcSize; ++index)
            {
                src[index] = static_cast<uint8_t>((index * 37U + 11U) & 0xFFU);
            }

            std::vector<uint8_t> wrapper3(srcSize * 4U + 16U, 0x00);
            std::vector<uint8_t> helper3(srcSize * 4U + 16U, 0x00);
            const size_t wrapper3Size = Wrapper::encode3StepBytes(wrapper3.data(), src.data(), src.size());
            const size_t helper3Size = lw::OneWireEncoding::encodeStepBytes(helper3.data(),
                                                                             src.data(),
                                                                             src.size(),
                                                                             lw::OneWireEncoding::EncodedOne3Step,
                                                                             lw::OneWireEncoding::EncodedZero3Step,
                                                                             3);

            TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(wrapper3Size), static_cast<uint32_t>(helper3Size));
            for (size_t i = 0; i < wrapper3Size; ++i)
            {
                TEST_ASSERT_EQUAL_UINT8(wrapper3[i], helper3[i]);
            }

            std::vector<uint8_t> wrapper4(srcSize * 4U + 16U, 0x00);
            std::vector<uint8_t> helper4(srcSize * 4U + 16U, 0x00);
            const size_t wrapper4Size = Wrapper::encode4StepBytes(wrapper4.data(), src.data(), src.size());
            const size_t helper4Size = lw::OneWireEncoding::encodeStepBytes(helper4.data(),
                                                                             src.data(),
                                                                             src.size(),
                                                                             lw::OneWireEncoding::EncodedOne4Step,
                                                                             lw::OneWireEncoding::EncodedZero4Step,
                                                                             4);

            TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(wrapper4Size), static_cast<uint32_t>(helper4Size));
            for (size_t i = 0; i < wrapper4Size; ++i)
            {
                TEST_ASSERT_EQUAL_UINT8(wrapper4[i], helper4[i]);
            }

            TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(Wrapper::expandedPayloadSizeBytes(srcSize, lw::EncodedClockDataBitPattern::ThreeStep)),
                                     static_cast<uint32_t>(lw::OneWireEncoding::expandedPayloadSizeBytes(srcSize, lw::EncodedClockDataBitPattern::ThreeStep)));
            TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(Wrapper::expandedPayloadSizeBytes(srcSize, lw::EncodedClockDataBitPattern::FourStep)),
                                     static_cast<uint32_t>(lw::OneWireEncoding::expandedPayloadSizeBytes(srcSize, lw::EncodedClockDataBitPattern::FourStep)));
        }
    }

    void test_1_1_13_helper_encode_in_place_parity(void)
    {
        const std::vector<size_t> sizes{0, 1, 3, 7, 19, 64};
        const std::array<lw::OneWireTiming, 2> timings{lw::timing::Ws2812x, lw::OneWireTiming::fromTargetKbps<lw::EncodedClockDataBitPattern::FourStep>(800)};

        for (const auto &timing : timings)
        {
            for (const size_t srcSize : sizes)
            {
                std::vector<uint8_t> src(srcSize, 0);
                for (size_t index = 0; index < srcSize; ++index)
                {
                    src[index] = static_cast<uint8_t>((index * 53U + 7U) & 0xFFU);
                }

                const size_t requiredBytes = lw::OneWireEncoding::expandedPayloadSizeBytes(srcSize, timing.bitPattern());

                std::vector<uint8_t> expected(requiredBytes + 8U, 0x00);
                const size_t expectedSize = (timing.bitPattern() == lw::EncodedClockDataBitPattern::FourStep)
                                                ? lw::OneWireEncoding::encodeStepBytes(expected.data(),
                                                                                       src.data(),
                                                                                       src.size(),
                                                                                       lw::OneWireEncoding::EncodedOne4Step,
                                                                                       lw::OneWireEncoding::EncodedZero4Step,
                                                                                       4)
                                                : lw::OneWireEncoding::encodeStepBytes(expected.data(),
                                                                                       src.data(),
                                                                                       src.size(),
                                                                                       lw::OneWireEncoding::EncodedOne3Step,
                                                                                       lw::OneWireEncoding::EncodedZero3Step,
                                                                                       3);

                std::vector<uint8_t> inPlace(requiredBytes + 8U, 0x00);
                for (size_t i = 0; i < srcSize; ++i)
                {
                    inPlace[i] = src[i];
                }

                const size_t inPlaceSize = lw::OneWireEncoding::encodeInPlace(inPlace.data(),
                                                                               srcSize,
                                                                               inPlace.data(),
                                                                               inPlace.size(),
                                                                               timing);

                TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(expectedSize), static_cast<uint32_t>(inPlaceSize));
                for (size_t i = 0; i < expectedSize; ++i)
                {
                    TEST_ASSERT_EQUAL_UINT8(expected[i], inPlace[i]);
                }
            }
        }
    }
}

void setUp(void)
{
    ArduinoFakeReset();

    gMicrosNow = 0;
    When(Method(ArduinoFake(Function), micros)).AlwaysDo(
        []() -> unsigned long
        {
            return gMicrosNow;
        });
    When(Method(ArduinoFake(Function), millis)).AlwaysReturn(0UL);
    When(Method(ArduinoFake(Function), yield)).AlwaysDo(
        []()
        {
        });
}

void tearDown(void)
{
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(test_1_1_1_construction_and_begin_initialization);
    RUN_TEST(test_1_1_2_3step_4step_encode_length);
    RUN_TEST(test_1_1_3_encode_golden_patterns);
    RUN_TEST(test_1_1_4_wrapper_does_not_manage_transactions);
    RUN_TEST(test_1_1_4b_protocol_idle_high_prefix_and_inverts_encoding);
    RUN_TEST(test_1_1_5_timing_and_readiness_gate);
    RUN_TEST(test_1_1_6_bitrate_dependent_frame_duration);
    RUN_TEST(test_1_1_7_protocol_integration_length_consistency_ws2812x);
    RUN_TEST(test_1_1_8_p0_byte_boundary_carry_integrity);
    RUN_TEST(test_1_1_9_p0_large_payload_resizing_stability);
    RUN_TEST(test_1_1_10_ws2812x_16bit_components_emit_both_bytes);
    RUN_TEST(test_1_1_11_edge_contract_cases);
    RUN_TEST(test_1_1_12_helper_parity_with_wrapper_encoding);
    RUN_TEST(test_1_1_13_helper_encode_in_place_parity);
    return UNITY_END();
}

