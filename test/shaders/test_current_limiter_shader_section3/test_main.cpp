#include <unity.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

#include "virtual/colors/Color.h"
#include "virtual/colors/CurrentLimiterShader.h"

namespace
{
    using Color = npb::Color;
    using Settings = npb::CurrentLimiterShaderSettings<Color>;
    using Shader = npb::CurrentLimiterShader<Color>;

    uint32_t estimate_pixel_milliamps(std::span<const Color> colors,
                                      const std::array<uint16_t, Color::ChannelCount> &milliampsPerChannel,
                                      bool rgbwDerating)
    {
        uint64_t weightedDraw = 0;
        for (const auto &color : colors)
        {
            uint64_t pixelWeighted = 0;
            for (size_t ch = 0; ch < Color::ChannelCount; ++ch)
            {
                pixelWeighted += static_cast<uint64_t>(color[ch]) * milliampsPerChannel[ch];
            }

            if (rgbwDerating && (Color::ChannelCount >= 4))
            {
                pixelWeighted = (pixelWeighted * 3ULL) / 4ULL;
            }

            weightedDraw += pixelWeighted;
        }

        return static_cast<uint32_t>(weightedDraw / Color::MaxComponent);
    }

    uint8_t scale_component(uint8_t value, uint32_t scale)
    {
        return static_cast<uint8_t>((static_cast<uint64_t>(value) * scale + 127ULL) / 255ULL);
    }

    std::vector<Color> make_reference_frame(void)
    {
        return {
            Color{100, 120, 140, 160, 180},
            Color{10, 20, 30, 40, 50}};
    }

    Settings make_reference_settings(void)
    {
        Settings settings{};
        settings.maxMilliamps = 200;
        settings.milliampsPerChannel = {20, 10, 5, 1, 2};
        settings.controllerMilliamps = 30;
        settings.standbyMilliampsPerPixel = 3;
        settings.rgbwDerating = false;
        return settings;
    }

    void test_3_1_1_no_limit_path_max_zero(void)
    {
        Settings settings{};
        settings.maxMilliamps = 0;
        settings.milliampsPerChannel = {20, 20, 20, 20, 20};

        Shader shader(settings);

        auto frame = make_reference_frame();
        const auto original = frame;

        shader.apply(frame);

        TEST_ASSERT_TRUE(frame[0] == original[0]);
        TEST_ASSERT_TRUE(frame[1] == original[1]);
        TEST_ASSERT_EQUAL_UINT32(0U, shader.lastEstimatedMilliamps());
    }

    void test_3_1_2_under_budget_pass_through(void)
    {
        auto frame = make_reference_frame();
        const auto original = frame;

        Settings settings = make_reference_settings();
        settings.maxMilliamps = 500;

        Shader shader(settings);
        shader.apply(frame);

        const uint32_t pixelMilliamps = estimate_pixel_milliamps(frame, settings.milliampsPerChannel, settings.rgbwDerating);
        const uint32_t standby = static_cast<uint32_t>(settings.standbyMilliampsPerPixel) * static_cast<uint32_t>(frame.size());
        const uint32_t expectedEstimated = pixelMilliamps + settings.controllerMilliamps + standby;

        TEST_ASSERT_TRUE(frame[0] == original[0]);
        TEST_ASSERT_TRUE(frame[1] == original[1]);
        TEST_ASSERT_EQUAL_UINT32(expectedEstimated, shader.lastEstimatedMilliamps());
    }

    void test_3_1_3_over_budget_scaling(void)
    {
        std::vector<Color> frame{
            Color{255, 255, 255, 255, 255},
            Color{255, 255, 255, 255, 255}};

        Settings settings{};
        settings.maxMilliamps = 150;
        settings.controllerMilliamps = 30;
        settings.standbyMilliampsPerPixel = 2;
        settings.milliampsPerChannel = {20, 20, 20, 20, 20};
        settings.rgbwDerating = false;

        const uint32_t pixelMilliampsBefore = estimate_pixel_milliamps(frame, settings.milliampsPerChannel, settings.rgbwDerating);
        const uint32_t standby = static_cast<uint32_t>(settings.standbyMilliampsPerPixel) * static_cast<uint32_t>(frame.size());
        const uint32_t budgetForPixels = settings.maxMilliamps - settings.controllerMilliamps - standby;
        const uint32_t expectedScale = (budgetForPixels * 255U) / pixelMilliampsBefore;

        Shader shader(settings);
        shader.apply(frame);

        TEST_ASSERT_EQUAL_UINT8(scale_component(255, expectedScale), frame[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(scale_component(255, expectedScale), frame[0]['G']);
        TEST_ASSERT_EQUAL_UINT8(scale_component(255, expectedScale), frame[0]['B']);
        TEST_ASSERT_EQUAL_UINT8(scale_component(255, expectedScale), frame[0]['W']);
        TEST_ASSERT_EQUAL_UINT8(scale_component(255, expectedScale), frame[0]['C']);

        const uint32_t lastEstimated = shader.lastEstimatedMilliamps();
        TEST_ASSERT_TRUE(lastEstimated <= settings.maxMilliamps);
    }

    void test_3_1_4_controller_dominant_cutoff(void)
    {
        std::vector<Color> frame{
            Color{90, 80, 70, 60, 50},
            Color{9, 8, 7, 6, 5}};

        Settings settings{};
        settings.maxMilliamps = 50;
        settings.controllerMilliamps = 50;
        settings.standbyMilliampsPerPixel = 4;
        settings.milliampsPerChannel = {20, 20, 20, 20, 20};

        Shader shader(settings);
        shader.apply(frame);

        for (const auto &color : frame)
        {
            for (size_t ch = 0; ch < Color::ChannelCount; ++ch)
            {
                TEST_ASSERT_EQUAL_UINT8(0, color[ch]);
            }
        }

        const uint32_t expected = settings.controllerMilliamps
            + static_cast<uint32_t>(settings.standbyMilliampsPerPixel) * static_cast<uint32_t>(frame.size());
        TEST_ASSERT_EQUAL_UINT32(expected, shader.lastEstimatedMilliamps());
    }

    void test_3_2_1_weighted_draw_estimation_by_channel(void)
    {
        std::vector<Color> frame{
            Color{255, 128, 64, 32, 16},
            Color{1, 2, 3, 4, 5}};

        Settings settings{};
        settings.maxMilliamps = 5000;
        settings.controllerMilliamps = 10;
        settings.standbyMilliampsPerPixel = 1;
        settings.rgbwDerating = false;
        settings.milliampsPerChannel = {20, 10, 5, 2, 1};

        const uint32_t pixelMilliamps = estimate_pixel_milliamps(frame, settings.milliampsPerChannel, settings.rgbwDerating);
        const uint32_t expected = pixelMilliamps
            + settings.controllerMilliamps
            + static_cast<uint32_t>(settings.standbyMilliampsPerPixel) * static_cast<uint32_t>(frame.size());

        Shader shader(settings);
        shader.apply(frame);

        TEST_ASSERT_EQUAL_UINT32(expected, shader.lastEstimatedMilliamps());
    }

    void test_3_2_2_rgbw_derating_enabled(void)
    {
        std::vector<Color> frame{
            Color{255, 255, 255, 255, 255}};

        Settings settings{};
        settings.maxMilliamps = 1000;
        settings.controllerMilliamps = 0;
        settings.standbyMilliampsPerPixel = 0;
        settings.milliampsPerChannel = {20, 20, 20, 20, 20};
        settings.rgbwDerating = true;

        Shader shader(settings);
        shader.apply(frame);

        const uint32_t expectedPixel = estimate_pixel_milliamps(frame, settings.milliampsPerChannel, true);
        TEST_ASSERT_EQUAL_UINT32(expectedPixel, shader.lastEstimatedMilliamps());
    }

    void test_3_2_3_rgbw_derating_disabled(void)
    {
        std::vector<Color> frame{
            Color{255, 255, 255, 255, 255}};

        Settings settings{};
        settings.maxMilliamps = 1000;
        settings.controllerMilliamps = 0;
        settings.standbyMilliampsPerPixel = 0;
        settings.milliampsPerChannel = {20, 20, 20, 20, 20};
        settings.rgbwDerating = false;

        Shader shader(settings);
        shader.apply(frame);

        const uint32_t expectedPixel = estimate_pixel_milliamps(frame, settings.milliampsPerChannel, false);
        TEST_ASSERT_EQUAL_UINT32(expectedPixel, shader.lastEstimatedMilliamps());
    }

    void test_3_2_4_standby_current_budget_interaction(void)
    {
        std::vector<Color> frame{
            Color{255, 255, 255, 0, 0},
            Color{255, 255, 255, 0, 0}};

        Settings settings{};
        settings.maxMilliamps = 100;
        settings.controllerMilliamps = 10;
        settings.standbyMilliampsPerPixel = 20;
        settings.milliampsPerChannel = {20, 20, 20, 0, 0};
        settings.rgbwDerating = false;

        Shader shader(settings);
        shader.apply(frame);

        TEST_ASSERT_TRUE(frame[0]['R'] < 255);
        TEST_ASSERT_TRUE(frame[0]['G'] < 255);
        TEST_ASSERT_TRUE(frame[0]['B'] < 255);

        const uint32_t standby = static_cast<uint32_t>(settings.standbyMilliampsPerPixel) * static_cast<uint32_t>(frame.size());
        TEST_ASSERT_TRUE(shader.lastEstimatedMilliamps() >= settings.controllerMilliamps + standby);
    }

    void test_3_3_1_empty_frame_behavior(void)
    {
        std::vector<Color> frame{};

        Settings settings{};
        settings.maxMilliamps = 200;
        settings.controllerMilliamps = 33;
        settings.standbyMilliampsPerPixel = 2;
        settings.milliampsPerChannel = {20, 20, 20, 20, 20};

        Shader shader(settings);
        shader.apply(frame);

        TEST_ASSERT_EQUAL_UINT32(33U, shader.lastEstimatedMilliamps());
    }

    void test_3_3_2_extreme_component_values(void)
    {
        {
            std::vector<Color> frame{Color{0, 0, 0, 0, 0}};
            Settings settings{};
            settings.maxMilliamps = 50;
            settings.controllerMilliamps = 5;
            settings.standbyMilliampsPerPixel = 1;
            settings.milliampsPerChannel = {20, 20, 20, 20, 20};

            Shader shader(settings);
            shader.apply(frame);

            TEST_ASSERT_EQUAL_UINT8(0, frame[0]['R']);
            TEST_ASSERT_EQUAL_UINT8(0, frame[0]['G']);
            TEST_ASSERT_EQUAL_UINT8(0, frame[0]['B']);
        }

        {
            std::vector<Color> frame{Color{255, 255, 255, 255, 255}};
            Settings settings{};
            settings.maxMilliamps = 60;
            settings.controllerMilliamps = 10;
            settings.standbyMilliampsPerPixel = 1;
            settings.milliampsPerChannel = {20, 20, 20, 20, 20};
            settings.rgbwDerating = false;

            Shader shader(settings);
            shader.apply(frame);

            for (size_t ch = 0; ch < Color::ChannelCount; ++ch)
            {
                TEST_ASSERT_TRUE(frame[0][ch] <= std::numeric_limits<uint8_t>::max());
            }
            TEST_ASSERT_TRUE(shader.lastEstimatedMilliamps() <= settings.maxMilliamps);
        }
    }

    void test_3_3_3_scale_clamp_and_rounding_stability(void)
    {
        {
            std::vector<Color> frame{Color{200, 0, 0, 0, 0}};
            Settings settings{};
            settings.maxMilliamps = 100;
            settings.controllerMilliamps = 100;
            settings.standbyMilliampsPerPixel = 0;
            settings.milliampsPerChannel = {20, 0, 0, 0, 0};
            settings.rgbwDerating = false;

            Shader shader(settings);
            shader.apply(frame);
            TEST_ASSERT_EQUAL_UINT8(0, frame[0]['R']);
        }

        {
            std::vector<Color> frame{Color{255, 0, 0, 0, 0}};
            Settings settings{};
            settings.maxMilliamps = 101;
            settings.controllerMilliamps = 100;
            settings.standbyMilliampsPerPixel = 0;
            settings.milliampsPerChannel = {255, 0, 0, 0, 0};
            settings.rgbwDerating = false;

            Shader shader(settings);
            shader.apply(frame);
            TEST_ASSERT_EQUAL_UINT8(scale_component(255, 1), frame[0]['R']);
        }

        {
            std::vector<Color> frame{Color{255, 0, 0, 0, 0}};
            Settings settings{};
            settings.maxMilliamps = 354;
            settings.controllerMilliamps = 100;
            settings.standbyMilliampsPerPixel = 0;
            settings.milliampsPerChannel = {255, 0, 0, 0, 0};
            settings.rgbwDerating = false;

            Shader shader(settings);
            shader.apply(frame);
            TEST_ASSERT_EQUAL_UINT8(scale_component(255, 254), frame[0]['R']);
        }

        {
            std::vector<Color> frame{Color{123, 45, 67, 0, 0}};
            const auto original = frame;

            Settings settings{};
            settings.maxMilliamps = 1000;
            settings.controllerMilliamps = 0;
            settings.standbyMilliampsPerPixel = 0;
            settings.milliampsPerChannel = {1, 1, 1, 0, 0};
            settings.rgbwDerating = false;

            Shader shader(settings);
            shader.apply(frame);

            TEST_ASSERT_TRUE(frame[0] == original[0]);
        }
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
    RUN_TEST(test_3_1_1_no_limit_path_max_zero);
    RUN_TEST(test_3_1_2_under_budget_pass_through);
    RUN_TEST(test_3_1_3_over_budget_scaling);
    RUN_TEST(test_3_1_4_controller_dominant_cutoff);
    RUN_TEST(test_3_2_1_weighted_draw_estimation_by_channel);
    RUN_TEST(test_3_2_2_rgbw_derating_enabled);
    RUN_TEST(test_3_2_3_rgbw_derating_disabled);
    RUN_TEST(test_3_2_4_standby_current_budget_interaction);
    RUN_TEST(test_3_3_1_empty_frame_behavior);
    RUN_TEST(test_3_3_2_extreme_component_values);
    RUN_TEST(test_3_3_3_scale_clamp_and_rounding_stability);
    return UNITY_END();
}