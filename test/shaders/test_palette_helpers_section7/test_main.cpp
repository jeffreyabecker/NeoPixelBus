#include <unity.h>

#include <array>

#include "colors/palette/Palette.h"

namespace
{
    using Stop = lw::colors::palettes::PaletteStop<lw::Rgb8Color>;

    struct PaletteLikeRgb8
    {
        using StopType = Stop;

        explicit PaletteLikeRgb8(lw::span<const Stop> value)
            : _stops(value)
        {
        }

        lw::span<const Stop> stops() const
        {
            return _stops;
        }

    private:
        lw::span<const Stop> _stops;
    };

    constexpr std::array<Stop, 2> kGradientA = {
        Stop{0, lw::Rgb8Color(0, 0, 0)},
        Stop{255, lw::Rgb8Color(255, 255, 255)}};

    constexpr std::array<Stop, 2> kGradientB = {
        Stop{0, lw::Rgb8Color(255, 0, 0)},
        Stop{255, lw::Rgb8Color(255, 0, 0)}};

    lw::span<const Stop> gradientASpan()
    {
        return lw::span<const Stop>(kGradientA.data(), kGradientA.size());
    }

    lw::span<const Stop> gradientBSpan()
    {
        return lw::span<const Stop>(kGradientB.data(), kGradientB.size());
    }

    void test_helper_map_transition_progress_to_blend(void)
    {
        TEST_ASSERT_EQUAL_UINT8(0, lw::colors::palettes::mapTransitionProgressToBlend8(0, 100));
        TEST_ASSERT_EQUAL_UINT8(127, lw::colors::palettes::mapTransitionProgressToBlend8(50, 100));
        TEST_ASSERT_EQUAL_UINT8(255, lw::colors::palettes::mapTransitionProgressToBlend8(100, 100));
        TEST_ASSERT_EQUAL_UINT8(255, lw::colors::palettes::mapTransitionProgressToBlend8(120, 100));
        TEST_ASSERT_EQUAL_UINT8(255, lw::colors::palettes::mapTransitionProgressToBlend8(1, 0));
    }

    void test_sample_palette_single_range_overload(void)
    {
        std::array<lw::Rgb8Color, 3> out{};
        lw::IndexRange indexes(static_cast<size_t>(0),
                               static_cast<size_t>(128),
                               out.size());
        const PaletteLikeRgb8 paletteLike(gradientASpan());

        const size_t written = lw::colors::palettes::samplePalette(paletteLike,
                                                 indexes,
                                                 lw::span<lw::Rgb8Color>(out.data(), out.size()));

        TEST_ASSERT_EQUAL_UINT32(3, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(0, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(127, out[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(255, out[2]['R']);
    }

    void test_sample_palette_step_default_stops_convenience(void)
    {
        std::array<lw::Rgb8Color, 2> out{};
        const size_t written = lw::colors::palettes::samplePalette(gradientASpan(),
                                                 static_cast<size_t>(254),
                                                 lw::span<lw::Rgb8Color>(out.data(), out.size()));

        TEST_ASSERT_EQUAL_UINT32(2, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(253, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(254, out[1]['R']);
    }

    void test_sample_palette_transition_blend_progress8_overload(void)
    {
        const PaletteLikeRgb8 from(gradientASpan());
        const PaletteLikeRgb8 to(gradientBSpan());

        std::array<lw::Rgb8Color, 1> out{};
        lw::IndexRange indexes(static_cast<size_t>(128),
                               static_cast<size_t>(1),
                               out.size());

        const size_t written = lw::colors::palettes::samplePalette(from,
                                                 to,
                                                 indexes,
                                                 lw::span<lw::Rgb8Color>(out.data(), out.size()),
                                                 static_cast<uint8_t>(128));

        TEST_ASSERT_EQUAL_UINT32(1, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(191, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(63, out[0]['G']);
        TEST_ASSERT_EQUAL_UINT8(63, out[0]['B']);
    }

    void test_sample_palette_transition_duration_overload(void)
    {
        const PaletteLikeRgb8 from(gradientASpan());
        const PaletteLikeRgb8 to(gradientBSpan());

        std::array<lw::Rgb8Color, 1> out{};
        lw::IndexRange indexes(static_cast<size_t>(128),
                               static_cast<size_t>(1),
                               out.size());

        const size_t written = lw::colors::palettes::samplePalette(from,
                                                 to,
                                                 indexes,
                                                 lw::span<lw::Rgb8Color>(out.data(), out.size()),
                                                 static_cast<uint8_t>(50),
                                                 static_cast<uint8_t>(100));

        TEST_ASSERT_EQUAL_UINT32(1, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(190, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(64, out[0]['G']);
        TEST_ASSERT_EQUAL_UINT8(64, out[0]['B']);
    }

    void test_sample_palette_transition_step_default_convenience(void)
    {
        const PaletteLikeRgb8 from(gradientASpan());
        const PaletteLikeRgb8 to(gradientBSpan());
        std::array<lw::Rgb8Color, 2> out{};

        const size_t written = lw::colors::palettes::samplePalette(from,
                                                 to,
                                                 static_cast<size_t>(127),
                                                 lw::span<lw::Rgb8Color>(out.data(), out.size()),
                                                 static_cast<uint8_t>(128));

        TEST_ASSERT_EQUAL_UINT32(2, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(190, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(63, out[0]['G']);
        TEST_ASSERT_EQUAL_UINT8(63, out[0]['B']);
        TEST_ASSERT_EQUAL_UINT8(191, out[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(63, out[1]['G']);
        TEST_ASSERT_EQUAL_UINT8(63, out[1]['B']);
    }
}

void setUp(void)
{
}

void tearDown(void)
{
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_helper_map_transition_progress_to_blend);
    RUN_TEST(test_sample_palette_single_range_overload);
    RUN_TEST(test_sample_palette_step_default_stops_convenience);
    RUN_TEST(test_sample_palette_transition_blend_progress8_overload);
    RUN_TEST(test_sample_palette_transition_duration_overload);
    RUN_TEST(test_sample_palette_transition_step_default_convenience);
    return UNITY_END();
}
