#include <unity.h>

#include <type_traits>

#include "factory/descriptors/ProtocolDescriptors.h"

namespace
{
    void test_protocol_aliases_first_pass_compile(void)
    {
        static_assert(std::is_same<typename lw::factory::descriptors::APA102::ColorType, lw::Rgb8Color>::value,
                      "APA102 alias should default to Rgb8Color");
        static_assert(std::is_same<typename lw::factory::descriptors::APA102::CapabilityRequirement, lw::TransportTag>::value,
                      "APA102 alias should require TransportTag");
        static_assert(std::is_same<typename lw::factory::descriptors::APA102::DefaultChannelOrder,
                                   lw::ChannelOrder::BGR>::value,
                      "APA102 alias should default to BGR order");

        using WsDefault = lw::factory::descriptors::Ws2812x<>;
        static_assert(std::is_same<typename WsDefault::ColorType, lw::Color>::value,
                  "Ws2812x default descriptor should use Color");
        static_assert(std::is_same<typename WsDefault::CapabilityRequirement, lw::OneWireTransportTag>::value,
                      "Ws2812x default descriptor should require OneWireTransportTag");
        static_assert(std::is_same<typename WsDefault::DefaultChannelOrder,
                                   lw::ChannelOrder::GRB>::value,
                      "Ws2812x default descriptor should default to GRB order");

        static_assert(std::is_same<typename lw::factory::descriptors::Ws2812B::DefaultChannelOrder, lw::ChannelOrder::GRB>::value,
                      "Ws2812B should default to GRB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2812C::DefaultChannelOrder, lw::ChannelOrder::GRB>::value,
                      "Ws2812C should default to GRB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2811::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Ws2811 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2811C::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Ws2811C should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2813::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Ws2813 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2813Rgbw::DefaultChannelOrder, lw::ChannelOrder::GRBW>::value,
                      "Ws2813-RGBW should default to GRBW");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2814::DefaultChannelOrder, lw::ChannelOrder::RGBW>::value,
                      "Ws2814 should default to RGBW");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2814A::DefaultChannelOrder, lw::ChannelOrder::WRGB>::value,
                      "Ws2814A should default to WRGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2814B::DefaultChannelOrder, lw::ChannelOrder::RGBW>::value,
                      "Ws2814B should default to RGBW");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2814C::DefaultChannelOrder, lw::ChannelOrder::RGBW>::value,
                      "Ws2814C should default to RGBW");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2815::DefaultChannelOrder, lw::ChannelOrder::GRB>::value,
                      "Ws2815 should default to GRB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2815B::DefaultChannelOrder, lw::ChannelOrder::GRB>::value,
                      "Ws2815B should default to GRB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2818::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Ws2818 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2818B::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Ws2818B should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ws2805::DefaultChannelOrder, lw::ChannelOrder::RGBCW>::value,
                      "Ws2805 should default to RGBCW");
        static_assert(std::is_same<typename lw::factory::descriptors::Sk6812::DefaultChannelOrder, lw::ChannelOrder::GRB>::value,
                      "Sk6812 should default to GRB");
        static_assert(std::is_same<typename lw::factory::descriptors::Sk6813::DefaultChannelOrder, lw::ChannelOrder::GRB>::value,
                      "Sk6813 should default to GRB");
        static_assert(std::is_same<typename lw::factory::descriptors::Sk6813Hv::DefaultChannelOrder, lw::ChannelOrder::GRB>::value,
                      "Sk6813HV should default to GRB");
        static_assert(std::is_same<typename lw::factory::descriptors::Apa107::DefaultChannelOrder, lw::ChannelOrder::GRB>::value,
                      "Apa107 should default to GRB");
        static_assert(std::is_same<typename lw::factory::descriptors::Hc2912c2020::DefaultChannelOrder, lw::ChannelOrder::GRB>::value,
                      "HC2912C-2020 should default to GRB");
        static_assert(std::is_same<typename lw::factory::descriptors::Sm16703P::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Sm16703P should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Sm16703Sp::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Sm16703SP should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Sm16704::DefaultChannelOrder, lw::ChannelOrder::RGBW>::value,
                      "Sm16704 should default to RGBW");
        static_assert(std::is_same<typename lw::factory::descriptors::Sm16704Pb::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Sm16704PB should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Gs8208B::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Gs8208B should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ucs1903::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Ucs1903 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ucs2903::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Ucs2903 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ucs2904::DefaultChannelOrder, lw::ChannelOrder::RGBW>::value,
                      "Ucs2904 should default to RGBW");
        static_assert(std::is_same<typename lw::factory::descriptors::Ucs5603::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Ucs5603 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ucs7604::DefaultChannelOrder, lw::ChannelOrder::RGBW>::value,
                      "Ucs7604 should default to RGBW");
        static_assert(std::is_same<typename lw::factory::descriptors::Ucs8903::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Ucs8903 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Ucs8904::DefaultChannelOrder, lw::ChannelOrder::RGBW>::value,
                      "Ucs8904 should default to RGBW");
        static_assert(std::is_same<typename lw::factory::descriptors::Ucs8904B::DefaultChannelOrder, lw::ChannelOrder::RGBW>::value,
                      "Ucs8904B should default to RGBW");
        static_assert(std::is_same<typename lw::factory::descriptors::Tm1814::DefaultChannelOrder, lw::ChannelOrder::WRGB>::value,
                      "Tm1814 should default to WRGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Tm1903::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Tm1903 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Tm1914::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Tm1914 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Tm1934::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Tm1934 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Tm1829::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Tm1829 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Lb1908::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Lb1908 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Tm1803::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Tm1803 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Tm1804::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Tm1804 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Tm1809::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Tm1809 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Cs8812::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Cs8812 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Gs8206::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Gs8206 should default to RGB");
        static_assert(std::is_same<typename lw::factory::descriptors::Gs8208::DefaultChannelOrder, lw::ChannelOrder::RGB>::value,
                      "Gs8208 should default to RGB");

        TEST_ASSERT_TRUE(true);
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
    RUN_TEST(test_protocol_aliases_first_pass_compile);
    return UNITY_END();
}
