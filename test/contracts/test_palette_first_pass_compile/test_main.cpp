#include <unity.h>

#include <array>
#include <type_traits>

#include "colors/Palette.h"
#include "colors/PaletteCodec.h"

namespace
{
    void test_palette_first_pass_compile(void)
    {
        static_assert(lw::ColorType<lw::Rgb8Color>, "Rgb8Color must satisfy ColorType");
        static_assert(std::is_enum<lw::PaletteBlendMode>::value, "PaletteBlendMode must be enum");
        static_assert(std::is_enum<lw::PaletteWrapMode>::value, "PaletteWrapMode must be enum");

        lw::PaletteSampleOptions<lw::Rgb8Color> options;
        options.wrapMode = lw::PaletteWrapMode::Clamp;
        options.blendMode = lw::PaletteBlendMode::Linear;

        lw::PaletteStop<lw::Rgb8Color> stop{};
        stop.index = 0;
        stop.color = lw::Rgb8Color(1, 2, 3);

        lw::Palette<lw::Rgb8Color> palette(lw::span<const lw::PaletteStop<lw::Rgb8Color>>(&stop, 1));

        size_t bytesWritten = 0;
        std::array<uint8_t, 64> buffer{};
        const auto encodeErr = lw::encodePaletteBinary(palette, lw::span<uint8_t>(buffer.data(), buffer.size()), bytesWritten);
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::PaletteCodecError::InvalidStopCount), static_cast<int>(encodeErr));
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
    RUN_TEST(test_palette_first_pass_compile);
    return UNITY_END();
}
