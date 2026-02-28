#include <unity.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "buses/BusDriver.h"
#include "buses/ConcatBus.h"
#include "buses/MosaicBus.h"
#include "buses/PixelBus.h"
#include "buses/SegmentBus.h"
#include "colors/Color.h"
#include "protocols/IProtocol.h"

namespace
{
    using TestColor = npb::Rgbcw8Color;

    class ProtocolStub : public npb::IProtocol<TestColor>
    {
    public:
        explicit ProtocolStub(uint16_t pixelCount)
            : npb::IProtocol<TestColor>(pixelCount)
        {
        }

        void initialize() override
        {
            ++initializeCount;
        }

        void update(npb::span<const TestColor> colors) override
        {
            ++updateCount;
            lastFrame.assign(colors.begin(), colors.end());
        }

        bool isReadyToUpdate() const override
        {
            return readyToUpdate;
        }

        bool alwaysUpdate() const override
        {
            return alwaysUpdateEnabled;
        }

        int initializeCount{0};
        int updateCount{0};
        bool readyToUpdate{true};
        bool alwaysUpdateEnabled{false};
        std::vector<TestColor> lastFrame{};
    };

    class DriverStub
    {
    public:
        using ColorType = TestColor;

            void initialize()
        {
            ++initializeCount;
        }

            void update(npb::span<const TestColor> colors)
        {
            ++updateCount;
            lastFrame.assign(colors.begin(), colors.end());
        }

            bool isReadyToUpdate() const
        {
            return readyToUpdate;
        }

            bool alwaysUpdate() const
        {
            return alwaysUpdateEnabled;
        }

        int initializeCount{0};
        int updateCount{0};
        bool readyToUpdate{true};
        bool alwaysUpdateEnabled{false};
        std::vector<TestColor> lastFrame{};
    };

    class SpyBus : public npb::IPixelBus<TestColor>
    {
    public:
        explicit SpyBus(size_t count)
            : pixels(count)
        {
        }

        void begin() override
        {
            ++beginCount;
        }

        void show() override
        {
            ++showCount;
        }

        bool canShow() const override
        {
            return ready;
        }

        size_t pixelCount() const override
        {
            return pixels.size();
        }

        void setPixelColors(size_t offset,
                            npb::ColorIteratorT<TestColor> first,
                            npb::ColorIteratorT<TestColor> last) override
        {
            if (offset >= pixels.size())
            {
                return;
            }

            const auto requested = static_cast<size_t>(last - first);
            const auto count = std::min(requested, pixels.size() - offset);
            for (size_t idx = 0; idx < count; ++idx)
            {
                pixels[offset + idx] = first[static_cast<std::ptrdiff_t>(idx)];
            }
        }

        void getPixelColors(size_t offset,
                            npb::ColorIteratorT<TestColor> first,
                            npb::ColorIteratorT<TestColor> last) const override
        {
            if (offset >= pixels.size())
            {
                return;
            }

            const auto requested = static_cast<size_t>(last - first);
            const auto count = std::min(requested, pixels.size() - offset);
            for (size_t idx = 0; idx < count; ++idx)
            {
                first[static_cast<std::ptrdiff_t>(idx)] = pixels[offset + idx];
            }
        }

        void setPixelColor(size_t index, const TestColor &color) override
        {
            if (index < pixels.size())
            {
                pixels[index] = color;
            }
        }

        TestColor getPixelColor(size_t index) const override
        {
            if (index < pixels.size())
            {
                return pixels[index];
            }

            return TestColor{};
        }

        std::vector<TestColor> pixels{};
        bool ready{true};
        int beginCount{0};
        int showCount{0};
    };

    TestColor color_for_index(uint8_t base)
    {
        return TestColor{base, static_cast<uint8_t>(base + 1), static_cast<uint8_t>(base + 2), static_cast<uint8_t>(base + 3), static_cast<uint8_t>(base + 4)};
    }

    std::vector<TestColor> make_colors(size_t count, uint8_t start = 1)
    {
        std::vector<TestColor> values{};
        values.reserve(count);
        for (size_t idx = 0; idx < count; ++idx)
        {
            values.push_back(color_for_index(static_cast<uint8_t>(start + idx)));
        }
        return values;
    }

    void set_colors_iter(npb::IPixelBus<TestColor> &bus, size_t offset, npb::span<const TestColor> source)
    {
        npb::span<TestColor> mutableSource(const_cast<TestColor *>(source.data()), source.size());
        npb::SpanColorSourceT<TestColor> src(mutableSource);
        bus.setPixelColors(offset, src.begin(), src.end());
    }

    void get_colors_iter(const npb::IPixelBus<TestColor> &bus, size_t offset, npb::span<TestColor> dest)
    {
        npb::SpanColorSourceT<TestColor> out(dest);
        bus.getPixelColors(offset, out.begin(), out.end());
    }

    void assert_color_equal(const TestColor &a, const TestColor &b)
    {
        for (size_t ch = 0; ch < TestColor::ChannelCount; ++ch)
        {
            TEST_ASSERT_EQUAL_UINT8(a[ch], b[ch]);
        }
    }

    void test_1_1_1_bulk_set_get_round_trip_iterator_and_span(void)
    {
        auto protocol = new ProtocolStub{8};
        npb::OwningPixelBusT<TestColor> bus(protocol);

        const auto sourceA = make_colors(8, 10);
        const npb::span<const TestColor> sourceASpan(sourceA.data(), sourceA.size());
        npb::span<TestColor> mutableSourceA(const_cast<TestColor *>(sourceASpan.data()), sourceASpan.size());
        npb::SpanColorSourceT<TestColor> srcA(mutableSourceA);

        bus.setPixelColors(0, srcA.begin(), srcA.end());

        std::vector<TestColor> destA(8, TestColor{});
        npb::SpanColorSourceT<TestColor> outA(npb::span<TestColor>(destA.data(), destA.size()));
        bus.getPixelColors(0, outA.begin(), outA.end());

        for (size_t idx = 0; idx < sourceA.size(); ++idx)
        {
            assert_color_equal(sourceA[idx], destA[idx]);
        }

        const auto sourceB = make_colors(8, 50);
        bus.setPixelColors(0, npb::span<const TestColor>(sourceB.data(), sourceB.size()));

        std::vector<TestColor> destB(8, TestColor{});
        bus.getPixelColors(0, npb::span<TestColor>(destB.data(), destB.size()));

        for (size_t idx = 0; idx < sourceB.size(); ++idx)
        {
            assert_color_equal(sourceB[idx], destB[idx]);
        }
    }

    void test_1_1_2_end_range_partial_write_clamp(void)
    {
        auto protocol = new ProtocolStub{8};
        npb::OwningPixelBusT<TestColor> bus(protocol);

        const auto baseline = make_colors(8, 1);
        bus.setPixelColors(0, npb::span<const TestColor>(baseline.data(), baseline.size()));

        const auto oversized = make_colors(5, 100);
        bus.setPixelColors(6, npb::span<const TestColor>(oversized.data(), oversized.size()));

        std::vector<TestColor> out(8, TestColor{});
        bus.getPixelColors(0, npb::span<TestColor>(out.data(), out.size()));

        for (size_t idx = 0; idx < 6; ++idx)
        {
            assert_color_equal(baseline[idx], out[idx]);
        }
        assert_color_equal(oversized[0], out[6]);
        assert_color_equal(oversized[1], out[7]);
    }

    void test_1_1_3_dirty_always_update_show_behavior(void)
    {
        auto protocol = new ProtocolStub{4};
        npb::OwningPixelBusT<TestColor> bus(protocol);

        bus.show();
        TEST_ASSERT_EQUAL_INT(0, protocol->updateCount);

        bus.setPixelColor(0, color_for_index(11));
        bus.show();
        TEST_ASSERT_EQUAL_INT(1, protocol->updateCount);

        protocol->alwaysUpdateEnabled = true;
        bus.show();
        bus.show();
        TEST_ASSERT_EQUAL_INT(3, protocol->updateCount);
    }

    void test_1_1_4_out_of_range_single_pixel_safety(void)
    {
        auto protocol = new ProtocolStub{3};
        npb::OwningPixelBusT<TestColor> bus(protocol);

        bus.setPixelColor(0, color_for_index(7));
        bus.setPixelColor(100, color_for_index(99));

        const auto inRange = bus.getPixelColor(0);
        const auto outRange = bus.getPixelColor(100);

        TEST_ASSERT_EQUAL_UINT8(7, inRange['R']);
        TEST_ASSERT_EQUAL_UINT8(0, outRange['R']);
        TEST_ASSERT_EQUAL_UINT8(0, outRange['G']);
        TEST_ASSERT_EQUAL_UINT8(0, outRange['B']);
    }

    void test_1_1_5_p0_offset_greater_than_pixel_count_no_op(void)
    {
        auto protocol = new ProtocolStub{4};
        npb::OwningPixelBusT<TestColor> bus(protocol);

        const auto baseline = make_colors(4, 20);
        bus.setPixelColors(0, npb::span<const TestColor>(baseline.data(), baseline.size()));

        const auto source = make_colors(3, 90);
        bus.setPixelColors(99, npb::span<const TestColor>(source.data(), source.size()));

        std::vector<TestColor> out(4, TestColor{});
        bus.getPixelColors(0, npb::span<TestColor>(out.data(), out.size()));
        for (size_t idx = 0; idx < out.size(); ++idx)
        {
            assert_color_equal(baseline[idx], out[idx]);
        }

        std::vector<TestColor> getSentinel(2, color_for_index(200));
        bus.getPixelColors(99, npb::span<TestColor>(getSentinel.data(), getSentinel.size()));
        assert_color_equal(color_for_index(200), getSentinel[0]);
        assert_color_equal(color_for_index(200), getSentinel[1]);
    }

    void test_1_2_1_segment_origin_mapping(void)
    {
        auto protocol = new ProtocolStub{10};
        npb::OwningPixelBusT<TestColor> parent(protocol);
        npb::SegmentBus<TestColor> segment(parent, 4, 3);

        const auto value = color_for_index(77);
        segment.setPixelColor(0, value);

        assert_color_equal(value, parent.getPixelColor(4));
    }

    void test_1_2_2_segment_bulk_range_isolation(void)
    {
        auto protocol = new ProtocolStub{8};
        npb::OwningPixelBusT<TestColor> parent(protocol);
        const auto baseline = make_colors(8, 1);
        parent.setPixelColors(0, npb::span<const TestColor>(baseline.data(), baseline.size()));

        npb::SegmentBus<TestColor> segment(parent, 2, 4);
        const auto values = make_colors(4, 100);
        set_colors_iter(segment, 0, npb::span<const TestColor>(values.data(), values.size()));

        TEST_ASSERT_EQUAL_UINT8(1, parent.getPixelColor(0)['R']);
        TEST_ASSERT_EQUAL_UINT8(2, parent.getPixelColor(1)['R']);
        TEST_ASSERT_EQUAL_UINT8(100, parent.getPixelColor(2)['R']);
        TEST_ASSERT_EQUAL_UINT8(103, parent.getPixelColor(5)['R']);
        TEST_ASSERT_EQUAL_UINT8(7, parent.getPixelColor(6)['R']);
        TEST_ASSERT_EQUAL_UINT8(8, parent.getPixelColor(7)['R']);
    }

    void test_1_2_3_multi_segment_isolation(void)
    {
        auto protocol = new ProtocolStub{10};
        npb::OwningPixelBusT<TestColor> parent(protocol);
        const auto baseline = make_colors(10, 1);
        parent.setPixelColors(0, npb::span<const TestColor>(baseline.data(), baseline.size()));

        npb::SegmentBus<TestColor> segA(parent, 0, 5);
        npb::SegmentBus<TestColor> segB(parent, 5, 5);

        segA.setPixelColor(2, color_for_index(200));
        TEST_ASSERT_EQUAL_UINT8(200, parent.getPixelColor(2)['R']);
        TEST_ASSERT_EQUAL_UINT8(6, parent.getPixelColor(5)['R']);

        segB.setPixelColor(1, color_for_index(210));
        TEST_ASSERT_EQUAL_UINT8(210, parent.getPixelColor(6)['R']);
        TEST_ASSERT_EQUAL_UINT8(200, parent.getPixelColor(2)['R']);
    }

    void test_1_2_4_segment_offset_out_of_range_no_op(void)
    {
        auto protocol = new ProtocolStub{6};
        npb::OwningPixelBusT<TestColor> parent(protocol);
        const auto baseline = make_colors(6, 1);
        parent.setPixelColors(0, npb::span<const TestColor>(baseline.data(), baseline.size()));

        npb::SegmentBus<TestColor> segment(parent, 2, 3);
        const auto before = parent.getPixelColor(2);

        const auto source = make_colors(2, 90);
        set_colors_iter(segment, 3, npb::span<const TestColor>(source.data(), source.size()));

        std::vector<TestColor> out(2, color_for_index(199));
        get_colors_iter(segment, 3, npb::span<TestColor>(out.data(), out.size()));

        assert_color_equal(before, parent.getPixelColor(2));
        assert_color_equal(color_for_index(199), out[0]);
        assert_color_equal(color_for_index(199), out[1]);
    }

    void test_1_2_5_segment_oversize_clamp(void)
    {
        auto protocol = new ProtocolStub{8};
        npb::OwningPixelBusT<TestColor> parent(protocol);
        const auto baseline = make_colors(8, 1);
        parent.setPixelColors(0, npb::span<const TestColor>(baseline.data(), baseline.size()));

        npb::SegmentBus<TestColor> segment(parent, 2, 4);
        const auto source = make_colors(5, 120);
        set_colors_iter(segment, 3, npb::span<const TestColor>(source.data(), source.size()));

        TEST_ASSERT_EQUAL_UINT8(1, parent.getPixelColor(0)['R']);
        TEST_ASSERT_EQUAL_UINT8(5, parent.getPixelColor(4)['R']);
        TEST_ASSERT_EQUAL_UINT8(120, parent.getPixelColor(5)['R']);
        TEST_ASSERT_EQUAL_UINT8(7, parent.getPixelColor(6)['R']);
    }

    void test_1_3_1_concat_uneven_child_index_resolution(void)
    {
        SpyBus a(3);
        SpyBus b(2);
        SpyBus c(4);
        npb::ConcatBus<TestColor> concat(a, b, c);

        concat.setPixelColor(2, color_for_index(10));
        concat.setPixelColor(3, color_for_index(20));
        concat.setPixelColor(4, color_for_index(30));
        concat.setPixelColor(5, color_for_index(40));

        TEST_ASSERT_EQUAL_UINT8(10, a.getPixelColor(2)['R']);
        TEST_ASSERT_EQUAL_UINT8(20, b.getPixelColor(0)['R']);
        TEST_ASSERT_EQUAL_UINT8(30, b.getPixelColor(1)['R']);
        TEST_ASSERT_EQUAL_UINT8(40, c.getPixelColor(0)['R']);
    }

    void test_1_3_2_concat_pixel_count_aggregation(void)
    {
        SpyBus a(3);
        SpyBus b(5);
        SpyBus c(7);
        npb::ConcatBus<TestColor> concat(a, b, c);
        TEST_ASSERT_EQUAL_UINT32(15U, static_cast<uint32_t>(concat.pixelCount()));
    }

    void test_1_3_3_concat_lifecycle_fan_out(void)
    {
        SpyBus a(1);
        SpyBus b(1);
        SpyBus c(1);
        npb::ConcatBus<TestColor> concat(a, b, c);

        concat.begin();
        concat.show();

        TEST_ASSERT_EQUAL_INT(1, a.beginCount);
        TEST_ASSERT_EQUAL_INT(1, b.beginCount);
        TEST_ASSERT_EQUAL_INT(1, c.beginCount);
        TEST_ASSERT_EQUAL_INT(1, a.showCount);
        TEST_ASSERT_EQUAL_INT(1, b.showCount);
        TEST_ASSERT_EQUAL_INT(1, c.showCount);
    }

    void test_1_3_4_concat_remove_updates_mapping(void)
    {
        SpyBus a(2);
        SpyBus b(3);
        SpyBus c(2);

        std::vector<npb::IPixelBus<TestColor> *> buses{};
        buses.emplace_back(&a);
        buses.emplace_back(&b);
        buses.emplace_back(&c);

        npb::ConcatBus<TestColor> concat(std::move(buses));
        TEST_ASSERT_EQUAL_UINT32(7U, static_cast<uint32_t>(concat.pixelCount()));

        const bool removed = concat.remove(b);
        TEST_ASSERT_TRUE(removed);
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(concat.pixelCount()));

        concat.setPixelColor(3, color_for_index(99));
        TEST_ASSERT_EQUAL_UINT8(99, c.getPixelColor(1)['R']);
        TEST_ASSERT_EQUAL_UINT8(0, b.getPixelColor(2)['R']);
    }

    void test_1_3_5_concat_invalid_remove_add_behavior(void)
    {
        SpyBus a(2);
        SpyBus b(2);
        SpyBus outsider(2);
        npb::ConcatBus<TestColor> concat(a, b);

        const auto before = concat.pixelCount();
        TEST_ASSERT_FALSE(concat.remove(outsider));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(before), static_cast<uint32_t>(concat.pixelCount()));

        concat.add(nullptr);
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(before), static_cast<uint32_t>(concat.pixelCount()));
    }

    void test_1_4_1_mosaic_2d_coordinate_mapping(void)
    {
        SpyBus p0(4);
        SpyBus p1(4);

        npb::MosaicBusSettings<TestColor> cfg{};
        cfg.panelWidth = 2;
        cfg.panelHeight = 2;
        cfg.layout = npb::PanelLayout::RowMajor;
        cfg.tilesWide = 2;
        cfg.tilesHigh = 1;
        cfg.tileLayout = npb::PanelLayout::RowMajor;

        std::vector<npb::IPixelBus<TestColor> *> buses{};
        buses.emplace_back(&p0);
        buses.emplace_back(&p1);
        npb::MosaicBus<TestColor> mosaic(cfg, std::move(buses));

        mosaic.setPixelColor(0, 0, color_for_index(10));
        mosaic.setPixelColor(1, 1, color_for_index(20));
        mosaic.setPixelColor(2, 0, color_for_index(30));
        mosaic.setPixelColor(3, 1, color_for_index(40));

        TEST_ASSERT_EQUAL_UINT8(10, p0.getPixelColor(0)['R']);
        TEST_ASSERT_EQUAL_UINT8(20, p0.getPixelColor(3)['R']);
        TEST_ASSERT_EQUAL_UINT8(30, p1.getPixelColor(0)['R']);
        TEST_ASSERT_EQUAL_UINT8(40, p1.getPixelColor(3)['R']);
    }

    void test_1_4_2_mosaic_linear_flattening_consistency(void)
    {
        SpyBus p0(4);
        SpyBus p1(4);

        npb::MosaicBusSettings<TestColor> cfg{};
        cfg.panelWidth = 2;
        cfg.panelHeight = 2;
        cfg.layout = npb::PanelLayout::RowMajor;
        cfg.tilesWide = 2;
        cfg.tilesHigh = 1;
        cfg.tileLayout = npb::PanelLayout::RowMajor;

        std::vector<npb::IPixelBus<TestColor> *> buses{};
        buses.emplace_back(&p0);
        buses.emplace_back(&p1);
        npb::MosaicBus<TestColor> mosaic(cfg, std::move(buses));

        const auto linear = make_colors(8, 50);
        set_colors_iter(mosaic, 0, npb::span<const TestColor>(linear.data(), linear.size()));

        std::vector<TestColor> roundTrip(8, TestColor{});
        get_colors_iter(mosaic, 0, npb::span<TestColor>(roundTrip.data(), roundTrip.size()));

        for (size_t idx = 0; idx < linear.size(); ++idx)
        {
            assert_color_equal(linear[idx], roundTrip[idx]);
        }

        assert_color_equal(linear[0], mosaic.getPixelColor(0, 0));
        assert_color_equal(linear[3], mosaic.getPixelColor(1, 1));
        assert_color_equal(linear[4], mosaic.getPixelColor(2, 0));
        assert_color_equal(linear[7], mosaic.getPixelColor(3, 1));
    }

    void test_1_4_3_mosaic_can_show_all_children_gate(void)
    {
        SpyBus p0(1);
        SpyBus p1(1);

        npb::MosaicBusSettings<TestColor> cfg{};
        cfg.panelWidth = 1;
        cfg.panelHeight = 1;
        cfg.layout = npb::PanelLayout::RowMajor;
        cfg.tilesWide = 2;
        cfg.tilesHigh = 1;
        cfg.tileLayout = npb::PanelLayout::RowMajor;

        std::vector<npb::IPixelBus<TestColor> *> buses{};
        buses.emplace_back(&p0);
        buses.emplace_back(&p1);
        npb::MosaicBus<TestColor> mosaic(cfg, std::move(buses));

        p0.ready = true;
        p1.ready = false;
        TEST_ASSERT_FALSE(mosaic.canShow());

        p1.ready = true;
        TEST_ASSERT_TRUE(mosaic.canShow());
    }

    void test_1_4_4_mosaic_out_of_bounds_2d_safety(void)
    {
        SpyBus p0(4);

        npb::MosaicBusSettings<TestColor> cfg{};
        cfg.panelWidth = 2;
        cfg.panelHeight = 2;
        cfg.layout = npb::PanelLayout::RowMajor;
        cfg.tilesWide = 1;
        cfg.tilesHigh = 1;
        cfg.tileLayout = npb::PanelLayout::RowMajor;

        std::vector<npb::IPixelBus<TestColor> *> buses{};
        buses.emplace_back(&p0);
        npb::MosaicBus<TestColor> mosaic(cfg, std::move(buses));

        mosaic.setPixelColor(-1, 0, color_for_index(99));
        mosaic.setPixelColor(0, 5, color_for_index(88));

        const auto a = mosaic.getPixelColor(-1, 0);
        const auto b = mosaic.getPixelColor(0, 5);
        TEST_ASSERT_EQUAL_UINT8(0, a['R']);
        TEST_ASSERT_EQUAL_UINT8(0, b['R']);
        TEST_ASSERT_EQUAL_UINT8(0, p0.getPixelColor(0)['R']);
    }

    void test_1_4_5_mosaic_sparse_tile_safety_and_empty_geometry(void)
    {
        {
            SpyBus p0(4);
            SpyBus p1(4);
            SpyBus p2(4);

            npb::MosaicBusSettings<TestColor> cfg{};
            cfg.panelWidth = 2;
            cfg.panelHeight = 2;
            cfg.layout = npb::PanelLayout::RowMajor;
            cfg.tilesWide = 2;
            cfg.tilesHigh = 2;
            cfg.tileLayout = npb::PanelLayout::RowMajor;

            std::vector<npb::IPixelBus<TestColor> *> buses{};
            buses.emplace_back(&p0);
            buses.emplace_back(&p1);
            buses.emplace_back(&p2);
            npb::MosaicBus<TestColor> sparse(cfg, std::move(buses));
            sparse.setPixelColor(3, 3, color_for_index(123));

            const auto unresolved = sparse.getPixelColor(3, 3);
            TEST_ASSERT_EQUAL_UINT8(0, unresolved['R']);
        }

        {
            npb::MosaicBusSettings<TestColor> cfg{};
            cfg.panelWidth = 2;
            cfg.panelHeight = 2;
            cfg.layout = npb::PanelLayout::RowMajor;
            cfg.tilesWide = 2;
            cfg.tilesHigh = 2;
            cfg.tileLayout = npb::PanelLayout::RowMajor;

            std::vector<npb::IPixelBus<TestColor> *> none{};
            npb::MosaicBus<TestColor> empty(cfg, std::move(none));

            TEST_ASSERT_EQUAL_UINT16(0, empty.width());
            TEST_ASSERT_EQUAL_UINT16(0, empty.height());
        }
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
    RUN_TEST(test_1_1_1_bulk_set_get_round_trip_iterator_and_span);
    RUN_TEST(test_1_1_2_end_range_partial_write_clamp);
    RUN_TEST(test_1_1_3_dirty_always_update_show_behavior);
    RUN_TEST(test_1_1_4_out_of_range_single_pixel_safety);
    RUN_TEST(test_1_1_5_p0_offset_greater_than_pixel_count_no_op);
    RUN_TEST(test_1_2_1_segment_origin_mapping);
    RUN_TEST(test_1_2_2_segment_bulk_range_isolation);
    RUN_TEST(test_1_2_3_multi_segment_isolation);
    RUN_TEST(test_1_2_4_segment_offset_out_of_range_no_op);
    RUN_TEST(test_1_2_5_segment_oversize_clamp);
    RUN_TEST(test_1_3_1_concat_uneven_child_index_resolution);
    RUN_TEST(test_1_3_2_concat_pixel_count_aggregation);
    RUN_TEST(test_1_3_3_concat_lifecycle_fan_out);
    RUN_TEST(test_1_3_4_concat_remove_updates_mapping);
    RUN_TEST(test_1_3_5_concat_invalid_remove_add_behavior);
    RUN_TEST(test_1_4_1_mosaic_2d_coordinate_mapping);
    RUN_TEST(test_1_4_2_mosaic_linear_flattening_consistency);
    RUN_TEST(test_1_4_3_mosaic_can_show_all_children_gate);
    RUN_TEST(test_1_4_4_mosaic_out_of_bounds_2d_safety);
    RUN_TEST(test_1_4_5_mosaic_sparse_tile_safety_and_empty_geometry);
    return UNITY_END();
}

