#include <unity.h>

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>

#include "virtual/colors/Color.h"

namespace
{
    template <typename TColor>
    void assert_all_channels_zero(const TColor &color)
    {
        for (size_t idx = 0; idx < TColor::ChannelCount; ++idx)
        {
            TEST_ASSERT_EQUAL_UINT32(0U, static_cast<uint32_t>(color[idx]));
        }
    }

    template <typename TColor>
    void assert_prefix_tail(const TColor &color,
                            const typename TColor::ComponentType *prefix,
                            size_t prefixSize)
    {
        for (size_t idx = 0; idx < prefixSize; ++idx)
        {
            TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(prefix[idx]), static_cast<uint32_t>(color[idx]));
        }

        for (size_t idx = prefixSize; idx < TColor::ChannelCount; ++idx)
        {
            TEST_ASSERT_EQUAL_UINT32(0U, static_cast<uint32_t>(color[idx]));
        }
    }

    template <typename TColor>
    bool try_write_channel(TColor &color, size_t idx, typename TColor::ComponentType value)
    {
        if (idx >= TColor::ChannelCount)
        {
            return false;
        }

        color[idx] = value;
        return true;
    }

    template <typename TColor>
    bool try_read_channel(const TColor &color, size_t idx, typename TColor::ComponentType &value)
    {
        if (idx >= TColor::ChannelCount)
        {
            return false;
        }

        value = color[idx];
        return true;
    }

    void test_1_1_1_default_construction_zero_initialization(void)
    {
        assert_all_channels_zero(npb::Rgb8Color{});
        assert_all_channels_zero(npb::Rgbw8Color{});
        assert_all_channels_zero(npb::Rgbcw8Color{});
        assert_all_channels_zero(npb::Rgb16Color{});
        assert_all_channels_zero(npb::Rgbw16Color{});
        assert_all_channels_zero(npb::Rgbcw16Color{});
    }

    void test_1_1_2_variadic_constructor_prefix_assignment(void)
    {
        {
            const npb::Rgb8Color color(7, 9);
            const uint8_t expected[] = {7, 9};
            assert_prefix_tail(color, expected, 2);
        }

        {
            const npb::Rgbw8Color color(1, 2, 3);
            const uint8_t expected[] = {1, 2, 3};
            assert_prefix_tail(color, expected, 3);
        }

        {
            const npb::Rgbcw8Color color(5, 6, 7, 8);
            const uint8_t expected[] = {5, 6, 7, 8};
            assert_prefix_tail(color, expected, 4);
        }

        {
            const npb::Rgb16Color color(111, 222);
            const uint16_t expected[] = {111, 222};
            assert_prefix_tail(color, expected, 2);
        }

        {
            const npb::Rgbw16Color color(1000, 2000, 3000);
            const uint16_t expected[] = {1000, 2000, 3000};
            assert_prefix_tail(color, expected, 3);
        }

        {
            const npb::Rgbcw16Color color(10, 20, 30, 40, 50);
            const uint16_t expected[] = {10, 20, 30, 40, 50};
            assert_prefix_tail(color, expected, 5);
        }
    }

    void test_1_1_3_component_type_and_channel_metadata(void)
    {
        static_assert(npb::Rgb8Color::ChannelCount == 3);
        static_assert(npb::Rgbw8Color::ChannelCount == 4);
        static_assert(npb::Rgbcw8Color::ChannelCount == 5);
        static_assert(npb::Rgb16Color::ChannelCount == 3);
        static_assert(npb::Rgbw16Color::ChannelCount == 4);
        static_assert(npb::Rgbcw16Color::ChannelCount == 5);

        static_assert(std::is_same_v<npb::Rgb8Color::ComponentType, uint8_t>);
        static_assert(std::is_same_v<npb::Rgbw8Color::ComponentType, uint8_t>);
        static_assert(std::is_same_v<npb::Rgbcw8Color::ComponentType, uint8_t>);
        static_assert(std::is_same_v<npb::Rgb16Color::ComponentType, uint16_t>);
        static_assert(std::is_same_v<npb::Rgbw16Color::ComponentType, uint16_t>);
        static_assert(std::is_same_v<npb::Rgbcw16Color::ComponentType, uint16_t>);

        TEST_ASSERT_EQUAL_UINT8(std::numeric_limits<uint8_t>::max(), npb::Rgb8Color::MaxComponent);
        TEST_ASSERT_EQUAL_UINT8(std::numeric_limits<uint8_t>::max(), npb::Rgbw8Color::MaxComponent);
        TEST_ASSERT_EQUAL_UINT8(std::numeric_limits<uint8_t>::max(), npb::Rgbcw8Color::MaxComponent);
        TEST_ASSERT_EQUAL_UINT16(std::numeric_limits<uint16_t>::max(), npb::Rgb16Color::MaxComponent);
        TEST_ASSERT_EQUAL_UINT16(std::numeric_limits<uint16_t>::max(), npb::Rgbw16Color::MaxComponent);
        TEST_ASSERT_EQUAL_UINT16(std::numeric_limits<uint16_t>::max(), npb::Rgbcw16Color::MaxComponent);
    }

    void test_1_2_1_integral_index_read_write_round_trip(void)
    {
        npb::Rgbcw8Color color{};

        color[0] = 11;
        color[1] = 22;
        color[2] = 33;
        color[3] = 44;
        color[4] = 55;

        TEST_ASSERT_EQUAL_UINT8(11, color[0]);
        TEST_ASSERT_EQUAL_UINT8(22, color[1]);
        TEST_ASSERT_EQUAL_UINT8(33, color[2]);
        TEST_ASSERT_EQUAL_UINT8(44, color[3]);
        TEST_ASSERT_EQUAL_UINT8(55, color[4]);

        const npb::Rgbcw8Color &constView = color;
        TEST_ASSERT_EQUAL_UINT8(11, constView[0]);
        TEST_ASSERT_EQUAL_UINT8(22, constView[1]);
        TEST_ASSERT_EQUAL_UINT8(33, constView[2]);
        TEST_ASSERT_EQUAL_UINT8(44, constView[3]);
        TEST_ASSERT_EQUAL_UINT8(55, constView[4]);
    }

    void test_1_2_2_character_index_mapping_upper_lower_case(void)
    {
        npb::Rgbcw8Color color{1, 2, 3, 4, 5};

        TEST_ASSERT_EQUAL_UINT8(1, color['R']);
        TEST_ASSERT_EQUAL_UINT8(2, color['G']);
        TEST_ASSERT_EQUAL_UINT8(3, color['B']);
        TEST_ASSERT_EQUAL_UINT8(4, color['W']);
        TEST_ASSERT_EQUAL_UINT8(5, color['C']);

        TEST_ASSERT_EQUAL_UINT8(1, color['r']);
        TEST_ASSERT_EQUAL_UINT8(2, color['g']);
        TEST_ASSERT_EQUAL_UINT8(3, color['b']);
        TEST_ASSERT_EQUAL_UINT8(4, color['w']);
        TEST_ASSERT_EQUAL_UINT8(5, color['c']);
    }

    void test_1_2_3_unknown_channel_fallback_behavior(void)
    {
        npb::Rgbcw8Color color{77, 2, 3, 4, 5};

        TEST_ASSERT_EQUAL_UINT8(77, color['X']);
        TEST_ASSERT_EQUAL_UINT8(77, color['?']);
    }

    void test_1_2_4_wc_fallback_on_lower_channel_colors(void)
    {
        npb::Rgb8Color rgb{10, 20, 30};
        npb::Rgbw8Color rgbw{40, 50, 60, 70};

        TEST_ASSERT_EQUAL_UINT8(10, rgb['W']);
        TEST_ASSERT_EQUAL_UINT8(10, rgb['C']);

        TEST_ASSERT_EQUAL_UINT8(70, rgbw['W']);
        TEST_ASSERT_EQUAL_UINT8(40, rgbw['C']);
    }

    void test_1_3_1_equality_operator_correctness(void)
    {
        const npb::Rgbcw8Color lhs{1, 2, 3, 4, 5};
        const npb::Rgbcw8Color equal{1, 2, 3, 4, 5};
        const npb::Rgbcw8Color different{1, 2, 3, 9, 5};

        TEST_ASSERT_TRUE(lhs == equal);
        TEST_ASSERT_FALSE(lhs == different);
    }

    void test_1_3_2_channel_order_string_length_consistency(void)
    {
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::ChannelOrder::LengthRGB),
                                 static_cast<uint32_t>(std::char_traits<char>::length(npb::ChannelOrder::RGB)));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::ChannelOrder::LengthGRB),
                                 static_cast<uint32_t>(std::char_traits<char>::length(npb::ChannelOrder::GRB)));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::ChannelOrder::LengthBGR),
                                 static_cast<uint32_t>(std::char_traits<char>::length(npb::ChannelOrder::BGR)));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::ChannelOrder::LengthRGBW),
                                 static_cast<uint32_t>(std::char_traits<char>::length(npb::ChannelOrder::RGBW)));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::ChannelOrder::LengthGRBW),
                                 static_cast<uint32_t>(std::char_traits<char>::length(npb::ChannelOrder::GRBW)));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::ChannelOrder::LengthBGRW),
                                 static_cast<uint32_t>(std::char_traits<char>::length(npb::ChannelOrder::BGRW)));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::ChannelOrder::LengthRGBCW),
                                 static_cast<uint32_t>(std::char_traits<char>::length(npb::ChannelOrder::RGBCW)));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::ChannelOrder::LengthGRBCW),
                                 static_cast<uint32_t>(std::char_traits<char>::length(npb::ChannelOrder::GRBCW)));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::ChannelOrder::LengthBGRCW),
                                 static_cast<uint32_t>(std::char_traits<char>::length(npb::ChannelOrder::BGRCW)));
    }

    void test_1_4_1_widen_conversion_formula(void)
    {
        const npb::Rgbcw8Color src{0x00, 0x01, 0x7F, 0x80, 0xFF};
        const auto widened = npb::widen(src);

        TEST_ASSERT_EQUAL_UINT16(0x0000, widened[0]);
        TEST_ASSERT_EQUAL_UINT16(0x0101, widened[1]);
        TEST_ASSERT_EQUAL_UINT16(0x7F7F, widened[2]);
        TEST_ASSERT_EQUAL_UINT16(0x8080, widened[3]);
        TEST_ASSERT_EQUAL_UINT16(0xFFFF, widened[4]);
    }

    void test_1_4_2_narrow_conversion_formula(void)
    {
        const npb::Rgbcw16Color src{0x0000, 0x01FF, 0x7F00, 0x80AA, 0xFFFF};
        const auto narrowed = npb::narrow(src);

        TEST_ASSERT_EQUAL_UINT8(0x00, narrowed[0]);
        TEST_ASSERT_EQUAL_UINT8(0x01, narrowed[1]);
        TEST_ASSERT_EQUAL_UINT8(0x7F, narrowed[2]);
        TEST_ASSERT_EQUAL_UINT8(0x80, narrowed[3]);
        TEST_ASSERT_EQUAL_UINT8(0xFF, narrowed[4]);
    }

    void test_1_4_3_expand_ordering_and_zero_fill(void)
    {
        const npb::Rgb8Color src{9, 8, 7};
        const auto expanded = npb::expand<5>(src);

        TEST_ASSERT_EQUAL_UINT8(9, expanded[0]);
        TEST_ASSERT_EQUAL_UINT8(8, expanded[1]);
        TEST_ASSERT_EQUAL_UINT8(7, expanded[2]);
        TEST_ASSERT_EQUAL_UINT8(0, expanded[3]);
        TEST_ASSERT_EQUAL_UINT8(0, expanded[4]);
    }

    void test_1_4_4_compress_ordering(void)
    {
        const npb::Rgbcw16Color src{11, 22, 33, 44, 55};
        const auto compressed = npb::compress<3>(src);

        TEST_ASSERT_EQUAL_UINT16(11, compressed[0]);
        TEST_ASSERT_EQUAL_UINT16(22, compressed[1]);
        TEST_ASSERT_EQUAL_UINT16(33, compressed[2]);
    }

    void test_1_5_1_p0_out_of_range_integral_index_use_guarded(void)
    {
        npb::Rgb8Color color{10, 20, 30};
        uint8_t readValue = 0;

        TEST_ASSERT_TRUE(try_write_channel(color, 1, 99));
        TEST_ASSERT_TRUE(try_read_channel(color, 1, readValue));
        TEST_ASSERT_EQUAL_UINT8(99, readValue);

        TEST_ASSERT_FALSE(try_write_channel(color, 3, 77));
        TEST_ASSERT_FALSE(try_read_channel(color, 3, readValue));

        TEST_ASSERT_EQUAL_UINT8(10, color[0]);
        TEST_ASSERT_EQUAL_UINT8(99, color[1]);
        TEST_ASSERT_EQUAL_UINT8(30, color[2]);
    }

    void test_1_5_2_boundary_stress_for_conversion_helpers(void)
    {
        {
            const npb::Rgb8Color srcMin{0x00, 0x00, 0x00};
            const auto widenedMin = npb::widen(srcMin);
            TEST_ASSERT_EQUAL_UINT16(0x0000, widenedMin[0]);
            TEST_ASSERT_EQUAL_UINT16(0x0000, widenedMin[1]);
            TEST_ASSERT_EQUAL_UINT16(0x0000, widenedMin[2]);
        }

        {
            const npb::Rgb8Color srcMax{0xFF, 0xFF, 0xFF};
            const auto widenedMax = npb::widen(srcMax);
            TEST_ASSERT_EQUAL_UINT16(0xFFFF, widenedMax[0]);
            TEST_ASSERT_EQUAL_UINT16(0xFFFF, widenedMax[1]);
            TEST_ASSERT_EQUAL_UINT16(0xFFFF, widenedMax[2]);
        }

        {
            const npb::Rgb16Color srcMin{0x0000, 0x0000, 0x0000};
            const auto narrowedMin = npb::narrow(srcMin);
            TEST_ASSERT_EQUAL_UINT8(0x00, narrowedMin[0]);
            TEST_ASSERT_EQUAL_UINT8(0x00, narrowedMin[1]);
            TEST_ASSERT_EQUAL_UINT8(0x00, narrowedMin[2]);
        }

        {
            const npb::Rgb16Color srcMax{0xFFFF, 0xFFFF, 0xFFFF};
            const auto narrowedMax = npb::narrow(srcMax);
            TEST_ASSERT_EQUAL_UINT8(0xFF, narrowedMax[0]);
            TEST_ASSERT_EQUAL_UINT8(0xFF, narrowedMax[1]);
            TEST_ASSERT_EQUAL_UINT8(0xFF, narrowedMax[2]);
        }

        {
            const npb::Rgb8Color src{0xAA, 0x00, 0xFF};
            const auto expanded = npb::expand<5>(src);
            TEST_ASSERT_EQUAL_UINT8(0xAA, expanded[0]);
            TEST_ASSERT_EQUAL_UINT8(0x00, expanded[1]);
            TEST_ASSERT_EQUAL_UINT8(0xFF, expanded[2]);
            TEST_ASSERT_EQUAL_UINT8(0x00, expanded[3]);
            TEST_ASSERT_EQUAL_UINT8(0x00, expanded[4]);
        }

        {
            const npb::Rgbcw8Color src{0x12, 0x34, 0x56, 0x78, 0x9A};
            const auto compressed = npb::compress<3>(src);
            TEST_ASSERT_EQUAL_UINT8(0x12, compressed[0]);
            TEST_ASSERT_EQUAL_UINT8(0x34, compressed[1]);
            TEST_ASSERT_EQUAL_UINT8(0x56, compressed[2]);
        }
    }

    void test_1_6_1_parse_hex_rgbcw8_with_hash_prefix(void)
    {
        const auto parsed = npb::Rgbcw8Color::parseHex("#0102030405");

        TEST_ASSERT_EQUAL_UINT8(0x01, parsed[0]);
        TEST_ASSERT_EQUAL_UINT8(0x02, parsed[1]);
        TEST_ASSERT_EQUAL_UINT8(0x03, parsed[2]);
        TEST_ASSERT_EQUAL_UINT8(0x05, parsed[3]);
        TEST_ASSERT_EQUAL_UINT8(0x04, parsed[4]);
    }

    void test_1_6_2_parse_hex_rgbcw16_with_0x_prefix(void)
    {
        const auto parsed = npb::Rgbcw16Color::parseHex("0x00010002000300040005");

        TEST_ASSERT_EQUAL_UINT16(0x0001, parsed[0]);
        TEST_ASSERT_EQUAL_UINT16(0x0002, parsed[1]);
        TEST_ASSERT_EQUAL_UINT16(0x0003, parsed[2]);
        TEST_ASSERT_EQUAL_UINT16(0x0005, parsed[3]);
        TEST_ASSERT_EQUAL_UINT16(0x0004, parsed[4]);
    }

    void test_1_6_3_parse_hex_invalid_input_returns_zero(void)
    {
        const auto parsed = npb::Rgbcw8Color::parseHex("#GG");
        assert_all_channels_zero(parsed);
    }

    void test_1_6_4_parse_hex_custom_color_order_rgb8(void)
    {
        const auto parsed = npb::Rgb8Color::parseHex("010203", npb::ChannelOrder::GRB);

        TEST_ASSERT_EQUAL_UINT8(0x02, parsed[0]);
        TEST_ASSERT_EQUAL_UINT8(0x01, parsed[1]);
        TEST_ASSERT_EQUAL_UINT8(0x03, parsed[2]);
    }

    void test_1_6_5_parse_hex_default_order_rgbw(void)
    {
        const auto parsed = npb::Rgbw8Color::parseHex("01020304", nullptr);

        TEST_ASSERT_EQUAL_UINT8(0x01, parsed[0]);
        TEST_ASSERT_EQUAL_UINT8(0x02, parsed[1]);
        TEST_ASSERT_EQUAL_UINT8(0x03, parsed[2]);
        TEST_ASSERT_EQUAL_UINT8(0x04, parsed[3]);
    }

    void test_1_6_6_fill_hex_default_order_rgbcw8(void)
    {
        const npb::Rgbcw8Color color(0x11, 0x22, 0x33, 0x44, 0x55);
        std::array<uint8_t, 32> buffer{};

        color.fillHex(std::span<uint8_t>(buffer.data(), buffer.size()));

        TEST_ASSERT_EQUAL_STRING("1122335544", reinterpret_cast<const char *>(buffer.data()));
    }

    void test_1_6_7_fill_hex_custom_order_and_prefix(void)
    {
        const npb::Rgbw8Color color(0x11, 0x22, 0x33, 0x44);
        std::array<uint8_t, 32> buffer{};

        color.fillHex(std::span<uint8_t>(buffer.data(), buffer.size()), npb::ChannelOrder::GRBW, "#");

        TEST_ASSERT_EQUAL_STRING("#22113344", reinterpret_cast<const char *>(buffer.data()));
    }

    void test_1_6_8_fill_hex_round_trip_parse_rgb16(void)
    {
        const npb::Rgbcw16Color source(0x1111, 0x2222, 0x3333, 0x4444, 0x5555);
        std::array<uint8_t, 64> buffer{};

        source.fillHex(std::span<uint8_t>(buffer.data(), buffer.size()), npb::ChannelOrder::RGBCW, "0x");

        const auto parsed = npb::Rgbcw16Color::parseHex(reinterpret_cast<const char *>(buffer.data()), npb::ChannelOrder::RGBCW);
        TEST_ASSERT_TRUE(parsed == source);
    }

    void test_1_6_9_fill_hex_short_buffer_stays_bounded(void)
    {
        const npb::Rgb8Color color(0xAA, 0xBB, 0xCC);
        std::array<uint8_t, 4> buffer{};

        color.fillHex(std::span<uint8_t>(buffer.data(), buffer.size()), nullptr, "#");

        TEST_ASSERT_EQUAL_UINT8('#', buffer[0]);
        TEST_ASSERT_EQUAL_UINT8('A', buffer[1]);
        TEST_ASSERT_EQUAL_UINT8('A', buffer[2]);
        TEST_ASSERT_EQUAL_UINT8('B', buffer[3]);
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
    RUN_TEST(test_1_1_1_default_construction_zero_initialization);
    RUN_TEST(test_1_1_2_variadic_constructor_prefix_assignment);
    RUN_TEST(test_1_1_3_component_type_and_channel_metadata);
    RUN_TEST(test_1_2_1_integral_index_read_write_round_trip);
    RUN_TEST(test_1_2_2_character_index_mapping_upper_lower_case);
    RUN_TEST(test_1_2_3_unknown_channel_fallback_behavior);
    RUN_TEST(test_1_2_4_wc_fallback_on_lower_channel_colors);
    RUN_TEST(test_1_3_1_equality_operator_correctness);
    RUN_TEST(test_1_3_2_channel_order_string_length_consistency);
    RUN_TEST(test_1_4_1_widen_conversion_formula);
    RUN_TEST(test_1_4_2_narrow_conversion_formula);
    RUN_TEST(test_1_4_3_expand_ordering_and_zero_fill);
    RUN_TEST(test_1_4_4_compress_ordering);
    RUN_TEST(test_1_5_1_p0_out_of_range_integral_index_use_guarded);
    RUN_TEST(test_1_5_2_boundary_stress_for_conversion_helpers);
    RUN_TEST(test_1_6_1_parse_hex_rgbcw8_with_hash_prefix);
    RUN_TEST(test_1_6_2_parse_hex_rgbcw16_with_0x_prefix);
    RUN_TEST(test_1_6_3_parse_hex_invalid_input_returns_zero);
    RUN_TEST(test_1_6_4_parse_hex_custom_color_order_rgb8);
    RUN_TEST(test_1_6_5_parse_hex_default_order_rgbw);
    RUN_TEST(test_1_6_6_fill_hex_default_order_rgbcw8);
    RUN_TEST(test_1_6_7_fill_hex_custom_order_and_prefix);
    RUN_TEST(test_1_6_8_fill_hex_round_trip_parse_rgb16);
    RUN_TEST(test_1_6_9_fill_hex_short_buffer_stays_bounded);
    return UNITY_END();
}