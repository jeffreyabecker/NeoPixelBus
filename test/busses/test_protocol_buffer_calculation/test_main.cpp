#include <unity.h>

#include <array>
#include <vector>

#include "buses/impl/BufferAccessor.h"
#include "buses/PixelBus.h"
#include "colors/Color.h"
#include "protocols/DotStarProtocol.h"
#include "protocols/IProtocol.h"

namespace
{
    using TestColor = lw::Rgb8Color;

    constexpr lw::Apa102ProtocolSettings kApaSettings{};
    constexpr size_t kCompileTimeProtocolBytes = lw::Apa102Protocol<TestColor>::requiredBufferSize(5, kApaSettings);
    constexpr size_t kCompileTimeTotalBytes = lw::BufferAccessor<TestColor>::totalBytes(5, 0, kCompileTimeProtocolBytes);
    static_assert(kCompileTimeProtocolBytes > 0, "Expected non-zero APA102 protocol buffer bytes");
    static_assert(kCompileTimeTotalBytes == (5u * sizeof(TestColor) + kCompileTimeProtocolBytes),
                  "BufferAccessor total byte formula must be constexpr-correct");

    class MockProtocol : public lw::IProtocol<TestColor>
    {
    public:
        MockProtocol(uint16_t pixelCount, size_t requiredBytes)
            : lw::IProtocol<TestColor>(pixelCount)
            , _required(requiredBytes)
        {
        }

        void begin() override {}
        void update(lw::span<const TestColor>, lw::span<uint8_t> buffer = lw::span<uint8_t>{}) override
        {
            lastBuffer = buffer.data();
            lastSize = buffer.size();
        }

        size_t requiredBufferSizeBytes() const override
        {
            return _required;
        }

        bool alwaysUpdate() const override { return false; }

        const uint8_t *lastBuffer{nullptr};
        size_t lastSize{0};
    private:
        size_t _required{0};
    };

    void test_calculate_required_bytes(void)
    {
        std::array<lw::StrandExtent<TestColor>, 3> strands{};

        MockProtocol p1(0, 10);
        MockProtocol p2(0, 5);

        strands[0] = lw::StrandExtent<TestColor>{nullptr, nullptr, nullptr, 0, 0};
        strands[1] = lw::StrandExtent<TestColor>{&p1, nullptr, nullptr, 0, 0};
        strands[2] = lw::StrandExtent<TestColor>{&p2, nullptr, nullptr, 0, 0};

        auto span = lw::span<lw::StrandExtent<TestColor>>{strands.data(), strands.size()};

        size_t total = 0;
        for (const auto &s : span)
        {
            if (s.protocol == nullptr)
            {
                continue;
            }
            total += s.protocol->requiredBufferSizeBytes();
        }
        TEST_ASSERT_EQUAL_UINT32(15u, static_cast<uint32_t>(total));
    }

    void test_bind_assigns_slices_and_buffers(void)
    {
        std::array<lw::StrandExtent<TestColor>, 3> strands{};

        MockProtocol p1(0, 10);
        MockProtocol p2(0, 5);

        strands[0] = lw::StrandExtent<TestColor>{nullptr, nullptr, nullptr, 0, 0};
        strands[1] = lw::StrandExtent<TestColor>{&p1, nullptr, nullptr, 0, 0};
        strands[2] = lw::StrandExtent<TestColor>{&p2, nullptr, nullptr, 0, 0};

        lw::BufferAccessor<TestColor> access(0,
                           0,
                           {0, 10, 5});

        auto spanStrands = lw::span<lw::StrandExtent<TestColor>>{strands.data(), strands.size()};

        // Bind each protocol to its predeclared unified slice.
        for (size_t strandIndex = 0; strandIndex < spanStrands.size(); ++strandIndex)
        {
            const auto &strand = spanStrands[strandIndex];
            if (strand.protocol == nullptr)
            {
                continue;
            }

            strand.protocol->update(lw::span<const TestColor>{}, access.protocolSlice(strandIndex));
        }

        // verify protocols received buffers of correct sizes
        TEST_ASSERT_EQUAL_UINT32(0u, static_cast<uint32_t>(access.protocolSlice(0).size()));
        TEST_ASSERT_EQUAL_UINT32(10u, static_cast<uint32_t>(access.protocolSlice(1).size()));
        TEST_ASSERT_EQUAL_UINT32(5u, static_cast<uint32_t>(access.protocolSlice(2).size()));

        TEST_ASSERT_NOT_NULL(p1.lastBuffer);
        TEST_ASSERT_EQUAL_UINT32(10u, static_cast<uint32_t>(p1.lastSize));
        TEST_ASSERT_NOT_NULL(p2.lastBuffer);
        TEST_ASSERT_EQUAL_UINT32(5u, static_cast<uint32_t>(p2.lastSize));
    }

    void test_owning_buffer_protocol_slice_size_is_fixed(void)
    {
        std::array<lw::StrandExtent<TestColor>, 2> strands{};

        MockProtocol p1(0, 8);
        MockProtocol p2(0, 9);

        strands[0] = lw::StrandExtent<TestColor>{&p1, nullptr, nullptr, 0, 0};
        strands[1] = lw::StrandExtent<TestColor>{&p2, nullptr, nullptr, 0, 0};

        lw::BufferAccessor<TestColor> access(0,
                           0,
                           {8, 9});

        auto spanStrands = lw::span<lw::StrandExtent<TestColor>>{strands.data(), strands.size()};

        for (size_t strandIndex = 0; strandIndex < spanStrands.size(); ++strandIndex)
        {
            const auto &strand = spanStrands[strandIndex];
            strand.protocol->update(lw::span<const TestColor>{}, access.protocolSlice(strandIndex));
        }

        TEST_ASSERT_EQUAL_UINT32(8u, static_cast<uint32_t>(p1.lastSize));
        TEST_ASSERT_EQUAL_UINT32(9u, static_cast<uint32_t>(p2.lastSize));
    }

    void test_external_buffer_constructor_uses_provided_storage(void)
    {
        constexpr size_t kRootPixels = 2;
        constexpr size_t kShaderPixels = 1;
        constexpr size_t kProtocol0 = 3;
        constexpr size_t kProtocol1 = 4;
        constexpr size_t kProtocolBytes = kProtocol0 + kProtocol1;
        constexpr size_t kTotalBytes = lw::BufferAccessor<TestColor>::totalBytes(kRootPixels,
                                                                                  kShaderPixels,
                                                                                  kProtocolBytes);

        std::vector<uint8_t> backing(kTotalBytes, static_cast<uint8_t>(0));

        lw::BufferAccessor<TestColor> access(kRootPixels,
                                             kShaderPixels,
                                             {kProtocol0, kProtocol1},
                                             backing.data(),
                                             false);

        auto root = access.rootPixels();
        auto shader = access.shaderScratch();
        auto protocol0 = access.protocolSlice(0);
        auto protocol1 = access.protocolSlice(1);

        const size_t rootBytes = lw::BufferAccessor<TestColor>::pixelBytes(kRootPixels);
        const size_t shaderBytes = lw::BufferAccessor<TestColor>::pixelBytes(kShaderPixels);
        const size_t protocolOffset = rootBytes + shaderBytes;

        TEST_ASSERT_EQUAL_PTR(reinterpret_cast<void *>(backing.data()),
                              reinterpret_cast<void *>(root.data()));
        TEST_ASSERT_EQUAL_PTR(reinterpret_cast<void *>(backing.data() + rootBytes),
                              reinterpret_cast<void *>(shader.data()));
        TEST_ASSERT_EQUAL_PTR(reinterpret_cast<void *>(backing.data() + protocolOffset),
                              reinterpret_cast<void *>(protocol0.data()));
        TEST_ASSERT_EQUAL_PTR(reinterpret_cast<void *>(backing.data() + protocolOffset + kProtocol0),
                              reinterpret_cast<void *>(protocol1.data()));

        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(kProtocol0), static_cast<uint32_t>(protocol0.size()));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(kProtocol1), static_cast<uint32_t>(protocol1.size()));
    }

} // namespace

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(test_calculate_required_bytes);
    RUN_TEST(test_bind_assigns_slices_and_buffers);
    RUN_TEST(test_owning_buffer_protocol_slice_size_is_fixed);
    RUN_TEST(test_external_buffer_constructor_uses_provided_storage);
    return UNITY_END();
}
