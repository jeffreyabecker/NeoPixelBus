#include <unity.h>

#include <array>
#include <vector>

#include "colors/Color.h"
#include "core/PixelView.h"

namespace
{
using Color = lw::Rgb8Color;

void test_slice_returns_expected_subsection(void)
{
    std::array<Color, 6> pixels = {Color{1, 2, 3},    Color{4, 5, 6},    Color{7, 8, 9},
                                   Color{10, 11, 12}, Color{13, 14, 15}, Color{16, 17, 18}};

    std::vector<lw::span<Color>> chunks;
    chunks.emplace_back(pixels.data(), 2);
    chunks.emplace_back(pixels.data() + 2, 4);

    lw::PixelView<Color> view{lw::span<lw::span<Color>>(chunks.data(), chunks.size())};
    auto sliced = view.slice(1, 5);

    TEST_ASSERT_EQUAL_UINT32(4u, sliced.size());
    TEST_ASSERT_EQUAL_UINT8(4u, sliced[0]['R']);
    TEST_ASSERT_EQUAL_UINT8(7u, sliced[1]['R']);
    TEST_ASSERT_EQUAL_UINT8(10u, sliced[2]['R']);
    TEST_ASSERT_EQUAL_UINT8(13u, sliced[3]['R']);
}

void test_slice_write_through_updates_underlying_storage(void)
{
    std::array<Color, 4> pixels = {Color{10, 0, 0}, Color{20, 0, 0}, Color{30, 0, 0}, Color{40, 0, 0}};

    std::vector<lw::span<Color>> chunks;
    chunks.emplace_back(pixels.data(), 1);
    chunks.emplace_back(pixels.data() + 1, 3);

    lw::PixelView<Color> view{lw::span<lw::span<Color>>(chunks.data(), chunks.size())};
    auto sliced = view.slice(1, 3);

    sliced[1]['R'] = 99;

    TEST_ASSERT_EQUAL_UINT8(99u, pixels[2]['R']);
}

void test_slice_clamps_to_bounds_and_handles_reverse_range(void)
{
    std::array<Color, 3> pixels = {Color{1, 0, 0}, Color{2, 0, 0}, Color{3, 0, 0}};

    std::vector<lw::span<Color>> chunks;
    chunks.emplace_back(pixels.data(), pixels.size());

    lw::PixelView<Color> view{lw::span<lw::span<Color>>(chunks.data(), chunks.size())};

    auto clamped = view.slice(2, 100);
    TEST_ASSERT_EQUAL_UINT32(1u, clamped.size());
    TEST_ASSERT_EQUAL_UINT8(3u, clamped[0]['R']);

    auto reversed = view.slice(2, 1);
    TEST_ASSERT_EQUAL_UINT32(0u, reversed.size());
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
    RUN_TEST(test_slice_returns_expected_subsection);
    RUN_TEST(test_slice_write_through_updates_underlying_storage);
    RUN_TEST(test_slice_clamps_to_bounds_and_handles_reverse_range);
    return UNITY_END();
}
