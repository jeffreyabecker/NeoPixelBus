#include <unity.h>

#include <array>

#include "core/IndexIterator.h"
#include "colors/palette/Sampling.h"

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

    constexpr std::array<Stop, 2> kStops = {
        Stop{0, lw::Rgb8Color(0, 0, 0)},
        Stop{255, lw::Rgb8Color(255, 255, 255)}};

    lw::span<const Stop> makeStopsSpan()
    {
        return lw::span<const Stop>(kStops.data(), kStops.size());
    }

    void test_overload_stops_index_iter_output_span(void)
    {
        std::array<lw::Rgb8Color, 3> out{};
        lw::IndexIterator indexBegin(0, 128, out.size());

        const size_t written = lw::samplePalette(makeStopsSpan(),
                                                 indexBegin,
                                                 lw::IndexSentinel{},
                                                 lw::span<lw::Rgb8Color>(out.data(), out.size()));

        TEST_ASSERT_EQUAL_UINT32(3, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(0, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(127, out[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(0, out[2]['R']);
    }

    void test_overload_stops_index_iter_output_ptr_range(void)
    {
        std::array<lw::Rgb8Color, 2> out{};
        lw::IndexIterator indexBegin(10, 200, out.size());

        const size_t written = lw::samplePalette(makeStopsSpan(),
                                                 indexBegin,
                                                 lw::IndexSentinel{},
                                                 out.data(),
                                                 out.data() + out.size());

        TEST_ASSERT_EQUAL_UINT32(2, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(9, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(209, out[1]['R']);
    }

    void test_overload_stops_index_span_output_span(void)
    {
        const std::array<uint8_t, 3> indices = {0, 64, 255};
        std::array<lw::Rgb8Color, 3> out{};

        const size_t written = lw::samplePalette(makeStopsSpan(),
                                                 lw::span<const uint8_t>(indices.data(), indices.size()),
                                                 lw::span<lw::Rgb8Color>(out.data(), out.size()));

        TEST_ASSERT_EQUAL_UINT32(3, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(0, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(63, out[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(254, out[2]['R']);
    }

    void test_overload_stops_first_step_output_span(void)
    {
        std::array<lw::Rgb8Color, 3> out{};

        const size_t written = lw::samplePalette(makeStopsSpan(),
                                                 static_cast<uint8_t>(0),
                                                 static_cast<uint8_t>(64),
                                                 lw::span<lw::Rgb8Color>(out.data(), out.size()));

        TEST_ASSERT_EQUAL_UINT32(3, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(0, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(63, out[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(127, out[2]['R']);
    }

    void test_overload_stops_first_contiguous_output_span(void)
    {
        std::array<lw::Rgb8Color, 2> out{};

        const size_t written = lw::samplePalette(makeStopsSpan(),
                                                 static_cast<uint8_t>(5),
                                                 lw::span<lw::Rgb8Color>(out.data(), out.size()));

        TEST_ASSERT_EQUAL_UINT32(2, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(4, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(5, out[1]['R']);
    }

    void test_overload_scalar_sample(void)
    {
        const lw::Rgb8Color sampled = lw::samplePalette(makeStopsSpan(), static_cast<uint8_t>(128));
        TEST_ASSERT_EQUAL_UINT8(127, sampled['R']);
        TEST_ASSERT_EQUAL_UINT8(127, sampled['G']);
        TEST_ASSERT_EQUAL_UINT8(127, sampled['B']);
    }

    void test_overload_palette_like_and_options(void)
    {
        const PaletteLikeRgb8 paletteLike(makeStopsSpan());
        std::array<lw::Rgb8Color, 2> out{};
        lw::IndexIterator indexBegin(100, 50, out.size());

        lw::PaletteSampleOptions<lw::Rgb8Color> options;
        options.brightnessScale = 128;

        const size_t written = lw::samplePalette(paletteLike,
                                                 indexBegin,
                                                 lw::IndexSentinel{},
                                                 lw::span<lw::Rgb8Color>(out.data(), out.size()),
                                                 options);

        TEST_ASSERT_EQUAL_UINT32(2, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(49, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(74, out[1]['R']);
    }

    void test_overload_explicit_blend_strategy_template(void)
    {
        const std::array<uint8_t, 2> indices = {127, 128};
        std::array<lw::Rgb8Color, 2> out{};

        const size_t written = lw::samplePalette<lw::BlendNearestContiguous<>>(
            makeStopsSpan(),
            lw::span<const uint8_t>(indices.data(), indices.size()),
            lw::span<lw::Rgb8Color>(out.data(), out.size()));

        TEST_ASSERT_EQUAL_UINT32(2, static_cast<uint32_t>(written));
        TEST_ASSERT_EQUAL_UINT8(0, out[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(255, out[1]['R']);
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
    RUN_TEST(test_overload_stops_index_iter_output_span);
    RUN_TEST(test_overload_stops_index_iter_output_ptr_range);
    RUN_TEST(test_overload_stops_index_span_output_span);
    RUN_TEST(test_overload_stops_first_step_output_span);
    RUN_TEST(test_overload_stops_first_contiguous_output_span);
    RUN_TEST(test_overload_scalar_sample);
    RUN_TEST(test_overload_palette_like_and_options);
    RUN_TEST(test_overload_explicit_blend_strategy_template);
    return UNITY_END();
}
