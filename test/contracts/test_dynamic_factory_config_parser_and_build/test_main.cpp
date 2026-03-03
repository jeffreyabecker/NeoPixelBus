#include <unity.h>

#include <memory>

#include "factory/MakeDynamicBus.h"
#include "buses/OwningUnifiedPixelBus.h"
#include "core/IPixelBus.h"

namespace
{
    void test_parse_valid_minimal_config(void)
    {
        auto parsed = lw::factory::parseDynamicBusConfig("pixels=16;protocol=dotstar;transport=nil");

        TEST_ASSERT_TRUE(parsed.ok());
        TEST_ASSERT_EQUAL_UINT16(16, parsed.config.pixelCount);
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusProtocolKind::DotStar),
                              static_cast<int>(parsed.config.protocol));
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusTransportKind::Nil),
                              static_cast<int>(parsed.config.transport));
    }

    void test_parse_defaults_transport_to_platform_default(void)
    {
        auto parsed = lw::factory::parseDynamicBusConfig("pixels=8;protocol=ws2812");

        TEST_ASSERT_TRUE(parsed.ok());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusTransportKind::PlatformDefault),
                              static_cast<int>(parsed.config.transport));
    }

    void test_parse_rejects_unknown_key(void)
    {
        auto parsed = lw::factory::parseDynamicBusConfig("pixels=8;protocol=ws2812;oops=1");

        TEST_ASSERT_TRUE(parsed.failed());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusConfigParseError::UnknownKey),
                              static_cast<int>(parsed.error));
    }

    void test_make_dynamic_bus_dotstar_nil(void)
    {
        auto bus = lw::factory::makeBus("pixels=12;protocol=dotstar;transport=nil");

        TEST_ASSERT_NOT_NULL(bus.get());
        TEST_ASSERT_EQUAL_UINT16(12U, static_cast<uint16_t>(bus->pixelBuffer().size()));
    }

    void test_make_dynamic_bus_ws2812_platform_default(void)
    {
        auto bus = lw::factory::makeBus("pixels=20;protocol=ws2812;transport=platformdefault");

        TEST_ASSERT_NOT_NULL(bus.get());
        TEST_ASSERT_EQUAL_UINT16(20U, static_cast<uint16_t>(bus->pixelBuffer().size()));
    }

    void test_make_dynamic_bus_returns_null_on_parse_error(void)
    {
        auto bus = lw::factory::makeBus("pixels=0;protocol=dotstar");
        TEST_ASSERT_NULL(bus.get());

        auto detailed = lw::factory::tryMakeBus("pixels=0;protocol=dotstar");
        TEST_ASSERT_TRUE(detailed.failed());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusFactoryError::ParseFailed),
                              static_cast<int>(detailed.error));
    }

    void test_parse_named_bus_from_mixed_spec(void)
    {
        auto parsed = lw::factory::parseDynamicBusConfig(
            "bus.front.pixels=21;bus.front.protocol=ws2812;bus.rear.pixels=10;bus.rear.protocol=dotstar;bus.rear.transport=nil",
            "rear");

        TEST_ASSERT_TRUE(parsed.ok());
        TEST_ASSERT_EQUAL_UINT16(10, parsed.config.pixelCount);
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusProtocolKind::DotStar),
                              static_cast<int>(parsed.config.protocol));
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusTransportKind::Nil),
                              static_cast<int>(parsed.config.transport));
    }

    void test_make_dynamic_bus_named_overload(void)
    {
        auto bus = lw::factory::makeBus(
            "bus.main.pixels=33;bus.main.protocol=ws2811;bus.main.transport=platformdefault;bus.other.pixels=8;bus.other.protocol=dotstar",
            "main");

        TEST_ASSERT_NOT_NULL(bus.get());
        TEST_ASSERT_EQUAL_UINT16(33U, static_cast<uint16_t>(bus->pixelBuffer().size()));
    }

    void test_make_dynamic_bus_named_overload_returns_null_for_missing_name(void)
    {
        auto bus = lw::factory::makeBus(
            "bus.front.pixels=10;bus.front.protocol=ws2812",
            "rear");
        TEST_ASSERT_NULL(bus.get());

        auto detailed = lw::factory::tryMakeBus(
            "bus.front.pixels=10;bus.front.protocol=ws2812",
            "rear");
        TEST_ASSERT_TRUE(detailed.failed());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusFactoryError::ParseFailed),
                              static_cast<int>(detailed.error));
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusConfigParseError::MissingNamedBus),
                              static_cast<int>(detailed.parse.error));
    }

    void test_parse_dynamic_aggregate_children_valid(void)
    {
        auto parsed = lw::factory::parseDynamicAggregateConfig(
            "bus.front.pixels=30;bus.front.protocol=ws2812;bus.rear.pixels=30;bus.rear.protocol=dotstar;aggregate.children=front|rear");

        TEST_ASSERT_TRUE(parsed.ok());
        TEST_ASSERT_EQUAL_UINT8(2, parsed.config.childCount);
        TEST_ASSERT_EQUAL_STRING("front", parsed.config.children[0].value);
        TEST_ASSERT_EQUAL_STRING("rear", parsed.config.children[1].value);
    }

    void test_parse_dynamic_aggregate_rejects_unknown_child(void)
    {
        auto parsed = lw::factory::parseDynamicAggregateConfig(
            "bus.front.pixels=30;bus.front.protocol=ws2812;aggregate.children=front|rear");

        TEST_ASSERT_TRUE(parsed.failed());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusConfigParseError::UnknownChildName),
                              static_cast<int>(parsed.error));
    }

    void test_parse_dynamic_aggregate_rejects_duplicate_children(void)
    {
        auto parsed = lw::factory::parseDynamicAggregateConfig(
            "bus.front.pixels=30;bus.front.protocol=ws2812;aggregate.children=front|front");

        TEST_ASSERT_TRUE(parsed.failed());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusConfigParseError::DuplicateName),
                              static_cast<int>(parsed.error));
    }

    void test_make_dynamic_aggregate_bus_uses_unified_dynamic_owning_bus(void)
    {
        auto bus = lw::factory::makeDynamicAggregateBus(
            "bus.front.pixels=11;bus.front.protocol=ws2812;bus.front.transport=nil;"
            "bus.rear.pixels=9;bus.rear.protocol=dotstar;bus.rear.transport=platformdefault;"
            "aggregate.children=front|rear");

        TEST_ASSERT_NOT_NULL(bus.get());

        auto *typed = dynamic_cast<lw::UnifiedDynamicBus<lw::Rgb8Color> *>(bus.get());
        TEST_ASSERT_NOT_NULL(typed);
        TEST_ASSERT_EQUAL_UINT16(20U, static_cast<uint16_t>(bus->pixelBuffer().size()));
    }

    void test_make_dynamic_aggregate_bus_returns_null_on_parse_error(void)
    {
        auto bus = lw::factory::makeDynamicAggregateBus(
            "bus.front.pixels=11;bus.front.protocol=ws2812;aggregate.children=front|rear");
        TEST_ASSERT_NULL(bus.get());

        auto detailed = lw::factory::tryMakeDynamicAggregateBus(
            "bus.front.pixels=11;bus.front.protocol=ws2812;aggregate.children=front|rear");
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
    RUN_TEST(test_parse_valid_minimal_config);
    RUN_TEST(test_parse_defaults_transport_to_platform_default);
    RUN_TEST(test_parse_rejects_unknown_key);
    RUN_TEST(test_make_dynamic_bus_dotstar_nil);
    RUN_TEST(test_make_dynamic_bus_ws2812_platform_default);
    RUN_TEST(test_make_dynamic_bus_returns_null_on_parse_error);
    RUN_TEST(test_parse_named_bus_from_mixed_spec);
    RUN_TEST(test_make_dynamic_bus_named_overload);
    RUN_TEST(test_make_dynamic_bus_named_overload_returns_null_for_missing_name);
    RUN_TEST(test_parse_dynamic_aggregate_children_valid);
    RUN_TEST(test_parse_dynamic_aggregate_rejects_unknown_child);
    RUN_TEST(test_parse_dynamic_aggregate_rejects_duplicate_children);
    RUN_TEST(test_make_dynamic_aggregate_bus_uses_unified_dynamic_owning_bus);
    RUN_TEST(test_make_dynamic_aggregate_bus_returns_null_on_parse_error);
    return UNITY_END();
}
