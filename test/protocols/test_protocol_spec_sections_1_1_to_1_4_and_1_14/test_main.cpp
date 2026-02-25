#include <unity.h>

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include <ArduinoFake.h>

#include "virtual/protocols/DotStarProtocol.h"
#include "virtual/protocols/Hd108Protocol.h"
#include "virtual/protocols/PixieProtocol.h"
#include "virtual/protocols/Ws2801Protocol.h"
#include "virtual/protocols/Ws2812xProtocol.h"

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
            calls.emplace_back("begin");
        }

        void beginTransaction() override
        {
            ++beginTransactionCount;
            calls.emplace_back("beginTransaction");
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            ++transmitCount;
            calls.emplace_back("transmit");
            packets.emplace_back(data.begin(), data.end());
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

        int beginCount{0};
        int beginTransactionCount{0};
        int transmitCount{0};
        int endTransactionCount{0};
        bool ready{true};
        std::vector<std::string> calls{};
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
            calls.emplace_back("begin");
        }

        void beginTransaction() override
        {
            ++beginTransactionCount;
            calls.emplace_back("beginTransaction");
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            ++transmitCount;
            calls.emplace_back("transmit");
            packets.emplace_back(data.begin(), data.end());
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

        int beginCount{0};
        int beginTransactionCount{0};
        int transmitCount{0};
        int endTransactionCount{0};
        bool ready{true};
        std::vector<std::string> calls{};
        std::vector<std::vector<uint8_t>> packets{};
    };

    static uint32_t gMicrosNow = 0;

    void assert_bytes_equal(const std::vector<uint8_t>& actual,
                            const std::vector<uint8_t>& expected)
    {
        TEST_ASSERT_EQUAL_UINT32(expected.size(), static_cast<uint32_t>(actual.size()));
        for (size_t idx = 0; idx < expected.size(); ++idx)
        {
            TEST_ASSERT_EQUAL_UINT8(expected[idx], actual[idx]);
        }
    }

    void test_1_1_1_dotstar_construction_and_begin(void)
    {
        auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();

        npb::DotStarProtocol protocol(3, npb::DotStarProtocolSettings{std::move(transport)});
        protocol.initialize();

        TEST_ASSERT_EQUAL_INT(1, spy->beginCount);
    }

    void test_1_1_2_dotstar_end_frame_extra_byte_calculation(void)
    {
        const std::array<uint16_t, 6> counts{0, 1, 15, 16, 17, 32};
        for (const auto pixelCount : counts)
        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::DotStarProtocol protocol(pixelCount, npb::DotStarProtocolSettings{std::move(transport)});

            std::vector<npb::Rgb8Color> colors(pixelCount, npb::Rgb8Color{1, 2, 3});
            protocol.update(colors);

            const size_t extra = (static_cast<size_t>(pixelCount) + 15u) / 16u;
            const size_t expectedCalls = 4u + 1u + 4u + extra;
            TEST_ASSERT_EQUAL_UINT32(expectedCalls, static_cast<uint32_t>(spy->packets.size()));
        }
    }

    void test_1_1_3_and_1_1_4_dotstar_fixed_brightness_and_luminance_serialization(void)
    {
        const std::array<npb::Rgb8Color, 2> colors{
            npb::Rgb8Color{0x11, 0x22, 0x33},
            npb::Rgb8Color{0x44, 0x55, 0x66}};

        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::DotStarProtocol protocol(
                2,
                npb::DotStarProtocolSettings{std::move(transport), npb::ChannelOrder::GRB, npb::DotStarMode::FixedBrightness});

            protocol.update(colors);

            TEST_ASSERT_EQUAL_UINT32(8U, static_cast<uint32_t>(spy->packets[4].size()));
            const std::vector<uint8_t> expected{0xFF, 0x22, 0x11, 0x33, 0xFF, 0x55, 0x44, 0x66};
            assert_bytes_equal(spy->packets[4], expected);
        }

        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::DotStarProtocol protocol(
                2,
                npb::DotStarProtocolSettings{std::move(transport), npb::ChannelOrder::BGR, npb::DotStarMode::Luminance});

            protocol.update(colors);

            const std::vector<uint8_t> expected{0xFF, 0x33, 0x22, 0x11, 0xFF, 0x66, 0x55, 0x44};
            assert_bytes_equal(spy->packets[4], expected);
        }
    }

    void test_1_1_5_dotstar_framing_and_transaction_sequence(void)
    {
        auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();
        npb::DotStarProtocol protocol(1, npb::DotStarProtocolSettings{std::move(transport)});

        protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{1, 2, 3}});

        TEST_ASSERT_EQUAL_INT(1, spy->beginTransactionCount);
        TEST_ASSERT_EQUAL_INT(1, spy->endTransactionCount);
        TEST_ASSERT_EQUAL_STRING("beginTransaction", spy->calls[0].c_str());
        TEST_ASSERT_EQUAL_STRING("endTransaction", spy->calls.back().c_str());
    }

    void test_1_1_6_and_1_1_7_dotstar_oversized_and_channel_order_edge_contract(void)
    {
        const std::array<npb::Rgb8Color, 3> oversized{
            npb::Rgb8Color{1, 2, 3},
            npb::Rgb8Color{4, 5, 6},
            npb::Rgb8Color{7, 8, 9}};

        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::DotStarProtocol protocol(2, npb::DotStarProtocolSettings{std::move(transport), npb::ChannelOrder::BGR, npb::DotStarMode::FixedBrightness});
            protocol.update(oversized);

            TEST_ASSERT_EQUAL_UINT32(8U, static_cast<uint32_t>(spy->packets[4].size()));
        }

        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::DotStarProtocol protocol(2, npb::DotStarProtocolSettings{std::move(transport), "", npb::DotStarMode::FixedBrightness});
            protocol.update(std::array<npb::Rgb8Color, 2>{npb::Rgb8Color{10, 11, 12}, npb::Rgb8Color{13, 14, 15}});

            TEST_ASSERT_EQUAL_UINT32(8U, static_cast<uint32_t>(spy->packets[4].size()));
        }
    }

    void test_1_2_1_and_1_2_2_hd108_size_aliases_and_big_endian_payload(void)
    {
        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Hd108RgbProtocol protocol(1, npb::Hd108ProtocolSettings{std::move(transport), npb::ChannelOrder::RGB});

            protocol.update(std::array<npb::Rgb16Color, 1>{npb::Rgb16Color{0x1234, 0x4567, 0x89AB}});

            TEST_ASSERT_EQUAL_UINT32(8U, static_cast<uint32_t>(spy->packets[16].size()));
            const std::vector<uint8_t> expected{0xFF, 0xFF, 0x12, 0x34, 0x45, 0x67, 0x89, 0xAB};
            assert_bytes_equal(spy->packets[16], expected);
        }

        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Hd108RgbcwProtocol protocol(1, npb::Hd108ProtocolSettings{std::move(transport), "RGBCW"});

            protocol.update(std::array<npb::Rgbcw16Color, 1>{npb::Rgbcw16Color{1, 2, 3, 4, 5}});

            TEST_ASSERT_EQUAL_UINT32(12U, static_cast<uint32_t>(spy->packets[16].size()));
        }
    }

    void test_1_2_3_hd108_framing_and_transaction_sequence(void)
    {
        auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();
        npb::Hd108RgbProtocol protocol(2, npb::Hd108ProtocolSettings{std::move(transport), npb::ChannelOrder::RGB});

        protocol.update(std::array<npb::Rgb16Color, 2>{npb::Rgb16Color{1, 2, 3}, npb::Rgb16Color{4, 5, 6}});

        TEST_ASSERT_EQUAL_INT(1, spy->beginTransactionCount);
        TEST_ASSERT_EQUAL_INT(1, spy->endTransactionCount);
        TEST_ASSERT_EQUAL_UINT32(21U, static_cast<uint32_t>(spy->packets.size()));
        TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(spy->packets.front().size()));
        TEST_ASSERT_EQUAL_UINT8(0x00, spy->packets.front()[0]);
        TEST_ASSERT_EQUAL_UINT8(0xFF, spy->packets.back()[0]);
    }

    void test_1_2_4_and_1_2_5_hd108_oversized_and_channel_order_edge_contract(void)
    {
        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Hd108RgbProtocol protocol(2, npb::Hd108ProtocolSettings{std::move(transport), npb::ChannelOrder::RGB});
            protocol.update(std::array<npb::Rgb16Color, 3>{
                npb::Rgb16Color{0x0102, 0x0304, 0x0506},
                npb::Rgb16Color{0x0708, 0x090A, 0x0B0C},
                npb::Rgb16Color{0x0D0E, 0x0F10, 0x1112}});

            TEST_ASSERT_EQUAL_UINT32(16U, static_cast<uint32_t>(spy->packets[16].size()));
        }

        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Hd108RgbProtocol protocol(1, npb::Hd108ProtocolSettings{std::move(transport), ""});
            protocol.update(std::array<npb::Rgb16Color, 1>{npb::Rgb16Color{0x1234, 0x5678, 0x9ABC}});

            TEST_ASSERT_EQUAL_UINT32(8U, static_cast<uint32_t>(spy->packets[16].size()));
            TEST_ASSERT_EQUAL_UINT8(0xFF, spy->packets[16][0]);
            TEST_ASSERT_EQUAL_UINT8(0xFF, spy->packets[16][1]);
            TEST_ASSERT_EQUAL_UINT8(0x12, spy->packets[16][2]);
            TEST_ASSERT_EQUAL_UINT8(0x34, spy->packets[16][3]);
        }
    }

    void test_1_3_1_ws2801_serialization_order_variants(void)
    {
        const std::array<npb::Rgb8Color, 2> colors{
            npb::Rgb8Color{1, 2, 3},
            npb::Rgb8Color{4, 5, 6}};

        auto run_case = [&](const char* order, const std::vector<uint8_t>& expected)
        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Ws2801Protocol protocol(2, npb::Ws2801ProtocolSettings{std::move(transport), order});

            protocol.update(colors);
            assert_bytes_equal(spy->packets[0], expected);
        };

        run_case(npb::ChannelOrder::RGB, std::vector<uint8_t>{1, 2, 3, 4, 5, 6});
        run_case(npb::ChannelOrder::GRB, std::vector<uint8_t>{2, 1, 3, 5, 4, 6});
        run_case(npb::ChannelOrder::BGR, std::vector<uint8_t>{3, 2, 1, 6, 5, 4});
    }

    void test_1_3_2_ws2801_transaction_and_latch_timing(void)
    {
        auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();
        npb::Ws2801Protocol protocol(1, npb::Ws2801ProtocolSettings{std::move(transport)});

        gMicrosNow = 1000;
        protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{1, 2, 3}});

        TEST_ASSERT_EQUAL_INT(1, spy->beginTransactionCount);
        TEST_ASSERT_EQUAL_INT(1, spy->endTransactionCount);

        gMicrosNow = 1499;
        TEST_ASSERT_FALSE(protocol.isReadyToUpdate());

        gMicrosNow = 1500;
        TEST_ASSERT_TRUE(protocol.isReadyToUpdate());
    }

    void test_1_3_3_ws2801_oversized_and_channel_order_edge_contract(void)
    {
        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Ws2801Protocol protocol(1, npb::Ws2801ProtocolSettings{std::move(transport), npb::ChannelOrder::RGB});
            protocol.update(std::array<npb::Rgb8Color, 2>{npb::Rgb8Color{1, 2, 3}, npb::Rgb8Color{4, 5, 6}});

            TEST_ASSERT_EQUAL_UINT32(3U, static_cast<uint32_t>(spy->packets[0].size()));
        }

        {
            auto transport = std::make_unique<TransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Ws2801Protocol protocol(1, npb::Ws2801ProtocolSettings{std::move(transport), ""});
            protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{7, 8, 9}});

            TEST_ASSERT_EQUAL_UINT32(3U, static_cast<uint32_t>(spy->packets[0].size()));
            TEST_ASSERT_EQUAL_UINT8(7U, spy->packets[0][0]);
            TEST_ASSERT_EQUAL_UINT8(7U, spy->packets[0][1]);
            TEST_ASSERT_EQUAL_UINT8(7U, spy->packets[0][2]);
        }
    }

    void test_1_4_1_pixie_serialization_transaction_and_1_4_2_always_update(void)
    {
        auto transport = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();
        npb::PixieProtocol protocol(2, npb::PixieProtocolSettings{std::move(transport), npb::ChannelOrder::RGB});

        gMicrosNow = 2000;
        protocol.update(std::array<npb::Rgb8Color, 2>{npb::Rgb8Color{1, 2, 3}, npb::Rgb8Color{4, 5, 6}});

        TEST_ASSERT_EQUAL_INT(1, spy->beginTransactionCount);
        TEST_ASSERT_EQUAL_INT(1, spy->endTransactionCount);
        assert_bytes_equal(spy->packets[0], std::vector<uint8_t>{1, 2, 3, 4, 5, 6});
        TEST_ASSERT_TRUE(protocol.alwaysUpdate());

        gMicrosNow = 2999;
        TEST_ASSERT_FALSE(protocol.isReadyToUpdate());
        gMicrosNow = 3000;
        TEST_ASSERT_TRUE(protocol.isReadyToUpdate());
    }

    void test_1_4_3_pixie_oversized_and_channel_order_edge_contract(void)
    {
        {
            auto transport = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::PixieProtocol protocol(1, npb::PixieProtocolSettings{std::move(transport), npb::ChannelOrder::RGB});

            gMicrosNow = 2000;
            protocol.update(std::array<npb::Rgb8Color, 2>{npb::Rgb8Color{1, 2, 3}, npb::Rgb8Color{4, 5, 6}});

            TEST_ASSERT_EQUAL_UINT32(3U, static_cast<uint32_t>(spy->packets[0].size()));
        }

        {
            auto transport = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::PixieProtocol protocol(1, npb::PixieProtocolSettings{std::move(transport), ""});

            gMicrosNow = 2000;
            protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{9, 10, 11}});

            TEST_ASSERT_EQUAL_UINT32(3U, static_cast<uint32_t>(spy->packets[0].size()));
            TEST_ASSERT_EQUAL_UINT8(9U, spy->packets[0][0]);
            TEST_ASSERT_EQUAL_UINT8(9U, spy->packets[0][1]);
            TEST_ASSERT_EQUAL_UINT8(9U, spy->packets[0][2]);
        }
    }

    void test_1_14_1_constructor_equivalence_and_1_14_3_serialization_for_8_16_bit(void)
    {
        const std::array<npb::Rgb8Color, 1> colors8{
            npb::Rgb8Color{0x11, 0x22, 0x33},
        };

        auto transportA = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
        auto* spyA = transportA.get();
        npb::Ws2812xProtocol<npb::Rgb8Color> protocolA(
            1,
            npb::Ws2812xProtocolSettings{std::move(transportA), npb::ChannelOrder::GRB});

        auto transportB = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
        auto* spyB = transportB.get();
        npb::Ws2812xProtocol<npb::Rgb8Color> protocolB(
            1,
            npb::ChannelOrder::GRB,
            std::move(transportB));

        protocolA.update(colors8);
        protocolB.update(colors8);

        TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(spyA->packets.size()));
        TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(spyB->packets.size()));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(spyA->packets[0].size()),
                     static_cast<uint32_t>(spyB->packets[0].size()));
        TEST_ASSERT_GREATER_THAN_UINT32(0U, static_cast<uint32_t>(spyA->packets[0].size()));

        {
            auto transport16 = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
            auto* spy16 = transport16.get();
            npb::Ws2812xProtocol<npb::Rgb16Color> protocol16(
                1,
                npb::Ws2812xProtocolSettings{std::move(transport16), npb::ChannelOrder::RGB});

            protocol16.update(std::array<npb::Rgb16Color, 1>{npb::Rgb16Color{0x12AB, 0x34CD, 0x56EF}});
            assert_bytes_equal(spy16->packets[0], std::vector<uint8_t>{0x12, 0xAB, 0x34, 0xCD, 0x56, 0xEF});
            TEST_ASSERT_EQUAL_UINT32(2U * npb::Rgb16Color::ChannelCount,
                                     static_cast<uint32_t>(spy16->packets[0].size()));
        }

        {
            auto transport16 = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
            auto* spy16 = transport16.get();
            npb::Ws2812xProtocol<npb::Rgbw16Color> protocol16(
                1,
                npb::Ws2812xProtocolSettings{std::move(transport16), npb::ChannelOrder::RGBW});

            protocol16.update(std::array<npb::Rgbw16Color, 1>{npb::Rgbw16Color{0x0102, 0x0304, 0x0506, 0x0708}});
            assert_bytes_equal(spy16->packets[0], std::vector<uint8_t>{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});
            TEST_ASSERT_EQUAL_UINT32(2U * npb::Rgbw16Color::ChannelCount,
                                     static_cast<uint32_t>(spy16->packets[0].size()));
        }

        {
            auto transport16 = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
            auto* spy16 = transport16.get();
            npb::Ws2812xProtocol<npb::Rgbcw16Color> protocol16(
                1,
                npb::Ws2812xProtocolSettings{std::move(transport16), npb::ChannelOrder::RGBCW});

            protocol16.update(std::array<npb::Rgbcw16Color, 1>{npb::Rgbcw16Color{0x1112, 0x1314, 0x1516, 0x1718, 0x191A}});
            assert_bytes_equal(spy16->packets[0], std::vector<uint8_t>{0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x19, 0x1A, 0x17, 0x18});
            TEST_ASSERT_EQUAL_UINT32(2U * npb::Rgbcw16Color::ChannelCount,
                                     static_cast<uint32_t>(spy16->packets[0].size()));
        }
    }

    void test_1_14_2_channel_order_count_resolution(void)
    {
        const npb::Color one{1, 2, 3, 4, 5};

        auto run_case = [&](const char* order, const std::vector<uint8_t>& expected)
        {
            auto transport = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Ws2812xProtocol<npb::Color> protocol(
                1,
                npb::Ws2812xProtocolSettings{std::move(transport), order});

            protocol.update(std::array<npb::Color, 1>{one});
            assert_bytes_equal(spy->packets[0], expected);
        };

        run_case(nullptr, std::vector<uint8_t>{2, 1, 3});
        run_case(npb::ChannelOrder::GRBCW, std::vector<uint8_t>{2, 1, 3, 5, 4});

        {
            auto transport = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
            auto* spy = transport.get();
            npb::Ws2812xProtocol<npb::Color> protocol(
                1,
                npb::Ws2812xProtocolSettings{std::move(transport), ""});

            protocol.update(std::array<npb::Color, 1>{one});
            TEST_ASSERT_EQUAL_UINT32(3U, static_cast<uint32_t>(spy->packets[0].size()));
        }
    }

    void test_1_14_4_ws2812x_readiness_wait_loop_contract(void)
    {
        auto transport = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();
        spy->ready = false;

        npb::Ws2812xProtocol<npb::Rgb8Color> protocol(
            1,
            npb::Ws2812xProtocolSettings{std::move(transport), npb::ChannelOrder::RGB});

        int yieldCount = 0;
        When(Method(ArduinoFake(Function), yield)).AlwaysDo(
            [&]()
            {
                ++yieldCount;
                if (yieldCount == 3)
                {
                    spy->ready = true;
                }
            });

        protocol.update(std::array<npb::Rgb8Color, 1>{npb::Rgb8Color{9, 8, 7}});

        TEST_ASSERT_GREATER_THAN_INT(0, yieldCount);
        TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(spy->packets.size()));
        assert_bytes_equal(spy->packets[0], std::vector<uint8_t>{9, 8, 7});
    }

    void test_1_14_5_ws2812x_oversized_span_contract(void)
    {
        auto transport = std::make_unique<OneWireTransportSpy>(TransportSpySettings{});
        auto* spy = transport.get();
        npb::Ws2812xProtocol<npb::Rgb8Color> protocol(
            2,
            npb::Ws2812xProtocolSettings{std::move(transport), npb::ChannelOrder::GRB});

        protocol.update(std::array<npb::Rgb8Color, 3>{
            npb::Rgb8Color{1, 2, 3},
            npb::Rgb8Color{4, 5, 6},
            npb::Rgb8Color{7, 8, 9}});

        TEST_ASSERT_EQUAL_UINT32(6U, static_cast<uint32_t>(spy->packets[0].size()));
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
    RUN_TEST(test_1_1_1_dotstar_construction_and_begin);
    RUN_TEST(test_1_1_2_dotstar_end_frame_extra_byte_calculation);
    RUN_TEST(test_1_1_3_and_1_1_4_dotstar_fixed_brightness_and_luminance_serialization);
    RUN_TEST(test_1_1_5_dotstar_framing_and_transaction_sequence);
    RUN_TEST(test_1_1_6_and_1_1_7_dotstar_oversized_and_channel_order_edge_contract);
    RUN_TEST(test_1_2_1_and_1_2_2_hd108_size_aliases_and_big_endian_payload);
    RUN_TEST(test_1_2_3_hd108_framing_and_transaction_sequence);
    RUN_TEST(test_1_2_4_and_1_2_5_hd108_oversized_and_channel_order_edge_contract);
    RUN_TEST(test_1_3_1_ws2801_serialization_order_variants);
    RUN_TEST(test_1_3_2_ws2801_transaction_and_latch_timing);
    RUN_TEST(test_1_3_3_ws2801_oversized_and_channel_order_edge_contract);
    RUN_TEST(test_1_4_1_pixie_serialization_transaction_and_1_4_2_always_update);
    RUN_TEST(test_1_4_3_pixie_oversized_and_channel_order_edge_contract);
    RUN_TEST(test_1_14_1_constructor_equivalence_and_1_14_3_serialization_for_8_16_bit);
    RUN_TEST(test_1_14_2_channel_order_count_resolution);
    RUN_TEST(test_1_14_4_ws2812x_readiness_wait_loop_contract);
    RUN_TEST(test_1_14_5_ws2812x_oversized_span_contract);
    return UNITY_END();
}
