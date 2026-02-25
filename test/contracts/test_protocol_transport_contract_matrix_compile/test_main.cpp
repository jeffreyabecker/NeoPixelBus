#include <unity.h>

#include <concepts>

#include "virtual/colors/Color.h"
#include "virtual/factory/ProtocolConfigs.h"
#include "virtual/factory/Traits.h"
#include "virtual/factory/TransportConfigs.h"
#include "virtual/protocols/DebugProtocol.h"
#include "virtual/protocols/DotStarProtocol.h"
#include "virtual/protocols/Hd108Protocol.h"
#include "virtual/protocols/IProtocol.h"
#include "virtual/protocols/Lpd6803Protocol.h"
#include "virtual/protocols/Lpd8806Protocol.h"
#include "virtual/protocols/NilProtocol.h"
#include "virtual/protocols/P9813Protocol.h"
#include "virtual/protocols/PixieProtocol.h"
#include "virtual/protocols/Sm16716Protocol.h"
#include "virtual/protocols/Sm168xProtocol.h"
#include "virtual/protocols/Tlc5947Protocol.h"
#include "virtual/protocols/Tlc59711Protocol.h"
#include "virtual/protocols/Tm1814Protocol.h"
#include "virtual/protocols/Tm1914Protocol.h"
#include "virtual/protocols/Ws2801Protocol.h"
#include "virtual/protocols/Ws2812xProtocol.h"
#include "virtual/transports/DebugTransport.h"
#include "virtual/transports/ITransport.h"
#include "virtual/transports/OneWireWrapper.h"
#include "virtual/transports/PrintTransport.h"

namespace
{
    template <typename TProtocol>
    consteval void assertProtocolContracts()
    {
        static_assert(npb::ProtocolType<TProtocol>);
        static_assert(std::derived_from<TProtocol, npb::IProtocol<typename TProtocol::ColorType>>);
        static_assert(npb::ProtocolPixelSettingsConstructible<TProtocol>);
        static_assert(npb::ProtocolSettingsTransportBindable<TProtocol>);
    }

    template <typename TTransport>
    consteval void assertTransportContracts()
    {
        static_assert(npb::TransportLike<TTransport>);
        static_assert(npb::SettingsConstructibleTransportLike<TTransport>);
    }

    using OneWireNilTransport = npb::OneWireTransport<npb::NilTransport>;

    consteval bool runContractAssertions()
    {
        static_assert(npb::TransportSettingsWithInvert<npb::NilTransportSettings>);
        static_assert(npb::TransportSettingsWithInvert<npb::PrintTransportSettings>);
        static_assert(npb::TransportSettingsWithInvert<npb::DebugTransportSettings>);
        static_assert(npb::TransportSettingsWithInvert<npb::DebugOneWireTransportSettings>);
        static_assert(npb::TransportSettingsWithInvert<npb::OneWireWrapperSettings<npb::NilTransportSettings>>);

        assertProtocolContracts<npb::DotStarProtocol>();
        assertProtocolContracts<npb::Hd108RgbProtocol>();
        assertProtocolContracts<npb::Ws2801Protocol>();
        assertProtocolContracts<npb::PixieProtocol>();
        assertProtocolContracts<npb::Lpd6803Protocol>();
        assertProtocolContracts<npb::Lpd8806Protocol>();
        assertProtocolContracts<npb::P9813Protocol>();
        assertProtocolContracts<npb::Sm168xProtocol<npb::Rgb8Color>>();
        assertProtocolContracts<npb::Sm16716Protocol>();
        assertProtocolContracts<npb::Tlc5947Protocol<npb::Rgb16Color>>();
        assertProtocolContracts<npb::Tlc59711Protocol>();
        assertProtocolContracts<npb::Tm1814Protocol>();
        assertProtocolContracts<npb::Tm1914Protocol>();
        assertProtocolContracts<npb::Ws2812xProtocol<npb::Rgb8Color>>();
        assertProtocolContracts<npb::NilProtocol<npb::Rgb8Color>>();
        assertProtocolContracts<npb::DebugProtocol<npb::Rgb8Color>>();

        assertTransportContracts<npb::NilTransport>();
        assertTransportContracts<npb::PrintTransport>();
        assertTransportContracts<npb::DebugTransport>();
        assertTransportContracts<npb::DebugOneWireTransport>();
        static_assert(!npb::TransportLike<OneWireNilTransport>);

        static_assert(npb::ProtocolTransportCompatible<npb::DotStarProtocol, npb::NilTransport>);
        static_assert(!npb::ProtocolTransportCompatible<npb::DotStarProtocol, OneWireNilTransport>);

        static_assert(npb::ProtocolTransportCompatible<npb::Ws2812xProtocol<npb::Rgb8Color>, npb::DebugOneWireTransport>);
        static_assert(!npb::ProtocolTransportCompatible<npb::Ws2812xProtocol<npb::Rgb8Color>, npb::NilTransport>);

        static_assert(npb::ProtocolTransportCompatible<npb::PixieProtocol, npb::DebugOneWireTransport>);
        static_assert(!npb::ProtocolTransportCompatible<npb::PixieProtocol, npb::NilTransport>);

        static_assert(npb::ProtocolTransportCompatible<npb::NilProtocol<npb::Rgb8Color>, npb::NilTransport>);
        static_assert(npb::ProtocolTransportCompatible<npb::NilProtocol<npb::Rgb8Color>, npb::DebugOneWireTransport>);
        static_assert(npb::ProtocolTransportCompatible<npb::DebugProtocol<npb::Rgb8Color>, npb::PrintTransport>);

        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Ws2812>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Sk6812>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Ucs8904>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Nil<npb::Rgb8Color>>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::DebugProtocolConfig<npb::Rgb8Color>>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::DotStar>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Hd108<npb::Rgb16Color>>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Lpd6803>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Lpd8806>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::P9813>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Pixie>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Sm16716>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Sm168x<npb::Rgb8Color>>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Tlc5947<npb::Rgb16Color>>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Tlc59711>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Tm1814>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Tm1914>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Ws2801>);
        static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Ws2812xRaw<npb::Rgb8Color>>);

        static_assert(npb::factory::FactoryTransportConfig<npb::factory::Debug>);
        static_assert(npb::factory::FactoryTransportConfig<npb::factory::NilTransportConfig>);
        static_assert(npb::factory::FactoryTransportConfig<npb::factory::PrintTransportConfig>);
        static_assert(npb::factory::FactoryTransportConfig<npb::factory::DebugTransportConfig>);
        static_assert(npb::factory::FactoryTransportConfig<npb::factory::DebugOneWireTransportConfig>);
        static_assert(npb::factory::FactoryTransportConfig<npb::factory::OneWire<npb::NilTransport>>);

        return true;
    }

    static_assert(runContractAssertions());

    void test_contract_matrix_compiles(void)
    {
        TEST_ASSERT_TRUE(true);
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
    RUN_TEST(test_contract_matrix_compiles);
    return UNITY_END();
}
