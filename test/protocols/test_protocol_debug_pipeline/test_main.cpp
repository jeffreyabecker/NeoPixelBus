#include <unity.h>

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include <ArduinoFake.h>

#include "virtual/colors/Color.h"
#include "virtual/protocols/DebugProtocol.h"
#include "virtual/protocols/IProtocol.h"
#include "virtual/transports/DebugTransport.h"

namespace
{
    class ProtocolSpy : public npb::IProtocol<npb::Color>
    {
    public:
        void initialize() override
        {
            ++initializeCount;
        }

        void update(std::span<const npb::Color> colors) override
        {
            ++updateCount;
            lastFrame.assign(colors.begin(), colors.end());
        }

        bool isReadyToUpdate() const override
        {
            return ready;
        }

        bool alwaysUpdate() const override
        {
            return always;
        }

        int initializeCount{0};
        int updateCount{0};
        bool ready{true};
        bool always{false};
        std::vector<npb::Color> lastFrame{};
    };

    struct TransportSpyConfig
    {
    };

    class TransportSpy : public npb::ITransport
    {
    public:
        using TransportConfigType = TransportSpyConfig;
        using TransportCategory = npb::TransportTag;

        explicit TransportSpy(TransportConfigType)
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

        void endTransaction() override
        {
            ++endTransactionCount;
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            transmitted.assign(data.begin(), data.end());
        }

        int beginCount{0};
        int beginTransactionCount{0};
        int endTransactionCount{0};
        std::vector<uint8_t> transmitted{};
    };

    void test_debug_protocol_forwards_to_inner_protocol_without_output(void)
    {
        ProtocolSpy inner{};

        npb::DebugProtocolSettings settings{};
        settings.output = nullptr;
        settings.invert = false;
        settings.protocol = inner;

        npb::DebugProtocol<npb::Color> protocol(2, std::move(settings));

        std::vector<npb::Color> colors{
            npb::Color{0x01, 0x02, 0x03, 0x04, 0x05},
            npb::Color{0xAA, 0xBB, 0xCC, 0xDD, 0xEE}};

        protocol.initialize();
        protocol.update(colors);

        TEST_ASSERT_EQUAL_INT(1, inner.initializeCount);
        TEST_ASSERT_EQUAL_INT(0, inner.updateCount);
        TEST_ASSERT_EQUAL_UINT32(0U, static_cast<uint32_t>(inner.lastFrame.size()));
    }

    void test_debug_protocol_ready_and_always_update_delegate_to_inner_protocol(void)
    {
        ProtocolSpy inner{};
        inner.ready = false;
        inner.always = true;

        npb::DebugProtocolSettings settings{};
        settings.output = nullptr;
        settings.protocol = inner;

        npb::DebugProtocol<npb::Color> protocol(1, std::move(settings));

        TEST_ASSERT_FALSE(protocol.isReadyToUpdate());
        TEST_ASSERT_TRUE(protocol.alwaysUpdate());

    }

    void test_debug_transport_forwards_bytes_without_output(void)
    {
        npb::DebugTransportConfigT<TransportSpyConfig> config{};
        config.output = nullptr;
        config.invert = false;

        npb::DebugTransportT<TransportSpy, TransportSpyConfig> transport(std::move(config));

        const std::array<uint8_t, 3> bytes{0x12, 0x34, 0xAB};

        transport.begin();
        transport.beginTransaction();
        transport.transmitBytes(bytes);
        transport.endTransaction();

        TEST_ASSERT_EQUAL_INT(1, transport.beginCount);
        TEST_ASSERT_EQUAL_INT(1, transport.beginTransactionCount);
        TEST_ASSERT_EQUAL_INT(1, transport.endTransactionCount);
        TEST_ASSERT_EQUAL_UINT32(3U, static_cast<uint32_t>(transport.transmitted.size()));
        TEST_ASSERT_EQUAL_UINT8(0x12, transport.transmitted[0]);
        TEST_ASSERT_EQUAL_UINT8(0x34, transport.transmitted[1]);
        TEST_ASSERT_EQUAL_UINT8(0xAB, transport.transmitted[2]);

    }

    void test_debug_transport_invert_does_not_change_forwarded_bytes(void)
    {
        npb::DebugTransportConfigT<TransportSpyConfig> config{};
        config.output = nullptr;
        config.invert = true;

        npb::DebugTransportT<TransportSpy, TransportSpyConfig> transport(std::move(config));

        const std::array<uint8_t, 2> bytes{0x00, 0x0F};
        transport.transmitBytes(bytes);

        TEST_ASSERT_EQUAL_UINT8(0x00, transport.transmitted[0]);
        TEST_ASSERT_EQUAL_UINT8(0x0F, transport.transmitted[1]);

    }
}

void setUp(void)
{
    ArduinoFakeReset();
}

void tearDown(void)
{
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(test_debug_protocol_forwards_to_inner_protocol_without_output);
    RUN_TEST(test_debug_protocol_ready_and_always_update_delegate_to_inner_protocol);
    RUN_TEST(test_debug_transport_forwards_bytes_without_output);
    RUN_TEST(test_debug_transport_invert_does_not_change_forwarded_bytes);
    return UNITY_END();
}
