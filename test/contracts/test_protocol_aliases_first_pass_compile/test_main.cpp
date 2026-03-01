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
        static_assert(std::is_same<typename WsDefault::ColorType, lw::Rgb8Color>::value,
                      "Ws2812x default descriptor should use Rgb8Color");
        static_assert(std::is_same<typename WsDefault::CapabilityRequirement, lw::OneWireTransportTag>::value,
                      "Ws2812x default descriptor should require OneWireTransportTag");
        static_assert(std::is_same<typename WsDefault::DefaultChannelOrder,
                                   lw::ChannelOrder::GRB>::value,
                      "Ws2812x default descriptor should default to GRB order");

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
