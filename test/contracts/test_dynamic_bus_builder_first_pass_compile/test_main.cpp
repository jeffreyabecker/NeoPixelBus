#include <unity.h>

#include "colors/GammaShader.h"
#include "factory/DynamicBusBuilder.h"
#include "protocols/PixieProtocol.h"

namespace lw
{
    class TestClockedTransport : public ITransport
    {
    public:
        using TransportSettingsType = TransportSettingsBase;

        explicit TestClockedTransport(TransportSettingsType settings)
            : _settings(std::move(settings))
        {
            lastConstructedSettings = _settings;
        }

        void begin() override
        {
        }

        void transmitBytes(span<uint8_t>) override
        {
        }

        static inline TransportSettingsType lastConstructedSettings{};

    private:
        TransportSettingsType _settings{};
    };
}

namespace lw
{
namespace factory
{
namespace descriptors
{
    struct TestClockedTransport
    {
    };
}

    struct TestClockedTransportOptions
    {
        uint32_t clockRateHz = 0;
        bool invert = false;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::TestClockedTransport, void>
        : TransportDescriptorTraitDefaults<lw::TestClockedTransport::TransportSettingsType>
    {
        using TransportType = lw::TestClockedTransport;
        using SettingsType = typename TransportType::TransportSettingsType;
        using Base = TransportDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings,
                                      uint16_t,
                                      const OneWireTiming * = nullptr)
        {
            return settings;
        }

        static SettingsType fromConfig(const TestClockedTransportOptions &config,
                                       uint16_t)
        {
            SettingsType settings{};
            settings.clockRateHz = config.clockRateHz;
            settings.invert = config.invert;
            return settings;
        }
    };
}
}

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
        TEST_ASSERT_EQUAL_UINT16(18U, static_cast<uint16_t>(result.bus->pixelBuffer().size()));
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

        auto *typed = dynamic_cast<lw::UnifiedDynamicBus<lw::Rgb8Color> *>(result.bus.get());
        TEST_ASSERT_NOT_NULL(typed);
        TEST_ASSERT_EQUAL_UINT16(20U, static_cast<uint16_t>(result.bus->pixelBuffer().size()));
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

        auto *typed = dynamic_cast<lw::UnifiedDynamicBus<lw::Rgb8Color> *>(result.bus.get());
        TEST_ASSERT_NOT_NULL(typed);
        TEST_ASSERT_EQUAL_UINT32(1u, static_cast<uint32_t>(typed->strands().size()));

        auto *shader = dynamic_cast<lw::GammaShader<lw::Rgb8Color> *>(typed->strands()[0].shader);
        TEST_ASSERT_NOT_NULL(shader);
    }

    void test_dynamic_bus_builder_manual_onewire_timing_four_step_sets_transport_clock_when_unset(void)
    {
        lw::factory::DynamicBusBuilder<> builder{};

        const lw::OneWireTiming timing{
            300,
            900,
            900,
            300,
            50000,
            lw::EncodedClockDataBitPattern::FourStep};

        lw::factory::TestClockedTransportOptions transportOptions{};
        transportOptions.clockRateHz = 0;

        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::Ws2812T<lw::Rgb8Color>,
                                         lw::factory::descriptors::TestClockedTransport>("timed",
                                                                                          8,
                                                                                          timing,
                                                                                          transportOptions)));

        auto result = builder.tryBuild<lw::Rgb8Color>("timed");
        TEST_ASSERT_TRUE(result.ok());
        TEST_ASSERT_EQUAL_UINT32(timing.encodedDataRateHz(),
                                 lw::TestClockedTransport::lastConstructedSettings.clockRateHz);
    }

    void test_dynamic_bus_builder_manual_transport_clock_rate_is_preserved(void)
    {
        lw::factory::DynamicBusBuilder<> builder{};

        const lw::OneWireTiming timing{
            400,
            850,
            800,
            450,
            50000,
            lw::EncodedClockDataBitPattern::FourStep};

        lw::factory::TestClockedTransportOptions transportOptions{};
        transportOptions.clockRateHz = 2400000;

        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::Ws2812T<lw::Rgb8Color>,
                                         lw::factory::descriptors::TestClockedTransport>("timed-explicit",
                                                                                          8,
                                                                                          timing,
                                                                                          transportOptions)));

        auto result = builder.tryBuild<lw::Rgb8Color>("timed-explicit");
        TEST_ASSERT_TRUE(result.ok());
        TEST_ASSERT_EQUAL_UINT32(2400000U,
                                 lw::TestClockedTransport::lastConstructedSettings.clockRateHz);
    }

    void test_dynamic_bus_builder_supports_pixie_ws2813_and_hd108(void)
    {
        lw::factory::DynamicBusBuilder<> builder{};

        TEST_ASSERT_TRUE((builder.addBus<lw::PixieProtocol>("pixie", 6)));
        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::Ws2813T<lw::Rgb8Color>>("ws2813", 7)));
        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::HD108,
                                         lw::factory::descriptors::Nil>("hd108", 5)));

        auto pixie = builder.tryBuild<lw::Rgb8Color>("pixie");
        auto ws2813 = builder.tryBuild<lw::Rgb8Color>("ws2813");
        auto hd108 = builder.tryBuild<lw::Rgb16Color>("hd108");

        TEST_ASSERT_TRUE(pixie.ok());
        TEST_ASSERT_TRUE(ws2813.ok());
        TEST_ASSERT_TRUE(hd108.ok());
    }

    void test_dynamic_bus_builder_supports_non_default_channel_order_and_larger_interface_color(void)
    {
        lw::factory::DynamicBusBuilder<> builder{};

        lw::factory::Ws2812xOptions protocolOptions{};
        protocolOptions.channelOrder = lw::ChannelOrder::RGB::value;

        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::Ws2812T<lw::Rgb8Color>,
                                         lw::factory::descriptors::PlatformDefault>("ordered",
                                                                                    9,
                                                                                    protocolOptions,
                                                                                    lw::factory::PlatformDefaultOptions{})));

        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::Ws2812T<lw::Rgb16Color>,
                                         lw::factory::descriptors::PlatformDefault>("wide", 9)));

        auto ordered = builder.tryBuild<lw::Rgb8Color>("ordered");
        auto wide = builder.tryBuild<lw::Rgb16Color>("wide");

        TEST_ASSERT_TRUE(ordered.ok());
        TEST_ASSERT_TRUE(wide.ok());
    }

    void test_dynamic_bus_builder_aggregate_topology_is_linear(void)
    {
        lw::factory::DynamicBusBuilder<> builder{};

        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::APA102,
                                         lw::factory::descriptors::Nil>("left", 4)));
        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::APA102,
                                         lw::factory::descriptors::Nil>("right", 6)));
        TEST_ASSERT_TRUE(builder.addAggregate("wall", {"left", "right"}));

        auto result = builder.tryBuild<lw::Rgb8Color>("wall");
        TEST_ASSERT_TRUE(result.ok());

        const auto *topology = result.bus->topologyOrNull();
        TEST_ASSERT_NOT_NULL(topology);
        TEST_ASSERT_EQUAL_UINT16(10U, topology->width());
        TEST_ASSERT_EQUAL_UINT16(1U, topology->height());
    }

    void test_dynamic_bus_builder_aggregate_topology_from_settings_overload(void)
    {
        lw::factory::DynamicBusBuilder<> builder{};

        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::APA102,
                                         lw::factory::descriptors::Nil>("left", 5)));
        TEST_ASSERT_TRUE((builder.addBus<lw::factory::descriptors::APA102,
                                         lw::factory::descriptors::Nil>("right", 5)));

        lw::TopologySettings topology{};
        topology.panelWidth = 1;
        topology.panelHeight = 5;
        topology.layout = lw::PanelLayout::RowMajor;
        topology.tilesWide = 2;
        topology.tilesHigh = 1;
        topology.tileLayout = lw::PanelLayout::RowMajor;
        topology.mosaicRotation = false;

        TEST_ASSERT_TRUE(builder.addAggregate("wall", topology, {"left", "right"}));

        auto result = builder.tryBuild<lw::Rgb8Color>("wall");
        TEST_ASSERT_TRUE(result.ok());

        const auto *resolvedTopology = result.bus->topologyOrNull();
        TEST_ASSERT_NOT_NULL(resolvedTopology);
        TEST_ASSERT_EQUAL_UINT16(2U, resolvedTopology->width());
        TEST_ASSERT_EQUAL_UINT16(5U, resolvedTopology->height());
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
    RUN_TEST(test_dynamic_bus_builder_manual_onewire_timing_four_step_sets_transport_clock_when_unset);
    RUN_TEST(test_dynamic_bus_builder_manual_transport_clock_rate_is_preserved);
    RUN_TEST(test_dynamic_bus_builder_supports_pixie_ws2813_and_hd108);
    RUN_TEST(test_dynamic_bus_builder_supports_non_default_channel_order_and_larger_interface_color);
    RUN_TEST(test_dynamic_bus_builder_aggregate_topology_is_linear);
    RUN_TEST(test_dynamic_bus_builder_aggregate_topology_from_settings_overload);
    return UNITY_END();
}
