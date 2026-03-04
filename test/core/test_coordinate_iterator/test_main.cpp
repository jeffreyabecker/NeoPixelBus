#include <unity.h>

#include "core/CoordinateIterator.h"
#include "colors/palette/Traits.h"

static_assert(lw::IsBeginEndRange<lw::CoordinateRange>::value,
              "CoordinateRange must satisfy IsBeginEndRange");

namespace
{
    void test_coordinate_iterator_iterates_row_major_inclusive_bounds(void)
    {
        lw::CoordinateIterator it(2, 3, 4, 4);
        const lw::CoordinateSentinel end{};

        TEST_ASSERT_FALSE(it == end);

        auto c = *it;
        TEST_ASSERT_EQUAL_size_t(2, c.X);
        TEST_ASSERT_EQUAL_size_t(3, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(3, c.X);
        TEST_ASSERT_EQUAL_size_t(3, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(4, c.X);
        TEST_ASSERT_EQUAL_size_t(3, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(2, c.X);
        TEST_ASSERT_EQUAL_size_t(4, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(3, c.X);
        TEST_ASSERT_EQUAL_size_t(4, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(4, c.X);
        TEST_ASSERT_EQUAL_size_t(4, c.Y);

        ++it;
        TEST_ASSERT_TRUE(it == end);
    }

    void test_coordinate_iterator_applies_step_x_and_step_y(void)
    {
        lw::CoordinateIterator it(0, 0, 4, 4, 2, 3);
        const lw::CoordinateSentinel end{};

        auto c = *it;
        TEST_ASSERT_EQUAL_size_t(0, c.X);
        TEST_ASSERT_EQUAL_size_t(0, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(2, c.X);
        TEST_ASSERT_EQUAL_size_t(0, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(4, c.X);
        TEST_ASSERT_EQUAL_size_t(0, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(0, c.X);
        TEST_ASSERT_EQUAL_size_t(3, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(2, c.X);
        TEST_ASSERT_EQUAL_size_t(3, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(4, c.X);
        TEST_ASSERT_EQUAL_size_t(3, c.Y);

        ++it;
        TEST_ASSERT_TRUE(it == end);
    }

    void test_coordinate_iterator_post_increment_returns_previous(void)
    {
        lw::CoordinateIterator it(5, 6, 6, 6);

        const auto before = it++;
        const auto c0 = *before;
        TEST_ASSERT_EQUAL_size_t(5, c0.X);
        TEST_ASSERT_EQUAL_size_t(6, c0.Y);

        const auto c1 = *it;
        TEST_ASSERT_EQUAL_size_t(6, c1.X);
        TEST_ASSERT_EQUAL_size_t(6, c1.Y);
    }

    void test_coordinate_iterator_empty_range_is_end(void)
    {
        lw::CoordinateIterator it(3, 2, 1, 4);
        const lw::CoordinateSentinel end{};

        TEST_ASSERT_TRUE(it == end);
    }

    void test_coordinate_range_from_bounds_iterates_row_major(void)
    {
        const lw::CoordinateRange range = lw::CoordinateRange::fromBounds(1, 2, 2, 3);
        auto it = range.begin();
        const auto end = range.end();

        auto c = *it;
        TEST_ASSERT_EQUAL_size_t(1, c.X);
        TEST_ASSERT_EQUAL_size_t(2, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(2, c.X);
        TEST_ASSERT_EQUAL_size_t(2, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(1, c.X);
        TEST_ASSERT_EQUAL_size_t(3, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(2, c.X);
        TEST_ASSERT_EQUAL_size_t(3, c.Y);

        ++it;
        TEST_ASSERT_TRUE(it == end);
    }

    void test_coordinate_range_from_bounds_step_applies_stride(void)
    {
        const lw::CoordinateRange range = lw::CoordinateRange::fromBoundsStep(0, 0, 4, 4, 2, 3);
        auto it = range.begin();
        const auto end = range.end();

        auto c = *it;
        TEST_ASSERT_EQUAL_size_t(0, c.X);
        TEST_ASSERT_EQUAL_size_t(0, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(2, c.X);
        TEST_ASSERT_EQUAL_size_t(0, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(4, c.X);
        TEST_ASSERT_EQUAL_size_t(0, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(0, c.X);
        TEST_ASSERT_EQUAL_size_t(3, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(2, c.X);
        TEST_ASSERT_EQUAL_size_t(3, c.Y);

        ++it;
        c = *it;
        TEST_ASSERT_EQUAL_size_t(4, c.X);
        TEST_ASSERT_EQUAL_size_t(3, c.Y);

        ++it;
        TEST_ASSERT_TRUE(it == end);
    }

    void test_coordinate_range_from_origin_size_empty_when_zero_dimension(void)
    {
        const lw::CoordinateRange range = lw::CoordinateRange::fromOriginSize(0, 4);
        const auto it = range.begin();
        const auto end = range.end();

        TEST_ASSERT_TRUE(it == end);
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
    RUN_TEST(test_coordinate_iterator_iterates_row_major_inclusive_bounds);
    RUN_TEST(test_coordinate_iterator_applies_step_x_and_step_y);
    RUN_TEST(test_coordinate_iterator_post_increment_returns_previous);
    RUN_TEST(test_coordinate_iterator_empty_range_is_end);
    RUN_TEST(test_coordinate_range_from_bounds_iterates_row_major);
    RUN_TEST(test_coordinate_range_from_bounds_step_applies_stride);
    RUN_TEST(test_coordinate_range_from_origin_size_empty_when_zero_dimension);
    return UNITY_END();
}