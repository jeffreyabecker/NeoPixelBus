#include <unity.h>

#include <type_traits>

#include "factory/descriptors/ProtocolDescriptors.h"

namespace
{
    void test_protocol_aliases_first_pass_compile(void)
    {
        static_assert(std::is_same<typename npb::factory::descriptors::APA102::ColorType, npb::Rgb8Color>::value,
                      "APA102 alias should default to Rgb8Color");
        static_assert(std::is_same<typename npb::factory::descriptors::APA102::CapabilityRequirement, npb::TransportTag>::value,
                      "APA102 alias should require TransportTag");
        static_assert(std::is_same<typename npb::factory::descriptors::APA102::DefaultChannelOrder,
                                   npb::factory::descriptors::ChannelOrderBGR>::value,
                      "APA102 alias should default to BGR order");

        using WsDefault = npb::factory::descriptors::Ws2812x<>;
        static_assert(std::is_same<typename WsDefault::ColorType, npb::Rgb8Color>::value,
                      "Ws2812x default descriptor should use Rgb8Color");
        static_assert(std::is_same<typename WsDefault::CapabilityRequirement, npb::OneWireTransportTag>::value,
                      "Ws2812x default descriptor should require OneWireTransportTag");
        static_assert(std::is_same<typename WsDefault::DefaultChannelOrder,
                                   npb::factory::descriptors::ChannelOrderGRB>::value,
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
