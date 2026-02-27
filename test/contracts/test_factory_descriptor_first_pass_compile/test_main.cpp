#include <unity.h>

#include <type_traits>

#include "factory/MakeBus.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/descriptors/TransportDescriptors.h"
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
        static_assert(std::is_same<typename DotStarDesc::DefaultChannelOrder, npb::factory::descriptors::ChannelOrderBGR>::value,
                      "DotStar descriptor should expose default channel order");

        static_assert(std::is_same<typename Ws2812xDesc::ColorType, npb::Rgb8Color>::value,
                      "Ws2812x descriptor should expose ColorType");
        static_assert(std::is_same<typename Ws2812xDesc::CapabilityRequirement, npb::OneWireTransportTag>::value,
                      "Ws2812x descriptor should expose one-wire capability requirement");
        static_assert(std::is_same<typename Ws2812xDesc::DefaultChannelOrder, npb::factory::descriptors::ChannelOrderGRB>::value,
                      "Ws2812x descriptor should expose default channel order");

        static_assert(std::is_same<typename npb::factory::descriptors::NeoSpi::Capability, npb::TransportTag>::value,
                      "NeoSpi descriptor should expose transport capability");
        static_assert(std::is_same<typename npb::factory::descriptors::RpPioOneWire::Capability, npb::OneWireTransportTag>::value,
                      "RpPioOneWire descriptor should expose one-wire capability");

        auto dotstarDefaults = npb::factory::resolveProtocolSettings<DotStarDesc>(npb::factory::DotStarOptions{});
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::BGR, dotstarDefaults.channelOrder);

        auto wsDefaults = npb::factory::resolveProtocolSettings<Ws2812xDesc>(npb::factory::Ws2812xOptions{});
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::GRB, wsDefaults.channelOrder);
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
        protocolOptions.channelOrder = npb::ChannelOrder::RGB;
        protocolOptions.mode = npb::DotStarMode::Luminance;

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
        protocolOptions.channelOrder = npb::ChannelOrder::GRB;

        auto settings = npb::factory::resolveProtocolSettings<ProtocolDesc>(protocolOptions);

        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::GRB, settings.channelOrder);
    }

    void test_dotstar_templated_options_default_channel_order(void)
    {
        npb::factory::DotStarOptionsT<npb::factory::DotStarChannelOrderRGB> options{};
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::RGB, options.channelOrder);

        npb::factory::DotStarOptionsT<npb::factory::DotStarChannelOrderRGBW> rgbwOptions{};
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::RGBW, rgbwOptions.channelOrder);

        npb::factory::DotStarOptionsT<npb::factory::DotStarChannelOrderGRBW> grbwOptions{};
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::GRBW, grbwOptions.channelOrder);

        npb::factory::DotStarOptionsT<npb::factory::DotStarChannelOrderBGRW> bgrwOptions{};
        TEST_ASSERT_EQUAL_PTR(npb::ChannelOrder::BGRW, bgrwOptions.channelOrder);
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
        wsOptions.channelOrder = npb::ChannelOrder::GRB;

        auto explicitProtocolBus = npb::factory::makeBus<Ws2812xDesc, NilDesc>(
            12,
            wsOptions,
            npb::OneWireTiming::Ws2812x,
            npb::NilTransportSettings{});

        TEST_ASSERT_EQUAL_UINT32(24U, static_cast<uint32_t>(omittedProtocolBus.pixelCount()));
        TEST_ASSERT_EQUAL_UINT32(12U, static_cast<uint32_t>(explicitProtocolBus.pixelCount()));
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
    RUN_TEST(test_descriptor_factory_explicit_protocol_and_transport_config);
    RUN_TEST(test_dotstar_descriptor_parallel_options_config);
    RUN_TEST(test_ws2812x_descriptor_parallel_options_config);
    RUN_TEST(test_dotstar_templated_options_default_channel_order);
    RUN_TEST(test_onewirewrapper_timing_first_overloads_compile_and_construct);
    return UNITY_END();
}
