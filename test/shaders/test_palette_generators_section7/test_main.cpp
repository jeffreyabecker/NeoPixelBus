#include <unity.h>

#include <array>

#include "colors/palette/Palette.h"

namespace
{
    using Stop = lw::PaletteStop<lw::Rgb8Color>;

    void test_rainbow_generator_stop_shape_and_update(void)
    {
        lw::RainbowPaletteGenerator<lw::Rgb8Color, 8> rainbow;
        const auto before = rainbow.stops();

        TEST_ASSERT_EQUAL_UINT32(8, static_cast<uint32_t>(before.size()));
        TEST_ASSERT_EQUAL_UINT8(0, before[0].index);
        TEST_ASSERT_EQUAL_UINT8(255, before[7].index);

        const lw::Rgb8Color firstBefore = before[0].color;
        rainbow.update(16);
        const auto after = rainbow.stops();

        TEST_ASSERT_TRUE(!(after[0].color == firstBefore));
    }

    void test_random_smooth_generator_is_deterministic(void)
    {
        lw::RandomSmoothPaletteGenerator<lw::Rgb8Color, 6> a(12345u, 20);
        lw::RandomSmoothPaletteGenerator<lw::Rgb8Color, 6> b(12345u, 20);

        for (int i = 0; i < 12; ++i)
        {
            a.update();
            b.update();
        }

        const auto sa = a.stops();
        const auto sb = b.stops();

        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(sa.size()), static_cast<uint32_t>(sb.size()));
        for (size_t i = 0; i < sa.size(); ++i)
        {
            TEST_ASSERT_EQUAL_UINT8(sa[i].index, sb[i].index);
            TEST_ASSERT_TRUE(sa[i].color == sb[i].color);
        }
    }

    void test_random_smooth_generator_changes_over_time(void)
    {
        lw::RandomSmoothPaletteGenerator<lw::Rgb8Color, 6> generator(999u, 17);
        const auto before = generator.stops();
        const lw::Rgb8Color firstBefore = before[0].color;

        generator.update();
        generator.update();

        const auto after = generator.stops();
        TEST_ASSERT_TRUE(!(after[0].color == firstBefore));
    }

    void test_random_cycle_generator_is_deterministic(void)
    {
        lw::RandomCyclePaletteGenerator<lw::Rgb8Color, 5> a(42u, 32);
        lw::RandomCyclePaletteGenerator<lw::Rgb8Color, 5> b(42u, 32);

        for (int i = 0; i < 16; ++i)
        {
            a.update();
            b.update();
        }

        const auto sa = a.stops();
        const auto sb = b.stops();

        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(sa.size()), static_cast<uint32_t>(sb.size()));
        for (size_t i = 0; i < sa.size(); ++i)
        {
            TEST_ASSERT_EQUAL_UINT8(sa[i].index, sb[i].index);
            TEST_ASSERT_TRUE(sa[i].color == sb[i].color);
        }
    }

    void test_random_cycle_generator_rotates_and_samples(void)
    {
        lw::RandomCyclePaletteGenerator<lw::Rgb8Color, 5> generator(77u, 64);
        const auto beforeStopsView = generator.stops();
        std::array<Stop, 5> beforeStops{};
        for (size_t i = 0; i < beforeStops.size(); ++i)
        {
            beforeStops[i] = beforeStopsView[i];
        }
        for (int i = 0; i < 8; ++i)
        {
            generator.update();
        }

        const auto afterStops = generator.stops();

        bool anyStopChanged = false;
        for (size_t i = 0; i < afterStops.size(); ++i)
        {
            if (!(afterStops[i].color == beforeStops[i].color))
            {
                anyStopChanged = true;
                break;
            }
        }

        TEST_ASSERT_TRUE(anyStopChanged);
    }

    void test_generators_satisfy_palette_like_usage(void)
    {
        lw::RainbowPaletteGenerator<lw::Rgb8Color, 6> rainbow;
        lw::RandomSmoothPaletteGenerator<lw::Rgb8Color, 6> smooth(1u, 25);
        lw::RandomCyclePaletteGenerator<lw::Rgb8Color, 6> cycle(2u, 25);

        std::array<lw::Rgb8Color, 4> out{};
        const size_t rainbowWritten = lw::samplePalette(rainbow,
                                                        static_cast<uint8_t>(0),
                                                        static_cast<uint8_t>(32),
                                                        lw::span<lw::Rgb8Color>(out.data(), out.size()));
        const size_t smoothWritten = lw::samplePalette(smooth,
                                                       static_cast<uint8_t>(0),
                                                       static_cast<uint8_t>(32),
                                                       lw::span<lw::Rgb8Color>(out.data(), out.size()));
        const size_t cycleWritten = lw::samplePalette(cycle,
                                                      static_cast<uint8_t>(0),
                                                      static_cast<uint8_t>(32),
                                                      lw::span<lw::Rgb8Color>(out.data(), out.size()));

        TEST_ASSERT_EQUAL_UINT32(4, static_cast<uint32_t>(rainbowWritten));
        TEST_ASSERT_EQUAL_UINT32(4, static_cast<uint32_t>(smoothWritten));
        TEST_ASSERT_EQUAL_UINT32(4, static_cast<uint32_t>(cycleWritten));
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
    RUN_TEST(test_rainbow_generator_stop_shape_and_update);
    RUN_TEST(test_random_smooth_generator_is_deterministic);
    RUN_TEST(test_random_smooth_generator_changes_over_time);
    RUN_TEST(test_random_cycle_generator_is_deterministic);
    RUN_TEST(test_random_cycle_generator_rotates_and_samples);
    RUN_TEST(test_generators_satisfy_palette_like_usage);
    return UNITY_END();
}
