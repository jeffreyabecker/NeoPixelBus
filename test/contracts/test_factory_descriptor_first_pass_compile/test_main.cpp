#include <unity.h>

#include <type_traits>

#include "buses/MakePixelBus.h"
#include "buses/PixelBus.h"
#include "colors/GammaShader.h"
#include "protocols/DotStarProtocol.h"
#include "protocols/ProtocolAliases.h"
#include "protocols/Ws2812xProtocol.h"
#include "transports/NilTransport.h"

namespace
{
    struct ClockedSettings
    {
        bool invert{false};
        uint32_t clockRateHz{0};

        static ClockedSettings normalize(ClockedSettings settings)
        {
            return settings;
        }
    };

    class ClockedTransport : public lw::ITransport
    {
    public:
        using TransportSettingsType = ClockedSettings;

        explicit ClockedTransport(TransportSettingsType settings = {})
            : settings_(settings)
        {
        }

        void begin() override
        {
        }

        void transmitBytes(lw::span<uint8_t>) override
        {
        }

        TransportSettingsType settings_{};
    };

    void test_make_pixel_bus_typed_protocol_transport(void)
    {
        using Protocol = lw::Apa102Protocol<lw::Rgb8Color>;
        using BusType = lw::PixelBus<Protocol, lw::NilTransport, lw::NilShader<lw::Rgb8Color>>;

        auto bus = lw::factory::makePixelBus<Protocol, lw::NilTransport>(
            16,
            lw::Apa102ProtocolSettings{},
            lw::NilTransportSettings{});

        static_assert(std::is_same<BusType, decltype(bus)>::value,
                      "makePixelBus should return typed PixelBus");
        TEST_ASSERT_EQUAL_UINT32(16U, static_cast<uint32_t>(bus.pixelCount()));
    }

    void test_make_pixel_bus_typed_with_shader(void)
    {
        using Protocol = lw::Apa102Protocol<lw::Rgb8Color>;
        using Shader = lw::GammaShader<lw::Rgb8Color>;

        auto bus = lw::factory::makePixelBus<Protocol, lw::NilTransport>(
            8,
            lw::Apa102ProtocolSettings{},
            lw::NilTransportSettings{},
            Shader{});

        static_assert(std::is_same<Shader, lw::remove_cvref_t<decltype(bus.shader())>>::value,
                      "Shader type should be preserved in PixelBus");
        TEST_ASSERT_EQUAL_UINT32(8U, static_cast<uint32_t>(bus.pixelCount()));
    }

    void test_make_pixel_bus_ws_alias_defaults_transport_rate(void)
    {
        using Alias = lw::protocols::Ws2812<>;

        auto bus = lw::factory::makePixelBus<Alias, ClockedTransport>(
            10,
            ClockedSettings{});

        TEST_ASSERT_EQUAL_UINT32(lw::timing::Generic800.encodedDataRateHz(),
                                 bus.transport().settings_.clockRateHz);
        TEST_ASSERT_FALSE(bus.transport().settings_.invert);
    }

    void test_make_pixel_bus_ws_alias_timing_override(void)
    {
        using Alias = lw::protocols::Ws2812<>;

        auto bus = lw::factory::makePixelBus<Alias, ClockedTransport>(
            12,
            lw::OneWireTiming::Ws2811,
            ClockedSettings{});

        TEST_ASSERT_EQUAL_UINT32(lw::timing::Ws2811.encodedDataRateHz(),
                                 bus.transport().settings_.clockRateHz);
    }

    void test_direct_pixel_bus_with_alias_spec(void)
    {
        using Alias = lw::protocols::APA102;

        lw::PixelBus<Alias, lw::NilTransport> bus(
            6,
            lw::NilTransportSettings{});

        auto &settings = static_cast<lw::Apa102ProtocolSettings &>(bus.protocol().settings());
        TEST_ASSERT_EQUAL_PTR(lw::ChannelOrder::BGR::value, settings.channelOrder);
    }

    void test_alias_type_is_direct_protocol(void)
    {
        static_assert(std::is_same<lw::protocols::Ws2812Type<lw::Rgb8Color>,
                                   lw::Ws2812xProtocol<lw::Rgb8Color, lw::Rgb8Color>>::value,
                      "Ws2812Type should resolve to direct protocol type");
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
    RUN_TEST(test_make_pixel_bus_typed_protocol_transport);
    RUN_TEST(test_make_pixel_bus_typed_with_shader);
    RUN_TEST(test_make_pixel_bus_ws_alias_defaults_transport_rate);
    RUN_TEST(test_make_pixel_bus_ws_alias_timing_override);
    RUN_TEST(test_direct_pixel_bus_with_alias_spec);
    RUN_TEST(test_alias_type_is_direct_protocol);
    return UNITY_END();
}
