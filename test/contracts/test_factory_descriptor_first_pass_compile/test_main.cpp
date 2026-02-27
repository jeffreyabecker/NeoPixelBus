#include <unity.h>

#include <type_traits>

#include "factory/MakeBus.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/descriptors/TransportDescriptors.h"
#include "protocols/DotStarProtocol.h"
#include "transports/NilTransport.h"

namespace
{
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
        using ProtocolTraits = npb::factory::ProtocolDescriptorTraits<npb::factory::descriptors::DotStar>;

        static_assert(std::is_same<typename ProtocolTraits::ProtocolType, npb::DotStarProtocol>::value,
                      "DotStar descriptor should resolve to DotStarProtocol");

        npb::factory::DotStarOptions protocolOptions{};
        protocolOptions.channelOrder = npb::ChannelOrder::RGB;
        protocolOptions.mode = npb::DotStarMode::Luminance;

        auto bus = npb::factory::makeBus<npb::factory::descriptors::DotStar, npb::factory::descriptors::Nil>(
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
    RUN_TEST(test_descriptor_traits_default_mapping_with_nil_transport);
    RUN_TEST(test_descriptor_factory_explicit_protocol_and_transport_config);
    RUN_TEST(test_dotstar_descriptor_parallel_options_config);
    RUN_TEST(test_ws2812x_descriptor_parallel_options_config);
    RUN_TEST(test_dotstar_templated_options_default_channel_order);
    return UNITY_END();
}
