#include <unity.h>

#include <array>
#include <algorithm>
#include <string>
#include <type_traits>
#include <vector>

#include "NeoPixelBus.h"

namespace
{
    using TestColor = npb::Rgbcw8Color;

    struct WritableSink
    {
        size_t write(const uint8_t *data, size_t length)
        {
            if (data == nullptr || length == 0)
            {
                return 0;
            }

            bytes.insert(bytes.end(), data, data + length);
            return length;
        }

        std::string asString() const
        {
            return std::string(bytes.begin(), bytes.end());
        }

        std::vector<uint8_t> bytes{};
    };

    static_assert(npb::Writable<WritableSink>);

    struct NonWritableSink
    {
        int value{0};
    };

    static_assert(!npb::Writable<NonWritableSink>);

    static_assert(std::is_base_of<npb::IProtocol<TestColor>, npb::NilProtocol<TestColor>>::value);
    static_assert(npb::ProtocolPixelSettingsConstructible<npb::NilProtocol<TestColor>>);
    static_assert(npb::ProtocolSettingsTransportBindable<npb::NilProtocol<TestColor>>);
    static_assert(std::is_base_of<npb::IShader<TestColor>, npb::NilShader<TestColor>>::value);
    static_assert(std::is_base_of<npb::IPixelBus<TestColor>, npb::NilBusT<TestColor>>::value);

#if defined(NPB_HAS_CPP20_CONCEPTS)
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Ws2812>);
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Ws2812xRaw<npb::Rgb8Color>>);
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::DotStar>);
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Hd108<npb::Rgb16Color>>);
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Pixie>);
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Tlc5947<npb::Rgb16Color>>);
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Sm168x<npb::Rgb8Color>>);
    static_assert(npb::factory::FactoryProtocolConfig<npb::factory::Tm1814>);

    static_assert(npb::factory::FactoryTransportConfig<npb::factory::Debug>);
    static_assert(npb::factory::FactoryTransportConfig<npb::factory::NilTransportConfig>);
    static_assert(npb::factory::FactoryTransportConfig<npb::factory::PrintTransportConfig>);
    static_assert(npb::factory::FactoryTransportConfig<npb::factory::DebugOneWireTransportConfig>);
    static_assert(npb::factory::FactoryTransportConfig<npb::factory::OneWire<npb::NilTransport>>);
    static_assert(npb::factory::ShaderInstanceForColor<npb::GammaShader<npb::Rgb8Color>, npb::Rgb8Color>);

    using Ws2812DebugBus = npb::factory::Bus<npb::factory::Ws2812, npb::factory::Debug>;
    static_assert(std::is_base_of<npb::IPixelBus<npb::Rgb8Color>, Ws2812DebugBus>::value);

    using PixieDebugBus = npb::factory::Bus<npb::factory::Pixie, npb::factory::Debug>;
    static_assert(std::is_base_of<npb::IPixelBus<npb::Rgb8Color>, PixieDebugBus>::value);
#endif

    void test_nil_types_compile_and_smoke(void)
    {
        npb::NilShader<TestColor> shader;
        std::array<TestColor, 2> colors{TestColor{1, 2, 3}, TestColor{4, 5, 6}};
        shader.apply(colors);

        npb::NilProtocol<TestColor> protocol(2, npb::NilProtocolSettings{});
        protocol.initialize();
        protocol.update(colors);

        npb::NilBusT<TestColor> owningBus(4);

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
        shaderSettings.shader = &shader;
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

#if defined(NPB_HAS_CPP20_CONCEPTS)
    void test_factory_make_bus_compile_and_smoke(void)
    {
        Ws2812DebugBus inferredBus = npb::factory::makeBus(
            4,
            npb::factory::Ws2812{.colorOrder = npb::ChannelOrder::GRB},
            npb::factory::Debug{.output = nullptr, .invert = false});
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(inferredBus.pixelCount()));

        Ws2812DebugBus explicitBus = npb::factory::makeBus<npb::factory::Ws2812, npb::factory::Debug>(8);
        TEST_ASSERT_EQUAL_UINT32(8U, static_cast<uint32_t>(explicitBus.pixelCount()));

        PixieDebugBus pixieBus = npb::factory::makeBus(
            6,
            npb::factory::Pixie{.settings = npb::PixieProtocolSettings{.bus = nullptr, .channelOrder = npb::ChannelOrder::RGB}},
            npb::factory::Debug{.output = nullptr, .invert = false});
        TEST_ASSERT_EQUAL_UINT32(6U, static_cast<uint32_t>(pixieBus.pixelCount()));

        using ShaderFactoryType = decltype(npb::factory::makeShader(
            npb::factory::makeShader({.gamma = 2.6f, .enableColorGamma = true, .enableBrightnessGamma = true}),
            npb::factory::makeShader(npb::factory::CurrentLimiterRgb{
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
            npb::factory::makeShader(
                npb::factory::makeShader({.gamma = 2.6f, .enableColorGamma = true, .enableBrightnessGamma = true}),
                npb::factory::makeShader(npb::factory::CurrentLimiterRgb{
                    .maxMilliamps = 5000,
                    .milliampsPerChannel = npb::factory::ChannelMilliamps{.R = 20, .G = 20, .B = 20},
                    .controllerMilliamps = 50,
                    .standbyMilliampsPerPixel = 1,
                    .rgbwDerating = true,
                })));
        TEST_ASSERT_EQUAL_UINT32(8U, static_cast<uint32_t>(shadedBus.pixelCount()));

        using ShaderInstanceType = npb::GammaShader<npb::Rgb8Color>;
        using Ws2812DebugEmbeddedShaderBus = npb::factory::Bus<npb::factory::Ws2812,
                                                               npb::factory::Debug,
                                                               ShaderInstanceType>;

        Ws2812DebugEmbeddedShaderBus embeddedShaderBus = npb::factory::makeBus(
            10,
            npb::factory::Ws2812{.colorOrder = npb::ChannelOrder::GRB},
            npb::factory::Debug{.output = nullptr, .invert = false},
            ShaderInstanceType{typename ShaderInstanceType::SettingsType{.gamma = 2.2f, .enableColorGamma = true, .enableBrightnessGamma = false}});
        TEST_ASSERT_EQUAL_UINT32(10U, static_cast<uint32_t>(embeddedShaderBus.pixelCount()));

        using MyShader = npb::factory::Shader<npb::factory::Ws2812,
                                              npb::factory::Gamma,
                                              npb::factory::CurrentLimiter<npb::factory::Ws2812::ColorType>>;
        using MyBus = npb::factory::Bus<npb::factory::Ws2812,
                                        npb::factory::Debug,
                                        MyShader>;

        MyShader shader = npb::factory::makeShader(
            npb::factory::makeShader({.gamma = 2.6f, .enableColorGamma = true, .enableBrightnessGamma = true}),
            npb::factory::makeShader(npb::factory::CurrentLimiter<npb::factory::Ws2812::ColorType>{
                .maxMilliamps = 5000,
                .milliampsPerChannel = npb::factory::ChannelMilliamps{.R = 20, .G = 20, .B = 20},
                .controllerMilliamps = 50,
                .standbyMilliampsPerPixel = 1,
                .rgbwDerating = true,
            }));

        MyBus busFromShaderA = npb::factory::makeBus(
            60,
            npb::factory::Ws2812{.colorOrder = npb::ChannelOrder::GRB},
            npb::factory::Debug{.output = nullptr, .invert = false},
            shader);

        MyBus busFromShaderB = npb::factory::makeBus(
            60,
            npb::factory::Ws2812{.colorOrder = npb::ChannelOrder::GRB},
            npb::factory::Debug{.output = nullptr, .invert = false},
            shader);

        TEST_ASSERT_EQUAL_UINT32(60U, static_cast<uint32_t>(busFromShaderA.pixelCount()));
        TEST_ASSERT_EQUAL_UINT32(60U, static_cast<uint32_t>(busFromShaderB.pixelCount()));
    }
#endif

    void test_writable_template_consumers_compile_and_smoke(void)
    {
        WritableSink transportSink{};
        npb::PrintTransportT<WritableSink> printTransport(transportSink);
        const std::array<uint8_t, 3> payload{0x10, 0x20, 0x30};
        printTransport.transmitBytes(payload);
        TEST_ASSERT_EQUAL_UINT32(3U, static_cast<uint32_t>(transportSink.bytes.size()));

        WritableSink debugTransportSink{};
        npb::DebugTransportT<npb::NilTransport, npb::NilTransportSettings, WritableSink> debugTransport(debugTransportSink, false);
        debugTransport.begin();
        debugTransport.beginTransaction();
        debugTransport.transmitBytes(payload);
        debugTransport.endTransaction();
        const std::string debugTransportText = debugTransportSink.asString();
        TEST_ASSERT_TRUE(debugTransportText.find("[BUS] begin") != std::string::npos);
        TEST_ASSERT_TRUE(debugTransportText.find("[BUS] bytes(3)") != std::string::npos);

        WritableSink protocolSink{};
        npb::DebugProtocolSettingsT<TestColor, WritableSink> protocolSettings{};
        protocolSettings.output = &protocolSink;

        npb::DebugProtocol<TestColor, WritableSink> debugProtocol(2, std::move(protocolSettings));
        const std::array<TestColor, 1> colors{TestColor{1, 2, 3, 4, 5}};
        debugProtocol.initialize();
        debugProtocol.update(colors);

        const std::string protocolText = protocolSink.asString();
        TEST_ASSERT_TRUE(protocolText.find("[PROTOCOL] begin pixelCount=2") != std::string::npos);
        TEST_ASSERT_TRUE(protocolText.find("[PROTOCOL] colors(1)") != std::string::npos);
    }

#if defined(NPB_HAS_CPP20_CONCEPTS)
    void test_factory_serial_convenience_helpers_compile(void)
    {
        const auto printCfg = npb::factory::printSerial();
        const auto debugCfg = npb::factory::debugSerial();
        const auto debugTransportCfg = npb::factory::debugTransportSerial();
        const auto debugOneWireCfg = npb::factory::debugOneWireSerial();
        const auto debugProtocolCfg = npb::factory::debugProtocolSerial<TestColor>();

        TEST_ASSERT_NOT_NULL(printCfg.settings.output);
        TEST_ASSERT_NOT_NULL(debugCfg.output);
        TEST_ASSERT_NOT_NULL(debugTransportCfg.settings.output);
        TEST_ASSERT_NOT_NULL(debugOneWireCfg.settings.output);
        TEST_ASSERT_NOT_NULL(debugProtocolCfg.settings.output);
    }
#endif
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
#if defined(NPB_HAS_CPP20_CONCEPTS)
    RUN_TEST(test_factory_make_bus_compile_and_smoke);
#endif
    RUN_TEST(test_writable_template_consumers_compile_and_smoke);
#if defined(NPB_HAS_CPP20_CONCEPTS)
    RUN_TEST(test_factory_serial_convenience_helpers_compile);
#endif
    return UNITY_END();
}
