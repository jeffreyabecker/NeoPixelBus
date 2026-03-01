#include <unity.h>

#include <array>
#include <vector>

#include "buses/PixelBus.h"
#include "colors/Color.h"
#include "colors/IShader.h"
#include "protocols/IProtocol.h"

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

        void initialize() override
        {
        }

        void update(lw::span<const TestColor> colors) override
        {
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

        const TestColor *lastSource{nullptr};
        std::vector<TestColor> captured{};
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

    void test_shader_buffer_copy_path_preserves_root_buffer(void)
    {
        std::vector<TestColor> rootColors{
            TestColor{1, 2, 3},
            TestColor{4, 5, 6}};

        CaptureProtocol protocol(2);
        IncrementRedShader shader;
        std::array<lw::StrandExtent<TestColor>, 1> strands{lw::StrandExtent<TestColor>{&protocol, nullptr, &shader, 0, 2}};

        lw::PixelBus<TestColor> bus(
            lw::BufferHolder<TestColor>{rootColors.size(), rootColors.data(), false},
            lw::BufferHolder<TestColor>{2, nullptr, true},
            lw::Topology::linear(rootColors.size()),
            lw::span<lw::StrandExtent<TestColor>>{strands.data(), strands.size()});

        bus.show();

        TEST_ASSERT_EQUAL_UINT8(1, rootColors[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(4, rootColors[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(2, protocol.captured[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(5, protocol.captured[1]['R']);
        TEST_ASSERT_TRUE(protocol.lastSource != rootColors.data());
    }

    void test_no_shader_buffer_path_applies_in_place(void)
    {
        std::vector<TestColor> rootColors{
            TestColor{10, 2, 3},
            TestColor{20, 5, 6}};

        CaptureProtocol protocol(2);
        IncrementRedShader shader;
        std::array<lw::StrandExtent<TestColor>, 1> strands{lw::StrandExtent<TestColor>{&protocol, nullptr, &shader, 0, 2}};

        lw::PixelBus<TestColor> bus(
            lw::BufferHolder<TestColor>{rootColors.size(), rootColors.data(), false},
            lw::BufferHolder<TestColor>::nil(),
            lw::Topology::linear(rootColors.size()),
            lw::span<lw::StrandExtent<TestColor>>{strands.data(), strands.size()});

        bus.show();

        TEST_ASSERT_EQUAL_UINT8(11, rootColors[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(21, rootColors[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(11, protocol.captured[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(21, protocol.captured[1]['R']);
        TEST_ASSERT_TRUE(protocol.lastSource == rootColors.data());
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
