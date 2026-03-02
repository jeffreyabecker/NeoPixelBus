#include <unity.h>

#include "colors/GammaShader.h"
#include "factory/DynamicBusBuilder.h"

namespace
{
    void test_dynamic_bus_builder_builds_named_single_bus(void)
    {
        lw::factory::DynamicBusBuilder<> builder{};

        const bool added = builder.addBus<lw::factory::descriptors::APA102,
                                          lw::factory::descriptors::Nil>("front", 18);
        TEST_ASSERT_TRUE(added);

        auto requirement = builder.colorRequirement("front");
        TEST_ASSERT_TRUE(requirement.ok());
        TEST_ASSERT_TRUE((requirement.is<lw::Rgb8Color>()));
        TEST_ASSERT_TRUE((builder.canBuildAs<lw::Rgb8Color>("front")));

        auto result = builder.tryBuild<lw::Rgb8Color>("front");
        TEST_ASSERT_TRUE(result.ok());
        TEST_ASSERT_NOT_NULL(result.bus.get());

        auto *assignable = dynamic_cast<lw::IAssignableBufferBus<lw::Rgb8Color> *>(result.bus.get());
        TEST_ASSERT_NOT_NULL(assignable);
        TEST_ASSERT_EQUAL_UINT16(18, assignable->pixelCount());
    }

    void test_dynamic_bus_builder_builds_named_aggregate_bus(void)
    {
        lw::factory::DynamicBusBuilder<> builder{};

        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::Ws2812T<lw::Rgb8Color>,
                                             lw::factory::descriptors::PlatformDefault>("left", 11)));
        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::APA102,
                                             lw::factory::descriptors::Nil>("right", 9)));
        TEST_ASSERT_TRUE(builder.addAggregate("wall", {"left", "right"}));

        auto result = builder.tryBuild<lw::Rgb8Color>("wall");
        TEST_ASSERT_TRUE(result.ok());

        auto *typed = dynamic_cast<lw::UnifiedDynamicOwningBus<lw::Rgb8Color> *>(result.bus.get());
        TEST_ASSERT_NOT_NULL(typed);

        auto *assignable = dynamic_cast<lw::IAssignableBufferBus<lw::Rgb8Color> *>(result.bus.get());
        TEST_ASSERT_NOT_NULL(assignable);
        TEST_ASSERT_EQUAL_UINT16(20, assignable->pixelCount());
    }

    void test_dynamic_bus_builder_detects_invalid_aggregate_child(void)
    {
        lw::factory::DynamicBusBuilder<> builder{};

        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::APA102,
                                             lw::factory::descriptors::Nil>("left", 8)));
        TEST_ASSERT_TRUE(builder.addAggregate("bad", {"left", "missing"}));

        auto result = builder.tryBuild<lw::Rgb8Color>("bad");
        TEST_ASSERT_TRUE(result.failed());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderError::InvalidAggregateRef),
                              static_cast<int>(result.error));
        TEST_ASSERT_EQUAL_UINT32(1u, static_cast<uint32_t>(result.childIndex));
    }

    void test_dynamic_bus_builder_detects_aggregate_cycles(void)
    {
        lw::factory::DynamicBusBuilder<> builder{};

        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::APA102,
                                             lw::factory::descriptors::Nil>("leaf", 5)));
        TEST_ASSERT_TRUE(builder.addAggregate("a", {"b", "leaf"}));
        TEST_ASSERT_TRUE(builder.addAggregate("b", {"a"}));

        auto result = builder.tryBuild<lw::Rgb8Color>("a");
        TEST_ASSERT_TRUE(result.failed());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderError::CycleDetected),
                              static_cast<int>(result.error));
    }

    void test_dynamic_bus_builder_rejects_duplicate_names(void)
    {
        lw::factory::DynamicBusBuilder<> builder{};

        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::APA102,
                                             lw::factory::descriptors::Nil>("front", 10)));
        TEST_ASSERT_FALSE(builder.addAggregate("front", {"front"}));
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderError::DuplicateName),
                              static_cast<int>(builder.lastError()));
    }

    void test_dynamic_bus_builder_reports_color_mismatch_at_build_time(void)
    {
        lw::factory::DynamicBusBuilder<> builder{};

        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::Ws2812T<lw::Rgb16Color>,
                                             lw::factory::descriptors::PlatformDefault>("high", 7)));

        auto requirement = builder.colorRequirement("high");
        TEST_ASSERT_TRUE(requirement.ok());
        TEST_ASSERT_TRUE((requirement.is<lw::Rgb16Color>()));
        TEST_ASSERT_FALSE((requirement.is<lw::Rgb8Color>()));

        auto mismatch = builder.tryBuild<lw::Rgb8Color>("high");
        TEST_ASSERT_TRUE(mismatch.failed());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::factory::DynamicBusBuilderError::ColorMismatch),
                              static_cast<int>(mismatch.error));
    }

    void test_dynamic_bus_builder_accepts_explicit_shader_descriptor_and_config(void)
    {
        lw::factory::DynamicBusBuilder<> builder{};

        lw::factory::GammaOptions<lw::Rgb8Color> gamma{};
        gamma.gamma = 2.2f;
        gamma.enableColorGamma = true;
        gamma.enableBrightnessGamma = false;

        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::APA102,
                                             lw::factory::descriptors::Nil,
                                             lw::factory::descriptors::Gamma<lw::Rgb8Color>>("front",
                                                                                              12,
                                                                                              lw::factory::ProtocolDescriptorTraits<lw::factory::descriptors::APA102>::defaultSettings(),
                                                                                              lw::factory::TransportDescriptorTraits<lw::factory::descriptors::Nil>::defaultSettings(12),
                                                                                              gamma)));

        auto result = builder.tryBuild<lw::Rgb8Color>("front");
        TEST_ASSERT_TRUE(result.ok());

        auto *typed = dynamic_cast<lw::UnifiedDynamicOwningBus<lw::Rgb8Color> *>(result.bus.get());
        TEST_ASSERT_NOT_NULL(typed);
        TEST_ASSERT_EQUAL_UINT32(1u, static_cast<uint32_t>(typed->strands().size()));

        auto *shader = dynamic_cast<lw::GammaShader<lw::Rgb8Color> *>(typed->strands()[0].shader);
        TEST_ASSERT_NOT_NULL(shader);
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
    RUN_TEST(test_dynamic_bus_builder_builds_named_single_bus);
    RUN_TEST(test_dynamic_bus_builder_builds_named_aggregate_bus);
    RUN_TEST(test_dynamic_bus_builder_detects_invalid_aggregate_child);
    RUN_TEST(test_dynamic_bus_builder_detects_aggregate_cycles);
    RUN_TEST(test_dynamic_bus_builder_rejects_duplicate_names);
    RUN_TEST(test_dynamic_bus_builder_reports_color_mismatch_at_build_time);
    RUN_TEST(test_dynamic_bus_builder_accepts_explicit_shader_descriptor_and_config);
    return UNITY_END();
}
