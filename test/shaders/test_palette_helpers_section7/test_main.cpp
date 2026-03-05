#include <unity.h>

#include <array>

#include "colors/palette/Palette.h"

namespace
{
    using Stop = lw::PaletteStop<lw::Rgb8Color>;

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
        TEST_ASSERT_EQUAL_UINT8(0, lw::mapTransitionProgressToBlend8(0, 100));
        TEST_ASSERT_EQUAL_UINT8(127, lw::mapTransitionProgressToBlend8(50, 100));
        TEST_ASSERT_EQUAL_UINT8(255, lw::mapTransitionProgressToBlend8(100, 100));
        TEST_ASSERT_EQUAL_UINT8(255, lw::mapTransitionProgressToBlend8(120, 100));
        TEST_ASSERT_EQUAL_UINT8(255, lw::mapTransitionProgressToBlend8(1, 0));
    }

    void test_helper_sample_palette_at_position(void)
    {
        const lw::Rgb8Color sampled = lw::samplePaletteAtPosition(gradientASpan(),
                                                                  static_cast<size_t>(5),
                                                                  static_cast<size_t>(10));

        TEST_ASSERT_EQUAL_UINT8(140, sampled['R']);
        TEST_ASSERT_EQUAL_UINT8(140, sampled['G']);
        TEST_ASSERT_EQUAL_UINT8(140, sampled['B']);
    }

    void test_helper_sample_palette_by_position_span(void)
    {
        std::array<lw::Rgb8Color, 3> out{};
        const size_t written = lw::samplePaletteByPosition(gradientASpan(),
                                                           static_cast<size_t>(0),
                                                           static_cast<size_t>(1),
                                                           static_cast<size_t>(4),
                                                           lw::span<lw::Rgb8Color>(out.data(), out.size()));

        TEST_ASSERT_EQUAL_UINT32(3, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(0, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(84, out[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(169, out[2]['R']);
    }

    void test_helper_palette_like_position_helpers(void)
    {
        const PaletteLikeRgb8 paletteLike(gradientASpan());
        std::array<lw::Rgb8Color, 2> out{};

        const size_t written = lw::samplePaletteByPosition(paletteLike,
                                                           static_cast<size_t>(1),
                                                           static_cast<size_t>(2),
                                                           static_cast<size_t>(4),
                                                           lw::span<lw::Rgb8Color>(out.data(), out.size()));

        TEST_ASSERT_EQUAL_UINT32(2, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(84, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(254, out[1]['R']);

        const lw::Rgb8Color sampled = lw::samplePaletteAtPosition(paletteLike,
                                                                  static_cast<size_t>(2),
                                                                  static_cast<size_t>(4));
        TEST_ASSERT_EQUAL_UINT8(169, sampled['R']);
    }

    void test_helper_sample_palette_transition_stops(void)
    {
        const lw::Rgb8Color sampled = lw::samplePaletteTransition(gradientASpan(),
                                                                  gradientBSpan(),
                                                                  static_cast<uint8_t>(128),
                                                                  static_cast<size_t>(50),
                                                                  static_cast<size_t>(100));

        TEST_ASSERT_EQUAL_UINT8(190, sampled['R']);
        TEST_ASSERT_EQUAL_UINT8(64, sampled['G']);
        TEST_ASSERT_EQUAL_UINT8(64, sampled['B']);
    }

    void test_helper_sample_palette_transition_palette_like(void)
    {
        const PaletteLikeRgb8 from(gradientASpan());
        const PaletteLikeRgb8 to(gradientBSpan());

        const lw::Rgb8Color sampled = lw::samplePaletteTransition(from,
                                                                  to,
                                                                  static_cast<uint8_t>(64),
                                                                  static_cast<size_t>(25),
                                                                  static_cast<size_t>(100));

        TEST_ASSERT_EQUAL_UINT8(110, sampled['R']);
        TEST_ASSERT_EQUAL_UINT8(47, sampled['G']);
        TEST_ASSERT_EQUAL_UINT8(47, sampled['B']);
    }

    void test_helper_sample_palette_with_modes_stops(void)
    {
        std::array<lw::Rgb8Color, 3> out{};
        const size_t written = lw::samplePaletteWithModes(gradientASpan(),
                                                          static_cast<uint8_t>(0),
                                                          static_cast<uint8_t>(128),
                                                          lw::span<lw::Rgb8Color>(out.data(), out.size()),
                                                          lw::PaletteBlendMode::Nearest,
                                                          lw::PaletteWrapMode::Clamp);

        TEST_ASSERT_EQUAL_UINT32(3, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(0, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(255, out[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(255, out[2]['R']);
    }

    void test_helper_sample_palette_with_modes_palette_like(void)
    {
        const PaletteLikeRgb8 paletteLike(gradientASpan());
        std::array<lw::Rgb8Color, 2> out{};

        const size_t written = lw::samplePaletteWithModes(paletteLike,
                                                          static_cast<uint8_t>(250),
                                                          static_cast<uint8_t>(16),
                                                          lw::span<lw::Rgb8Color>(out.data(), out.size()),
                                                          lw::PaletteBlendMode::Linear,
                                                          lw::PaletteWrapMode::Circular);

        TEST_ASSERT_EQUAL_UINT32(2, static_cast<uint32_t>(written));
        TEST_ASSERT_TRUE(out[0]['R'] > 240);
        TEST_ASSERT_TRUE(out[1]['R'] < 20);
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
    RUN_TEST(test_helper_sample_palette_at_position);
    RUN_TEST(test_helper_sample_palette_by_position_span);
    RUN_TEST(test_helper_palette_like_position_helpers);
    RUN_TEST(test_helper_sample_palette_transition_stops);
    RUN_TEST(test_helper_sample_palette_transition_palette_like);
    RUN_TEST(test_helper_sample_palette_with_modes_stops);
    RUN_TEST(test_helper_sample_palette_with_modes_palette_like);
    return UNITY_END();
}
