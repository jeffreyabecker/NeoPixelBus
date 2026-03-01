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
    using MakeBusExpr = decltype(npb::factory::makeBus<TProtocolDesc, TTransportDesc>(std::declval<uint16_t>(), std::declval<TArgs>()...));

    void test_descriptor_metadata_spike_shape(void)
    {
        using DotStarDesc = npb::factory::descriptors::DotStar<>;
        using Ws2812xDesc = npb::factory::descriptors::Ws2812x<>;

        static_assert(std::is_same<typename DotStarDesc::ColorType, npb::Rgb8Color>::value,
                      "DotStar descriptor should expose ColorType");
        static_assert(std::is_same<typename DotStarDesc::CapabilityRequirement, npb::TransportTag>::value,
                      "DotStar descriptor should expose transport capability requirement");
        static_assert(std::is_same<typename DotStarDesc::DefaultChannelOrder, npb::ChannelOrder::BGR>::value,
                      "DotStar descriptor should expose default channel order");

        static_assert(std::is_same<typename Ws2812xDesc::ColorType, npb::Rgb8Color>::value,
                      "Ws2812x descriptor should expose ColorType");
        static_assert(std::is_same<typename Ws2812xDesc::CapabilityRequirement, npb::OneWireTransportTag>::value,
                      "Ws2812x descriptor should expose one-wire capability requirement");
        static_assert(std::is_same<typename Ws2812xDesc::DefaultChannelOrder, npb::ChannelOrder::GRB>::value,
                      "Ws2812x descriptor should expose default channel order");
        static_assert(std::is_same<typename npb::factory::descriptors::Ws2812x<npb::Rgbcw8Color, npb::OneWireTransportTag, npb::ChannelOrder::GRBCW>::DefaultChannelOrder,
                       npb::ChannelOrder::GRBCW>::value,
                  "Ws2812x 5-channel descriptor should support GRBCW default order");

        static_assert(std::is_same<typename npb::factory::descriptors::NeoSpi::Capability, npb::TransportTag>::value,
                      "NeoSpi descriptor should expose transport capability");
        static_assert(std::is_same<typename npb::factory::descriptors::RpPio::Capability, npb::TransportTag>::value,
                  "RpPio descriptor should expose transport capability");
        static_assert(std::is_same<typename npb::factory::descriptors::PlatformDefault::Capability, npb::TransportTag>::value,
                  "PlatformDefault descriptor should expose transport capability");

    #if defined(ARDUINO_ARCH_NATIVE)
        static_assert(std::is_same<npb::factory::descriptors::PlatformDefault, npb::factory::descriptors::Nil>::value,
                  "PlatformDefault descriptor alias should map to Nil on native");
        static_assert(std::is_same<npb::factory::PlatformDefaultOptions, npb::factory::NilOptions>::value,
                  "PlatformDefaultOptions alias should map to NilOptions on native");
    #endif

        auto dotstarDefaults = npb::factory::resolveProtocolSettings<DotStarDesc>(npb::factory::DotStarOptions{});
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::BGR::value, dotstarDefaults.channelOrder);

        auto wsDefaults = npb::factory::resolveProtocolSettings<Ws2812xDesc>(npb::factory::Ws2812xOptions{});
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::GRB::value, wsDefaults.channelOrder);
    }

    void test_descriptor_traits_default_mapping_with_nil_transport(void)
    {
        using ProtocolTraits = npb::factory::ProtocolDescriptorTraits<npb::factory::descriptors::APA102>;
        using TransportTraits = npb::factory::TransportDescriptorTraits<npb::factory::descriptors::Nil>;

        static_assert(std::is_same<typename ProtocolTraits::ProtocolType, npb::DotStarProtocol>::value,
                      "Protocol descriptor should resolve to concrete protocol type");
        static_assert(std::is_same<typename TransportTraits::TransportType, npb::NilTransport>::value,
                      "Transport descriptor should resolve to concrete transport type");

        auto bus = npb::factory::makeBus<npb::factory::descriptors::APA102, npb::factory::descriptors::Nil>(
            60,
            npb::NilTransportSettings{});
        TEST_ASSERT_EQUAL_UINT32(60U, static_cast<uint32_t>(bus.pixelCount()));
    }

    void test_platform_default_descriptor_maps_and_constructs_on_native(void)
    {
#if defined(ARDUINO_ARCH_NATIVE)
        auto bus = npb::factory::makeBus<npb::factory::descriptors::APA102, npb::factory::descriptors::PlatformDefault>(
            24,
            npb::factory::PlatformDefaultOptions{});

        TEST_ASSERT_EQUAL_UINT32(24U, static_cast<uint32_t>(bus.pixelCount()));
#else
        TEST_ASSERT_TRUE(true);
#endif
    }

    void test_descriptor_factory_explicit_protocol_and_transport_config(void)
    {
        auto bus = npb::factory::makeBus<npb::factory::descriptors::APA102, npb::factory::descriptors::Nil>(
            16,
            npb::DotStarProtocol::SettingsType{},
            npb::NilTransportSettings{});

        TEST_ASSERT_EQUAL_UINT32(16U, static_cast<uint32_t>(bus.pixelCount()));
    }

    void test_dotstar_descriptor_parallel_options_config(void)
    {
        using ProtocolTraits = npb::factory::ProtocolDescriptorTraits<npb::factory::descriptors::DotStar<>>;

        static_assert(std::is_same<typename ProtocolTraits::ProtocolType, npb::DotStarProtocol>::value,
                      "DotStar descriptor should resolve to DotStarProtocol");

        npb::factory::DotStarOptions protocolOptions{};
        protocolOptions.channelOrder = npb::ChannelOrder::RGB::value;

        auto bus = npb::factory::makeBus<npb::factory::descriptors::DotStar<>, npb::factory::descriptors::Nil>(
            8,
            protocolOptions,
            npb::factory::NilOptions{});

        TEST_ASSERT_EQUAL_UINT32(8U, static_cast<uint32_t>(bus.pixelCount()));
    }

    void test_ws2812x_descriptor_parallel_options_config(void)
    {
        using ProtocolDesc = npb::factory::descriptors::Ws2812x<npb::Rgb8Color>;
        using ProtocolTraits = npb::factory::ProtocolDescriptorTraits<ProtocolDesc>;

        static_assert(std::is_same<typename ProtocolTraits::ProtocolType, npb::Ws2812xProtocol<npb::Rgb8Color>>::value,
                      "Ws2812x descriptor should resolve to Ws2812xProtocol<TColor>");

        npb::factory::Ws2812xOptions protocolOptions{};
        protocolOptions.channelOrder = npb::ChannelOrder::GRB::value;

        auto settings = npb::factory::resolveProtocolSettings<ProtocolDesc>(protocolOptions);

        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::GRB::value, settings.channelOrder);
    }

    void test_protocol_channel_order_normalization_for_five_channel_cw(void)
    {
        using WsCwDesc = npb::factory::descriptors::Ws2812x<npb::Rgbcw8Color, npb::OneWireTransportTag, npb::ChannelOrder::GRBCW>;
        using Defaults = npb::factory::ProtocolDescriptorTraitDefaults<npb::Ws2812xProtocol<npb::Rgbcw8Color>::SettingsType>;

        npb::factory::Ws2812xOptions wsOptions{};
        wsOptions.channelOrder = npb::ChannelOrder::GRB::value;
        auto wsSettings = npb::factory::resolveProtocolSettings<WsCwDesc>(wsOptions);
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::GRBCW::value, wsSettings.channelOrder);

        const char *coerced = Defaults::normalizeChannelOrder<npb::Rgbcw8Color>(
            npb::ChannelOrder::BGRW::value,
            npb::ChannelOrder::RGBCW::value);
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::BGRCW::value, coerced);
    }

    void test_dotstar_templated_options_default_channel_order(void)
    {
        npb::factory::DotStarOptionsT<npb::ChannelOrder::RGB> options{};
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::RGB::value, options.channelOrder);

        npb::factory::DotStarOptionsT<npb::ChannelOrder::RGBW> rgbwOptions{};
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::RGBW::value, rgbwOptions.channelOrder);

        npb::factory::DotStarOptionsT<npb::ChannelOrder::GRBW> grbwOptions{};
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::GRBW::value, grbwOptions.channelOrder);

        npb::factory::DotStarOptionsT<npb::ChannelOrder::BGRW> bgrwOptions{};
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::BGRW::value, bgrwOptions.channelOrder);
    }

    void test_onewirewrapper_timing_first_overloads_compile_and_construct(void)
    {
        using Ws2812xDesc = npb::factory::descriptors::Ws2812x<>;
        using NilDesc = npb::factory::descriptors::Nil;

        static_assert(IsDetected<MakeBusExpr<Ws2812xDesc, NilDesc, npb::OneWireTiming, npb::NilTransportSettings>>::value,
                      "Timing-first protocol-omitted one-wire makeBus overload should be available");
        static_assert(IsDetected<MakeBusExpr<Ws2812xDesc, NilDesc, npb::factory::Ws2812xOptions, npb::OneWireTiming, npb::NilTransportSettings>>::value,
                      "Timing-first explicit-protocol one-wire makeBus overload should be available");

        auto omittedProtocolBus = npb::factory::makeBus<Ws2812xDesc, NilDesc>(
            24,
            npb::OneWireTiming::Ws2812x,
            npb::NilTransportSettings{});

        npb::factory::Ws2812xOptions wsOptions{};
        wsOptions.channelOrder = npb::ChannelOrder::GRB::value;

        auto explicitProtocolBus = npb::factory::makeBus<Ws2812xDesc, NilDesc>(
            12,
            wsOptions,
            npb::OneWireTiming::Ws2812x,
            npb::NilTransportSettings{});

        TEST_ASSERT_EQUAL_UINT32(24U, static_cast<uint32_t>(omittedProtocolBus.pixelCount()));
        TEST_ASSERT_EQUAL_UINT32(12U, static_cast<uint32_t>(explicitProtocolBus.pixelCount()));
    }

    void test_invalid_protocol_transport_combinations_not_detected(void)
    {
        using Ws2812xDesc = npb::factory::descriptors::Ws2812x<>;
        using DotStarDesc = npb::factory::descriptors::DotStar<>;
        using NilDesc = npb::factory::descriptors::Nil;

        using WsTraits = npb::factory::ProtocolDescriptorTraits<Ws2812xDesc>;
        using DotTraits = npb::factory::ProtocolDescriptorTraits<DotStarDesc>;
        using NilTraits = npb::factory::TransportDescriptorTraits<NilDesc>;

        using WsProtocol = typename WsTraits::ProtocolType;
        using DotProtocol = typename DotTraits::ProtocolType;
        using NilTransport = typename NilTraits::TransportType;
        using WrappedNilTransport = npb::OneWireWrapper<NilTransport>;

        static_assert(!npb::factory::DescriptorCapabilityCompatible<Ws2812xDesc, NilDesc, WsProtocol, NilTransport>,
                      "One-wire protocol must not be directly compatible with plain transport category");
        static_assert(npb::factory::DescriptorWrappedOneWireCapabilityCompatible<Ws2812xDesc, NilDesc, WsProtocol, NilTransport>,
                      "One-wire protocol must use wrapped one-wire compatibility path");
        static_assert(!npb::BusDriverProtocolTransportCompatible<WsProtocol, NilTransport>,
                      "One-wire protocol must not bind directly to non-one-wire transport");
        static_assert(npb::BusDriverProtocolTransportCompatible<WsProtocol, WrappedNilTransport>,
                      "One-wire protocol must bind when transport is wrapped as one-wire");

        static_assert(npb::factory::DescriptorCapabilityCompatible<DotStarDesc, NilDesc, DotProtocol, NilTransport>,
                      "DotStar must be directly compatible with plain transport category");
        static_assert(!npb::factory::DescriptorWrappedOneWireCapabilityCompatible<DotStarDesc, NilDesc, DotProtocol, NilTransport>,
                      "DotStar must not require one-wire wrapped compatibility path");

        TEST_ASSERT_TRUE(true);
    }

    void test_shader_descriptor_traits_and_factory_compile_construct(void)
    {
        using GammaDesc = npb::factory::descriptors::Gamma<>;
        using CurrentLimiterDesc = npb::factory::descriptors::CurrentLimiter<>;

        using GammaTraits = npb::factory::ShaderDescriptorTraits<GammaDesc>;
        using CurrentLimiterTraits = npb::factory::ShaderDescriptorTraits<CurrentLimiterDesc>;

        static_assert(std::is_same<typename GammaTraits::ShaderType, npb::GammaShader<npb::Rgb8Color>>::value,
                      "Gamma descriptor should resolve to GammaShader<TColor>");
        static_assert(std::is_same<typename CurrentLimiterTraits::ShaderType, npb::CurrentLimiterShader<npb::Rgb8Color>>::value,
                      "CurrentLimiter descriptor should resolve to CurrentLimiterShader<TColor>");

        npb::factory::GammaOptions<> gammaOptions{};
        gammaOptions.gamma = 2.2f;
        gammaOptions.enableColorGamma = true;

        auto gammaShader = npb::factory::makeShader<GammaDesc>(gammaOptions);
        auto limiterShader = npb::factory::makeShader<CurrentLimiterDesc>(npb::factory::CurrentLimiterOptions<>{});

        static_assert(std::is_base_of<npb::IShader<npb::Rgb8Color>, decltype(gammaShader)>::value,
                      "makeShader<GammaDesc> should return IShader-compatible type");
        static_assert(std::is_base_of<npb::IShader<npb::Rgb8Color>, decltype(limiterShader)>::value,
                      "makeShader<CurrentLimiterDesc> should return IShader-compatible type");

        auto aggregate = npb::factory::makeShader(gammaShader, limiterShader);
        static_assert(std::is_base_of<npb::IShader<npb::Rgb8Color>, decltype(aggregate)>::value,
                      "makeShader(shader, shader) should return an aggregate IShader-compatible type");

        std::array<npb::Rgb8Color, 2> colors{
            npb::Rgb8Color{8, 16, 24},
            npb::Rgb8Color{32, 40, 48}};
        aggregate.apply(npb::span<npb::Rgb8Color>{colors.data(), colors.size()});
        TEST_ASSERT_TRUE(true);
    }

    void test_composite_bus_factories_compile_and_construct(void)
    {
        auto busA = npb::factory::makeBus<npb::factory::descriptors::APA102, npb::factory::descriptors::Nil>(
            2,
            npb::factory::NilOptions{});
        auto busB = npb::factory::makeBus<npb::factory::descriptors::APA102, npb::factory::descriptors::Nil>(
            2,
            npb::factory::NilOptions{});

        auto concat = npb::factory::makeBus(busA, busB);
        concat.begin();
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(concat.pixelBuffer().size()));

        npb::MosaicBusSettings mosaicConfig{};
        mosaicConfig.panelWidth = 1;
        mosaicConfig.panelHeight = 2;
        mosaicConfig.layout = npb::PanelLayout::RowMajor;
        mosaicConfig.tilesWide = 2;
        mosaicConfig.tilesHigh = 1;
        mosaicConfig.tileLayout = npb::PanelLayout::RowMajor;
        mosaicConfig.mosaicRotation = false;

        auto mosaic = npb::factory::makeBus(std::move(mosaicConfig), busA, busB);
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(mosaic.pixelCount()));
        TEST_ASSERT_EQUAL_UINT16(2U, mosaic.width());
        TEST_ASSERT_EQUAL_UINT16(2U, mosaic.height());
    }

    void test_composite_owner_factories_compile_and_construct(void)
    {
        auto busA = npb::factory::makeBus<npb::factory::descriptors::APA102, npb::factory::descriptors::Nil>(
            2,
            npb::factory::NilOptions{});
        auto busB = npb::factory::makeBus<npb::factory::descriptors::APA102, npb::factory::descriptors::Nil>(
            2,
            npb::factory::NilOptions{});

        auto staticConcat = npb::factory::makeStaticConcatBus(std::move(busA), std::move(busB));
        staticConcat.begin();
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(staticConcat.pixelBuffer().size()));

        auto mosaicBusA = npb::factory::makeBus<npb::factory::descriptors::APA102, npb::factory::descriptors::Nil>(
            2,
            npb::factory::NilOptions{});
        auto mosaicBusB = npb::factory::makeBus<npb::factory::descriptors::APA102, npb::factory::descriptors::Nil>(
            2,
            npb::factory::NilOptions{});

        npb::MosaicBusSettings mosaicConfig{};
        mosaicConfig.panelWidth = 1;
        mosaicConfig.panelHeight = 2;
        mosaicConfig.layout = npb::PanelLayout::RowMajor;
        mosaicConfig.tilesWide = 2;
        mosaicConfig.tilesHigh = 1;
        mosaicConfig.tileLayout = npb::PanelLayout::RowMajor;
        mosaicConfig.mosaicRotation = false;

        auto staticMosaic = npb::factory::makeStaticMosaicBus(std::move(mosaicConfig), std::move(mosaicBusA), std::move(mosaicBusB));
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(staticMosaic.pixelCount()));

        auto concatRootOwned = npb::factory::makeBus(
            std::initializer_list<uint16_t>{1, 2, 3},
            npb::factory::makeBus<npb::factory::descriptors::APA102, npb::factory::descriptors::Nil>(
                1,
                npb::factory::NilOptions{}),
            npb::factory::makeBus<npb::factory::descriptors::APA102, npb::factory::descriptors::Nil>(
                2,
                npb::factory::NilOptions{}),
            npb::factory::makeBus<npb::factory::descriptors::APA102, npb::factory::descriptors::Nil>(
                3,
                npb::factory::NilOptions{}));

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
    RUN_TEST(test_protocol_channel_order_normalization_for_five_channel_cw);
    RUN_TEST(test_dotstar_templated_options_default_channel_order);
    RUN_TEST(test_onewirewrapper_timing_first_overloads_compile_and_construct);
    RUN_TEST(test_invalid_protocol_transport_combinations_not_detected);
    RUN_TEST(test_shader_descriptor_traits_and_factory_compile_construct);
    RUN_TEST(test_composite_bus_factories_compile_and_construct);
    RUN_TEST(test_composite_owner_factories_compile_and_construct);
    return UNITY_END();
}
