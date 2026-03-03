#include <unity.h>

#include "colors/Palette.h"

namespace
{
    void test_7_1_1_map_position_clamp_behaviour(void)
    {
        TEST_ASSERT_EQUAL_UINT8(0, lw::mapPositionToPaletteIndex(0, 5, lw::PaletteWrapMode::Clamp));
        TEST_ASSERT_EQUAL_UINT8(127, lw::mapPositionToPaletteIndex(2, 5, lw::PaletteWrapMode::Clamp));
        TEST_ASSERT_EQUAL_UINT8(255, lw::mapPositionToPaletteIndex(4, 5, lw::PaletteWrapMode::Clamp));
        TEST_ASSERT_EQUAL_UINT8(255, lw::mapPositionToPaletteIndex(99, 5, lw::PaletteWrapMode::Clamp));
    }

    void test_7_1_2_map_position_wrap_behaviour(void)
    {
        TEST_ASSERT_EQUAL_UINT8(0, lw::mapPositionToPaletteIndex(0, 5, lw::PaletteWrapMode::Wrap));
        TEST_ASSERT_EQUAL_UINT8(204, lw::mapPositionToPaletteIndex(4, 5, lw::PaletteWrapMode::Wrap));
        TEST_ASSERT_EQUAL_UINT8(0, lw::mapPositionToPaletteIndex(5, 5, lw::PaletteWrapMode::Wrap));
        TEST_ASSERT_EQUAL_UINT8(153, lw::mapPositionToPaletteIndex(8, 5, lw::PaletteWrapMode::Wrap));
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
    RUN_TEST(test_7_1_1_map_position_clamp_behaviour);
    RUN_TEST(test_7_1_2_map_position_wrap_behaviour);
    return UNITY_END();
}
