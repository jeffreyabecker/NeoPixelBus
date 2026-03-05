#include <unity.h>

#include <cstdint>
#include <vector>

#include "colors/CCTWhiteBalanceShader.h"
#include "colors/Color.h"

namespace
{
    using Color = lw::Rgbcw8Color;
    using Settings = lw::CCTWhiteBalanceShaderSettings<Color>;
    using Shader = lw::CCTWhiteBalanceShader<Color>;
    using Interlock = lw::CCTColorInterlock;

    uint8_t scale_component(uint8_t value, uint8_t unit)
    {
        return static_cast<uint8_t>((static_cast<uint16_t>(value) * static_cast<uint16_t>(unit) + 127u) / 255u);
    }

    void test_8_1_1_none_interlock_preserves_rgb_and_rewrites_white_from_controls(void)
    {
        std::vector<Color> frame{Color{10, 20, 30, 64, 200}};

        Settings settings{};
        settings.lowKelvin = 2200;
        settings.highKelvin = 9000;
        settings.colorInterlock = Interlock::None;

        Shader shader(settings);
        shader.apply(lw::span<Color>{frame.data(), frame.size()});

        TEST_ASSERT_EQUAL_UINT8(10, frame[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(20, frame[0]['G']);
        TEST_ASSERT_EQUAL_UINT8(30, frame[0]['B']);

        const uint8_t expectedWarm = scale_component(200, static_cast<uint8_t>(255u - 64u));
        const uint8_t expectedCool = scale_component(200, 64);
        TEST_ASSERT_EQUAL_UINT8(expectedWarm, frame[0]['W']);
        TEST_ASSERT_EQUAL_UINT8(expectedCool, frame[0]['C']);
    }

    void test_8_1_2_force_off_interlock_sets_rgb_to_zero(void)
    {
        std::vector<Color> frame{Color{255, 128, 64, 128, 128}};

        Settings settings{};
        settings.colorInterlock = Interlock::ForceOff;

        Shader shader(settings);
        shader.apply(lw::span<Color>{frame.data(), frame.size()});

        TEST_ASSERT_EQUAL_UINT8(0, frame[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(0, frame[0]['G']);
        TEST_ASSERT_EQUAL_UINT8(0, frame[0]['B']);
    }

    void test_8_1_3_force_on_interlock_sets_rgb_to_max(void)
    {
        std::vector<Color> frame{Color{1, 2, 3, 10, 220}};

        Settings settings{};
        settings.colorInterlock = Interlock::ForceOn;

        Shader shader(settings);
        shader.apply(lw::span<Color>{frame.data(), frame.size()});

        TEST_ASSERT_EQUAL_UINT8(255, frame[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(255, frame[0]['G']);
        TEST_ASSERT_EQUAL_UINT8(255, frame[0]['B']);
    }

    void test_8_1_4_match_white_interlock_rewrites_rgb_from_kelvin_and_brightness(void)
    {
        std::vector<Color> warmFrame{Color{0, 0, 0, 0, 255}};
        std::vector<Color> coolFrame{Color{0, 0, 0, 255, 255}};
        std::vector<Color> dimFrame{Color{0, 0, 0, 255, 64}};

        Settings settings{};
        settings.lowKelvin = 2700;
        settings.highKelvin = 6500;
        settings.colorInterlock = Interlock::MatchWhite;

        Shader shader(settings);
        shader.apply(lw::span<Color>{warmFrame.data(), warmFrame.size()});
        shader.apply(lw::span<Color>{coolFrame.data(), coolFrame.size()});
        shader.apply(lw::span<Color>{dimFrame.data(), dimFrame.size()});

        TEST_ASSERT_TRUE(warmFrame[0]['R'] >= warmFrame[0]['B']);
        TEST_ASSERT_TRUE(coolFrame[0]['B'] > warmFrame[0]['B']);
        TEST_ASSERT_TRUE(coolFrame[0]['R'] <= warmFrame[0]['R']);

        TEST_ASSERT_TRUE(dimFrame[0]['R'] < coolFrame[0]['R']);
        TEST_ASSERT_TRUE(dimFrame[0]['G'] < coolFrame[0]['G']);
        TEST_ASSERT_TRUE(dimFrame[0]['B'] < coolFrame[0]['B']);
    }

    void test_8_1_5_brightness_zero_turns_white_channels_off(void)
    {
        std::vector<Color> frame{Color{50, 60, 70, 200, 0}};

        Settings settings{};
        settings.colorInterlock = Interlock::None;

        Shader shader(settings);
        shader.apply(lw::span<Color>{frame.data(), frame.size()});

        TEST_ASSERT_EQUAL_UINT8(0, frame[0]['W']);
        TEST_ASSERT_EQUAL_UINT8(0, frame[0]['C']);
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
    RUN_TEST(test_8_1_1_none_interlock_preserves_rgb_and_rewrites_white_from_controls);
    RUN_TEST(test_8_1_2_force_off_interlock_sets_rgb_to_zero);
    RUN_TEST(test_8_1_3_force_on_interlock_sets_rgb_to_max);
    RUN_TEST(test_8_1_4_match_white_interlock_rewrites_rgb_from_kelvin_and_brightness);
    RUN_TEST(test_8_1_5_brightness_zero_turns_white_channels_off);
    return UNITY_END();
}
