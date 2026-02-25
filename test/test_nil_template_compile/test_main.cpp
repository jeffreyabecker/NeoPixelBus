#include <unity.h>

#include <array>
#include <concepts>

#include "VirtualNeoPixelBus.h"

namespace
{
    using TestColor = npb::Rgbcw8Color;

    static_assert(std::derived_from<npb::NilProtocol<TestColor>, npb::IProtocol<TestColor>>);
    static_assert(npb::ProtocolPixelSettingsConstructible<npb::NilProtocol<TestColor>>);
    static_assert(npb::ProtocolSettingsTransportBindable<npb::NilProtocol<TestColor>>);
    static_assert(std::derived_from<npb::NilShader<TestColor>, npb::IShader<TestColor>>);
    static_assert(std::derived_from<npb::NilBusT<TestColor>, npb::IPixelBus<TestColor>>);

    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Ws2812>);
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Ws2812xRaw<npb::Rgb8Color>>);
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::DotStar>);
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Hd108<npb::Rgb16Color>>);
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Tlc5947<npb::Rgb16Color>>);
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Sm168x<npb::Rgb8Color>>);
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Tm1814>);

    static_assert(npb::factory::FactoryTransportConfig<npb::factory::Debug>);
    static_assert(npb::factory::FactoryTransportConfig<npb::factory::NilTransportConfig>);
    static_assert(npb::factory::FactoryTransportConfig<npb::factory::PrintTransportConfig>);
    static_assert(npb::factory::FactoryTransportConfig<npb::factory::DebugOneWireTransportConfig>);
    static_assert(npb::factory::FactoryTransportConfig<npb::factory::OneWire<npb::NilTransport>>);

    using Ws2812DebugBus = npb::factory::Bus<npb::factory::Ws2812, npb::factory::Debug>;
    static_assert(std::derived_from<Ws2812DebugBus, npb::IPixelBus<npb::Rgb8Color>>);

    void test_nil_types_compile_and_smoke(void)
    {
        npb::NilShader<TestColor> shader;
        std::array<TestColor, 2> colors{TestColor{1, 2, 3}, TestColor{4, 5, 6}};
        shader.apply(colors);

        npb::NilProtocol<TestColor> protocol(2, npb::NilProtocolSettings{});
        protocol.initialize();
        protocol.update(colors);

        auto owningBus = npb::factory::makeOwningDriverPixelBus<npb::NilTransport, npb::NilProtocol<TestColor>>(
            4,
            npb::NilTransportSettings{},
            npb::NilProtocolSettings{});

        owningBus.begin();
        owningBus.setPixelColor(0, TestColor{10, 11, 12});
        owningBus.show();
        TEST_ASSERT_TRUE(owningBus.canShow());
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(owningBus.pixelCount()));
    }

    void test_nil_protocol_shader_wrappers_compile_and_smoke(void)
    {
        npb::NilShader<TestColor> shader;

        using ShaderProtocol = npb::WithShader<TestColor, npb::NilProtocol<TestColor>>;
        ShaderProtocol::SettingsType shaderSettings{};
        shaderSettings.shader = shader;
        ShaderProtocol withShader(2, std::move(shaderSettings), npb::NilProtocolSettings{});

        std::array<TestColor, 1> colors{TestColor{7, 8, 9}};
        withShader.update(colors);
        TEST_ASSERT_TRUE(withShader.isReadyToUpdate());

        using EmbeddedShaderProtocol = npb::WithEmbeddedShader<TestColor,
                                                               npb::NilShader<TestColor>,
                                                               npb::NilProtocol<TestColor>>;
        EmbeddedShaderProtocol::SettingsType embeddedSettings{};
        embeddedSettings.shader = npb::NilShader<TestColor>{};
        EmbeddedShaderProtocol withEmbeddedShader(2, std::move(embeddedSettings), npb::NilProtocolSettings{});

        withEmbeddedShader.update(colors);
        TEST_ASSERT_TRUE(withEmbeddedShader.isReadyToUpdate());
    }

    void test_factory_make_bus_compile_and_smoke(void)
    {
        Ws2812DebugBus inferredBus = npb::factory::makeBus(
            4,
            npb::factory::Ws2812{.colorOrder = npb::ChannelOrder::GRB},
            npb::factory::Debug{.output = nullptr, .invert = false});
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(inferredBus.pixelCount()));

        Ws2812DebugBus explicitBus = npb::factory::makeBus<npb::factory::Ws2812, npb::factory::Debug>(8);
        TEST_ASSERT_EQUAL_UINT32(8U, static_cast<uint32_t>(explicitBus.pixelCount()));

        using ShaderFactoryType = decltype(npb::factory::makeAggregateShader(
            npb::factory::makeGammaShader({.gamma = 2.6f, .enableColorGamma = true, .enableBrightnessGamma = true}),
            npb::factory::makeCurrentLimiterShader(npb::factory::CurrentLimiterRgb{
                .maxMilliamps = 5000,
                .milliampsPerChannel = npb::factory::ChannelMilliamps{.R = 20, .G = 20, .B = 20},
                .controllerMilliamps = 50,
                .standbyMilliampsPerPixel = 1,
                .rgbwDerating = true,
            })));
        using Ws2812DebugShadedBus = npb::factory::Bus<npb::factory::Ws2812,
                                   npb::factory::Debug,
                                   ShaderFactoryType>;

        Ws2812DebugShadedBus shadedBus = npb::factory::makeBus(
            8,
            npb::factory::Ws2812{.colorOrder = npb::ChannelOrder::GRB},
            npb::factory::Debug{.output = nullptr, .invert = false},
            npb::factory::makeAggregateShader(
                npb::factory::makeGammaShader({.gamma = 2.6f, .enableColorGamma = true, .enableBrightnessGamma = true}),
                npb::factory::makeCurrentLimiterShader(npb::factory::CurrentLimiterRgb{
                    .maxMilliamps = 5000,
                    .milliampsPerChannel = npb::factory::ChannelMilliamps{.R = 20, .G = 20, .B = 20},
                    .controllerMilliamps = 50,
                    .standbyMilliampsPerPixel = 1,
                    .rgbwDerating = true,
                })));
        TEST_ASSERT_EQUAL_UINT32(8U, static_cast<uint32_t>(shadedBus.pixelCount()));
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
    RUN_TEST(test_nil_types_compile_and_smoke);
    RUN_TEST(test_nil_protocol_shader_wrappers_compile_and_smoke);
    RUN_TEST(test_factory_make_bus_compile_and_smoke);
    return UNITY_END();
}
