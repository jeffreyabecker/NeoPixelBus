#include <unity.h>

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include <ArduinoFake.h>

#include "colors/Color.h"
#include "protocols/DebugProtocol.h"
#include "protocols/IProtocol.h"
#include "transports/PrintTransport.h"

namespace
{
    using TestColor = lw::Rgbcw8Color;

    class ProtocolSpy : public lw::IProtocol<TestColor>
    {
    public:
        void initialize() override
        {
            ++initializeCount;
        }

        void update(lw::span<const TestColor> colors) override
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
        std::vector<TestColor> lastFrame{};
    };

    class WritableSpy
    {
    public:
        size_t write(const uint8_t *data, size_t size)
        {
            if (data == nullptr || size == 0)
            {
                return 0;
            }

            bytes.insert(bytes.end(), data, data + size);
            return size;
        }

        std::vector<uint8_t> bytes{};
    };

    void test_debug_protocol_forwards_to_inner_protocol_without_output(void)
    {
        ProtocolSpy inner{};

        lw::DebugProtocolSettingsT<TestColor> settings{};
        settings.output = nullptr;
        settings.invert = false;
        settings.protocol = &inner;

        lw::DebugProtocol<TestColor> protocol(2, std::move(settings));

        std::vector<TestColor> colors{
            TestColor{0x01, 0x02, 0x03, 0x04, 0x05},
            TestColor{0xAA, 0xBB, 0xCC, 0xDD, 0xEE}};

        protocol.initialize();
        protocol.update(lw::span<const TestColor>{colors.data(), colors.size()});

        TEST_ASSERT_EQUAL_INT(1, inner.initializeCount);
        TEST_ASSERT_EQUAL_INT(0, inner.updateCount);
        TEST_ASSERT_EQUAL_UINT32(0U, static_cast<uint32_t>(inner.lastFrame.size()));
    }

    void test_debug_protocol_ready_and_always_update_delegate_to_inner_protocol(void)
    {
        ProtocolSpy inner{};
        inner.ready = false;
        inner.always = true;

        lw::DebugProtocolSettingsT<TestColor> settings{};
        settings.output = nullptr;
        settings.protocol = &inner;

        lw::DebugProtocol<TestColor> protocol(1, std::move(settings));

        TEST_ASSERT_FALSE(protocol.isReadyToUpdate());
        TEST_ASSERT_TRUE(protocol.alwaysUpdate());

    }

    void test_print_transport_forwards_raw_bytes_without_ascii_or_debug(void)
    {
        WritableSpy writable{};
        lw::PrintTransportSettingsT<WritableSpy> config{};
        config.output = &writable;
        config.asciiOutput = false;
        config.debugOutput = false;

        lw::PrintTransportT<WritableSpy> transport(std::move(config));

        std::array<uint8_t, 3> bytes{0x12, 0x34, 0xAB};

        transport.begin();
        transport.beginTransaction();
        transport.transmitBytes(bytes);
        transport.endTransaction();

        TEST_ASSERT_EQUAL_UINT32(3U, static_cast<uint32_t>(writable.bytes.size()));
        TEST_ASSERT_EQUAL_UINT8(0x12, writable.bytes[0]);
        TEST_ASSERT_EQUAL_UINT8(0x34, writable.bytes[1]);
        TEST_ASSERT_EQUAL_UINT8(0xAB, writable.bytes[2]);

    }

    void test_print_transport_ascii_output_hex_encodes_bytes(void)
    {
        WritableSpy writable{};
        lw::PrintTransportSettingsT<WritableSpy> config{};
        config.output = &writable;
        config.asciiOutput = true;
        config.debugOutput = false;

        lw::PrintTransportT<WritableSpy> transport(std::move(config));

        std::array<uint8_t, 2> bytes{0x00, 0xAF};
        transport.transmitBytes(bytes);

        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(writable.bytes.size()));
        TEST_ASSERT_EQUAL_UINT8('0', writable.bytes[0]);
        TEST_ASSERT_EQUAL_UINT8('0', writable.bytes[1]);
        TEST_ASSERT_EQUAL_UINT8('A', writable.bytes[2]);
        TEST_ASSERT_EQUAL_UINT8('F', writable.bytes[3]);

    }

    void test_print_transport_debug_output_emits_event_messages(void)
    {
        WritableSpy writable{};
        lw::PrintTransportSettingsT<WritableSpy> config{};
        config.output = &writable;
        config.asciiOutput = false;
        config.debugOutput = true;

        lw::PrintTransportT<WritableSpy> transport(std::move(config));

        std::array<uint8_t, 2> bytes{0x12, 0x34};
        transport.begin();
        transport.beginTransaction();
        transport.transmitBytes(bytes);
        transport.endTransaction();

        std::string output(writable.bytes.begin(), writable.bytes.end());
        TEST_ASSERT_NOT_EQUAL(-1, static_cast<int>(output.find("[BUS] begin")));
        TEST_ASSERT_NOT_EQUAL(-1, static_cast<int>(output.find("[BUS] beginTransaction")));
        TEST_ASSERT_NOT_EQUAL(-1, static_cast<int>(output.find("[BUS] bytes(2)")));
        TEST_ASSERT_NOT_EQUAL(-1, static_cast<int>(output.find("[BUS] endTransaction")));
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
    RUN_TEST(test_print_transport_forwards_raw_bytes_without_ascii_or_debug);
    RUN_TEST(test_print_transport_ascii_output_hex_encodes_bytes);
    RUN_TEST(test_print_transport_debug_output_emits_event_messages);
    return UNITY_END();
}

