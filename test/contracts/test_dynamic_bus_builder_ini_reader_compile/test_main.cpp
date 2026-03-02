#include <unity.h>

#include <cstring>

#include "factory/BuildDynamicBusBuilderFromIni.h"

namespace
{
    void test_build_dynamic_bus_builder_from_ini_builds_bus_and_aggregate(void)
    {
        char config[] =
            "[bus:left]\n"
            "pixels=4\n"
            "protocol=apa102\n"
            "transport=nil\n"
            "\n"
            "[bus:right]\n"
            "pixels=6\n"
            "protocol=ws2812\n"
            "transport=platform-default\n"
            "\n"
            "[bus:wall]\n"
            "kind=aggregate\n"
            "children=left|right\n";

        const auto parsed = lw::factory::tryBuildDynamicBusBuilderFromIni<>(lw::span<char>{config, std::strlen(config)});
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderIniError::None),
                      static_cast<int>(parsed.error));

        auto built = parsed.builder.tryBuild<lw::Rgb8Color>("wall");
        TEST_ASSERT_TRUE(built.ok());
        TEST_ASSERT_NOT_NULL(built.bus.get());
        TEST_ASSERT_EQUAL_UINT16(10U, static_cast<uint16_t>(built.bus->pixelBuffer().size()));
    }

    void test_build_dynamic_bus_builder_from_ini_applies_onewire_timing_keys(void)
    {
        char config[] =
            "[bus:strip]\n"
            "pixels=8\n"
            "protocol=ws2812\n"
            "transport=platform-default\n"
            "protocol:timing.t0h-ns=300\n"
            "protocol:timing.t0l-ns=900\n"
            "protocol:timing.t1h-ns=900\n"
            "protocol:timing.t1l-ns=300\n"
            "protocol:timing.reset-ns=50000\n"
            "protocol:timing.cadence=4step\n";

        const auto parsed = lw::factory::tryBuildDynamicBusBuilderFromIni<>(lw::span<char>{config, std::strlen(config)});
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderIniError::None),
                      static_cast<int>(parsed.error));

        auto built = parsed.builder.tryBuild<lw::Rgb8Color>("strip");
        TEST_ASSERT_TRUE(built.ok());
        TEST_ASSERT_NOT_NULL(built.bus.get());
    }

    void test_build_dynamic_bus_builder_from_ini_reports_unknown_protocol(void)
    {
        char config[] =
            "[bus:bad]\n"
            "pixels=10\n"
            "protocol=not-a-protocol\n"
            "transport=nil\n";

        const auto parsed = lw::factory::tryBuildDynamicBusBuilderFromIni<>(lw::span<char>{config, std::strlen(config)});
        TEST_ASSERT_TRUE(parsed.failed());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderIniError::UnknownProtocol),
                              static_cast<int>(parsed.error));
    }

    void test_build_dynamic_bus_builder_from_ini_accepts_descriptor_token_aliases(void)
    {
        char config[] =
            "[bus:a]\n"
            "pixels=5\n"
            "protocol=dotstar\n"
            "transport=nil\n"
            "\n"
            "[bus:b]\n"
            "pixels=5\n"
            "protocol=ws2812x\n"
            "transport=platform-default\n"
            "\n"
            "[bus:wall]\n"
            "kind=aggregate\n"
            "children=a|b\n";

        const auto parsed = lw::factory::tryBuildDynamicBusBuilderFromIni<>(lw::span<char>{config, std::strlen(config)});
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderIniError::None),
                              static_cast<int>(parsed.error));

        auto built = parsed.builder.tryBuild<lw::Rgb8Color>("wall");
        TEST_ASSERT_TRUE(built.ok());
        TEST_ASSERT_NOT_NULL(built.bus.get());
    }

    void test_build_dynamic_bus_builder_from_ini_builder_is_independent_of_source_buffer_lifetime(void)
    {
        char config[] =
            "[bus:left]\n"
            "pixels=4\n"
            "protocol=apa102\n"
            "transport=nil\n"
            "\n"
            "[bus:right]\n"
            "pixels=6\n"
            "protocol=ws2812\n"
            "transport=platform-default\n"
            "\n"
            "[bus:wall]\n"
            "kind=aggregate\n"
            "children=left|right\n";

        auto parsed = lw::factory::tryBuildDynamicBusBuilderFromIni<>(lw::span<char>{config, std::strlen(config)});
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderIniError::None),
                              static_cast<int>(parsed.error));

        const size_t length = std::strlen(config);
        std::memset(config, '?', length);

        auto built = parsed.builder.tryBuild<lw::Rgb8Color>("wall");
        TEST_ASSERT_TRUE(built.ok());
        TEST_ASSERT_NOT_NULL(built.bus.get());
        TEST_ASSERT_EQUAL_UINT16(10U, static_cast<uint16_t>(built.bus->pixelBuffer().size()));
    }

    void test_build_dynamic_bus_builder_from_ini_accepts_non_canonical_channel_order_permutation(void)
    {
        char config[] =
            "[bus:strip]\n"
            "pixels=8\n"
            "protocol=ws2812\n"
            "transport=platform-default\n"
            "protocol:channel-order=rbg\n";

        const auto parsed = lw::factory::tryBuildDynamicBusBuilderFromIni<>(lw::span<char>{config, std::strlen(config)});
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderIniError::None),
                              static_cast<int>(parsed.error));

        auto built = parsed.builder.tryBuild<lw::Rgb8Color>("strip");
        TEST_ASSERT_TRUE(built.ok());
        TEST_ASSERT_NOT_NULL(built.bus.get());
    }

    void test_build_dynamic_bus_builder_from_ini_ignores_invalid_channel_order_token(void)
    {
        char config[] =
            "[bus:strip]\n"
            "pixels=8\n"
            "protocol=ws2812\n"
            "transport=platform-default\n"
            "protocol:channel-order=rrg\n";

        const auto parsed = lw::factory::tryBuildDynamicBusBuilderFromIni<>(lw::span<char>{config, std::strlen(config)});
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderIniError::None),
                              static_cast<int>(parsed.error));

        auto built = parsed.builder.tryBuild<lw::Rgb8Color>("strip");
        TEST_ASSERT_TRUE(built.ok());
        TEST_ASSERT_NOT_NULL(built.bus.get());
    }

    void test_build_dynamic_bus_builder_from_ini_parses_tiled_layout_mode_and_rotation(void)
    {
        char config[] =
            "[bus:left]\n"
            "pixels=4\n"
            "protocol=apa102\n"
            "transport=nil\n"
            "\n"
            "[bus:right]\n"
            "pixels=4\n"
            "protocol=apa102\n"
            "transport=nil\n"
            "\n"
            "[bus:mosaic]\n"
            "kind=aggregate\n"
            "children=left|right\n"
            "topology=tiled\n"
            "panel-width=2\n"
            "panel-height=2\n"
            "layout=row-major\n"
            "layout-mode=serpentine\n"
            "layout-rotation=90\n"
            "tiles-wide=2\n"
            "tiles-high=1\n"
            "tile-layout=column-major\n"
            "tile-layout-mode=progressive\n"
            "tile-layout-rotation=270\n"
            "mosaic-rotation=true\n";

        const auto parsed = lw::factory::tryBuildDynamicBusBuilderFromIni<>(lw::span<char>{config, std::strlen(config)});
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderIniError::None),
                              static_cast<int>(parsed.error));

        auto built = parsed.builder.tryBuild<lw::Rgb8Color>("mosaic");
        TEST_ASSERT_TRUE(built.ok());
        TEST_ASSERT_NOT_NULL(built.bus.get());
    }

    void test_build_dynamic_bus_builder_from_ini_rejects_invalid_layout_rotation(void)
    {
        char config[] =
            "[bus:left]\n"
            "pixels=4\n"
            "protocol=apa102\n"
            "transport=nil\n"
            "\n"
            "[bus:right]\n"
            "pixels=4\n"
            "protocol=apa102\n"
            "transport=nil\n"
            "\n"
            "[bus:mosaic]\n"
            "kind=aggregate\n"
            "children=left|right\n"
            "topology=tiled\n"
            "panel-width=2\n"
            "panel-height=2\n"
            "layout=row-major\n"
            "layout-rotation=45\n"
            "tiles-wide=2\n"
            "tiles-high=1\n"
            "tile-layout=column-major\n"
            "mosaic-rotation=false\n";

        const auto parsed = lw::factory::tryBuildDynamicBusBuilderFromIni<>(lw::span<char>{config, std::strlen(config)});
        TEST_ASSERT_TRUE(parsed.failed());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderIniError::InvalidTopology),
                              static_cast<int>(parsed.error));
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
    RUN_TEST(test_build_dynamic_bus_builder_from_ini_builds_bus_and_aggregate);
    RUN_TEST(test_build_dynamic_bus_builder_from_ini_applies_onewire_timing_keys);
    RUN_TEST(test_build_dynamic_bus_builder_from_ini_reports_unknown_protocol);
    RUN_TEST(test_build_dynamic_bus_builder_from_ini_accepts_descriptor_token_aliases);
    RUN_TEST(test_build_dynamic_bus_builder_from_ini_builder_is_independent_of_source_buffer_lifetime);
    RUN_TEST(test_build_dynamic_bus_builder_from_ini_accepts_non_canonical_channel_order_permutation);
    RUN_TEST(test_build_dynamic_bus_builder_from_ini_ignores_invalid_channel_order_token);
    RUN_TEST(test_build_dynamic_bus_builder_from_ini_parses_tiled_layout_mode_and_rotation);
    RUN_TEST(test_build_dynamic_bus_builder_from_ini_rejects_invalid_layout_rotation);
    return UNITY_END();
}
