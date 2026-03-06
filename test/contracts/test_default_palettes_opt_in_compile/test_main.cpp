#include <unity.h>

#include <LumaWave.h>
#include <LumaWeave/DefaultPalettes.h>

namespace
{
void test_default_palettes_opt_in_compile(void)
{
    using namespace lw::palettes::wled;

    const auto party = Party<>();
    TEST_ASSERT_FALSE(party.empty());
    TEST_ASSERT_EQUAL_UINT32(16u, static_cast<uint32_t>(party.size()));

    const lw::Rgb8Color primary(10, 20, 30);
    const lw::Rgb8Color secondary(40, 50, 60);
    const lw::Rgb8Color tertiary(70, 80, 90);

    const auto color1 = Color1(primary);
    const auto split = Colors1And2(primary, secondary);
    const auto gradient = ColorGradient(primary, secondary, tertiary);
    const auto tri = ColorsOnly(primary, secondary, tertiary);

    TEST_ASSERT_EQUAL_UINT32(2u, static_cast<uint32_t>(color1.size()));
    TEST_ASSERT_EQUAL_UINT32(4u, static_cast<uint32_t>(split.size()));
    TEST_ASSERT_EQUAL_UINT32(3u, static_cast<uint32_t>(gradient.size()));
    TEST_ASSERT_EQUAL_UINT32(16u, static_cast<uint32_t>(tri.size()));

    const auto staticPalettes = StaticPalettes<>();
    TEST_ASSERT_TRUE(staticPalettes.size() >= 68u);
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
    RUN_TEST(test_default_palettes_opt_in_compile);
    return UNITY_END();
}
