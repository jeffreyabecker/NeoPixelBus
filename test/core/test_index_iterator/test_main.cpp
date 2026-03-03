#include <unity.h>

#include "core/IndexIterator.h"

namespace
{
    void test_index_iterator_progresses_with_step(void)
    {
        lw::IndexIterator it(10, 5, 3);
        const lw::IndexSentinel end{};

        TEST_ASSERT_FALSE(it == end);
        TEST_ASSERT_EQUAL_UINT8(10, *it);
        ++it;
        TEST_ASSERT_EQUAL_UINT8(15, *it);
        ++it;
        TEST_ASSERT_EQUAL_UINT8(20, *it);
        ++it;
        TEST_ASSERT_TRUE(it == end);
    }

    void test_index_iterator_wraps_uint8_sequence(void)
    {
        lw::IndexIterator it(250, 10, 3);
        const lw::IndexSentinel end{};

        TEST_ASSERT_EQUAL_UINT8(250, *it);
        ++it;
        TEST_ASSERT_EQUAL_UINT8(4, *it);
        ++it;
        TEST_ASSERT_EQUAL_UINT8(14, *it);
        ++it;
        TEST_ASSERT_TRUE(it == end);
    }

    void test_index_iterator_post_increment_returns_previous(void)
    {
        lw::IndexIterator it(1, 2, 2);

        const auto before = it++;
        TEST_ASSERT_EQUAL_UINT8(1, *before);
        TEST_ASSERT_EQUAL_UINT8(3, *it);
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
    RUN_TEST(test_index_iterator_progresses_with_step);
    RUN_TEST(test_index_iterator_wraps_uint8_sequence);
    RUN_TEST(test_index_iterator_post_increment_returns_previous);
    return UNITY_END();
}
