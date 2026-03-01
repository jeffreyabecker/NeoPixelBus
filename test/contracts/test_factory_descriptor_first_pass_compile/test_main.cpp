#include <unity.h>

#include <array>
#include <memory>
#include <type_traits>

#include "factory/MakeBus.h"
#include "factory/MakeCompositeBus.h"
#include "factory/MakeShader.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/descriptors/ShaderDescriptors.h"
#include "factory/descriptors/TransportDescriptors.h"
#include "colors/AggregateShader.h"
#include "colors/CurrentLimiterShader.h"
#include "colors/GammaShader.h"
#include "protocols/DotStarProtocol.h"
#include "transports/OneWireTiming.h"
#include "transports/NilTransport.h"

namespace
{
    template <typename TExpr, typename = void>
    struct IsDetected : std::false_type
    {
    };

    template <typename TExpr>
    struct IsDetected<TExpr, std::void_t<TExpr>> : std::true_type
    {
    };

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename... TArgs>
    using MakeBusExpr = decltype(lw::factory::makeBus<TProtocolDesc, TTransportDesc>(std::declval<uint16_t>(), std::declval<TArgs>()...));

    void test_descriptor_metadata_spike_shape(void)
    {
        using DotStarDesc = lw::factory::descriptors::DotStar<>;
        using Ws2812xDesc = lw::factory::descriptors::Ws2812x<>;

        static_assert(std::is_same<typename DotStarDesc::ColorType, lw::Rgb8Color>::value,
                      "DotStar descriptor should expose ColorType");
        static_assert(std::is_same<typename DotStarDesc::CapabilityRequirement, lw::TransportTag>::value,
                      "DotStar descriptor should expose transport capability requirement");
        static_assert(std::is_same<typename DotStarDesc::DefaultChannelOrder, lw::ChannelOrder::BGR>::value,
                      "DotStar descriptor should expose default channel order");

        static_assert(std::is_same<typename Ws2812xDesc::ColorType, lw::Rgb8Color>::value,
                      "Ws2812x descriptor should expose ColorType");
        static_assert(std::is_same<typename Ws2812xDesc::CapabilityRequirement, lw::OneWireTransportTag>::value,
                      "Ws2812x descriptor should expose one-wire capability requirement");
        static_assert(std::is_same<typename Ws2812xDesc::DefaultChannelOrder, lw::ChannelOrder::GRB>::value,
                      "Ws2812x descriptor should expose default channel order");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2812x<lw::Rgbcw8Color, lw::OneWireTransportTag, lw::ChannelOrder::GRBCW>::DefaultChannelOrder,
                       lw::ChannelOrder::GRBCW>::value,
                  "Ws2812x 5-channel descriptor should support GRBCW default order");

        static_assert(std::is_same<typename lw::factory::descriptors::NeoSpi::Capability, lw::TransportTag>::value,
                      "NeoSpi descriptor should expose transport capability");
        static_assert(std::is_same<typename lw::factory::descriptors::RpPio::Capability, lw::TransportTag>::value,
                  "RpPio descriptor should expose transport capability");
        static_assert(std::is_same<typename lw::factory::descriptors::PlatformDefault::Capability, lw::TransportTag>::value,
                  "PlatformDefault descriptor should expose transport capability");

    #if defined(ARDUINO_ARCH_NATIVE)
        static_assert(std::is_same<lw::factory::descriptors::PlatformDefault, lw::factory::descriptors::Nil>::value,
                  "PlatformDefault descriptor alias should map to Nil on native");
        static_assert(std::is_same<lw::factory::PlatformDefaultOptions, lw::factory::NilOptions>::value,
                  "PlatformDefaultOptions alias should map to NilOptions on native");
    #endif

        auto dotstarDefaults = lw::factory::resolveProtocolSettings<DotStarDesc>(lw::factory::DotStarOptions{});
        TEST_ASSERT_EQUAL_PTR(lw::ChannelOrder::BGR::value, dotstarDefaults.channelOrder);

        auto wsDefaults = lw::factory::resolveProtocolSettings<Ws2812xDesc>(lw::factory::Ws2812xOptions{});
        TEST_ASSERT_EQUAL_PTR(lw::ChannelOrder::GRB::value, wsDefaults.channelOrder);
    }

    void test_descriptor_traits_default_mapping_with_nil_transport(void)
    {
        using ProtocolTraits = lw::factory::ProtocolDescriptorTraits<lw::factory::descriptors::APA102>;
        using TransportTraits = lw::factory::TransportDescriptorTraits<lw::factory::descriptors::Nil>;

        static_assert(std::is_same<typename ProtocolTraits::ProtocolType, lw::DotStarProtocol>::value,
                      "Protocol descriptor should resolve to concrete protocol type");
        static_assert(std::is_same<typename TransportTraits::TransportType, lw::NilTransport>::value,
                      "Transport descriptor should resolve to concrete transport type");

        auto bus = lw::factory::makeBus<lw::factory::descriptors::APA102, lw::factory::descriptors::Nil>(
            60,
            lw::NilTransportSettings{});
        TEST_ASSERT_EQUAL_UINT32(60U, static_cast<uint32_t>(bus.pixelCount()));
    }

    void test_platform_default_descriptor_maps_and_constructs_on_native(void)
    {
#if defined(ARDUINO_ARCH_NATIVE)
        auto bus = lw::factory::makeBus<lw::factory::descriptors::APA102, lw::factory::descriptors::PlatformDefault>(
            24,
            lw::factory::PlatformDefaultOptions{});

        TEST_ASSERT_EQUAL_UINT32(24U, static_cast<uint32_t>(bus.pixelCount()));
#else
        TEST_ASSERT_TRUE(true);
#endif
    }

    void test_descriptor_factory_explicit_protocol_and_transport_config(void)
    {
        using DotStarBus = lw::factory::Bus<lw::factory::descriptors::APA102, lw::factory::descriptors::Nil>;
        using WsProtocol = typename lw::factory::ProtocolDescriptorTraits<lw::factory::descriptors::Ws2812>::ProtocolType;
        using PlatformDefaultTransport = typename lw::factory::TransportDescriptorTraits<lw::factory::descriptors::PlatformDefault>::TransportType;
        using WsGammaProtocol = lw::WithOwnedShader<typename WsProtocol::ColorType,
                                                     lw::GammaShader<typename WsProtocol::ColorType>,
                                                     WsProtocol>;
        using WsShadedBus = lw::factory::Bus<lw::factory::descriptors::Ws2812,
                                              lw::factory::descriptors::PlatformDefault,
                                              lw::GammaShader>;
        using WsShadedExpected = lw::StaticBusDriverPixelBusT<lw::OneWireWrapper<PlatformDefaultTransport>,
                                                                WsGammaProtocol>;

        auto bus = lw::factory::makeBus<lw::factory::descriptors::APA102, lw::factory::descriptors::Nil>(
            16,
            lw::DotStarProtocol::SettingsType{},
            lw::NilTransportSettings{});

        static_assert(std::is_same<DotStarBus, decltype(bus)>::value,
                      "Bus alias should match makeBus return type for direct descriptor-compatible transports");
        static_assert(std::is_same<WsShadedBus, WsShadedExpected>::value,
                      "Bus alias with shader template should deduce color-bound shader protocol and wrapped transport");

        TEST_ASSERT_EQUAL_UINT32(16U, static_cast<uint32_t>(bus.pixelCount()));
    }

    void test_dotstar_descriptor_parallel_options_config(void)
    {
        using ProtocolTraits = lw::factory::ProtocolDescriptorTraits<lw::factory::descriptors::DotStar<>>;

        static_assert(std::is_same<typename ProtocolTraits::ProtocolType, lw::DotStarProtocol>::value,
                      "DotStar descriptor should resolve to DotStarProtocol");

        lw::factory::DotStarOptions protocolOptions{};
        protocolOptions.channelOrder = lw::ChannelOrder::RGB::value;

        auto bus = lw::factory::makeBus<lw::factory::descriptors::DotStar<>, lw::factory::descriptors::Nil>(
            8,
            protocolOptions,
            lw::factory::NilOptions{});

        TEST_ASSERT_EQUAL_UINT32(8U, static_cast<uint32_t>(bus.pixelCount()));
    }

    void test_ws2812x_descriptor_parallel_options_config(void)
    {
        using ProtocolDesc = lw::factory::descriptors::Ws2812x<lw::Rgb8Color>;
        using ProtocolTraits = lw::factory::ProtocolDescriptorTraits<ProtocolDesc>;

        static_assert(std::is_same<typename ProtocolTraits::ProtocolType, lw::Ws2812xProtocol<lw::Rgb8Color>>::value,
                      "Ws2812x descriptor should resolve to Ws2812xProtocol<TColor>");

        lw::factory::Ws2812xOptions protocolOptions{};
        protocolOptions.channelOrder = lw::ChannelOrder::GRB::value;

        auto settings = lw::factory::resolveProtocolSettings<ProtocolDesc>(protocolOptions);

        TEST_ASSERT_EQUAL_PTR(lw::ChannelOrder::GRB::value, settings.channelOrder);
    }

    void test_ws2812x_alias_default_timing_flows_into_transport_settings(void)
    {
        using ProtocolDesc = lw::factory::descriptors::Ws2811;

        struct ClockedSettings
        {
            uint32_t clockRateHz{0};
            uint32_t baudRate{0};
        };

        auto protocolSettings = lw::factory::resolveProtocolSettings<ProtocolDesc>(
            lw::factory::ProtocolDescriptorTraits<ProtocolDesc>::defaultSettings());

        TEST_ASSERT_EQUAL_UINT32(lw::timing::Ws2811.t0hNs, protocolSettings.timing.t0hNs);
        TEST_ASSERT_EQUAL_UINT32(lw::timing::Ws2811.t0lNs, protocolSettings.timing.t0lNs);
        TEST_ASSERT_EQUAL_UINT32(lw::timing::Ws2811.t1hNs, protocolSettings.timing.t1hNs);
        TEST_ASSERT_EQUAL_UINT32(lw::timing::Ws2811.t1lNs, protocolSettings.timing.t1lNs);
        TEST_ASSERT_EQUAL_UINT32(lw::timing::Ws2811.resetNs, protocolSettings.timing.resetNs);

        ClockedSettings transportSettings{};
        lw::factory::ProtocolDescriptorTraits<ProtocolDesc>::mutateTransportSettings(
            10,
            protocolSettings,
            transportSettings);

        const uint32_t expectedEncodedRate = lw::timing::Ws2811.encodedDataRateHz();
        TEST_ASSERT_EQUAL_UINT32(expectedEncodedRate, transportSettings.clockRateHz);
        TEST_ASSERT_EQUAL_UINT32(expectedEncodedRate, transportSettings.baudRate);
    }

    void test_protocol_channel_order_normalization_for_five_channel_cw(void)
    {
        using WsCwDesc = lw::factory::descriptors::Ws2812x<lw::Rgbcw8Color, lw::OneWireTransportTag, lw::ChannelOrder::GRBCW>;
        using Defaults = lw::factory::ProtocolDescriptorTraitDefaults<lw::Ws2812xProtocol<lw::Rgbcw8Color>::SettingsType>;

        lw::factory::Ws2812xOptions wsOptions{};
        wsOptions.channelOrder = lw::ChannelOrder::GRB::value;
        auto wsSettings = lw::factory::resolveProtocolSettings<WsCwDesc>(wsOptions);
        TEST_ASSERT_EQUAL_PTR(lw::ChannelOrder::GRBCW::value, wsSettings.channelOrder);

        const char *coerced = Defaults::normalizeChannelOrder<lw::Rgbcw8Color>(
            lw::ChannelOrder::BGRW::value,
            lw::ChannelOrder::RGBCW::value);
        TEST_ASSERT_EQUAL_PTR(lw::ChannelOrder::BGRCW::value, coerced);
    }

    void test_dotstar_templated_options_default_channel_order(void)
    {
        lw::factory::DotStarOptionsT<lw::ChannelOrder::RGB> options{};
        TEST_ASSERT_EQUAL_PTR(lw::ChannelOrder::RGB::value, options.channelOrder);

        lw::factory::DotStarOptionsT<lw::ChannelOrder::RGBW> rgbwOptions{};
        TEST_ASSERT_EQUAL_PTR(lw::ChannelOrder::RGBW::value, rgbwOptions.channelOrder);

        lw::factory::DotStarOptionsT<lw::ChannelOrder::GRBW> grbwOptions{};
        TEST_ASSERT_EQUAL_PTR(lw::ChannelOrder::GRBW::value, grbwOptions.channelOrder);

        lw::factory::DotStarOptionsT<lw::ChannelOrder::BGRW> bgrwOptions{};
        TEST_ASSERT_EQUAL_PTR(lw::ChannelOrder::BGRW::value, bgrwOptions.channelOrder);
    }

    void test_onewirewrapper_timing_first_overloads_compile_and_construct(void)
    {
        using Ws2812xDesc = lw::factory::descriptors::Ws2812x<>;
        using NilDesc = lw::factory::descriptors::Nil;
        using WrappedBus = lw::factory::Bus<Ws2812xDesc, NilDesc>;

        static_assert(IsDetected<MakeBusExpr<Ws2812xDesc, NilDesc, lw::OneWireTiming, lw::NilTransportSettings>>::value,
                      "Timing-first protocol-omitted one-wire makeBus overload should be available");
        static_assert(IsDetected<MakeBusExpr<Ws2812xDesc, NilDesc, lw::factory::Ws2812xOptions, lw::OneWireTiming, lw::NilTransportSettings>>::value,
                      "Timing-first explicit-protocol one-wire makeBus overload should be available");

        auto omittedProtocolBus = lw::factory::makeBus<Ws2812xDesc, NilDesc>(
            24,
            lw::OneWireTiming::Ws2812x,
            lw::NilTransportSettings{});

        lw::factory::Ws2812xOptions wsOptions{};
        wsOptions.channelOrder = lw::ChannelOrder::GRB::value;

        auto explicitProtocolBus = lw::factory::makeBus<Ws2812xDesc, NilDesc>(
            12,
            wsOptions,
            lw::OneWireTiming::Ws2812x,
            lw::NilTransportSettings{});

        static_assert(std::is_same<WrappedBus, decltype(omittedProtocolBus)>::value,
                      "Bus alias should match makeBus return type for wrapped one-wire descriptor paths");
        static_assert(std::is_same<WrappedBus, decltype(explicitProtocolBus)>::value,
                      "Bus alias should match explicit-protocol wrapped one-wire makeBus return type");

        TEST_ASSERT_EQUAL_UINT32(24U, static_cast<uint32_t>(omittedProtocolBus.pixelCount()));
        TEST_ASSERT_EQUAL_UINT32(12U, static_cast<uint32_t>(explicitProtocolBus.pixelCount()));
    }

    void test_invalid_protocol_transport_combinations_not_detected(void)
    {
        using Ws2812xDesc = lw::factory::descriptors::Ws2812x<>;
        using DotStarDesc = lw::factory::descriptors::DotStar<>;
        using NilDesc = lw::factory::descriptors::Nil;

        using WsTraits = lw::factory::ProtocolDescriptorTraits<Ws2812xDesc>;
        using DotTraits = lw::factory::ProtocolDescriptorTraits<DotStarDesc>;
        using NilTraits = lw::factory::TransportDescriptorTraits<NilDesc>;

        using WsProtocol = typename WsTraits::ProtocolType;
        using DotProtocol = typename DotTraits::ProtocolType;
        using NilTransport = typename NilTraits::TransportType;
        using WrappedNilTransport = lw::OneWireWrapper<NilTransport>;

        static_assert(!lw::factory::DescriptorCapabilityCompatible<Ws2812xDesc, NilDesc, WsProtocol, NilTransport>,
                      "One-wire protocol must not be directly compatible with plain transport category");
        static_assert(lw::factory::DescriptorWrappedOneWireCapabilityCompatible<Ws2812xDesc, NilDesc, WsProtocol, NilTransport>,
                      "One-wire protocol must use wrapped one-wire compatibility path");
        static_assert(!lw::BusDriverProtocolTransportCompatible<WsProtocol, NilTransport>,
                      "One-wire protocol must not bind directly to non-one-wire transport");
        static_assert(lw::BusDriverProtocolTransportCompatible<WsProtocol, WrappedNilTransport>,
                      "One-wire protocol must bind when transport is wrapped as one-wire");

        static_assert(lw::factory::DescriptorCapabilityCompatible<DotStarDesc, NilDesc, DotProtocol, NilTransport>,
                      "DotStar must be directly compatible with plain transport category");
        static_assert(!lw::factory::DescriptorWrappedOneWireCapabilityCompatible<DotStarDesc, NilDesc, DotProtocol, NilTransport>,
                      "DotStar must not require one-wire wrapped compatibility path");

        TEST_ASSERT_TRUE(true);
    }

    void test_shader_descriptor_traits_and_factory_compile_construct(void)
    {
        using GammaDesc = lw::factory::descriptors::Gamma<>;
        using CurrentLimiterDesc = lw::factory::descriptors::CurrentLimiter<>;
        using MyShader = lw::factory::Shader<lw::Rgbw8Color,
                                              lw::factory::Gamma,
                                              lw::factory::WhiteBalance,
                                              lw::factory::CurrentLimiter>;
        using MyShaderExpected = lw::OwningAggregateShaderT<lw::Rgbw8Color,
                                                             lw::GammaShader<lw::Rgbw8Color>,
                                                             lw::WhiteBalanceShader<lw::Rgbw8Color>,
                                                             lw::CurrentLimiterShader<lw::Rgbw8Color>>;
        using SingleShader = lw::factory::Shader<lw::Rgb8Color, lw::factory::Gamma>;

        using GammaTraits = lw::factory::ShaderDescriptorTraits<GammaDesc>;
        using CurrentLimiterTraits = lw::factory::ShaderDescriptorTraits<CurrentLimiterDesc>;

        static_assert(std::is_same<typename GammaTraits::ShaderType, lw::GammaShader<lw::Rgb8Color>>::value,
                      "Gamma descriptor should resolve to GammaShader<TColor>");
        static_assert(std::is_same<typename CurrentLimiterTraits::ShaderType, lw::CurrentLimiterShader<lw::Rgb8Color>>::value,
                      "CurrentLimiter descriptor should resolve to CurrentLimiterShader<TColor>");
        static_assert(std::is_same<MyShader, MyShaderExpected>::value,
                  "Shader helper should resolve aggregate shader type from color and shader descriptor aliases");
        static_assert(std::is_same<SingleShader, lw::GammaShader<lw::Rgb8Color>>::value,
                  "Shader helper should resolve single shader type from color and shader descriptor alias");

        lw::factory::GammaOptions<> gammaOptions{};
        gammaOptions.gamma = 2.2f;
        gammaOptions.enableColorGamma = true;

        auto gammaShader = lw::factory::makeShader<GammaDesc>(gammaOptions);
        auto limiterShader = lw::factory::makeShader<CurrentLimiterDesc>(lw::factory::CurrentLimiterOptions<>{});

        static_assert(std::is_base_of<lw::IShader<lw::Rgb8Color>, decltype(gammaShader)>::value,
                      "makeShader<GammaDesc> should return IShader-compatible type");
        static_assert(std::is_base_of<lw::IShader<lw::Rgb8Color>, decltype(limiterShader)>::value,
                      "makeShader<CurrentLimiterDesc> should return IShader-compatible type");

        auto aggregate = lw::factory::makeShader(gammaShader, limiterShader);
        static_assert(std::is_base_of<lw::IShader<lw::Rgb8Color>, decltype(aggregate)>::value,
                      "makeShader(shader, shader) should return an aggregate IShader-compatible type");

        std::array<lw::Rgb8Color, 2> colors{
            lw::Rgb8Color{8, 16, 24},
            lw::Rgb8Color{32, 40, 48}};
        aggregate.apply(lw::span<lw::Rgb8Color>{colors.data(), colors.size()});
        TEST_ASSERT_TRUE(true);
    }

    void test_composite_bus_factories_compile_and_construct(void)
    {
        auto busA = lw::factory::makeBus<lw::factory::descriptors::APA102, lw::factory::descriptors::Nil>(
            2,
            lw::factory::NilOptions{});
        auto busB = lw::factory::makeBus<lw::factory::descriptors::APA102, lw::factory::descriptors::Nil>(
            2,
            lw::factory::NilOptions{});

        auto concat = lw::factory::makeBus(busA, busB);
        concat.begin();
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(concat.pixelBuffer().size()));

        lw::MosaicBusSettings mosaicConfig{};
        mosaicConfig.panelWidth = 1;
        mosaicConfig.panelHeight = 2;
        mosaicConfig.layout = lw::PanelLayout::RowMajor;
        mosaicConfig.tilesWide = 2;
        mosaicConfig.tilesHigh = 1;
        mosaicConfig.tileLayout = lw::PanelLayout::RowMajor;
        mosaicConfig.mosaicRotation = false;

        auto mosaic = lw::factory::makeBus(std::move(mosaicConfig), busA, busB);
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(mosaic.pixelCount()));
        TEST_ASSERT_EQUAL_UINT16(2U, mosaic.width());
        TEST_ASSERT_EQUAL_UINT16(2U, mosaic.height());
    }

    void test_composite_owner_factories_compile_and_construct(void)
    {
        auto busA = lw::factory::makeBus<lw::factory::descriptors::APA102, lw::factory::descriptors::Nil>(
            2,
            lw::factory::NilOptions{});
        auto busB = lw::factory::makeBus<lw::factory::descriptors::APA102, lw::factory::descriptors::Nil>(
            2,
            lw::factory::NilOptions{});

        auto staticConcat = lw::factory::makeStaticConcatBus(std::move(busA), std::move(busB));
        staticConcat.begin();
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(staticConcat.pixelBuffer().size()));

        auto mosaicBusA = lw::factory::makeBus<lw::factory::descriptors::APA102, lw::factory::descriptors::Nil>(
            2,
            lw::factory::NilOptions{});
        auto mosaicBusB = lw::factory::makeBus<lw::factory::descriptors::APA102, lw::factory::descriptors::Nil>(
            2,
            lw::factory::NilOptions{});

        lw::MosaicBusSettings mosaicConfig{};
        mosaicConfig.panelWidth = 1;
        mosaicConfig.panelHeight = 2;
        mosaicConfig.layout = lw::PanelLayout::RowMajor;
        mosaicConfig.tilesWide = 2;
        mosaicConfig.tilesHigh = 1;
        mosaicConfig.tileLayout = lw::PanelLayout::RowMajor;
        mosaicConfig.mosaicRotation = false;

        auto staticMosaic = lw::factory::makeStaticMosaicBus(std::move(mosaicConfig), std::move(mosaicBusA), std::move(mosaicBusB));
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(staticMosaic.pixelCount()));

        auto concatRootOwned = lw::factory::makeBus(
            std::initializer_list<uint16_t>{1, 2, 3},
            lw::factory::makeBus<lw::factory::descriptors::APA102, lw::factory::descriptors::Nil>(
                1,
                lw::factory::NilOptions{}),
            lw::factory::makeBus<lw::factory::descriptors::APA102, lw::factory::descriptors::Nil>(
                2,
                lw::factory::NilOptions{}),
            lw::factory::makeBus<lw::factory::descriptors::APA102, lw::factory::descriptors::Nil>(
                3,
                lw::factory::NilOptions{}));

        using StrandType = decltype(lw::factory::makeBus<lw::factory::descriptors::APA102, lw::factory::descriptors::Nil>(
            1,
            lw::factory::NilOptions{}));
        using MosaicAlias = lw::factory::MosaicBus<StrandType, StrandType, StrandType, StrandType, StrandType>;
        using MosaicExpected = lw::MosaicBus<typename StrandType::ColorType>;
        static_assert(std::is_same<MosaicAlias, MosaicExpected>::value,
                      "MosaicBus type helper should deduce to MosaicBus<BusColorType<TFirstBus>>");

        using ConcatAlias = lw::factory::ConcatBus<StrandType, StrandType, StrandType>;
        using ConcatExpected = lw::factory::RootOwnedConcatBusT<lw::factory::BusColorType<StrandType>,
                                     StrandType,
                                     StrandType,
                                     StrandType>;
        static_assert(std::is_same<ConcatAlias, ConcatExpected>::value,
                      "ConcatBus type helper should deduce to RootOwnedConcatBusT<BusColorType<TFirstBus>, TFirstBus, TOtherBuses...>");

        TEST_ASSERT_EQUAL_UINT32(6U, static_cast<uint32_t>(concatRootOwned.pixelCount()));
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
    RUN_TEST(test_descriptor_metadata_spike_shape);
    RUN_TEST(test_descriptor_traits_default_mapping_with_nil_transport);
    RUN_TEST(test_platform_default_descriptor_maps_and_constructs_on_native);
    RUN_TEST(test_descriptor_factory_explicit_protocol_and_transport_config);
    RUN_TEST(test_dotstar_descriptor_parallel_options_config);
    RUN_TEST(test_ws2812x_descriptor_parallel_options_config);
    RUN_TEST(test_ws2812x_alias_default_timing_flows_into_transport_settings);
    RUN_TEST(test_protocol_channel_order_normalization_for_five_channel_cw);
    RUN_TEST(test_dotstar_templated_options_default_channel_order);
    RUN_TEST(test_onewirewrapper_timing_first_overloads_compile_and_construct);
    RUN_TEST(test_invalid_protocol_transport_combinations_not_detected);
    RUN_TEST(test_shader_descriptor_traits_and_factory_compile_construct);
    RUN_TEST(test_composite_bus_factories_compile_and_construct);
    RUN_TEST(test_composite_owner_factories_compile_and_construct);
    return UNITY_END();
}
