#include <unity.h>

#include <algorithm>
#include <array>
#include <vector>

#include "buses/ReferenceBus.h"
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

        ~CaptureProtocol() override
        {
            ++destructorCount;
        }

        void begin() override
        {
            began = true;
        }

        void update(lw::span<const TestColor> colors,
                    lw::span<uint8_t> buffer = lw::span<uint8_t>{}) override
        {
            lastSource = colors.data();
            captured.assign(colors.begin(), colors.end());
            lastBufferSize = buffer.size();
            if (!buffer.empty())
            {
                std::fill(buffer.begin(), buffer.end(), 0x5A);
            }
        }

        lw::ProtocolSettings &settings() override
        {
            return _settings;
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

        size_t requiredBufferSizeBytes() const override
        {
            return 4U;
        }

        static int destructorCount;
        bool began{false};
        const TestColor *lastSource{nullptr};
        size_t lastBufferSize{0};
        std::vector<TestColor> captured{};

    private:
        lw::ProtocolSettings _settings{};
    };

    int CaptureProtocol::destructorCount = 0;

    class CaptureTransport : public lw::ITransport
    {
    public:
        ~CaptureTransport() override
        {
            ++destructorCount;
        }

        void begin() override
        {
            began = true;
        }

        void beginTransaction() override
        {
            ++beginTransactionCount;
        }

        void transmitBytes(lw::span<uint8_t> bytes) override
        {
            transmitted.assign(bytes.begin(), bytes.end());
        }

        void endTransaction() override
        {
            ++endTransactionCount;
        }

        static int destructorCount;
        bool began{false};
        size_t beginTransactionCount{0};
        size_t endTransactionCount{0};
        std::vector<uint8_t> transmitted{};
    };

    int CaptureTransport::destructorCount = 0;

    class IncrementRedShader : public lw::IShader<TestColor>
    {
    public:
        ~IncrementRedShader() override
        {
            ++destructorCount;
        }

        void apply(lw::span<TestColor> colors) override
        {
            for (auto &color : colors)
            {
                ++color['R'];
            }
        }

        static int destructorCount;
    };

    int IncrementRedShader::destructorCount = 0;

    struct OwnedColor
    {
        static int destructorCount;

        OwnedColor() = default;

        ~OwnedColor()
        {
            ++destructorCount;
        }
    };

    int OwnedColor::destructorCount = 0;

    class NoopProtocolOwnedColor : public lw::IProtocol<OwnedColor>
    {
    public:
        explicit NoopProtocolOwnedColor(uint16_t pixelCount)
            : lw::IProtocol<OwnedColor>(pixelCount)
        {
        }

        ~NoopProtocolOwnedColor() override
        {
            ++destructorCount;
        }

        void begin() override
        {
        }

        void update(lw::span<const OwnedColor>, lw::span<uint8_t> = lw::span<uint8_t>{}) override
        {
        }

        lw::ProtocolSettings &settings() override
        {
            return _settings;
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

        static int destructorCount;

    private:
        lw::ProtocolSettings _settings{};
    };

    int NoopProtocolOwnedColor::destructorCount = 0;

    class NoopTransportOwnedColor : public lw::ITransport
    {
    public:
        ~NoopTransportOwnedColor() override
        {
            ++destructorCount;
        }

        void begin() override
        {
        }

        void transmitBytes(lw::span<uint8_t>) override
        {
        }

        static int destructorCount;
    };

    int NoopTransportOwnedColor::destructorCount = 0;

    class NoopShaderOwnedColor : public lw::IShader<OwnedColor>
    {
    public:
        ~NoopShaderOwnedColor() override
        {
            ++destructorCount;
        }

        void apply(lw::span<OwnedColor>) override
        {
        }

        static int destructorCount;
    };

    int NoopShaderOwnedColor::destructorCount = 0;

    void resetDestructorCounters()
    {
        CaptureProtocol::destructorCount = 0;
        CaptureTransport::destructorCount = 0;
        IncrementRedShader::destructorCount = 0;
        NoopProtocolOwnedColor::destructorCount = 0;
        NoopTransportOwnedColor::destructorCount = 0;
        NoopShaderOwnedColor::destructorCount = 0;
        OwnedColor::destructorCount = 0;
    }

    void test_reference_bus_uses_shader_scratch_when_provided(void)
    {
        resetDestructorCounters();

        std::array<TestColor, 2> rootBuffer{
            TestColor{1, 2, 3},
            TestColor{4, 5, 6}};
        std::array<TestColor, 2> shaderBuffer{};
        std::array<uint8_t, 4> protocolBuffer{};

        CaptureProtocol protocol(2);
        CaptureTransport transport;
        IncrementRedShader shader;

        lw::busses::ReferenceBus<TestColor> bus(
            2,
            rootBuffer.data(),
            &protocol,
            protocolBuffer.data(),
            &transport,
            &shader,
            shaderBuffer.data(),
            false);

        bus.begin();
        bus.show();

        TEST_ASSERT_TRUE(protocol.began);
        TEST_ASSERT_TRUE(transport.began);
        TEST_ASSERT_TRUE(protocol.lastSource == shaderBuffer.data());
        TEST_ASSERT_EQUAL_UINT8(1, rootBuffer[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(4, rootBuffer[1]['R']);
        TEST_ASSERT_EQUAL_UINT8(2, shaderBuffer[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(5, shaderBuffer[1]['R']);
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(protocol.lastBufferSize));
        TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(transport.beginTransactionCount));
        TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(transport.endTransactionCount));
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(transport.transmitted.size()));
    }

    void test_reference_bus_applies_shader_in_place_without_scratch(void)
    {
        resetDestructorCounters();

        std::array<TestColor, 2> rootBuffer{
            TestColor{10, 2, 3},
            TestColor{20, 5, 6}};
        std::array<uint8_t, 4> protocolBuffer{};

        CaptureProtocol protocol(2);
        CaptureTransport transport;
        IncrementRedShader shader;

        lw::busses::ReferenceBus<TestColor> bus(
            2,
            rootBuffer.data(),
            &protocol,
            protocolBuffer.data(),
            &transport,
            &shader,
            nullptr,
            false);

        bus.show();

        TEST_ASSERT_TRUE(protocol.lastSource == rootBuffer.data());
        TEST_ASSERT_EQUAL_UINT8(11, rootBuffer[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(21, rootBuffer[1]['R']);
    }

    void test_reference_bus_owns_all_resources_when_enabled(void)
    {
        resetDestructorCounters();

        {
            auto *rootBuffer = new OwnedColor[1];
            auto *shaderBuffer = new OwnedColor[1];
            auto *protocolBuffer = new uint8_t[4];
            auto *protocol = new NoopProtocolOwnedColor(1);
            auto *transport = new NoopTransportOwnedColor();
            auto *shader = new NoopShaderOwnedColor();

            lw::busses::ReferenceBus<OwnedColor> bus(
                1,
                rootBuffer,
                protocol,
                protocolBuffer,
                transport,
                shader,
                shaderBuffer,
                true);
        }

        TEST_ASSERT_EQUAL_INT(1, NoopProtocolOwnedColor::destructorCount);
        TEST_ASSERT_EQUAL_INT(1, NoopTransportOwnedColor::destructorCount);
        TEST_ASSERT_EQUAL_INT(1, NoopShaderOwnedColor::destructorCount);
        TEST_ASSERT_EQUAL_INT(2, OwnedColor::destructorCount);
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
    RUN_TEST(test_reference_bus_uses_shader_scratch_when_provided);
    RUN_TEST(test_reference_bus_applies_shader_in_place_without_scratch);
    RUN_TEST(test_reference_bus_owns_all_resources_when_enabled);
    return UNITY_END();
}
