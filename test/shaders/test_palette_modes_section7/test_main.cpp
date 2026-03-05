#include <unity.h>

#include <array>

#include "core/IndexIterator.h"
#include "colors/palette/Palette.h"
#include "colors/palette/WrappedPaletteIndexes.h"

namespace
{
    using Stop = lw::PaletteStop<lw::Rgb8Color>;

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

    lw::Palette<lw::Rgb8Color> widePalette()
    {
        return lw::Palette<lw::Rgb8Color>(wideStops());
    }

    lw::span<const Stop> rangeStops()
    {
        return lw::span<const Stop>(kRangeStops.data(), kRangeStops.size());
    }

    lw::Palette<lw::Rgb8Color> rangePalette()
    {
        return lw::Palette<lw::Rgb8Color>(rangeStops());
    }

    lw::span<const Stop> tieStops()
    {
        return lw::span<const Stop>(kTieStops.data(), kTieStops.size());
    }

    lw::Palette<lw::Rgb8Color> tiePalette()
    {
        return lw::Palette<lw::Rgb8Color>(tieStops());
    }

    template <typename TBlend,
              typename TWrap = lw::WrapClamp,
              typename TPaletteLike>
    lw::Rgb8Color sampleScalar(const TPaletteLike &palette,
                               size_t paletteIndex)
    {
        std::array<lw::Rgb8Color, 1> sampled{};
        lw::IndexRange paletteIndexes(paletteIndex, 1, 1);
        lw::samplePalette<TBlend, TWrap>(palette,
                                         paletteIndexes,
                                         sampled);
        return sampled[0];
    }

    void test_blend_step_and_hold_midpoint(void)
    {
        const lw::Rgb8Color stepped = sampleScalar<lw::BlendStepContiguous>(widePalette(), 120);
        const lw::Rgb8Color heldLow = sampleScalar<lw::BlendHoldMidpointContiguous>(widePalette(), 127);
        const lw::Rgb8Color heldHigh = sampleScalar<lw::BlendHoldMidpointContiguous>(widePalette(), 128);

        TEST_ASSERT_EQUAL_UINT8(0, stepped['R']);
        TEST_ASSERT_EQUAL_UINT8(0, heldLow['R']);
        TEST_ASSERT_EQUAL_UINT8(255, heldHigh['R']);
    }

    void test_blend_smooth_cubic_cosine_family(void)
    {
        const lw::Rgb8Color linear = sampleScalar<lw::BlendLinearContiguous>(widePalette(), 64);
        const lw::Rgb8Color smooth = sampleScalar<lw::BlendSmoothstepContiguous>(widePalette(), 64);
        const lw::Rgb8Color cubic = sampleScalar<lw::BlendCubicContiguous>(widePalette(), 64);
        const lw::Rgb8Color cosine = sampleScalar<lw::BlendCosineContiguous>(widePalette(), 64);

        TEST_ASSERT_EQUAL_UINT8(64, linear['R']);
        TEST_ASSERT_TRUE(smooth['R'] < linear['R']);
        TEST_ASSERT_TRUE(cubic['R'] < smooth['R']);
        TEST_ASSERT_TRUE(cosine['R'] <= smooth['R']);
    }

    void test_blend_gamma_quantized_dithered(void)
    {
        const lw::Rgb8Color gamma = sampleScalar<lw::BlendGammaLinearContiguous>(widePalette(), 128);
        const lw::Rgb8Color quant4 = sampleScalar<lw::BlendQuantizedContiguous<4>, lw::WrapClamp>(widePalette(), 64);
        const lw::Rgb8Color dither = sampleScalar<lw::BlendDitheredLinearContiguous>(widePalette(), 63);

        TEST_ASSERT_TRUE(gamma['R'] > 127);
        TEST_ASSERT_EQUAL_UINT8(85, quant4['R']);
        TEST_ASSERT_TRUE(dither['R'] >= 62);
    }

    void test_nearest_tie_break_modes(void)
    {
        const lw::Rgb8Color stable = sampleScalar<lw::BlendNearestContiguous<lw::NearestTieStable>, lw::WrapClamp>(tiePalette(), 1);
        const lw::Rgb8Color left = sampleScalar<lw::BlendNearestContiguous<lw::NearestTieLeft>, lw::WrapClamp>(tiePalette(), 1);
        const lw::Rgb8Color right = sampleScalar<lw::BlendNearestContiguous<lw::NearestTieRight>, lw::WrapClamp>(tiePalette(), 1);

        TEST_ASSERT_EQUAL_UINT8(255, stable['R']);
        TEST_ASSERT_EQUAL_UINT8(255, left['R']);
        TEST_ASSERT_EQUAL_UINT8(255, right['B']);
    }

    void test_wrap_mode_index_mapping(void)
    {
        const uint8_t windowIndex = lw::WrapWindow<40, 200>::mapPositionToPaletteIndex(3, 5);
        const uint8_t moduloIndex = lw::WrapModuloSpan<10, 13>::mapPositionToPaletteIndex(7, 99);

        TEST_ASSERT_EQUAL_UINT8(255, lw::WrapClamp::mapPositionToPaletteIndex(100, 10));
        TEST_ASSERT_EQUAL_UINT8(63, lw::WrapMirror::mapPositionToPaletteIndex(7, 5));
        TEST_ASSERT_EQUAL_UINT8(0, lw::WrapHoldFirst::mapPositionToPaletteIndex(7, 5));
        TEST_ASSERT_EQUAL_UINT8(255, lw::WrapHoldLast::mapPositionToPaletteIndex(7, 5));
        TEST_ASSERT_EQUAL_UINT8(160, windowIndex);
        TEST_ASSERT_EQUAL_UINT8(13, moduloIndex);
        TEST_ASSERT_EQUAL_UINT8(96, lw::WrapOffsetCircular<32>::mapPositionToPaletteIndex(1, 4));
    }

    void test_wrap_blackout_position_sampling(void)
    {
        std::array<lw::Rgb8Color, 3> out{};
        const lw::Palette<lw::Rgb8Color> palette(rangeStops());
        lw::WrappedPaletteIndexes<lw::WrapBlackout> paletteIndexes(static_cast<size_t>(0),
                                                                    static_cast<size_t>(3),
                                                                    static_cast<size_t>(2),
                                                                    out.size(),
                                                                    palette.maxIndex());
        const size_t written = lw::samplePalette<lw::BlendLinearContiguous, lw::WrapBlackout>(
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
        const lw::Rgb8Color firstHeld = sampleScalar<lw::BlendLinearContiguous, lw::WrapHoldFirst>(rangePalette(), 10);
        const lw::Rgb8Color lastHeld = sampleScalar<lw::BlendLinearContiguous, lw::WrapHoldLast>(rangePalette(), 10);

        TEST_ASSERT_EQUAL_UINT8(10, firstHeld['R']);
        TEST_ASSERT_EQUAL_UINT8(200, lastHeld['R']);
    }

    void test_blend_mode_cost_smoke(void)
    {
        std::array<lw::Rgb8Color, 64> out{};
        lw::IndexRange paletteIndexes(0, 4, out.size());

        const size_t linearCount = lw::samplePalette<lw::BlendLinearContiguous>(widePalette(), paletteIndexes, lw::span<lw::Rgb8Color>(out.data(), out.size()));
        const size_t smoothCount = lw::samplePalette<lw::BlendSmoothstepContiguous>(widePalette(), paletteIndexes, lw::span<lw::Rgb8Color>(out.data(), out.size()));
        const size_t gammaCount = lw::samplePalette<lw::BlendGammaLinearContiguous>(widePalette(), paletteIndexes, lw::span<lw::Rgb8Color>(out.data(), out.size()));

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
