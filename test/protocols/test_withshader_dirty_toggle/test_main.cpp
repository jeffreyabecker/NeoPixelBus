#include <unity.h>

#include <cstdint>
#include <vector>

#include "protocols/IProtocol.h"
#include "protocols/WithShaderProtocol.h"

namespace
{
    struct CaptureProtocolSettings
    {
    };

    class CaptureProtocol : public npb::IProtocol<npb::Rgb8Color>
    {
    public:
        using SettingsType = CaptureProtocolSettings;
        using TransportCategory = npb::AnyTransportTag;

        CaptureProtocol(uint16_t pixelCount, SettingsType)
            : npb::IProtocol<npb::Rgb8Color>(pixelCount)
        {
        }

        void initialize() override
        {
            ++initializeCount;
        }

        void update(npb::span<const npb::Rgb8Color> colors) override
        {
            ++updateCount;
            lastSource = colors.data();
            captured.assign(colors.begin(), colors.end());
        }

        bool isReadyToUpdate() const override
        {
            return true;
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

        int initializeCount{0};
        int updateCount{0};
        const npb::Rgb8Color* lastSource{nullptr};
        std::vector<npb::Rgb8Color> captured{};
    };

    class IncrementRedShader : public npb::IShader<npb::Rgb8Color>
    {
    public:
        void apply(npb::span<npb::Rgb8Color> colors) override
        {
            for (auto& color : colors)
            {
                ++color['R'];
            }
        }
    };

    void test_withshader_default_uses_internal_copy(void)
    {
        std::vector<npb::Rgb8Color> colors{
            npb::Rgb8Color{1, 2, 3},
            npb::Rgb8Color{4, 5, 6}};

        IncrementRedShader shader;
        using SettingsType = npb::WithShaderProtocolSettings<npb::Rgb8Color, CaptureProtocol::SettingsType>;
        SettingsType settings{};
        settings.shader = &shader;

        npb::WithShader<npb::Rgb8Color, CaptureProtocol> protocol(2, settings);
        protocol.update(npb::span<const npb::Rgb8Color>{colors.data(), colors.size()});

        TEST_ASSERT_EQUAL_UINT8(1, colors[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(4, colors[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(2, protocol.captured[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(5, protocol.captured[1]['R']);
        TEST_ASSERT_TRUE(colors.data() != protocol.lastSource);
    }

    void test_withshader_allow_dirty_shaders_passes_through(void)
    {
        std::vector<npb::Rgb8Color> colors{
            npb::Rgb8Color{1, 2, 3},
            npb::Rgb8Color{4, 5, 6}};

        IncrementRedShader shader;
        using SettingsType = npb::WithShaderProtocolSettings<npb::Rgb8Color, CaptureProtocol::SettingsType>;
        SettingsType settings{};
        settings.shader = &shader;
        settings.allowDirtyShaders = true;

        npb::WithShader<npb::Rgb8Color, CaptureProtocol> protocol(2, settings);
        protocol.update(npb::span<const npb::Rgb8Color>{colors.data(), colors.size()});

        TEST_ASSERT_EQUAL_UINT8(2, colors[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(5, colors[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(2, protocol.captured[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(5, protocol.captured[1]['R']);
        TEST_ASSERT_TRUE(colors.data() == protocol.lastSource);
    }

    void test_withembeddedshader_default_uses_internal_copy(void)
    {
        std::vector<npb::Rgb8Color> colors{
            npb::Rgb8Color{10, 2, 3},
            npb::Rgb8Color{20, 5, 6}};

        using SettingsType = npb::WithEmbeddedShaderProtocolSettings<IncrementRedShader, CaptureProtocol::SettingsType>;
        SettingsType settings{};

        npb::WithEmbeddedShader<npb::Rgb8Color, IncrementRedShader, CaptureProtocol> protocol(2, settings);
        protocol.update(npb::span<const npb::Rgb8Color>{colors.data(), colors.size()});

        TEST_ASSERT_EQUAL_UINT8(10, colors[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(20, colors[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(11, protocol.captured[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(21, protocol.captured[1]['R']);
        TEST_ASSERT_TRUE(colors.data() != protocol.lastSource);
    }

    void test_withembeddedshader_allow_dirty_shaders_passes_through(void)
    {
        std::vector<npb::Rgb8Color> colors{
            npb::Rgb8Color{10, 2, 3},
            npb::Rgb8Color{20, 5, 6}};

        using SettingsType = npb::WithEmbeddedShaderProtocolSettings<IncrementRedShader, CaptureProtocol::SettingsType>;
        SettingsType settings{};
        settings.allowDirtyShaders = true;

        npb::WithEmbeddedShader<npb::Rgb8Color, IncrementRedShader, CaptureProtocol> protocol(2, settings);
        protocol.update(npb::span<const npb::Rgb8Color>{colors.data(), colors.size()});

        TEST_ASSERT_EQUAL_UINT8(11, colors[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(21, colors[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(11, protocol.captured[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(21, protocol.captured[1]['R']);
        TEST_ASSERT_TRUE(colors.data() == protocol.lastSource);
    }
}

void setUp(void)
{
}

void tearDown(void)
{
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(test_withshader_default_uses_internal_copy);
    RUN_TEST(test_withshader_allow_dirty_shaders_passes_through);
    RUN_TEST(test_withembeddedshader_default_uses_internal_copy);
    RUN_TEST(test_withembeddedshader_allow_dirty_shaders_passes_through);
    return UNITY_END();
}
