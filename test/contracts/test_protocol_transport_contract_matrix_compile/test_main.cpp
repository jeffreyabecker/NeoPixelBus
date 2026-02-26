#include <unity.h>

#include <type_traits>
#include <utility>

#include "colors/Color.h"
#include "factory/ProtocolConfigs.h"
#include "factory/Traits.h"
#include "factory/TransportConfigs.h"
#include "protocols/DebugProtocol.h"
#include "protocols/DotStarProtocol.h"
#include "protocols/Hd108Protocol.h"
#include "protocols/IProtocol.h"
#include "protocols/Lpd6803Protocol.h"
#include "protocols/Lpd8806Protocol.h"
#include "protocols/NilProtocol.h"
#include "protocols/P9813Protocol.h"
#include "protocols/PixieProtocol.h"
#include "protocols/Sm16716Protocol.h"
#include "protocols/Sm168xProtocol.h"
#include "protocols/Tlc5947Protocol.h"
#include "protocols/Tlc59711Protocol.h"
#include "protocols/Tm1814Protocol.h"
#include "protocols/Tm1914Protocol.h"
#include "protocols/Ws2801Protocol.h"
#include "protocols/Ws2812xProtocol.h"
#include "transports/DebugTransport.h"
#include "transports/ITransport.h"
#include "transports/OneWireWrapper.h"
#include "transports/PrintTransport.h"

struct InvalidProtocolConfigMissingColorType
{
    using ProtocolType = npb::Ws2812xProtocol<npb::Rgb8Color>;
    typename ProtocolType::SettingsType settings{};
};

struct InvalidProtocolConfigColorMismatch
{
    using ProtocolType = npb::Ws2812xProtocol<npb::Rgb8Color>;
    using ColorType = npb::Rgbw8Color;
    typename ProtocolType::SettingsType settings{};
};

namespace npb::factory
{

    template <>
    struct ProtocolConfigTraits<InvalidProtocolConfigMissingColorType>
    {
        using ProtocolType = InvalidProtocolConfigMissingColorType::ProtocolType;

        static typename ProtocolType::SettingsType toSettings(InvalidProtocolConfigMissingColorType config)
        {
            return std::move(config.settings);
        }
    };

    template <>
    struct ProtocolConfigTraits<InvalidProtocolConfigColorMismatch>
    {
        using ProtocolType = InvalidProtocolConfigColorMismatch::ProtocolType;

        static typename ProtocolType::SettingsType toSettings(InvalidProtocolConfigColorMismatch config)
        {
            return std::move(config.settings);
        }
    };

} // namespace npb::factory

namespace
{
    template <typename TProtocol>
    constexpr void assertProtocolContracts()
    {
        static_assert(npb::ProtocolType<TProtocol>);
        static_assert(std::is_base_of<npb::IProtocol<typename TProtocol::ColorType>, TProtocol>::value);
        static_assert(npb::ProtocolPixelSettingsConstructible<TProtocol>);
        static_assert(npb::ProtocolSettingsTransportBindable<TProtocol>);
        static_assert(std::is_convertible<decltype(std::declval<const TProtocol &>().pixelCount()), uint16_t>::value);
    }

    template <typename TTransport>
    constexpr void assertTransportContracts()
    {
        static_assert(npb::TransportLike<TTransport>);
        static_assert(npb::SettingsConstructibleTransportLike<TTransport>);
    }

    using OneWireNilTransport = npb::OneWireTransport<npb::NilTransport>;

    constexpr bool runContractAssertions()
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
        static_assert(std::is_same<typename npb::factory::Ws2812::ColorType, npb::Rgb8Color>::value);
        static_assert(std::is_same<typename npb::factory::Ws2812::ColorType,
                       typename npb::factory::ProtocolConfigTraits<npb::factory::Ws2812>::ProtocolType::ColorType>::value);
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
        static_assert(!npb::factory::FactoryProtocolConfig<InvalidProtocolConfigMissingColorType>);
        static_assert(!npb::factory::FactoryProtocolConfig<InvalidProtocolConfigColorMismatch>);

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

