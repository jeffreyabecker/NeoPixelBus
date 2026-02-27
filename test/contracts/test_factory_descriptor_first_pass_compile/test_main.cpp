#include <unity.h>

#include <type_traits>

#include "factory/MakeBus.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/descriptors/TransportDescriptors.h"
#include "protocols/DotStarProtocol.h"
#include "transports/DebugTransport.h"

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
    return UNITY_END();
}
