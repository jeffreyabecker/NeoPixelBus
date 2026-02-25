#include <unity.h>

#include <array>
#include <concepts>

#include "VirtualNeoPixelBus.h"

namespace
{
    using TestColor = npb::Color;

    static_assert(std::derived_from<npb::NilProtocol<TestColor>, npb::IProtocol<TestColor>>);
    static_assert(npb::ProtocolPixelSettingsConstructible<npb::NilProtocol<TestColor>>);
    static_assert(npb::ProtocolSettingsTransportBindable<npb::NilProtocol<TestColor>>);
    static_assert(std::derived_from<npb::NilShader<TestColor>, npb::IShader<TestColor>>);
    static_assert(std::derived_from<npb::NilBusT<TestColor>, npb::IPixelBus<TestColor>>);

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
            npb::NilTransportConfig{},
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
    return UNITY_END();
}
