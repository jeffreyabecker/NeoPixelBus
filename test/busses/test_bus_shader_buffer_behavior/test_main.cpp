#include <unity.h>

#include <algorithm>
#include <array>
#include <vector>

#include "buses/composite/CompositePixelBus.h"
#include "buses/composite/FixedBufferAccessor.h"
#include "colors/Color.h"
#include "colors/IShader.h"
#include "protocols/IProtocol.h"
#include "transports/ITransport.h"

namespace
{
    using TestColor = lw::Rgb8Color;

    class CaptureProtocol : public lw::IProtocol<TestColor>
    {
    public:
        explicit CaptureProtocol(uint16_t pixelCount)
            : lw::IProtocol<TestColor>(pixelCount)
        {
        }

        void begin() override
        {
        }

        void update(lw::span<const TestColor> colors, lw::span<uint8_t> buffer = lw::span<uint8_t>{}) override
        {
            (void)buffer;
            lastSource = colors.data();
            captured.assign(colors.begin(), colors.end());
        }

        lw::ProtocolSettings &settings() override
        {
            return _settings;
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

        const TestColor *lastSource{nullptr};
        std::vector<TestColor> captured{};

    private:
        lw::ProtocolSettings _settings{};
    };

    class IncrementRedShader : public lw::IShader<TestColor>
    {
    public:
        void apply(lw::span<TestColor> colors) override
        {
            for (auto &color : colors)
            {
                ++color['R'];
            }
        }
    };

    class NoopTransport : public lw::ITransport
    {
    public:
        void begin() override
        {
        }

        void transmitBytes(lw::span<uint8_t>) override
        {
        }
    };

    void test_shader_buffer_copy_path_preserves_root_buffer(void)
    {
        std::vector<TestColor> rootColors{
            TestColor{1, 2, 3},
            TestColor{4, 5, 6}};

        CaptureProtocol protocol(2);
        IncrementRedShader shader;
        NoopTransport transport;
        std::array<lw::StrandExtent<TestColor>, 1> strands{lw::StrandExtent<TestColor>{&protocol, &transport, &shader, 0, 2}};
        lw::FixedBufferAccessor<TestColor> accessor(rootColors.size(),
                             2,
                             {0});
        auto root = accessor.rootPixels();
        std::copy(rootColors.begin(), rootColors.end(), root.begin());

        lw::CompositePixelBus<TestColor> bus(
            accessor,
            lw::Topology::linear(rootColors.size()),
            lw::span<lw::StrandExtent<TestColor>>{strands.data(), strands.size()});

        bus.show();

        auto outRoot = accessor.rootPixels();
        TEST_ASSERT_EQUAL_UINT8(1, outRoot[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(4, outRoot[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(2, protocol.captured[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(5, protocol.captured[1]['R']);
        TEST_ASSERT_TRUE(protocol.lastSource != outRoot.data());
    }

    void test_no_shader_buffer_path_applies_in_place(void)
    {
        std::vector<TestColor> rootColors{
            TestColor{10, 2, 3},
            TestColor{20, 5, 6}};

        CaptureProtocol protocol(2);
        IncrementRedShader shader;
        NoopTransport transport;
        std::array<lw::StrandExtent<TestColor>, 1> strands{lw::StrandExtent<TestColor>{&protocol, &transport, &shader, 0, 2}};
        lw::FixedBufferAccessor<TestColor> accessor(rootColors.size(),
                             0,
                             {0});
        auto root = accessor.rootPixels();
        std::copy(rootColors.begin(), rootColors.end(), root.begin());

        lw::CompositePixelBus<TestColor> bus(
            accessor,
            lw::Topology::linear(rootColors.size()),
            lw::span<lw::StrandExtent<TestColor>>{strands.data(), strands.size()});

        bus.show();

        auto outRoot = accessor.rootPixels();
        TEST_ASSERT_EQUAL_UINT8(11, outRoot[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(21, outRoot[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(11, protocol.captured[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(21, protocol.captured[1]['R']);
        TEST_ASSERT_TRUE(protocol.lastSource == outRoot.data());
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
    RUN_TEST(test_shader_buffer_copy_path_preserves_root_buffer);
    RUN_TEST(test_no_shader_buffer_path_applies_in_place);
    return UNITY_END();
}
