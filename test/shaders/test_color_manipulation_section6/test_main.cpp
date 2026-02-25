#include <unity.h>

#include <cstdint>

#include "colors/Color.h"
#include "colors/ColorMath.h"

namespace
{
    void test_6_1_1_darken_saturating_subtract_rgb8(void)
    {
        npb::Rgb8Color color(5, 10, 250);
        npb::darken(color, static_cast<uint8_t>(10));

        TEST_ASSERT_EQUAL_UINT8(0, color[0]);
        TEST_ASSERT_EQUAL_UINT8(0, color[1]);
        TEST_ASSERT_EQUAL_UINT8(240, color[2]);
    }

    void test_6_1_2_lighten_saturating_add_rgb16(void)
    {
        npb::Rgb16Color color(65500, 10, 40000);
        npb::lighten(color, static_cast<uint16_t>(100));

        TEST_ASSERT_EQUAL_UINT16(65535, color[0]);
        TEST_ASSERT_EQUAL_UINT16(110, color[1]);
        TEST_ASSERT_EQUAL_UINT16(40100, color[2]);
    }

    void test_6_1_3_channel_agnostic_works_for_five_channels(void)
    {
        npb::Rgbcw8Color color(1, 2, 3, 4, 5);

        npb::lighten(color, static_cast<uint8_t>(10));
        TEST_ASSERT_EQUAL_UINT8(11, color[0]);
        TEST_ASSERT_EQUAL_UINT8(12, color[1]);
        TEST_ASSERT_EQUAL_UINT8(13, color[2]);
        TEST_ASSERT_EQUAL_UINT8(14, color[3]);
        TEST_ASSERT_EQUAL_UINT8(15, color[4]);

        npb::darken(color, static_cast<uint8_t>(20));
        TEST_ASSERT_EQUAL_UINT8(0, color[0]);
        TEST_ASSERT_EQUAL_UINT8(0, color[1]);
        TEST_ASSERT_EQUAL_UINT8(0, color[2]);
        TEST_ASSERT_EQUAL_UINT8(0, color[3]);
        TEST_ASSERT_EQUAL_UINT8(0, color[4]);
    }

    void test_6_2_1_linear_blend_float_endpoints_and_midpoint(void)
    {
        const npb::Rgb8Color left(10, 20, 30);
        const npb::Rgb8Color right(110, 220, 130);

        const auto at0 = npb::linearBlend(left, right, 0.0f);
        const auto at1 = npb::linearBlend(left, right, 1.0f);
        const auto atHalf = npb::linearBlend(left, right, 0.5f);

        TEST_ASSERT_EQUAL_UINT8(10, at0[0]);
        TEST_ASSERT_EQUAL_UINT8(20, at0[1]);
        TEST_ASSERT_EQUAL_UINT8(30, at0[2]);

        TEST_ASSERT_EQUAL_UINT8(110, at1[0]);
        TEST_ASSERT_EQUAL_UINT8(220, at1[1]);
        TEST_ASSERT_EQUAL_UINT8(130, at1[2]);

        TEST_ASSERT_EQUAL_UINT8(60, atHalf[0]);
        TEST_ASSERT_EQUAL_UINT8(120, atHalf[1]);
        TEST_ASSERT_EQUAL_UINT8(80, atHalf[2]);
    }

    void test_6_2_2_linear_blend_uint8_rounding_rgb8(void)
    {
        const npb::Rgb8Color left(0, 10, 255);
        const npb::Rgb8Color right(255, 110, 0);

        const auto at0 = npb::linearBlend(left, right, static_cast<uint8_t>(0));
        const auto at255 = npb::linearBlend(left, right, static_cast<uint8_t>(255));
        const auto at128 = npb::linearBlend(left, right, static_cast<uint8_t>(128));

        TEST_ASSERT_EQUAL_UINT8(0, at0[0]);
        TEST_ASSERT_EQUAL_UINT8(10, at0[1]);
        TEST_ASSERT_EQUAL_UINT8(255, at0[2]);

        TEST_ASSERT_EQUAL_UINT8(254, at255[0]);
        TEST_ASSERT_EQUAL_UINT8(109, at255[1]);
        TEST_ASSERT_EQUAL_UINT8(1, at255[2]);

        TEST_ASSERT_EQUAL_UINT8(127, at128[0]);
        TEST_ASSERT_EQUAL_UINT8(60, at128[1]);
        TEST_ASSERT_EQUAL_UINT8(127, at128[2]);
    }

    void test_6_2_3_linear_blend_uint8_rounding_rgb16(void)
    {
        const npb::Rgb16Color left(0, 1000, 65535);
        const npb::Rgb16Color right(65535, 3000, 0);

        const auto at128 = npb::linearBlend(left, right, static_cast<uint8_t>(128));

        TEST_ASSERT_EQUAL_UINT16(32767, at128[0]);
        TEST_ASSERT_EQUAL_UINT16(2000, at128[1]);
        TEST_ASSERT_EQUAL_UINT16(32767, at128[2]);
    }

    void test_6_2_4_bilinear_blend_weighted_interpolation(void)
    {
        const npb::Rgb8Color c00(0, 0, 0);
        const npb::Rgb8Color c01(100, 100, 100);
        const npb::Rgb8Color c10(200, 200, 200);
        const npb::Rgb8Color c11(255, 255, 255);

        const auto blended = npb::bilinearBlend(c00, c01, c10, c11, 0.5f, 0.5f);

        TEST_ASSERT_EQUAL_UINT8(138, blended[0]);
        TEST_ASSERT_EQUAL_UINT8(138, blended[1]);
        TEST_ASSERT_EQUAL_UINT8(138, blended[2]);
    }

    struct OverrideBackend
    {
        static constexpr void darken(npb::Rgbw8Color &color, uint8_t delta)
        {
            for (size_t idx = 0; idx < npb::Rgbw8Color::ChannelCount; ++idx)
            {
                color[idx] = static_cast<uint8_t>(color[idx] > delta ? color[idx] - delta : 0);
            }
        }

        static constexpr void lighten(npb::Rgbw8Color &color, uint8_t delta)
        {
            for (size_t idx = 0; idx < npb::Rgbw8Color::ChannelCount; ++idx)
            {
                const uint16_t next = static_cast<uint16_t>(color[idx]) + delta;
                color[idx] = static_cast<uint8_t>(next > 255 ? 255 : next);
            }
        }

        static constexpr npb::Rgbw8Color linearBlend(const npb::Rgbw8Color &, const npb::Rgbw8Color &, float)
        {
            return npb::Rgbw8Color(7, 7, 7, 7);
        }

        static constexpr npb::Rgbw8Color linearBlend(const npb::Rgbw8Color &, const npb::Rgbw8Color &, uint8_t)
        {
            return npb::Rgbw8Color(9, 9, 9, 9);
        }
    };
}

namespace npb
{
    template <>
    struct ColorMathBackendSelector<Rgbw8Color>
    {
        using Type = OverrideBackend;
    };
}

namespace
{
    void test_6_3_1_backend_selector_override_hook(void)
    {
        const npb::Rgbw8Color left(1, 2, 3, 4);
        const npb::Rgbw8Color right(9, 8, 7, 6);

        const auto byFloat = npb::linearBlend(left, right, 0.25f);
        const auto byUint8 = npb::linearBlend(left, right, static_cast<uint8_t>(64));

        TEST_ASSERT_EQUAL_UINT8(7, byFloat[0]);
        TEST_ASSERT_EQUAL_UINT8(7, byFloat[1]);
        TEST_ASSERT_EQUAL_UINT8(7, byFloat[2]);
        TEST_ASSERT_EQUAL_UINT8(7, byFloat[3]);

        TEST_ASSERT_EQUAL_UINT8(9, byUint8[0]);
        TEST_ASSERT_EQUAL_UINT8(9, byUint8[1]);
        TEST_ASSERT_EQUAL_UINT8(9, byUint8[2]);
        TEST_ASSERT_EQUAL_UINT8(9, byUint8[3]);
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
    RUN_TEST(test_6_1_1_darken_saturating_subtract_rgb8);
    RUN_TEST(test_6_1_2_lighten_saturating_add_rgb16);
    RUN_TEST(test_6_1_3_channel_agnostic_works_for_five_channels);
    RUN_TEST(test_6_2_1_linear_blend_float_endpoints_and_midpoint);
    RUN_TEST(test_6_2_2_linear_blend_uint8_rounding_rgb8);
    RUN_TEST(test_6_2_3_linear_blend_uint8_rounding_rgb16);
    RUN_TEST(test_6_2_4_bilinear_blend_weighted_interpolation);
    RUN_TEST(test_6_3_1_backend_selector_override_hook);
    return UNITY_END();
}

