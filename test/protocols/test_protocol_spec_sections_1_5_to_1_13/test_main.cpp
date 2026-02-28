#include <unity.h>

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include <ArduinoFake.h>

#include "protocols/Lpd6803Protocol.h"
#include "protocols/Lpd8806Protocol.h"
#include "protocols/P9813Protocol.h"
#include "protocols/Sm16716Protocol.h"
#include "protocols/Sm168xProtocol.h"
#include "protocols/Tlc5947Protocol.h"
#include "protocols/Tlc59711Protocol.h"
#include "protocols/Tm1814Protocol.h"
#include "protocols/Tm1914Protocol.h"

namespace
{
    using namespace fakeit;

    struct TransportSpySettings
    {
    };

    class TransportSpy : public npb::ITransport
    {
    public:
        using TransportSettingsType = TransportSpySettings;
        using TransportCategory = npb::TransportTag;

        explicit TransportSpy(TransportSettingsType)
        {
        }

        void begin() override
        {
            ++beginCount;
        }

        void beginTransaction() override
        {
            ++beginTransactionCount;
        }

        void transmitBytes(npb::span<uint8_t> data) override
        {
            ++transmitCount;
            packets.emplace_back(data.begin(), data.end());
        }

        void endTransaction() override
        {
            ++endTransactionCount;
        }

        bool isReadyToUpdate() const override
        {
            return ready;
        }

        int beginCount{0};
        int beginTransactionCount{0};
        int transmitCount{0};
        int endTransactionCount{0};
        bool ready{true};
        std::vector<std::vector<uint8_t>> packets{};
    };

    class OneWireTransportSpy : public npb::ITransport
    {
    public:
        using TransportSettingsType = TransportSpySettings;
        using TransportCategory = npb::OneWireTransportTag;

        explicit OneWireTransportSpy(TransportSettingsType)
        {
        }

        void begin() override
        {
            ++beginCount;
        }

        void beginTransaction() override
        {
            ++beginTransactionCount;
        }

        void transmitBytes(npb::span<uint8_t> data) override
        {
            ++transmitCount;
            packets.emplace_back(data.begin(), data.end());
        }

        void endTransaction() override
        {
            ++endTransactionCount;
        }

        bool isReadyToUpdate() const override
        {
            return ready;
        }

        int beginCount{0};
        int beginTransactionCount{0};
        int transmitCount{0};
        int endTransactionCount{0};
        bool ready{true};
        std::vector<std::vector<uint8_t>> packets{};
    };

    static uint32_t gMicrosNow = 0;

    void test_1_5_1_lpd6803_packed_5_5_5_serialization(void)
    {
        auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();

        npb::Lpd6803Protocol protocol(1, npb::Lpd6803ProtocolSettings{transport.get(), npb::ChannelOrder::RGB::value});
        protocol.initialize();
        protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{0xFF, 0x00, 0x88}});

        const std::vector<uint8_t> payload(spy->packets[0].begin() + 4, spy->packets[0].begin() + 6);
        TEST_ASSERT_EQUAL_UINT32(2U, static_cast<uint32_t>(payload.size()));
        TEST_ASSERT_EQUAL_UINT8(0xFC, payload[0]);
        TEST_ASSERT_EQUAL_UINT8(0x11, payload[1]);
    }

    void test_1_5_2_lpd6803_framing_end_frame_size(void)
    {
        const std::array<uint16_t, 4> counts{1, 8, 9, 16};
        for (uint16_t n : counts)
        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Lpd6803Protocol protocol(n, npb::Lpd6803ProtocolSettings{transport.get(), npb::ChannelOrder::RGB::value});
            protocol.initialize();
            std::vector<npb::Rgb8Color> colors(n, npb::Rgb8Color{1, 2, 3});

            protocol.update(npb::span<const npb::Rgb8Color>{colors.data(), colors.size()});

            const size_t expectedSize = 4u + (static_cast<size_t>(n) * 2u) + ((static_cast<size_t>(n) + 7u) / 8u);
            TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(spy->packets.size()));
            TEST_ASSERT_EQUAL_UINT32(expectedSize, static_cast<uint32_t>(spy->packets[0].size()));
        }
    }

    void test_1_5_3_lpd6803_oversized_and_channel_order_safety(void)
    {
        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Lpd6803Protocol protocol(1, npb::Lpd6803ProtocolSettings{transport.get(), npb::ChannelOrder::RGB::value});
            protocol.initialize();
            protocol.update(std::array<npb::Rgb8Color, 2>{npb::Rgb8Color{1, 2, 3}, npb::Rgb8Color{4, 5, 6}});

            TEST_ASSERT_EQUAL_UINT32(2U, static_cast<uint32_t>(
                std::distance(spy->packets[0].begin() + 4, spy->packets[0].begin() + 6)));
        }

        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Lpd6803Protocol protocol(1, npb::Lpd6803ProtocolSettings{transport.get(), ""});
            protocol.initialize();
            protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{9, 10, 11}});

            TEST_ASSERT_EQUAL_UINT32(2U, static_cast<uint32_t>(
                std::distance(spy->packets[0].begin() + 4, spy->packets[0].begin() + 6)));
        }
    }

    void test_1_6_1_lpd8806_7bit_plus_msb_serialization(void)
    {
        auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();

        npb::Lpd8806Protocol protocol(1, npb::Lpd8806ProtocolSettings{transport.get(), npb::ChannelOrder::RGB::value});
        protocol.initialize();
        protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{0x00, 0xFF, 0x80}});

        const auto& payload = std::vector<uint8_t>(spy->packets[0].begin() + 1, spy->packets[0].begin() + 4);
        TEST_ASSERT_EQUAL_UINT32(3U, static_cast<uint32_t>(payload.size()));
        TEST_ASSERT_EQUAL_UINT8(0x80, payload[0]);
        TEST_ASSERT_EQUAL_UINT8(0xFF, payload[1]);
        TEST_ASSERT_EQUAL_UINT8(0xC0, payload[2]);
    }

    void test_1_6_2_lpd8806_symmetric_start_end_framing(void)
    {
        const std::array<uint16_t, 3> counts{1, 32, 33};
        for (uint16_t n : counts)
        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Lpd8806Protocol protocol(n, npb::Lpd8806ProtocolSettings{transport.get(), npb::ChannelOrder::RGB::value});
            protocol.initialize();
            std::vector<npb::Rgb8Color> colors(n, npb::Rgb8Color{1, 2, 3});

            protocol.update(npb::span<const npb::Rgb8Color>{colors.data(), colors.size()});

            const size_t frameSize = (static_cast<size_t>(n) + 31u) / 32u;
            TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(spy->packets.size()));
            TEST_ASSERT_EQUAL_UINT32(frameSize + (static_cast<size_t>(n) * 3u) + frameSize,
                                     static_cast<uint32_t>(spy->packets[0].size()));
            TEST_ASSERT_EQUAL_UINT8(0x00, spy->packets[0][0]);
            TEST_ASSERT_EQUAL_UINT8(0xFF, spy->packets[0].back());
        }
    }

    void test_1_6_3_lpd8806_oversized_and_channel_order_safety(void)
    {
        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Lpd8806Protocol protocol(1, npb::Lpd8806ProtocolSettings{transport.get(), npb::ChannelOrder::RGB::value});
            protocol.initialize();
            protocol.update(std::array<npb::Rgb8Color, 2>{npb::Rgb8Color{1, 2, 3}, npb::Rgb8Color{4, 5, 6}});

            TEST_ASSERT_EQUAL_UINT32(3U, static_cast<uint32_t>(
                std::distance(spy->packets[0].begin() + 1, spy->packets[0].begin() + 4)));
        }

        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Lpd8806Protocol protocol(1, npb::Lpd8806ProtocolSettings{transport.get(), ""});
            protocol.initialize();
            protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{9, 10, 11}});

            TEST_ASSERT_EQUAL_UINT32(3U, static_cast<uint32_t>(
                std::distance(spy->packets[0].begin() + 1, spy->packets[0].begin() + 4)));
        }
    }

    void test_1_7_1_and_1_7_2_p9813_header_checksum_and_framing(void)
    {
        auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();

        npb::P9813Protocol protocol(1, npb::P9813ProtocolSettings{transport.get()});
        protocol.initialize();
        protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{0x80, 0x40, 0x00}});

        const auto& payload = std::vector<uint8_t>(spy->packets[0].begin() + 4, spy->packets[0].begin() + 8);
        const uint8_t expectedHeader = static_cast<uint8_t>(0xC0 | (((~0x00u >> 6) & 0x03) << 4) | (((~0x40u >> 6) & 0x03) << 2) | ((~0x80u >> 6) & 0x03));
        TEST_ASSERT_EQUAL_UINT8(expectedHeader, payload[0]);
        TEST_ASSERT_EQUAL_UINT8(0x00, payload[1]);
        TEST_ASSERT_EQUAL_UINT8(0x40, payload[2]);
        TEST_ASSERT_EQUAL_UINT8(0x80, payload[3]);

        TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(spy->packets.size()));
    }

    void test_1_7_3_p9813_oversized_span_safety(void)
    {
        auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();

        npb::P9813Protocol protocol(1, npb::P9813ProtocolSettings{transport.get()});
        protocol.initialize();
        protocol.update(std::array<npb::Rgb8Color, 2>{npb::Rgb8Color{1, 2, 3}, npb::Rgb8Color{4, 5, 6}});

        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(
            std::distance(spy->packets[0].begin() + 4, spy->packets[0].begin() + 8)));
    }

    void test_1_8_1_sm168x_variant_resolution_and_frame_sizing(void)
    {
        auto run_case = [&](npb::Sm168xVariant variant, size_t expectedFrameSize)
        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();

            npb::Sm168xProtocolSettings settings{};
            settings.bus = transport.get();
            settings.channelOrder = "RGBCW";
            settings.variant = variant;

            npb::Sm168xProtocol<npb::Rgbcw8Color> protocol(2, std::move(settings));
            protocol.update(std::array<npb::Rgbcw8Color, 2>{npb::Rgbcw8Color{1, 2, 3, 4, 5}, npb::Rgbcw8Color{6, 7, 8, 9, 10}});

            TEST_ASSERT_EQUAL_UINT32(expectedFrameSize, static_cast<uint32_t>(spy->packets[0].size()));
        };

        run_case(npb::Sm168xVariant::ThreeChannel, 8U);
        run_case(npb::Sm168xVariant::FourChannel, 10U);
        run_case(npb::Sm168xVariant::FiveChannel, 14U);
    }

    void test_1_8_3_sm168x_settings_trailer_encoding_masks(void)
    {
        auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();

        npb::Sm168xProtocolSettings settings{};
        settings.bus = transport.get();
        settings.channelOrder = "RGBCW";
        settings.variant = npb::Sm168xVariant::FiveChannel;
        settings.gains = {31, 32, 33, 1, 0};

        npb::Sm168xProtocol<npb::Rgbcw8Color> protocol(1, std::move(settings));
        protocol.update(std::array<npb::Rgbcw8Color, 1>{npb::Rgbcw8Color{10, 11, 12, 13, 14}});

        const auto& frame = spy->packets[0];
        TEST_ASSERT_EQUAL_UINT32(9U, static_cast<uint32_t>(frame.size()));
        TEST_ASSERT_EQUAL_UINT8(0xF8, frame[5]);
        TEST_ASSERT_EQUAL_UINT8(0x02, frame[6]);
        TEST_ASSERT_EQUAL_UINT8(0x00, frame[7]);
        TEST_ASSERT_EQUAL_UINT8(0x9F, frame[8]);
    }

    void test_1_8_4_sm168x_oversized_and_order_safety(void)
    {
        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();

            npb::Sm168xProtocolSettings settings{};
            settings.bus = transport.get();
            settings.channelOrder = "RGBCW";
            settings.variant = npb::Sm168xVariant::ThreeChannel;

            npb::Sm168xProtocol<npb::Rgbcw8Color> protocol(1, std::move(settings));
            protocol.update(std::array<npb::Rgbcw8Color, 2>{npb::Rgbcw8Color{1, 2, 3, 4, 5}, npb::Rgbcw8Color{6, 7, 8, 9, 10}});

            TEST_ASSERT_EQUAL_UINT32(5U, static_cast<uint32_t>(spy->packets[0].size()));
        }

        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();

            npb::Sm168xProtocolSettings settings{};
            settings.bus = transport.get();
            settings.channelOrder = "";
            settings.variant = npb::Sm168xVariant::ThreeChannel;

            npb::Sm168xProtocol<npb::Rgbcw8Color> protocol(1, std::move(settings));
            protocol.update(std::array<npb::Rgbcw8Color, 1>{npb::Rgbcw8Color{11, 12, 13, 14, 15}});

            TEST_ASSERT_EQUAL_UINT32(5U, static_cast<uint32_t>(spy->packets[0].size()));
        }
    }

    void test_1_9_1_sm16716_buffer_size_and_start_bit_prefix(void)
    {
        auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();

        npb::Sm16716Protocol protocol(1, npb::Sm16716ProtocolSettings{transport.get(), npb::ChannelOrder::RGB::value});
        protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{0, 0, 0}});

        TEST_ASSERT_EQUAL_UINT32(10U, static_cast<uint32_t>(spy->packets[0].size()));
        TEST_ASSERT_EQUAL_UINT8(0x00, spy->packets[0][0]);
        TEST_ASSERT_EQUAL_UINT8(0x00, spy->packets[0][1]);
        TEST_ASSERT_EQUAL_UINT8(0x20, spy->packets[0][6]);
    }

    void test_1_9_3_sm16716_oversized_and_order_safety(void)
    {
        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Sm16716Protocol protocol(1, npb::Sm16716ProtocolSettings{transport.get(), npb::ChannelOrder::RGB::value});
            protocol.update(std::array<npb::Rgb8Color, 2>{npb::Rgb8Color{1, 2, 3}, npb::Rgb8Color{4, 5, 6}});

            TEST_ASSERT_EQUAL_UINT32(10U, static_cast<uint32_t>(spy->packets[0].size()));
        }

        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Sm16716Protocol protocol(1, npb::Sm16716ProtocolSettings{transport.get(), ""});
            protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{7, 8, 9}});

            TEST_ASSERT_EQUAL_UINT32(10U, static_cast<uint32_t>(spy->packets[0].size()));
        }
    }

    void test_1_10_1_and_1_10_4_tlc5947_strategy_sizing_and_ready_contract(void)
    {
        auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();
        spy->ready = false;

        npb::Tlc5947ProtocolSettings settings{};
        settings.bus = transport.get();
        settings.latchPin = npb::PinNotUsed;
        settings.channelOrder = npb::ChannelOrder::RGB::value;
        settings.pixelStrategy = npb::Tlc5947PixelStrategy::ForceRgb;

        npb::Tlc5947Protocol<npb::Rgb16Color> protocol(9, std::move(settings));
        protocol.update(std::array<npb::Rgb16Color, 9>{
            npb::Rgb16Color{1, 2, 3}, npb::Rgb16Color{4, 5, 6}, npb::Rgb16Color{7, 8, 9},
            npb::Rgb16Color{10, 11, 12}, npb::Rgb16Color{13, 14, 15}, npb::Rgb16Color{16, 17, 18},
            npb::Rgb16Color{19, 20, 21}, npb::Rgb16Color{22, 23, 24}, npb::Rgb16Color{25, 26, 27}});

        TEST_ASSERT_TRUE(protocol.isReadyToUpdate());
        TEST_ASSERT_EQUAL_UINT32(72U, static_cast<uint32_t>(spy->packets[0].size()));
    }

    void test_1_11_1_and_1_11_3_tlc59711_header_encoding_and_latch_guard(void)
    {
        auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();

        npb::Tlc59711Settings cfg{};
        cfg.outtmg = true;
        cfg.extgck = true;
        cfg.tmgrst = false;
        cfg.dsprpt = true;
        cfg.blank = true;
        cfg.bcRed = 1;
        cfg.bcGreen = 2;
        cfg.bcBlue = 3;

        npb::Tlc59711Protocol protocol(1, npb::Tlc59711ProtocolSettings{transport.get(), cfg});
        protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{1, 2, 3}});

        const auto& frame = spy->packets[0];
        TEST_ASSERT_EQUAL_UINT8(0x97, frame[0]);
        TEST_ASSERT_EQUAL_UINT8(0x60, frame[1]);
        TEST_ASSERT_EQUAL_UINT8(0xC1, frame[2]);
        TEST_ASSERT_EQUAL_UINT8(0x01, frame[3]);

        Verify(Method(ArduinoFake(Function), delayMicroseconds).Using(20)).Once();
    }

    void test_1_12_1_1_12_2_1_12_3_tm1814_currents_inversion_and_payload_order(void)
    {
        auto transport = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();

        npb::Tm1814ProtocolSettings settings{};
        settings.bus = transport.get();
        settings.channelOrder = "WRGB";
        settings.current.redMilliAmps = 10;
        settings.current.greenMilliAmps = 190;
        settings.current.blueMilliAmps = 380;
        settings.current.whiteMilliAmps = 1000;

        npb::Tm1814Protocol protocol(1, std::move(settings));
        protocol.update(std::array<npb::Rgbw8Color, 1>{npb::Rgbw8Color{1, 2, 3, 4}});

        const auto& frame = spy->packets[0];
        TEST_ASSERT_EQUAL_UINT8(63, frame[0]);
        TEST_ASSERT_EQUAL_UINT8(0, frame[1]);
        TEST_ASSERT_EQUAL_UINT8(25, frame[2]);
        TEST_ASSERT_EQUAL_UINT8(63, frame[3]);
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(~63), frame[4]);
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(~0), frame[5]);
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(~25), frame[6]);
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(~63), frame[7]);
        TEST_ASSERT_EQUAL_UINT8(4, frame[8]);
        TEST_ASSERT_EQUAL_UINT8(1, frame[9]);
        TEST_ASSERT_EQUAL_UINT8(2, frame[10]);
        TEST_ASSERT_EQUAL_UINT8(3, frame[11]);
    }

    void test_1_12_4_tm1814_oversized_and_order_safety(void)
    {
        {
            auto transport = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();

            npb::Tm1814ProtocolSettings settings{};
            settings.bus = transport.get();
            settings.channelOrder = "WRGB";

            npb::Tm1814Protocol protocol(1, std::move(settings));
            protocol.update(std::array<npb::Rgbw8Color, 2>{npb::Rgbw8Color{1, 2, 3, 4}, npb::Rgbw8Color{5, 6, 7, 8}});

            TEST_ASSERT_EQUAL_UINT32(12U, static_cast<uint32_t>(spy->packets[0].size()));
        }

        {
            auto transport = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();

            npb::Tm1814ProtocolSettings settings{};
            settings.bus = transport.get();
            settings.channelOrder = "";

            npb::Tm1814Protocol protocol(1, std::move(settings));
            protocol.update(std::array<npb::Rgbw8Color, 1>{npb::Rgbw8Color{9, 10, 11, 12}});

            TEST_ASSERT_EQUAL_UINT32(12U, static_cast<uint32_t>(spy->packets[0].size()));
        }
    }

    void test_1_13_1_and_1_13_2_tm1914_mode_matrix_inversion_and_payload_order(void)
    {
        auto run_mode = [&](npb::Tm1914Mode mode, uint8_t expectedMode)
        {
            auto transport = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();

            npb::Tm1914ProtocolSettings settings{};
            settings.bus = transport.get();
            settings.channelOrder = npb::ChannelOrder::GRB::value;
            settings.mode = mode;

            npb::Tm1914Protocol protocol(1, std::move(settings));
            protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{1, 2, 3}});

            const auto& frame = spy->packets[0];
            TEST_ASSERT_EQUAL_UINT8(0xFF, frame[0]);
            TEST_ASSERT_EQUAL_UINT8(0xFF, frame[1]);
            TEST_ASSERT_EQUAL_UINT8(expectedMode, frame[2]);
            TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(~frame[2]), frame[5]);
            TEST_ASSERT_EQUAL_UINT8(2, frame[6]);
            TEST_ASSERT_EQUAL_UINT8(1, frame[7]);
            TEST_ASSERT_EQUAL_UINT8(3, frame[8]);
        };

        run_mode(npb::Tm1914Mode::DinFdinAutoSwitch, 0xFF);
        run_mode(npb::Tm1914Mode::DinOnly, 0xF5);
        run_mode(npb::Tm1914Mode::FdinOnly, 0xFA);
    }

    void test_1_13_3_tm1914_oversized_and_order_safety(void)
    {
        {
            auto transport = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();

            npb::Tm1914ProtocolSettings settings{};
            settings.bus = transport.get();
            settings.channelOrder = npb::ChannelOrder::GRB::value;

            npb::Tm1914Protocol protocol(1, std::move(settings));
            protocol.update(std::array<npb::Rgb8Color, 2>{npb::Rgb8Color{1, 2, 3}, npb::Rgb8Color{4, 5, 6}});

            TEST_ASSERT_EQUAL_UINT32(9U, static_cast<uint32_t>(spy->packets[0].size()));
        }

        {
            auto transport = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();

            npb::Tm1914ProtocolSettings settings{};
            settings.bus = transport.get();
            settings.channelOrder = "";

            npb::Tm1914Protocol protocol(1, std::move(settings));
            protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{7, 8, 9}});

            TEST_ASSERT_EQUAL_UINT32(9U, static_cast<uint32_t>(spy->packets[0].size()));
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
    When(Method(ArduinoFake(Function), delayMicroseconds)).AlwaysDo(
        [](unsigned int)
        {
        });
}

void tearDown(void)
{
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(test_1_5_1_lpd6803_packed_5_5_5_serialization);
    RUN_TEST(test_1_5_2_lpd6803_framing_end_frame_size);
    RUN_TEST(test_1_5_3_lpd6803_oversized_and_channel_order_safety);
    RUN_TEST(test_1_6_1_lpd8806_7bit_plus_msb_serialization);
    RUN_TEST(test_1_6_2_lpd8806_symmetric_start_end_framing);
    RUN_TEST(test_1_6_3_lpd8806_oversized_and_channel_order_safety);
    RUN_TEST(test_1_7_1_and_1_7_2_p9813_header_checksum_and_framing);
    RUN_TEST(test_1_7_3_p9813_oversized_span_safety);
    RUN_TEST(test_1_8_1_sm168x_variant_resolution_and_frame_sizing);
    RUN_TEST(test_1_8_3_sm168x_settings_trailer_encoding_masks);
    RUN_TEST(test_1_8_4_sm168x_oversized_and_order_safety);
    RUN_TEST(test_1_9_1_sm16716_buffer_size_and_start_bit_prefix);
    RUN_TEST(test_1_9_3_sm16716_oversized_and_order_safety);
    RUN_TEST(test_1_10_1_and_1_10_4_tlc5947_strategy_sizing_and_ready_contract);
    RUN_TEST(test_1_11_1_and_1_11_3_tlc59711_header_encoding_and_latch_guard);
    RUN_TEST(test_1_12_1_1_12_2_1_12_3_tm1814_currents_inversion_and_payload_order);
    RUN_TEST(test_1_12_4_tm1814_oversized_and_order_safety);
    RUN_TEST(test_1_13_1_and_1_13_2_tm1914_mode_matrix_inversion_and_payload_order);
    RUN_TEST(test_1_13_3_tm1914_oversized_and_order_safety);
    return UNITY_END();
}

