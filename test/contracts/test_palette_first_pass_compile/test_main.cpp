#include <unity.h>

#include <array>
#include <type_traits>

#include "core/IndexIterator.h"
#include "colors/palette/Palette.h"

namespace
{
    void test_palette_first_pass_compile(void)
    {
        static_assert(lw::ColorType<lw::Rgb8Color>, "Rgb8Color must satisfy ColorType");
        static_assert(lw::IsPaletteLike<lw::Palette<lw::Rgb8Color>>::value, "Palette<TColor> must satisfy IsPaletteLike");
        static_assert(std::is_class<lw::BlendLinearContiguous<>>::value, "BlendLinearContiguous must be class template");
        static_assert(std::is_class<lw::BlendNearestContiguous<>>::value, "BlendNearestContiguous must be class template");
        static_assert(std::is_class<lw::WrapClamp>::value, "WrapClamp must be class");
        static_assert(std::is_class<lw::WrapCircular>::value, "WrapCircular must be class");

        lw::PaletteSampleOptions<lw::Rgb8Color> options;

        lw::PaletteStop<lw::Rgb8Color> stop{};
        stop.index = 0;
        stop.color = lw::Rgb8Color(1, 2, 3);

        lw::Palette<lw::Rgb8Color> palette(lw::span<const lw::PaletteStop<lw::Rgb8Color>>(&stop, 1));
        TEST_ASSERT_TRUE(palette.size() == 1);

        std::array<lw::PaletteStop<lw::Rgb8Color>, 2> sampleStops = {
            lw::PaletteStop<lw::Rgb8Color>{0, lw::Rgb8Color(0, 0, 0)},
            lw::PaletteStop<lw::Rgb8Color>{255, lw::Rgb8Color(255, 255, 255)}};
        lw::Palette<lw::Rgb8Color> samplePaletteLike(
            lw::span<const lw::PaletteStop<lw::Rgb8Color>>(sampleStops.data(), sampleStops.size()));
        std::array<lw::Rgb8Color, 2> sampledOutput{};
        lw::IndexRange sampleIndexes(0, 128, sampledOutput.size());
        const size_t sampledCount = lw::samplePalette(
            samplePaletteLike,
            sampleIndexes,
            lw::span<lw::Rgb8Color>(sampledOutput.data(), sampledOutput.size()),
            options);
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(sampledOutput.size()), static_cast<uint32_t>(sampledCount));

        lw::IndexRange nearestSampleIndexes(0, 128, sampledOutput.size());
        const size_t nearestSampledCount = lw::samplePalette<lw::BlendNearestContiguous<>>(
            samplePaletteLike,
            nearestSampleIndexes,
            lw::span<lw::Rgb8Color>(sampledOutput.data(), sampledOutput.size()),
            options);
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(sampledOutput.size()), static_cast<uint32_t>(nearestSampledCount));

        lw::IndexIterator indexIt(10, 5, 3);
        const lw::IndexSentinel indexEnd{};
        TEST_ASSERT_FALSE(indexIt == indexEnd);
        TEST_ASSERT_EQUAL_UINT8(10, *indexIt);
        ++indexIt;
        TEST_ASSERT_EQUAL_UINT8(15, *indexIt);
        ++indexIt;
        TEST_ASSERT_EQUAL_UINT8(20, *indexIt);
        ++indexIt;
        TEST_ASSERT_TRUE(indexIt == indexEnd);

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
    RUN_TEST(test_palette_first_pass_compile);
    return UNITY_END();
}
