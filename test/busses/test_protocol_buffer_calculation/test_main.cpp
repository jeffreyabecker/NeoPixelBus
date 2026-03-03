#include <unity.h>

#include <array>
#include <vector>

#include "buses/PixelBus.h"
#include "colors/Color.h"
#include "protocols/IProtocol.h"
#include "core/UnifiedOwningBufferAccessSurface.h"

namespace
{
    using TestColor = lw::Rgb8Color;

    class MockProtocol : public lw::IProtocol<TestColor>
    {
    public:
        MockProtocol(uint16_t pixelCount, size_t requiredBytes)
            : lw::IProtocol<TestColor>(pixelCount)
            , _required(requiredBytes)
        {
        }

        void initialize() override {}
        void update(lw::span<const TestColor>, lw::span<uint8_t> buffer = lw::span<uint8_t>{}) override
        {
            (void)buffer;
        }

        void setBuffer(lw::span<uint8_t> buffer) override
        {
            lastBuffer = buffer.data();
            lastSize = buffer.size();
        }

        void bindTransport(lw::ITransport *transport) override
        {
            bound = (transport != nullptr);
        }

        size_t requiredBufferSizeBytes() const override
        {
            return _required;
        }

        bool isReadyToUpdate() const override { return true; }
        bool alwaysUpdate() const override { return false; }

        const uint8_t *lastBuffer{nullptr};
        size_t lastSize{0};
        bool bound{false};

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

        lw::UnifiedOwningBufferAccessSurface<TestColor> access(0,
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

            strand.protocol->bindTransport(strand.transport);
            strand.protocol->setBuffer(access.protocolSlice(strandIndex));
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

    void test_unified_surface_protocol_slice_size_is_fixed(void)
    {
        std::array<lw::StrandExtent<TestColor>, 2> strands{};

        MockProtocol p1(0, 8);
        MockProtocol p2(0, 9);

        strands[0] = lw::StrandExtent<TestColor>{&p1, nullptr, nullptr, 0, 0};
        strands[1] = lw::StrandExtent<TestColor>{&p2, nullptr, nullptr, 0, 0};

        lw::UnifiedOwningBufferAccessSurface<TestColor> access(0,
                                       0,
                                       {8, 9});

        auto spanStrands = lw::span<lw::StrandExtent<TestColor>>{strands.data(), strands.size()};

        for (size_t strandIndex = 0; strandIndex < spanStrands.size(); ++strandIndex)
        {
            const auto &strand = spanStrands[strandIndex];
            strand.protocol->bindTransport(strand.transport);
            strand.protocol->setBuffer(access.protocolSlice(strandIndex));
        }

        TEST_ASSERT_EQUAL_UINT32(8u, static_cast<uint32_t>(p1.lastSize));
        TEST_ASSERT_EQUAL_UINT32(9u, static_cast<uint32_t>(p2.lastSize));
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
    RUN_TEST(test_unified_surface_protocol_slice_size_is_fixed);
    return UNITY_END();
}
