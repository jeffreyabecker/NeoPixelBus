#include <unity.h>

#include <cstdint>

#include "colors/Color.h"
#include "colors/HslColor.h"
#include "colors/HsbColor.h"
#include "colors/HueBlend.h"

namespace
{
    void assert_rgb8_exact(const lw::Rgb8Color &actual, uint8_t r, uint8_t g, uint8_t b)
    {
        TEST_ASSERT_EQUAL_UINT8(r, actual['R']);
        TEST_ASSERT_EQUAL_UINT8(g, actual['G']);
        TEST_ASSERT_EQUAL_UINT8(b, actual['B']);
    }

    void assert_rgb16_near(const lw::Rgb16Color &actual,
                           uint16_t r,
                           uint16_t g,
                           uint16_t b,
                           uint16_t tolerance)
    {
        TEST_ASSERT_UINT16_WITHIN(tolerance, r, actual['R']);
        TEST_ASSERT_UINT16_WITHIN(tolerance, g, actual['G']);
        TEST_ASSERT_UINT16_WITHIN(tolerance, b, actual['B']);
    }

    void test_5_1_1_hsl_to_rgb8_canonical_vectors(void)
    {
        assert_rgb8_exact(lw::toRgb8(lw::HslColor(0.0f, 1.0f, 0.5f)), 255, 0, 0);
        assert_rgb8_exact(lw::toRgb8(lw::HslColor(1.0f / 3.0f, 1.0f, 0.5f)), 0, 255, 0);
        assert_rgb8_exact(lw::toRgb8(lw::HslColor(2.0f / 3.0f, 1.0f, 0.5f)), 0, 0, 255);
        assert_rgb8_exact(lw::toRgb8(lw::HslColor(0.25f, 0.0f, 0.5f)), 127, 127, 127);
    }

    void test_5_1_2_hsb_to_rgb8_canonical_vectors(void)
    {
        assert_rgb8_exact(lw::toRgb8(lw::HsbColor(0.0f, 1.0f, 1.0f)), 255, 0, 0);
        assert_rgb8_exact(lw::toRgb8(lw::HsbColor(1.0f / 3.0f, 1.0f, 1.0f)), 0, 255, 0);
        assert_rgb8_exact(lw::toRgb8(lw::HsbColor(2.0f / 3.0f, 1.0f, 1.0f)), 0, 0, 255);
        assert_rgb8_exact(lw::toRgb8(lw::HsbColor(0.6f, 0.0f, 0.5f)), 127, 127, 127);
    }

    void test_5_1_3_rgb_to_hsl_canonical_vectors(void)
    {
        const lw::HslColor red(lw::Rgb8Color(255, 0, 0));
        const lw::HslColor green(lw::Rgb8Color(0, 255, 0));
        const lw::HslColor gray(lw::Rgb8Color(64, 64, 64));

        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, red.H);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, red.S);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, red.L);

        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f / 3.0f, green.H);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, green.S);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, green.L);

        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, gray.S);
        TEST_ASSERT_FLOAT_WITHIN(0.005f, 64.0f / 255.0f, gray.L);
    }

    void test_5_1_4_rgb_to_hsb_canonical_vectors(void)
    {
        const lw::HsbColor red(lw::Rgb8Color(255, 0, 0));
        const lw::HsbColor blue(lw::Rgb8Color(0, 0, 255));
        const lw::HsbColor gray(lw::Rgb8Color(128, 128, 128));

        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, red.H);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, red.S);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, red.B);

        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 2.0f / 3.0f, blue.H);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, blue.S);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, blue.B);

        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, gray.S);
        TEST_ASSERT_FLOAT_WITHIN(0.005f, 128.0f / 255.0f, gray.B);
    }

    void test_5_2_1_round_trip_tolerance_rgb8(void)
    {
        const lw::Rgb8Color source(12, 200, 77);

        const lw::Rgb8Color fromHsl = lw::toRgb8(lw::HslColor(source));
        const lw::Rgb8Color fromHsb = lw::toRgb8(lw::HsbColor(source));

        TEST_ASSERT_UINT8_WITHIN(2, source['R'], fromHsl['R']);
        TEST_ASSERT_UINT8_WITHIN(2, source['G'], fromHsl['G']);
        TEST_ASSERT_UINT8_WITHIN(2, source['B'], fromHsl['B']);

        TEST_ASSERT_UINT8_WITHIN(2, source['R'], fromHsb['R']);
        TEST_ASSERT_UINT8_WITHIN(2, source['G'], fromHsb['G']);
        TEST_ASSERT_UINT8_WITHIN(2, source['B'], fromHsb['B']);
    }

    void test_5_2_2_round_trip_tolerance_rgb16(void)
    {
        const lw::Rgb16Color source(1234, 54321, 32100);

        const lw::Rgb16Color fromHsl = lw::toRgb16(lw::HslColor(source));
        const lw::Rgb16Color fromHsb = lw::toRgb16(lw::HsbColor(source));

        assert_rgb16_near(fromHsl, source['R'], source['G'], source['B'], 700);
        assert_rgb16_near(fromHsb, source['R'], source['G'], source['B'], 700);
    }

    void test_5_3_1_hue_blend_policy_wrap_behavior(void)
    {
        constexpr float left = 0.99f;
        constexpr float right = 0.01f;
        constexpr float progress = 0.5f;

        const float shortest = lw::HueBlendShortestDistance::HueBlend(left, right, progress);
        const float longest = lw::HueBlendLongestDistance::HueBlend(left, right, progress);
        const float clockwise = lw::HueBlendClockwiseDirection::HueBlend(left, right, progress);
        const float counterClockwise = lw::HueBlendCounterClockwiseDirection::HueBlend(left, right, progress);

        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, shortest);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, longest);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, clockwise);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, counterClockwise);
    }

    void test_5_3_2_hsl_linear_blend_uses_policy(void)
    {
        const lw::HslColor left(0.99f, 0.2f, 0.3f);
        const lw::HslColor right(0.01f, 0.6f, 0.7f);

        const auto shortest = lw::HslColor::LinearBlend<lw::HueBlendShortestDistance>(left, right, 0.5f);
        const auto longest = lw::HslColor::LinearBlend<lw::HueBlendLongestDistance>(left, right, 0.5f);

        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, shortest.H);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, longest.H);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.4f, shortest.S);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, shortest.L);
    }

    void test_5_3_3_hsb_bilinear_blend_smoke(void)
    {
        const lw::HsbColor c00(0.00f, 0.0f, 0.0f);
        const lw::HsbColor c01(0.20f, 0.2f, 0.2f);
        const lw::HsbColor c10(0.40f, 0.4f, 0.4f);
        const lw::HsbColor c11(0.60f, 0.6f, 0.6f);

        const auto blended = lw::HsbColor::BilinearBlend<lw::HueBlendShortestDistance>(
            c00,
            c01,
            c10,
            c11,
            0.5f,
            0.5f);

        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.30f, blended.H);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.30f, blended.S);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.30f, blended.B);
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
    RUN_TEST(test_5_1_1_hsl_to_rgb8_canonical_vectors);
    RUN_TEST(test_5_1_2_hsb_to_rgb8_canonical_vectors);
    RUN_TEST(test_5_1_3_rgb_to_hsl_canonical_vectors);
    RUN_TEST(test_5_1_4_rgb_to_hsb_canonical_vectors);
    RUN_TEST(test_5_2_1_round_trip_tolerance_rgb8);
    RUN_TEST(test_5_2_2_round_trip_tolerance_rgb16);
    RUN_TEST(test_5_3_1_hue_blend_policy_wrap_behavior);
    RUN_TEST(test_5_3_2_hsl_linear_blend_uses_policy);
    RUN_TEST(test_5_3_3_hsb_bilinear_blend_smoke);
    return UNITY_END();
}

