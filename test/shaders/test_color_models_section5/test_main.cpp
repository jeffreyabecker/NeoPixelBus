#include <unity.h>

#include <cstdint>

#include "virtual/colors/Color.h"
#include "virtual/colors/HslColor.h"
#include "virtual/colors/HsbColor.h"
#include "virtual/colors/HueBlend.h"

namespace
{
    void assert_rgb8_exact(const npb::Rgb8Color &actual, uint8_t r, uint8_t g, uint8_t b)
    {
        TEST_ASSERT_EQUAL_UINT8(r, actual[0]);
        TEST_ASSERT_EQUAL_UINT8(g, actual[1]);
        TEST_ASSERT_EQUAL_UINT8(b, actual[2]);
    }

    void assert_rgb16_near(const npb::Rgb16Color &actual,
                           uint16_t r,
                           uint16_t g,
                           uint16_t b,
                           uint16_t tolerance)
    {
        TEST_ASSERT_UINT16_WITHIN(tolerance, r, actual[0]);
        TEST_ASSERT_UINT16_WITHIN(tolerance, g, actual[1]);
        TEST_ASSERT_UINT16_WITHIN(tolerance, b, actual[2]);
    }

    void test_5_1_1_hsl_to_rgb8_canonical_vectors(void)
    {
        assert_rgb8_exact(npb::toRgb8(npb::HslColor(0.0f, 1.0f, 0.5f)), 255, 0, 0);
        assert_rgb8_exact(npb::toRgb8(npb::HslColor(1.0f / 3.0f, 1.0f, 0.5f)), 0, 255, 0);
        assert_rgb8_exact(npb::toRgb8(npb::HslColor(2.0f / 3.0f, 1.0f, 0.5f)), 0, 0, 255);
        assert_rgb8_exact(npb::toRgb8(npb::HslColor(0.25f, 0.0f, 0.5f)), 127, 127, 127);
    }

    void test_5_1_2_hsb_to_rgb8_canonical_vectors(void)
    {
        assert_rgb8_exact(npb::toRgb8(npb::HsbColor(0.0f, 1.0f, 1.0f)), 255, 0, 0);
        assert_rgb8_exact(npb::toRgb8(npb::HsbColor(1.0f / 3.0f, 1.0f, 1.0f)), 0, 255, 0);
        assert_rgb8_exact(npb::toRgb8(npb::HsbColor(2.0f / 3.0f, 1.0f, 1.0f)), 0, 0, 255);
        assert_rgb8_exact(npb::toRgb8(npb::HsbColor(0.6f, 0.0f, 0.5f)), 127, 127, 127);
    }

    void test_5_1_3_rgb_to_hsl_canonical_vectors(void)
    {
        const npb::HslColor red(npb::Rgb8Color(255, 0, 0));
        const npb::HslColor green(npb::Rgb8Color(0, 255, 0));
        const npb::HslColor gray(npb::Rgb8Color(64, 64, 64));

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
        const npb::HsbColor red(npb::Rgb8Color(255, 0, 0));
        const npb::HsbColor blue(npb::Rgb8Color(0, 0, 255));
        const npb::HsbColor gray(npb::Rgb8Color(128, 128, 128));

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
        const npb::Rgb8Color source(12, 200, 77);

        const npb::Rgb8Color fromHsl = npb::toRgb8(npb::HslColor(source));
        const npb::Rgb8Color fromHsb = npb::toRgb8(npb::HsbColor(source));

        TEST_ASSERT_UINT8_WITHIN(2, source[0], fromHsl[0]);
        TEST_ASSERT_UINT8_WITHIN(2, source[1], fromHsl[1]);
        TEST_ASSERT_UINT8_WITHIN(2, source[2], fromHsl[2]);

        TEST_ASSERT_UINT8_WITHIN(2, source[0], fromHsb[0]);
        TEST_ASSERT_UINT8_WITHIN(2, source[1], fromHsb[1]);
        TEST_ASSERT_UINT8_WITHIN(2, source[2], fromHsb[2]);
    }

    void test_5_2_2_round_trip_tolerance_rgb16(void)
    {
        const npb::Rgb16Color source(1234, 54321, 32100);

        const npb::Rgb16Color fromHsl = npb::toRgb16(npb::HslColor(source));
        const npb::Rgb16Color fromHsb = npb::toRgb16(npb::HsbColor(source));

        assert_rgb16_near(fromHsl, source[0], source[1], source[2], 700);
        assert_rgb16_near(fromHsb, source[0], source[1], source[2], 700);
    }

    void test_5_3_1_hue_blend_policy_wrap_behavior(void)
    {
        constexpr float left = 0.99f;
        constexpr float right = 0.01f;
        constexpr float progress = 0.5f;

        const float shortest = npb::HueBlendShortestDistance::HueBlend(left, right, progress);
        const float longest = npb::HueBlendLongestDistance::HueBlend(left, right, progress);
        const float clockwise = npb::HueBlendClockwiseDirection::HueBlend(left, right, progress);
        const float counterClockwise = npb::HueBlendCounterClockwiseDirection::HueBlend(left, right, progress);

        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, shortest);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, longest);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, clockwise);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, counterClockwise);
    }

    void test_5_3_2_hsl_linear_blend_uses_policy(void)
    {
        const npb::HslColor left(0.99f, 0.2f, 0.3f);
        const npb::HslColor right(0.01f, 0.6f, 0.7f);

        const auto shortest = npb::HslColor::LinearBlend<npb::HueBlendShortestDistance>(left, right, 0.5f);
        const auto longest = npb::HslColor::LinearBlend<npb::HueBlendLongestDistance>(left, right, 0.5f);

        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, shortest.H);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, longest.H);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.4f, shortest.S);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, shortest.L);
    }

    void test_5_3_3_hsb_bilinear_blend_smoke(void)
    {
        const npb::HsbColor c00(0.00f, 0.0f, 0.0f);
        const npb::HsbColor c01(0.20f, 0.2f, 0.2f);
        const npb::HsbColor c10(0.40f, 0.4f, 0.4f);
        const npb::HsbColor c11(0.60f, 0.6f, 0.6f);

        const auto blended = npb::HsbColor::BilinearBlend<npb::HueBlendShortestDistance>(
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
