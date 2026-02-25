#include <unity.h>

#include <span>
#include <vector>

#include "buses/PixelBus.h"
#include "colors/Color.h"
#include "protocols/IProtocol.h"

namespace
{
    using TestColor = npb::Rgbcw8Color;

    class ProtocolStub : public npb::IProtocol<TestColor>
    {
    public:
        void initialize() override
        {
            ++initializeCount;
        }

        void update(std::span<const TestColor> colors) override
        {
            ++updateCount;
            lastFrame.assign(colors.begin(), colors.end());
        }

        bool isReadyToUpdate() const override
        {
            return readyToUpdate;
        }

        bool alwaysUpdate() const override
        {
            return alwaysUpdateEnabled;
        }

        int initializeCount{0};
        int updateCount{0};
        bool readyToUpdate{true};
        bool alwaysUpdateEnabled{false};
        std::vector<TestColor> lastFrame{};
    };

    void test_begin_calls_protocol_initialize(void)
    {
        ProtocolStub protocol{};
        npb::PixelBusT<TestColor> bus(4, protocol);

        bus.begin();

        TEST_ASSERT_EQUAL_INT(1, protocol.initializeCount);
    }

    void test_show_does_not_update_when_clean_and_not_always_update(void)
    {
        ProtocolStub protocol{};
        npb::PixelBusT<TestColor> bus(4, protocol);

        bus.show();

        TEST_ASSERT_EQUAL_INT(0, protocol.updateCount);
    }

    void test_set_pixel_color_marks_dirty_and_show_updates(void)
    {
        ProtocolStub protocol{};
        npb::PixelBusT<TestColor> bus(3, protocol);

        const TestColor color{1, 2, 3, 4, 5};
        bus.setPixelColor(1, color);
        bus.show();

        TEST_ASSERT_EQUAL_INT(1, protocol.updateCount);
        TEST_ASSERT_EQUAL_UINT32(3U, static_cast<uint32_t>(protocol.lastFrame.size()));
        TEST_ASSERT_EQUAL_UINT8(1, protocol.lastFrame[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(2, protocol.lastFrame[1]['G']);
        TEST_ASSERT_EQUAL_UINT8(3, protocol.lastFrame[1]['B']);
        TEST_ASSERT_EQUAL_UINT8(4, protocol.lastFrame[1]['W']);
        TEST_ASSERT_EQUAL_UINT8(5, protocol.lastFrame[1]['C']);
    }

    void test_show_updates_when_always_update_enabled(void)
    {
        ProtocolStub protocol{};
        protocol.alwaysUpdateEnabled = true;
        npb::PixelBusT<TestColor> bus(2, protocol);

        bus.show();
        bus.show();

        TEST_ASSERT_EQUAL_INT(2, protocol.updateCount);
    }

    void test_can_show_delegates_protocol_ready_state(void)
    {
        ProtocolStub protocol{};
        npb::PixelBusT<TestColor> bus(2, protocol);

        protocol.readyToUpdate = true;
        TEST_ASSERT_TRUE(bus.canShow());

        protocol.readyToUpdate = false;
        TEST_ASSERT_FALSE(bus.canShow());
    }
}

void setUp(void)
{
}

void tearDown(void)
{
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(test_begin_calls_protocol_initialize);
    RUN_TEST(test_show_does_not_update_when_clean_and_not_always_update);
    RUN_TEST(test_set_pixel_color_marks_dirty_and_show_updates);
    RUN_TEST(test_show_updates_when_always_update_enabled);
    RUN_TEST(test_can_show_delegates_protocol_ready_state);
    return UNITY_END();
}

