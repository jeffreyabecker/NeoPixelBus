#include <unity.h>

#include "colors/palette/Palette.h"

namespace
{
void test_7_1_1_map_position_clamp_behaviour(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, lw::colors::palettes::WrapClamp::mapPositionToPaletteIndex(0, 5));
    TEST_ASSERT_EQUAL_UINT8(127, lw::colors::palettes::WrapClamp::mapPositionToPaletteIndex(2, 5));
    TEST_ASSERT_EQUAL_UINT8(255, lw::colors::palettes::WrapClamp::mapPositionToPaletteIndex(4, 5));
    TEST_ASSERT_EQUAL_UINT8(255, lw::colors::palettes::WrapClamp::mapPositionToPaletteIndex(99, 5));
}

void test_7_1_2_map_position_wrap_behaviour(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, lw::colors::palettes::WrapCircular::mapPositionToPaletteIndex(0, 5));
    TEST_ASSERT_EQUAL_UINT8(204, lw::colors::palettes::WrapCircular::mapPositionToPaletteIndex(4, 5));
    TEST_ASSERT_EQUAL_UINT8(0, lw::colors::palettes::WrapCircular::mapPositionToPaletteIndex(5, 5));
    TEST_ASSERT_EQUAL_UINT8(153, lw::colors::palettes::WrapCircular::mapPositionToPaletteIndex(8, 5));
}
} // namespace

void setUp(void)
{
}

void tearDown(void)
{
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(test_7_1_1_map_position_clamp_behaviour);
    RUN_TEST(test_7_1_2_map_position_wrap_behaviour);
    return UNITY_END();
}
