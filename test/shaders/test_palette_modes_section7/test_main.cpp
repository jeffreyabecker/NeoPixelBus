#include <unity.h>

#include <array>

#include "core/IndexIterator.h"
#include "colors/palette/Palette.h"
#include "colors/palette/WrappedPaletteIndexes.h"

namespace
{
    using Stop = lw::colors::palettes::PaletteStop<lw::Rgb8Color>;

    constexpr std::array<Stop, 2> kWideStops = {
        Stop{0, lw::Rgb8Color(0, 0, 0)},
        Stop{255, lw::Rgb8Color(255, 255, 255)}};

    constexpr std::array<Stop, 3> kRangeStops = {
        Stop{50, lw::Rgb8Color(10, 0, 0)},
        Stop{120, lw::Rgb8Color(100, 0, 0)},
        Stop{200, lw::Rgb8Color(200, 0, 0)}};

    constexpr std::array<Stop, 2> kTieStops = {
        Stop{0, lw::Rgb8Color(255, 0, 0)},
        Stop{2, lw::Rgb8Color(0, 0, 255)}};

    lw::span<const Stop> wideStops()
    {
        return lw::span<const Stop>(kWideStops.data(), kWideStops.size());
    }

    lw::colors::palettes::Palette<lw::Rgb8Color> widePalette()
    {
        return lw::colors::palettes::Palette<lw::Rgb8Color>(wideStops());
    }

    lw::span<const Stop> rangeStops()
    {
        return lw::span<const Stop>(kRangeStops.data(), kRangeStops.size());
    }

    lw::colors::palettes::Palette<lw::Rgb8Color> rangePalette()
    {
        return lw::colors::palettes::Palette<lw::Rgb8Color>(rangeStops());
    }

    lw::span<const Stop> tieStops()
    {
        return lw::span<const Stop>(kTieStops.data(), kTieStops.size());
    }

    lw::colors::palettes::Palette<lw::Rgb8Color> tiePalette()
    {
        return lw::colors::palettes::Palette<lw::Rgb8Color>(tieStops());
    }

    template <typename TBlend,
              typename TWrap = lw::colors::palettes::WrapClamp,
              typename TPaletteLike>
    lw::Rgb8Color sampleScalar(const TPaletteLike &palette,
                               size_t paletteIndex)
    {
        std::array<lw::Rgb8Color, 1> sampled{};
        lw::IndexRange paletteIndexes(paletteIndex, 1, 1);
        lw::colors::palettes::samplePalette<TBlend, TWrap>(palette,
                                         paletteIndexes,
                                         sampled);
        return sampled[0];
    }

    void test_blend_step_and_hold_midpoint(void)
    {
        const lw::Rgb8Color stepped = sampleScalar<lw::colors::palettes::BlendStepContiguous>(widePalette(), 120);
        const lw::Rgb8Color heldLow = sampleScalar<lw::colors::palettes::BlendHoldMidpointContiguous>(widePalette(), 127);
        const lw::Rgb8Color heldHigh = sampleScalar<lw::colors::palettes::BlendHoldMidpointContiguous>(widePalette(), 128);

        TEST_ASSERT_EQUAL_UINT8(0, stepped['R']);
        TEST_ASSERT_EQUAL_UINT8(0, heldLow['R']);
        TEST_ASSERT_EQUAL_UINT8(255, heldHigh['R']);
    }

    void test_blend_smooth_cubic_cosine_family(void)
    {
        const lw::Rgb8Color linear = sampleScalar<lw::colors::palettes::BlendLinearContiguous>(widePalette(), 64);
        const lw::Rgb8Color smooth = sampleScalar<lw::colors::palettes::BlendSmoothstepContiguous>(widePalette(), 64);
        const lw::Rgb8Color cubic = sampleScalar<lw::colors::palettes::BlendCubicContiguous>(widePalette(), 64);
        const lw::Rgb8Color cosine = sampleScalar<lw::colors::palettes::BlendCosineContiguous>(widePalette(), 64);

        TEST_ASSERT_EQUAL_UINT8(63, linear['R']);
        TEST_ASSERT_TRUE(smooth['R'] < linear['R']);
        TEST_ASSERT_TRUE(cubic['R'] < smooth['R']);
        TEST_ASSERT_TRUE(cosine['R'] <= smooth['R']);
    }

    void test_blend_gamma_quantized_dithered(void)
    {
        const lw::Rgb8Color gamma = sampleScalar<lw::colors::palettes::BlendGammaLinearContiguous>(widePalette(), 128);
        const lw::Rgb8Color quant4 = sampleScalar<lw::colors::palettes::BlendQuantizedContiguous<4>, lw::colors::palettes::WrapClamp>(widePalette(), 64);
        const lw::Rgb8Color dither = sampleScalar<lw::colors::palettes::BlendDitheredLinearContiguous>(widePalette(), 63);

        TEST_ASSERT_TRUE(gamma['R'] > 127);
        TEST_ASSERT_EQUAL_UINT8(85, quant4['R']);
        TEST_ASSERT_TRUE(dither['R'] >= 62);
    }

    void test_nearest_tie_break_modes(void)
    {
        const lw::Rgb8Color stable = sampleScalar<lw::colors::palettes::BlendNearestContiguous<lw::colors::palettes::NearestTieStable>, lw::colors::palettes::WrapClamp>(tiePalette(), 1);
        const lw::Rgb8Color left = sampleScalar<lw::colors::palettes::BlendNearestContiguous<lw::colors::palettes::NearestTieLeft>, lw::colors::palettes::WrapClamp>(tiePalette(), 1);
        const lw::Rgb8Color right = sampleScalar<lw::colors::palettes::BlendNearestContiguous<lw::colors::palettes::NearestTieRight>, lw::colors::palettes::WrapClamp>(tiePalette(), 1);

        TEST_ASSERT_EQUAL_UINT8(255, stable['R']);
        TEST_ASSERT_EQUAL_UINT8(255, left['R']);
        TEST_ASSERT_EQUAL_UINT8(255, right['B']);
    }

    void test_wrap_mode_index_mapping(void)
    {
        const uint8_t windowIndex = lw::colors::palettes::WrapWindow<40, 200>::mapPositionToPaletteIndex(3, 5);
        const uint8_t moduloIndex = lw::colors::palettes::WrapModuloSpan<10, 13>::mapPositionToPaletteIndex(7, 99);

        TEST_ASSERT_EQUAL_UINT8(255, lw::colors::palettes::WrapClamp::mapPositionToPaletteIndex(100, 10));
        TEST_ASSERT_EQUAL_UINT8(63, lw::colors::palettes::WrapMirror::mapPositionToPaletteIndex(7, 5));
        TEST_ASSERT_EQUAL_UINT8(0, lw::colors::palettes::WrapHoldFirst::mapPositionToPaletteIndex(7, 5));
        TEST_ASSERT_EQUAL_UINT8(255, lw::colors::palettes::WrapHoldLast::mapPositionToPaletteIndex(7, 5));
        TEST_ASSERT_EQUAL_UINT8(160, windowIndex);
        TEST_ASSERT_EQUAL_UINT8(13, moduloIndex);
        TEST_ASSERT_EQUAL_UINT8(96, lw::colors::palettes::WrapOffsetCircular<32>::mapPositionToPaletteIndex(1, 4));
    }

    void test_wrap_blackout_position_sampling(void)
    {
        std::array<lw::Rgb8Color, 3> out{};
        const lw::colors::palettes::Palette<lw::Rgb8Color> palette(rangeStops());
        lw::colors::palettes::WrappedPaletteIndexes<lw::colors::palettes::WrapBlackout> paletteIndexes(static_cast<size_t>(0),
                                                                    static_cast<size_t>(3),
                                                                    static_cast<size_t>(2),
                                                                    out.size(),
                                                                    palette.maxIndex());
        const size_t written = lw::colors::palettes::samplePalette<lw::colors::palettes::BlendLinearContiguous, lw::colors::palettes::WrapBlackout>(
            palette,
            paletteIndexes,
            lw::span<lw::Rgb8Color>(out.data(), out.size()));

        TEST_ASSERT_EQUAL_UINT32(3, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(10, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(0, out[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(0, out[2]['R']);
    }

    void test_wrap_hold_first_last_with_linear_sampling(void)
    {
        const lw::Rgb8Color firstHeld = sampleScalar<lw::colors::palettes::BlendLinearContiguous, lw::colors::palettes::WrapHoldFirst>(rangePalette(), 10);
        const lw::Rgb8Color lastHeld = sampleScalar<lw::colors::palettes::BlendLinearContiguous, lw::colors::palettes::WrapHoldLast>(rangePalette(), 10);

        TEST_ASSERT_EQUAL_UINT8(10, firstHeld['R']);
        TEST_ASSERT_EQUAL_UINT8(200, lastHeld['R']);
    }

    void test_blend_mode_cost_smoke(void)
    {
        std::array<lw::Rgb8Color, 64> out{};
        lw::IndexRange paletteIndexes(0, 4, out.size());

        const size_t linearCount = lw::colors::palettes::samplePalette<lw::colors::palettes::BlendLinearContiguous>(widePalette(), paletteIndexes, lw::span<lw::Rgb8Color>(out.data(), out.size()));
        const size_t smoothCount = lw::colors::palettes::samplePalette<lw::colors::palettes::BlendSmoothstepContiguous>(widePalette(), paletteIndexes, lw::span<lw::Rgb8Color>(out.data(), out.size()));
        const size_t gammaCount = lw::colors::palettes::samplePalette<lw::colors::palettes::BlendGammaLinearContiguous>(widePalette(), paletteIndexes, lw::span<lw::Rgb8Color>(out.data(), out.size()));

        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(out.size()), static_cast<uint32_t>(linearCount));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(out.size()), static_cast<uint32_t>(smoothCount));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(out.size()), static_cast<uint32_t>(gammaCount));
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
    RUN_TEST(test_blend_step_and_hold_midpoint);
    RUN_TEST(test_blend_smooth_cubic_cosine_family);
    RUN_TEST(test_blend_gamma_quantized_dithered);
    RUN_TEST(test_nearest_tie_break_modes);
    RUN_TEST(test_wrap_mode_index_mapping);
    RUN_TEST(test_wrap_blackout_position_sampling);
    RUN_TEST(test_wrap_hold_first_last_with_linear_sampling);
    RUN_TEST(test_blend_mode_cost_smoke);
    return UNITY_END();
}
