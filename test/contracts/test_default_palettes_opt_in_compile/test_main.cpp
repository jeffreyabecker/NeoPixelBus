#include <unity.h>

#include <LumaWave.h>
#include <LumaWave/DefaultPalettes.h>

namespace
{
void test_default_palettes_opt_in_compile(void)
{
    using namespace lw::palettes;

    const auto staticPalettes = StaticPalettes<>();
    TEST_ASSERT_TRUE(staticPalettes.size() >= 68u);

    const auto party = staticPalettes.front().create();
    TEST_ASSERT_FALSE(party.stops().empty());
    TEST_ASSERT_EQUAL_UINT32(16u, static_cast<uint32_t>(party.stops().size()));

    const lw::Rgb8Color primary(10, 20, 30);
    const lw::Rgb8Color secondary(40, 50, 60);
    const lw::Rgb8Color tertiary(70, 80, 90);

    using PaletteType = lw::colors::palettes::Palette<lw::Rgb8Color>;
    using Stop = lw::colors::palettes::PaletteStop<lw::Rgb8Color>;

    const std::array<Stop, 2> color1Stops = {
        Stop{0, primary},
        Stop{255, primary},
    };
    const std::array<Stop, 4> splitStops = {
        Stop{0, primary},
        Stop{127, primary},
        Stop{128, secondary},
        Stop{255, secondary},
    };
    const std::array<Stop, 3> gradientStops = {
        Stop{0, tertiary},
        Stop{127, secondary},
        Stop{255, primary},
    };
    const std::array<Stop, 16> triStops = {
        Stop{0, primary},     Stop{16, primary},    Stop{32, primary},   Stop{48, primary},
        Stop{64, primary},    Stop{80, secondary},  Stop{96, secondary}, Stop{112, secondary},
        Stop{128, secondary}, Stop{144, secondary}, Stop{160, tertiary}, Stop{176, tertiary},
        Stop{192, tertiary},  Stop{208, tertiary},  Stop{224, tertiary}, Stop{255, primary},
    };

    const PaletteType color1(color1Stops);
    const PaletteType split(splitStops);
    const PaletteType gradient(gradientStops);
    const PaletteType tri(triStops);

    TEST_ASSERT_EQUAL_UINT32(2u, static_cast<uint32_t>(color1.stops().size()));
    TEST_ASSERT_EQUAL_UINT32(4u, static_cast<uint32_t>(split.stops().size()));
    TEST_ASSERT_EQUAL_UINT32(3u, static_cast<uint32_t>(gradient.stops().size()));
    TEST_ASSERT_EQUAL_UINT32(16u, static_cast<uint32_t>(tri.stops().size()));
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
