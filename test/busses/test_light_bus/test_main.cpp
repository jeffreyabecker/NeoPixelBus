#include <unity.h>

#include "buses/LightBus.h"
#include "colors/Color.h"
#include "colors/IShader.h"
#include "transports/ILightDriver.h"

namespace
{
using TestColor = lw::Rgb8Color;

class MockLightDriver : public lw::transports::ILightDriver<TestColor>
{
  public:
    struct LightDriverSettingsType : lw::transports::LightDriverSettingsBase
    {
        bool initiallyReady{true};

        static LightDriverSettingsType normalize(LightDriverSettingsType settings) { return settings; }
    };

    using ColorType = TestColor;

    explicit MockLightDriver(LightDriverSettingsType settings) : ready(settings.initiallyReady) {}

    void begin() override { began = true; }

    bool isReadyToUpdate() const override { return ready; }

    void write(const TestColor& color) override
    {
        ++writeCount;
        lastColor = color;
    }

    bool began{false};
    bool ready{true};
    size_t writeCount{0};
    TestColor lastColor{};
};

class IncrementRedShader : public lw::IShader<TestColor>
{
  public:
    void apply(lw::span<TestColor> colors) override
    {
        for (auto& color : colors)
        {
            ++color['R'];
        }
    }
};

void test_light_bus_applies_shader_and_preserves_root_pixel(void)
{
    lw::busses::LightBus<TestColor, MockLightDriver, IncrementRedShader> bus(MockLightDriver::LightDriverSettingsType{},
                                                                             IncrementRedShader{});

    bus.begin();
    auto& pixels = bus.pixels();
    pixels[0] = TestColor{3, 4, 5};
    bus.show();

    TEST_ASSERT_TRUE(bus.driver().began);
    TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(bus.driver().writeCount));
    TEST_ASSERT_EQUAL_UINT8(4, bus.driver().lastColor['R']);
    TEST_ASSERT_EQUAL_UINT8(3, bus.pixel()['R']);

    const auto scratch = bus.shaderScratch();
    TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(scratch.size()));
    TEST_ASSERT_EQUAL_UINT8(4, scratch[0]['R']);
}

void test_light_bus_nil_shader_writes_root_and_uses_dirty_guard(void)
{
    lw::busses::LightBus<TestColor, MockLightDriver> bus(MockLightDriver::LightDriverSettingsType{});

    bus.pixel() = TestColor{9, 1, 2};
    bus.show();
    bus.show();

    TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(bus.driver().writeCount));
    TEST_ASSERT_EQUAL_UINT8(9, bus.driver().lastColor['R']);

    const auto scratch = bus.shaderScratch();
    TEST_ASSERT_EQUAL_UINT32(0U, static_cast<uint32_t>(scratch.size()));
}

void test_light_bus_checks_driver_readiness(void)
{
    auto settings = MockLightDriver::LightDriverSettingsType{};
    settings.initiallyReady = false;
    lw::busses::LightBus<TestColor, MockLightDriver> bus(settings);
    bus.pixel() = TestColor{7, 8, 9};

    TEST_ASSERT_FALSE(bus.isReadyToUpdate());
    bus.show();
    TEST_ASSERT_EQUAL_UINT32(0U, static_cast<uint32_t>(bus.driver().writeCount));

    bus.driver().ready = true;
    TEST_ASSERT_TRUE(bus.isReadyToUpdate());
    bus.show();
    TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(bus.driver().writeCount));
}
} // namespace

void setUp(void)
{
}

void tearDown(void)
{
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(test_light_bus_applies_shader_and_preserves_root_pixel);
    RUN_TEST(test_light_bus_nil_shader_writes_root_and_uses_dirty_guard);
    RUN_TEST(test_light_bus_checks_driver_readiness);
    return UNITY_END();
}
