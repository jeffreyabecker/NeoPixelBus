#include <unity.h>

#include <array>
#include <type_traits>
#include <vector>

#include "core/IndexIterator.h"
#include "colors/palette/Palette.h"

namespace
{
void test_palette_first_pass_compile(void)
{
    static_assert(lw::ColorType<lw::Rgb8Color>, "Rgb8Color must satisfy ColorType");
    static_assert(lw::colors::palettes::IsPaletteLike<lw::colors::palettes::Palette<lw::Rgb8Color>>::value,
                  "Palette<TColor> must satisfy IsPaletteLike");
    static_assert(std::is_class<lw::colors::palettes::BlendLinearContiguous>::value,
                  "BlendLinearContiguous must be class");
    static_assert(std::is_class<lw::colors::palettes::BlendNearestContiguous<>>::value,
                  "BlendNearestContiguous must be class template");
    static_assert(std::is_class<lw::colors::palettes::blend::Linear>::value,
                  "blend::Linear must alias BlendLinearContiguous");
    static_assert(std::is_class<lw::colors::palettes::blend::Nearest<>>::value,
                  "blend::Nearest must alias BlendNearestContiguous");
    static_assert(
        std::is_class<lw::colors::palettes::blend::Interpolated<lw::colors::palettes::blend::op::Midpoint>>::value,
        "blend::Interpolated<blend::op::Midpoint> must be valid");
    static_assert(std::is_class<lw::colors::palettes::WrapClamp>::value, "WrapClamp must be class");
    static_assert(std::is_class<lw::colors::palettes::WrapCircular>::value, "WrapCircular must be class");

    lw::colors::palettes::PaletteSampleOptions<lw::Rgb8Color> options;

    lw::colors::palettes::PaletteStop<lw::Rgb8Color> stop{};
    stop.index = 0;
    stop.color = lw::Rgb8Color(1, 2, 3);

    lw::colors::palettes::Palette<lw::Rgb8Color> palette(
        lw::span<const lw::colors::palettes::PaletteStop<lw::Rgb8Color>>(&stop, 1));
    TEST_ASSERT_TRUE(palette.stops().size() == 1);

    std::array<lw::colors::palettes::PaletteStop<lw::Rgb8Color>, 2> sampleStops = {
        lw::colors::palettes::PaletteStop<lw::Rgb8Color>{0, lw::Rgb8Color(0, 0, 0)},
        lw::colors::palettes::PaletteStop<lw::Rgb8Color>{255, lw::Rgb8Color(255, 255, 255)}};
    lw::colors::palettes::Palette<lw::Rgb8Color> samplePaletteLike(
        lw::span<const lw::colors::palettes::PaletteStop<lw::Rgb8Color>>(sampleStops.data(), sampleStops.size()));
    lw::colors::palettes::Palette<lw::Rgb8Color> ownedPaletteLike(
        std::vector<lw::colors::palettes::PaletteStop<lw::Rgb8Color>>(sampleStops.begin(), sampleStops.end()));
    std::array<lw::Rgb8Color, 2> sampledOutput{};
    lw::IndexRange sampleIndexes(0, 128, sampledOutput.size());
    const size_t sampledCount = lw::colors::palettes::samplePalette(
        samplePaletteLike, sampleIndexes, lw::span<lw::Rgb8Color>(sampledOutput.data(), sampledOutput.size()), options);
    TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(sampledOutput.size()), static_cast<uint32_t>(sampledCount));

    const size_t ownedSampledCount = lw::colors::palettes::samplePalette(
        ownedPaletteLike, sampleIndexes, lw::span<lw::Rgb8Color>(sampledOutput.data(), sampledOutput.size()), options);
    TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(sampledOutput.size()), static_cast<uint32_t>(ownedSampledCount));

    lw::IndexRange nearestSampleIndexes(0, 128, sampledOutput.size());
    const size_t nearestSampledCount =
        lw::colors::palettes::samplePalette<lw::colors::palettes::BlendNearestContiguous<>,
                                            lw::colors::palettes::WrapClamp>(
            samplePaletteLike, nearestSampleIndexes,
            lw::span<lw::Rgb8Color>(sampledOutput.data(), sampledOutput.size()), options);
    TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(sampledOutput.size()), static_cast<uint32_t>(nearestSampledCount));

    lw::IndexRange midpointIndexes(0, 128, sampledOutput.size());
    const size_t midpointSampledCount = lw::colors::palettes::samplePalette<
        lw::colors::palettes::blend::Interpolated<lw::colors::palettes::blend::op::Midpoint>,
        lw::colors::palettes::WrapClamp>(samplePaletteLike, midpointIndexes,
                                         lw::span<lw::Rgb8Color>(sampledOutput.data(), sampledOutput.size()), options);
    TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(sampledOutput.size()), static_cast<uint32_t>(midpointSampledCount));

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
} // namespace

void setUp(void)
{
}

void tearDown(void)
{
}

int main(int, char**)
{
    UNITY_BEGIN();
    RUN_TEST(test_palette_first_pass_compile);
    return UNITY_END();
}
