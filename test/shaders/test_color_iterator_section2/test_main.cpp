#include <unity.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <vector>

#include "colors/Color.h"
#include "colors/ColorIterator.h"

namespace
{
    using TestColor = npb::Rgbcw8Color;
    using TestIterator = npb::ColorIteratorT<TestColor>;

    bool try_advance_iterator(TestIterator &it, std::ptrdiff_t delta)
    {
        if (delta >= 0)
        {
            const uint32_t maxPosition = std::numeric_limits<uint16_t>::max();
            const uint32_t position = it.position();
            const uint32_t add = static_cast<uint32_t>(delta);
            if (add > (maxPosition - position))
            {
                return false;
            }

            it += delta;
            return true;
        }

        const uint32_t position = it.position();
        const uint32_t subtract = static_cast<uint32_t>(-delta);
        if (subtract > position)
        {
            return false;
        }

        it += delta;
        return true;
    }

    void test_2_1_1_increment_decrement_semantics(void)
    {
        std::array<TestColor, 6> buffer{
            TestColor{1, 0, 0, 0, 0},
            TestColor{2, 0, 0, 0, 0},
            TestColor{3, 0, 0, 0, 0},
            TestColor{4, 0, 0, 0, 0},
            TestColor{5, 0, 0, 0, 0},
            TestColor{6, 0, 0, 0, 0}};

        TestIterator it(
            [&](uint16_t idx) -> TestColor &
            { return buffer[idx]; },
            2);

        TestIterator postInc = it++;
        TEST_ASSERT_EQUAL_UINT16(2, postInc.position());
        TEST_ASSERT_EQUAL_UINT16(3, it.position());
        TEST_ASSERT_EQUAL_UINT8(3, (*postInc)['R']);
        TEST_ASSERT_EQUAL_UINT8(4, (*it)['R']);

        TestIterator &preInc = ++it;
        TEST_ASSERT_EQUAL_UINT16(4, it.position());
        TEST_ASSERT_EQUAL_UINT16(4, preInc.position());
        TEST_ASSERT_EQUAL_UINT8(5, (*it)['R']);

        TestIterator postDec = it--;
        TEST_ASSERT_EQUAL_UINT16(4, postDec.position());
        TEST_ASSERT_EQUAL_UINT16(3, it.position());
        TEST_ASSERT_EQUAL_UINT8(5, (*postDec)['R']);
        TEST_ASSERT_EQUAL_UINT8(4, (*it)['R']);

        TestIterator &preDec = --it;
        TEST_ASSERT_EQUAL_UINT16(2, it.position());
        TEST_ASSERT_EQUAL_UINT16(2, preDec.position());
        TEST_ASSERT_EQUAL_UINT8(3, (*it)['R']);
    }

    void test_2_1_2_arithmetic_and_distance_semantics(void)
    {
        std::array<TestColor, 8> buffer{};
        TestIterator begin(
            [&](uint16_t idx) -> TestColor &
            { return buffer[idx]; },
            0);
        TestIterator end(
            [&](uint16_t idx) -> TestColor &
            { return buffer[idx]; },
            static_cast<uint16_t>(buffer.size()));

        auto it = begin + 3;
        TEST_ASSERT_EQUAL_UINT16(3, it.position());

        it = 2 + it;
        TEST_ASSERT_EQUAL_UINT16(5, it.position());

        it -= 1;
        TEST_ASSERT_EQUAL_UINT16(4, it.position());

        it += 2;
        TEST_ASSERT_EQUAL_UINT16(6, it.position());

        auto shiftedBack = it - 4;
        TEST_ASSERT_EQUAL_UINT16(2, shiftedBack.position());

        TEST_ASSERT_EQUAL_INT32(8, static_cast<int32_t>(end - begin));
        TEST_ASSERT_EQUAL_INT32(4, static_cast<int32_t>(it - shiftedBack));
        TEST_ASSERT_EQUAL_INT32(-4, static_cast<int32_t>(shiftedBack - it));
    }

    void test_2_1_3_dereference_and_subscript_reference_semantics(void)
    {
        std::array<TestColor, 4> buffer{
            TestColor{1, 1, 1, 1, 1},
            TestColor{2, 2, 2, 2, 2},
            TestColor{3, 3, 3, 3, 3},
            TestColor{4, 4, 4, 4, 4}};

        TestIterator it(
            [&](uint16_t idx) -> TestColor &
            { return buffer[idx]; },
            1);

        (*it)['G'] = 77;
        it[2]['B'] = 88;

        TEST_ASSERT_EQUAL_UINT8(77, buffer[1]['G']);
        TEST_ASSERT_EQUAL_UINT8(88, buffer[3]['B']);
    }

    void test_2_1_4_comparison_semantics(void)
    {
        std::array<TestColor, 5> buffer{};

        TestIterator a(
            [&](uint16_t idx) -> TestColor &
            { return buffer[idx]; },
            1);
        TestIterator b(
            [&](uint16_t idx) -> TestColor &
            { return buffer[idx]; },
            3);
        TestIterator c(
            [&](uint16_t idx) -> TestColor &
            { return buffer[idx]; },
            1);

        TEST_ASSERT_TRUE(a == c);
        TEST_ASSERT_FALSE(a == b);
        TEST_ASSERT_TRUE(a < b);
        TEST_ASSERT_TRUE(a <= b);
        TEST_ASSERT_TRUE(b > a);
        TEST_ASSERT_TRUE(b >= a);
        TEST_ASSERT_FALSE(b < a);
    }

    void test_2_2_1_solid_color_source_range_length(void)
    {
        npb::SolidColorSourceT<TestColor> source{TestColor{9, 8, 7, 6, 5}, 37};
        const auto distance = source.end() - source.begin();
        TEST_ASSERT_EQUAL_INT32(37, static_cast<int32_t>(distance));
    }

    void test_2_2_2_solid_color_source_mutability_contract(void)
    {
        npb::SolidColorSourceT<TestColor> source{TestColor{10, 20, 30, 40, 50}, 5};

        auto it = source.begin();
        (*it)['W'] = 99;

        TEST_ASSERT_EQUAL_UINT8(99, source.color['W']);
    }

    void test_2_2_3_span_color_source_constructor_equivalence(void)
    {
        std::array<TestColor, 4> spanBufferA{
            TestColor{1, 2, 3, 4, 5},
            TestColor{6, 7, 8, 9, 10},
            TestColor{11, 12, 13, 14, 15},
            TestColor{16, 17, 18, 19, 20}};

        std::array<TestColor, 4> spanBufferB = spanBufferA;

        npb::SpanColorSourceT<TestColor> fromSpan{npb::span<TestColor>(spanBufferA)};
        npb::SpanColorSourceT<TestColor> fromPtr(spanBufferB.data(), spanBufferB.size());

        auto itSpan = fromSpan.begin();
        auto itPtr = fromPtr.begin();

        for (size_t idx = 0; idx < spanBufferA.size(); ++idx)
        {
            TEST_ASSERT_EQUAL_UINT8((*itSpan)['R'], (*itPtr)['R']);
            TEST_ASSERT_EQUAL_UINT8((*itSpan)['G'], (*itPtr)['G']);
            ++itSpan;
            ++itPtr;
        }

        fromSpan.begin()[2]['C'] = 111;
        fromPtr.begin()[2]['C'] = 111;

        TEST_ASSERT_EQUAL_UINT8(111, spanBufferA[2]['C']);
        TEST_ASSERT_EQUAL_UINT8(111, spanBufferB[2]['C']);
    }

    void test_2_2_4_stl_interop_with_std_copy(void)
    {
        {
            npb::SolidColorSourceT<TestColor> source{TestColor{4, 5, 6, 7, 8}, 3};
            std::array<TestColor, 3> destination{};

            std::copy(source.begin(), source.end(), destination.begin());

            for (const auto &entry : destination)
            {
                TEST_ASSERT_TRUE(entry == source.color);
            }
        }

        {
            std::array<TestColor, 3> sourceBuffer{
                TestColor{1, 1, 1, 1, 1},
                TestColor{2, 2, 2, 2, 2},
                TestColor{3, 3, 3, 3, 3}};

            npb::SpanColorSourceT<TestColor> source(sourceBuffer.data(), sourceBuffer.size());
            std::array<TestColor, 3> destination{};

            std::copy(source.begin(), source.end(), destination.begin());

            TEST_ASSERT_TRUE(destination[0] == sourceBuffer[0]);
            TEST_ASSERT_TRUE(destination[1] == sourceBuffer[1]);
            TEST_ASSERT_TRUE(destination[2] == sourceBuffer[2]);
        }
    }

    void test_2_3_1_p0_span_size_truncation(void)
    {
        constexpr size_t OversizeCount = static_cast<size_t>(std::numeric_limits<uint16_t>::max()) + 10U;
        std::vector<TestColor> oversized(OversizeCount);

        npb::SpanColorSourceT<TestColor> source(oversized.data(), oversized.size());

        const auto begin = source.begin();
        const auto end = source.end();
        const auto observedDistance = end - begin;

        TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(OversizeCount), end.position());
        TEST_ASSERT_EQUAL_INT32(static_cast<int32_t>(static_cast<uint16_t>(OversizeCount)),
                                static_cast<int32_t>(observedDistance));
        TEST_ASSERT_TRUE(static_cast<size_t>(observedDistance) < OversizeCount);
    }

    void test_2_3_2_p0_iterator_arithmetic_overflow_underflow_guarded(void)
    {
        std::array<TestColor, 1> buffer{TestColor{1, 2, 3, 4, 5}};

        TestIterator low(
            [&](uint16_t idx) -> TestColor &
            { return buffer[idx % buffer.size()]; },
            0);

        TestIterator high(
            [&](uint16_t idx) -> TestColor &
            { return buffer[idx % buffer.size()]; },
            std::numeric_limits<uint16_t>::max());

        TEST_ASSERT_FALSE(try_advance_iterator(low, -1));
        TEST_ASSERT_EQUAL_UINT16(0, low.position());

        TEST_ASSERT_FALSE(try_advance_iterator(high, +1));
        TEST_ASSERT_EQUAL_UINT16(std::numeric_limits<uint16_t>::max(), high.position());

        TEST_ASSERT_TRUE(try_advance_iterator(low, +1));
        TEST_ASSERT_EQUAL_UINT16(1, low.position());

        TEST_ASSERT_TRUE(try_advance_iterator(high, -1));
        TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(std::numeric_limits<uint16_t>::max() - 1), high.position());
    }

    void test_2_3_3_position_only_equality_caveat(void)
    {
        std::array<TestColor, 2> firstBuffer{
            TestColor{1, 0, 0, 0, 0},
            TestColor{2, 0, 0, 0, 0}};
        std::array<TestColor, 2> secondBuffer{
            TestColor{9, 0, 0, 0, 0},
            TestColor{8, 0, 0, 0, 0}};

        TestIterator first(
            [&](uint16_t idx) -> TestColor &
            { return firstBuffer[idx]; },
            1);

        TestIterator second(
            [&](uint16_t idx) -> TestColor &
            { return secondBuffer[idx]; },
            1);

        TEST_ASSERT_TRUE(first == second);
    }

    void test_2_3_4_default_constructed_iterator_contract(void)
    {
        TestIterator a;
        TestIterator b;

        TEST_ASSERT_TRUE(a == b);
        TEST_ASSERT_EQUAL_UINT16(0, a.position());
        TEST_ASSERT_EQUAL_UINT16(0, b.position());
    }
}

void setUp(void)
{
}

void tearDown(void)
{
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(test_2_1_1_increment_decrement_semantics);
    RUN_TEST(test_2_1_2_arithmetic_and_distance_semantics);
    RUN_TEST(test_2_1_3_dereference_and_subscript_reference_semantics);
    RUN_TEST(test_2_1_4_comparison_semantics);
    RUN_TEST(test_2_2_1_solid_color_source_range_length);
    RUN_TEST(test_2_2_2_solid_color_source_mutability_contract);
    RUN_TEST(test_2_2_3_span_color_source_constructor_equivalence);
    RUN_TEST(test_2_2_4_stl_interop_with_std_copy);
    RUN_TEST(test_2_3_1_p0_span_size_truncation);
    RUN_TEST(test_2_3_2_p0_iterator_arithmetic_overflow_underflow_guarded);
    RUN_TEST(test_2_3_3_position_only_equality_caveat);
    RUN_TEST(test_2_3_4_default_constructed_iterator_contract);
    return UNITY_END();
}
