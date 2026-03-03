#include <unity.h>

#include <array>

#include "colors/Palette.h"

namespace
{
    using Stop = lw::PaletteStop<lw::Rgb8Color>;

    void test_7_1_1_map_position_clamp_behaviour(void)
    {
        TEST_ASSERT_EQUAL_UINT8(0, lw::mapPositionToPaletteIndex(0, 5, lw::PaletteWrapMode::Clamp));
        TEST_ASSERT_EQUAL_UINT8(127, lw::mapPositionToPaletteIndex(2, 5, lw::PaletteWrapMode::Clamp));
        TEST_ASSERT_EQUAL_UINT8(255, lw::mapPositionToPaletteIndex(4, 5, lw::PaletteWrapMode::Clamp));
        TEST_ASSERT_EQUAL_UINT8(255, lw::mapPositionToPaletteIndex(99, 5, lw::PaletteWrapMode::Clamp));
    }

    void test_7_1_2_map_position_wrap_behaviour(void)
    {
        TEST_ASSERT_EQUAL_UINT8(0, lw::mapPositionToPaletteIndex(0, 5, lw::PaletteWrapMode::Wrap));
        TEST_ASSERT_EQUAL_UINT8(204, lw::mapPositionToPaletteIndex(4, 5, lw::PaletteWrapMode::Wrap));
        TEST_ASSERT_EQUAL_UINT8(0, lw::mapPositionToPaletteIndex(5, 5, lw::PaletteWrapMode::Wrap));
        TEST_ASSERT_EQUAL_UINT8(153, lw::mapPositionToPaletteIndex(8, 5, lw::PaletteWrapMode::Wrap));
    }

    void test_7_2_1_sample_palette_linear_clamp_endpoints(void)
    {
        constexpr std::array<Stop, 2> stops = {
            Stop{0, lw::Rgb8Color(0, 0, 0)},
            Stop{255, lw::Rgb8Color(255, 255, 255)}};

        const lw::GradientPalette<lw::Rgb8Color> palette(lw::span<const Stop>(stops.data(), stops.size()));

        const auto atStart = lw::samplePalette(palette, 0);
        const auto atEnd = lw::samplePalette(palette, 255);
        const auto atMid = lw::samplePalette(palette, 128);

        TEST_ASSERT_EQUAL_UINT8(0, atStart['R']);
        TEST_ASSERT_EQUAL_UINT8(0, atStart['G']);
        TEST_ASSERT_EQUAL_UINT8(0, atStart['B']);

        TEST_ASSERT_EQUAL_UINT8(254, atEnd['R']);
        TEST_ASSERT_EQUAL_UINT8(254, atEnd['G']);
        TEST_ASSERT_EQUAL_UINT8(254, atEnd['B']);

        TEST_ASSERT_EQUAL_UINT8(127, atMid['R']);
        TEST_ASSERT_EQUAL_UINT8(127, atMid['G']);
        TEST_ASSERT_EQUAL_UINT8(127, atMid['B']);
    }

    void test_7_2_2_sample_palette_nearest_mode(void)
    {
        constexpr std::array<Stop, 2> stops = {
            Stop{0, lw::Rgb8Color(255, 0, 0)},
            Stop{200, lw::Rgb8Color(0, 0, 255)}};

        const lw::GradientPalette<lw::Rgb8Color> palette(lw::span<const Stop>(stops.data(), stops.size()));

        lw::PaletteSampleOptions<lw::Rgb8Color> options;
        options.blendMode = lw::PaletteBlendMode::Nearest;

        const auto sampled = lw::samplePalette(palette, 150, options);

        TEST_ASSERT_EQUAL_UINT8(0, sampled['R']);
        TEST_ASSERT_EQUAL_UINT8(0, sampled['G']);
        TEST_ASSERT_EQUAL_UINT8(255, sampled['B']);
    }

    void test_7_2_3_sample_palette_brightness_scaling(void)
    {
        constexpr std::array<Stop, 2> stops = {
            Stop{0, lw::Rgb8Color(255, 200, 100)},
            Stop{255, lw::Rgb8Color(255, 200, 100)}};

        const lw::GradientPalette<lw::Rgb8Color> palette(lw::span<const Stop>(stops.data(), stops.size()));

        lw::PaletteSampleOptions<lw::Rgb8Color> options;
        options.brightnessScale = 128;

        const auto sampled = lw::samplePalette(palette, 50, options);

        TEST_ASSERT_EQUAL_UINT8(128, sampled['R']);
        TEST_ASSERT_EQUAL_UINT8(100, sampled['G']);
        TEST_ASSERT_EQUAL_UINT8(50, sampled['B']);
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
    RUN_TEST(test_7_1_1_map_position_clamp_behaviour);
    RUN_TEST(test_7_1_2_map_position_wrap_behaviour);
    RUN_TEST(test_7_2_1_sample_palette_linear_clamp_endpoints);
    RUN_TEST(test_7_2_2_sample_palette_nearest_mode);
    RUN_TEST(test_7_2_3_sample_palette_brightness_scaling);
    return UNITY_END();
}
