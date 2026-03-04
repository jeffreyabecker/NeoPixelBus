#include <unity.h>

#include <memory>

#include "factory/MakeDynamicBus.h"
#include "factory/busses/DynamicBus.h"
#include "core/IPixelBus.h"

namespace
{
    void test_make_dynamic_bus_dotstar_nil(void)
    {
        auto bus = lw::factory::makeBus(
            "[bus:front]\n"
            "pixels=12\n"
            "protocol=dotstar\n"
            "transport=nil\n");

        TEST_ASSERT_NOT_NULL(bus.get());
        TEST_ASSERT_EQUAL_UINT16(12U, static_cast<uint16_t>(bus->pixelBuffer().size()));
    }

    void test_make_dynamic_bus_ws2812_platform_default(void)
    {
        auto bus = lw::factory::makeBus(
            "[bus:strip]\n"
            "pixels=20\n"
            "protocol=ws2812\n"
            "transport=platform-default\n");

        TEST_ASSERT_NOT_NULL(bus.get());
        TEST_ASSERT_EQUAL_UINT16(20U, static_cast<uint16_t>(bus->pixelBuffer().size()));
    }

    void test_make_dynamic_bus_returns_null_on_parse_error(void)
    {
        auto bus = lw::factory::makeBus(
            "[bus:strip]\n"
            "protocol=dotstar\n");
        TEST_ASSERT_NULL(bus.get());

        auto detailed = lw::factory::tryMakeBus(
            "[bus:strip]\n"
            "protocol=dotstar\n");
        TEST_ASSERT_TRUE(detailed.failed());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusFactoryError::ParseFailed),
                              static_cast<int>(detailed.error));
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderIniError::MissingPixels),
                              static_cast<int>(detailed.parse.error));
    }

    void test_make_dynamic_bus_named_overload(void)
    {
        auto bus = lw::factory::makeBus(
            "[bus:main]\n"
            "pixels=33\n"
            "protocol=ws2811\n"
            "transport=platform-default\n"
            "\n"
            "[bus:other]\n"
            "pixels=8\n"
            "protocol=dotstar\n",
            "main");

        TEST_ASSERT_NOT_NULL(bus.get());
        TEST_ASSERT_EQUAL_UINT16(33U, static_cast<uint16_t>(bus->pixelBuffer().size()));
    }

    void test_make_dynamic_bus_named_overload_returns_null_for_missing_name(void)
    {
        auto bus = lw::factory::makeBus(
            "[bus:front]\n"
            "pixels=10\n"
            "protocol=ws2812\n",
            "rear");
        TEST_ASSERT_NULL(bus.get());

        auto detailed = lw::factory::tryMakeBus(
            "[bus:front]\n"
            "pixels=10\n"
            "protocol=ws2812\n",
            "rear");
        TEST_ASSERT_TRUE(detailed.failed());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusFactoryError::ParseFailed),
                              static_cast<int>(detailed.error));
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderError::UnknownName),
                              static_cast<int>(detailed.builderError));
    }

    void test_make_dynamic_aggregate_bus_uses_unified_dynamic_owning_bus(void)
    {
        auto bus = lw::factory::makeDynamicAggregateBus(
            "[bus:front]\n"
            "pixels=11\n"
            "protocol=ws2812\n"
            "transport=nil\n"
            "\n"
            "[bus:rear]\n"
            "pixels=9\n"
            "protocol=dotstar\n"
            "transport=platform-default\n"
            "\n"
            "[bus:wall]\n"
            "kind=aggregate\n"
            "children=front|rear\n");

        TEST_ASSERT_NOT_NULL(bus.get());

        auto *typed = dynamic_cast<lw::UnifiedDynamicBus<lw::Rgb8Color> *>(bus.get());
        TEST_ASSERT_NOT_NULL(typed);
        TEST_ASSERT_EQUAL_UINT16(20U, static_cast<uint16_t>(bus->pixelBuffer().size()));
    }

    void test_make_dynamic_aggregate_bus_returns_null_on_parse_error(void)
    {
        auto bus = lw::factory::makeDynamicAggregateBus(
            "[bus:front]\n"
            "pixels=11\n"
            "protocol=ws2812\n"
            "\n"
            "[bus:wall]\n"
            "kind=aggregate\n"
            "children=front|rear\n");
        TEST_ASSERT_NULL(bus.get());

        auto detailed = lw::factory::tryMakeDynamicAggregateBus(
            "[bus:front]\n"
            "pixels=11\n"
            "protocol=ws2812\n"
            "\n"
            "[bus:wall]\n"
            "kind=aggregate\n"
            "children=front|rear\n");
        TEST_ASSERT_TRUE(detailed.failed());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusFactoryError::ParseFailed),
                              static_cast<int>(detailed.error));
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
    RUN_TEST(test_make_dynamic_bus_dotstar_nil);
    RUN_TEST(test_make_dynamic_bus_ws2812_platform_default);
    RUN_TEST(test_make_dynamic_bus_returns_null_on_parse_error);
    RUN_TEST(test_make_dynamic_bus_named_overload);
    RUN_TEST(test_make_dynamic_bus_named_overload_returns_null_for_missing_name);
    RUN_TEST(test_make_dynamic_aggregate_bus_uses_unified_dynamic_owning_bus);
    RUN_TEST(test_make_dynamic_aggregate_bus_returns_null_on_parse_error);
    return UNITY_END();
}
