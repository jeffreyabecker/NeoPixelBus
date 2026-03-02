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
            "transport=platformdefault\n"
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

        auto *assignable = dynamic_cast<lw::IAssignableBufferBus<lw::Rgb8Color> *>(built.bus.get());
        TEST_ASSERT_NOT_NULL(assignable);
        TEST_ASSERT_EQUAL_UINT16(10U, assignable->pixelCount());
    }

    void test_build_dynamic_bus_builder_from_ini_applies_onewire_timing_keys(void)
    {
        char config[] =
            "[bus:strip]\n"
            "pixels=8\n"
            "protocol=ws2812\n"
            "transport=platformdefault\n"
            "protocol:timing.t0hNs=300\n"
            "protocol:timing.t0lNs=900\n"
            "protocol:timing.t1hNs=900\n"
            "protocol:timing.t1lNs=300\n"
            "protocol:timing.resetNs=50000\n"
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
            "transport=platformdefault\n"
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
            "transport=platformdefault\n"
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

        auto *assignable = dynamic_cast<lw::IAssignableBufferBus<lw::Rgb8Color> *>(built.bus.get());
        TEST_ASSERT_NOT_NULL(assignable);
        TEST_ASSERT_EQUAL_UINT16(10U, assignable->pixelCount());
    }

    void test_build_dynamic_bus_builder_from_ini_accepts_non_canonical_channel_order_permutation(void)
    {
        char config[] =
            "[bus:strip]\n"
            "pixels=8\n"
            "protocol=ws2812\n"
            "transport=platformdefault\n"
            "protocol:channelOrder=rbg\n";

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
            "transport=platformdefault\n"
            "protocol:channelOrder=rrg\n";

        const auto parsed = lw::factory::tryBuildDynamicBusBuilderFromIni<>(lw::span<char>{config, std::strlen(config)});
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderIniError::None),
                              static_cast<int>(parsed.error));

        auto built = parsed.builder.tryBuild<lw::Rgb8Color>("strip");
        TEST_ASSERT_TRUE(built.ok());
        TEST_ASSERT_NOT_NULL(built.bus.get());
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
    return UNITY_END();
}
