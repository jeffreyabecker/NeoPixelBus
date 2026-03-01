#include <unity.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "factory/busses/BusDriver.h"
#include "buses/ConcatBus.h"
#include "buses/MosaicBus.h"
#include "buses/PixelBus.h"
#include "buses/SegmentBus.h"
#include "colors/Color.h"
#include "protocols/IProtocol.h"

namespace
{
    using TestColor = lw::Rgbcw8Color;

    class ProtocolStub : public lw::IProtocol<TestColor>
    {
    public:
        explicit ProtocolStub(uint16_t pixelCount)
            : lw::IProtocol<TestColor>(pixelCount)
        {
        }

        void initialize() override
        {
            ++initializeCount;
        }

        void update(lw::span<const TestColor> colors) override
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

            void update(lw::span<const TestColor> colors)
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

    class SpyBus : public lw::IAssignableBufferBus<TestColor>
    {
    public:
        explicit SpyBus(size_t count)
            : pixels(count)
            , _view(pixels.data(), pixels.size())
            , _pixelCount(static_cast<uint16_t>(count))
        {
        }

        explicit SpyBus(lw::span<TestColor> buffer)
            : _view(buffer)
            , _pixelCount(static_cast<uint16_t>(buffer.size()))
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

        uint16_t pixelCount() const override
        {
            return _pixelCount;
        }

        lw::span<TestColor> pixelBuffer() override
        {
            return _view;
        }

        void setBuffer(lw::span<TestColor> buffer) override
        {
            _view = buffer;
        }

        lw::span<const TestColor> pixelBuffer() const override
        {
            return lw::span<const TestColor>(_view.data(), _view.size());
        }

        std::vector<TestColor> pixels{};
        bool ready{true};
        int beginCount{0};
        int showCount{0};

    private:
        lw::span<TestColor> _view;
        uint16_t _pixelCount{0};
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

    size_t mosaic_pixel_count(const lw::MosaicBusSettings& cfg)
    {
        return static_cast<size_t>(cfg.panelWidth) *
               cfg.panelHeight *
               cfg.tilesWide *
               cfg.tilesHigh;
    }

    void set_colors_iter(lw::IPixelBus<TestColor> &bus, size_t offset, lw::span<const TestColor> source)
    {
        auto target = bus.pixelBuffer();
        if (offset >= target.size())
        {
            return;
        }

        const auto count = std::min(source.size(), target.size() - offset);
        std::copy_n(source.begin(), count, target.begin() + offset);
    }

    void get_colors_iter(const lw::IPixelBus<TestColor> &bus, size_t offset, lw::span<TestColor> dest)
    {
        auto source = bus.pixelBuffer();
        if (offset >= source.size())
        {
            return;
        }

        const auto count = std::min(dest.size(), source.size() - offset);
        std::copy_n(source.begin() + offset, count, dest.begin());
    }

    void assert_color_equal(const TestColor &a, const TestColor &b)
    {
        for (size_t ch = 0; ch < TestColor::ChannelCount; ++ch)
        {
            TEST_ASSERT_EQUAL_UINT8(a[ch], b[ch]);
        }
    }

    TestColor color_at(const lw::IPixelBus<TestColor>& bus, size_t index)
    {
        auto view = bus.pixelBuffer();
        if (index >= view.size())
        {
            return TestColor{};
        }

        return view[index];
    }

    void test_1_1_1_bulk_set_get_round_trip_iterator_and_span(void)
    {
        auto protocol = new ProtocolStub{8};
        lw::OwningPixelBusT<TestColor> bus(protocol);

        const auto sourceA = make_colors(8, 10);
        set_colors_iter(bus, 0, lw::span<const TestColor>(sourceA.data(), sourceA.size()));

        std::vector<TestColor> destA(8, TestColor{});
        get_colors_iter(bus, 0, lw::span<TestColor>(destA.data(), destA.size()));

        for (size_t idx = 0; idx < sourceA.size(); ++idx)
        {
            assert_color_equal(sourceA[idx], destA[idx]);
        }

        const auto sourceB = make_colors(8, 50);
        set_colors_iter(bus, 0, lw::span<const TestColor>(sourceB.data(), sourceB.size()));

        std::vector<TestColor> destB(8, TestColor{});
        get_colors_iter(bus, 0, lw::span<TestColor>(destB.data(), destB.size()));

        for (size_t idx = 0; idx < sourceB.size(); ++idx)
        {
            assert_color_equal(sourceB[idx], destB[idx]);
        }
    }

    void test_1_1_2_end_range_partial_write_clamp(void)
    {
        auto protocol = new ProtocolStub{8};
        lw::OwningPixelBusT<TestColor> bus(protocol);

        const auto baseline = make_colors(8, 1);
        set_colors_iter(bus, 0, lw::span<const TestColor>(baseline.data(), baseline.size()));

        const auto oversized = make_colors(5, 100);
        set_colors_iter(bus, 6, lw::span<const TestColor>(oversized.data(), oversized.size()));

        std::vector<TestColor> out(8, TestColor{});
        get_colors_iter(bus, 0, lw::span<TestColor>(out.data(), out.size()));

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
        lw::OwningPixelBusT<TestColor> bus(protocol);

        bus.show();
        TEST_ASSERT_EQUAL_INT(0, protocol->updateCount);

        auto pixels = bus.pixelBuffer();
        pixels[0] = color_for_index(11);
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
        lw::OwningPixelBusT<TestColor> bus(protocol);

        auto pixels = bus.pixelBuffer();
        pixels[0] = color_for_index(7);
        if (100 < pixels.size())
        {
            pixels[100] = color_for_index(99);
        }

        const auto inRange = color_at(bus, 0);
        const auto outRange = color_at(bus, 100);

        TEST_ASSERT_EQUAL_UINT8(7, inRange['R']);
        TEST_ASSERT_EQUAL_UINT8(0, outRange['R']);
        TEST_ASSERT_EQUAL_UINT8(0, outRange['G']);
        TEST_ASSERT_EQUAL_UINT8(0, outRange['B']);
    }

    void test_1_1_5_p0_offset_greater_than_pixel_count_no_op(void)
    {
        auto protocol = new ProtocolStub{4};
        lw::OwningPixelBusT<TestColor> bus(protocol);

        const auto baseline = make_colors(4, 20);
        set_colors_iter(bus, 0, lw::span<const TestColor>(baseline.data(), baseline.size()));

        const auto source = make_colors(3, 90);
        set_colors_iter(bus, 99, lw::span<const TestColor>(source.data(), source.size()));

        std::vector<TestColor> out(4, TestColor{});
        get_colors_iter(bus, 0, lw::span<TestColor>(out.data(), out.size()));
        for (size_t idx = 0; idx < out.size(); ++idx)
        {
            assert_color_equal(baseline[idx], out[idx]);
        }

        std::vector<TestColor> getSentinel(2, color_for_index(200));
        get_colors_iter(bus, 99, lw::span<TestColor>(getSentinel.data(), getSentinel.size()));
        assert_color_equal(color_for_index(200), getSentinel[0]);
        assert_color_equal(color_for_index(200), getSentinel[1]);
    }

    void test_1_2_1_segment_origin_mapping(void)
    {
        auto protocol = new ProtocolStub{10};
        lw::OwningPixelBusT<TestColor> parent(protocol);
        lw::SegmentBus<TestColor> segment(parent, 4, 3);

        const auto value = color_for_index(77);
        segment.pixelBuffer()[0] = value;

        assert_color_equal(value, color_at(parent, 4));
    }

    void test_1_2_2_segment_bulk_range_isolation(void)
    {
        auto protocol = new ProtocolStub{8};
        lw::OwningPixelBusT<TestColor> parent(protocol);
        const auto baseline = make_colors(8, 1);
        set_colors_iter(parent, 0, lw::span<const TestColor>(baseline.data(), baseline.size()));

        lw::SegmentBus<TestColor> segment(parent, 2, 4);
        const auto values = make_colors(4, 100);
        set_colors_iter(segment, 0, lw::span<const TestColor>(values.data(), values.size()));

        TEST_ASSERT_EQUAL_UINT8(1, color_at(parent, 0)['R']);
        TEST_ASSERT_EQUAL_UINT8(2, color_at(parent, 1)['R']);
        TEST_ASSERT_EQUAL_UINT8(100, color_at(parent, 2)['R']);
        TEST_ASSERT_EQUAL_UINT8(103, color_at(parent, 5)['R']);
        TEST_ASSERT_EQUAL_UINT8(7, color_at(parent, 6)['R']);
        TEST_ASSERT_EQUAL_UINT8(8, color_at(parent, 7)['R']);
    }

    void test_1_2_3_multi_segment_isolation(void)
    {
        auto protocol = new ProtocolStub{10};
        lw::OwningPixelBusT<TestColor> parent(protocol);
        const auto baseline = make_colors(10, 1);
        set_colors_iter(parent, 0, lw::span<const TestColor>(baseline.data(), baseline.size()));

        lw::SegmentBus<TestColor> segA(parent, 0, 5);
        lw::SegmentBus<TestColor> segB(parent, 5, 5);

        segA.pixelBuffer()[2] = color_for_index(200);
        TEST_ASSERT_EQUAL_UINT8(200, color_at(parent, 2)['R']);
        TEST_ASSERT_EQUAL_UINT8(6, color_at(parent, 5)['R']);

        segB.pixelBuffer()[1] = color_for_index(210);
        TEST_ASSERT_EQUAL_UINT8(210, color_at(parent, 6)['R']);
        TEST_ASSERT_EQUAL_UINT8(200, color_at(parent, 2)['R']);
    }

    void test_1_2_4_segment_offset_out_of_range_no_op(void)
    {
        auto protocol = new ProtocolStub{6};
        lw::OwningPixelBusT<TestColor> parent(protocol);
        const auto baseline = make_colors(6, 1);
        set_colors_iter(parent, 0, lw::span<const TestColor>(baseline.data(), baseline.size()));

        lw::SegmentBus<TestColor> segment(parent, 2, 3);
        const auto before = color_at(parent, 2);

        const auto source = make_colors(2, 90);
        set_colors_iter(segment, 3, lw::span<const TestColor>(source.data(), source.size()));

        std::vector<TestColor> out(2, color_for_index(199));
        get_colors_iter(segment, 3, lw::span<TestColor>(out.data(), out.size()));

        assert_color_equal(before, color_at(parent, 2));
        assert_color_equal(color_for_index(199), out[0]);
        assert_color_equal(color_for_index(199), out[1]);
    }

    void test_1_2_5_segment_oversize_clamp(void)
    {
        auto protocol = new ProtocolStub{8};
        lw::OwningPixelBusT<TestColor> parent(protocol);
        const auto baseline = make_colors(8, 1);
        set_colors_iter(parent, 0, lw::span<const TestColor>(baseline.data(), baseline.size()));

        lw::SegmentBus<TestColor> segment(parent, 2, 4);
        const auto source = make_colors(5, 120);
        set_colors_iter(segment, 3, lw::span<const TestColor>(source.data(), source.size()));

        TEST_ASSERT_EQUAL_UINT8(1, color_at(parent, 0)['R']);
        TEST_ASSERT_EQUAL_UINT8(5, color_at(parent, 4)['R']);
        TEST_ASSERT_EQUAL_UINT8(120, color_at(parent, 5)['R']);
        TEST_ASSERT_EQUAL_UINT8(7, color_at(parent, 6)['R']);
    }

    void test_1_3_1_concat_uneven_child_index_resolution(void)
    {
        auto owned = std::make_shared<std::vector<TestColor>>(9);
        SpyBus a(lw::span<TestColor>(owned->data(), 3));
        SpyBus b(lw::span<TestColor>(owned->data() + 3, 2));
        SpyBus c(lw::span<TestColor>(owned->data() + 5, 4));

        std::vector<lw::IAssignableBufferBus<TestColor> *> buses{&a, &b, &c};
        lw::ConcatBus<TestColor> concat(std::move(buses), lw::BufferHolder<TestColor>{owned->size(), owned->data(), false});

        auto pixels = concat.pixelBuffer();
        pixels[2] = color_for_index(10);
        pixels[3] = color_for_index(20);
        pixels[4] = color_for_index(30);
        pixels[5] = color_for_index(40);

        TEST_ASSERT_EQUAL_UINT8(10, color_at(a, 2)['R']);
        TEST_ASSERT_EQUAL_UINT8(20, color_at(b, 0)['R']);
        TEST_ASSERT_EQUAL_UINT8(30, color_at(b, 1)['R']);
        TEST_ASSERT_EQUAL_UINT8(40, color_at(c, 0)['R']);
    }

    void test_1_3_2_concat_pixel_count_aggregation(void)
    {
        auto owned = std::make_shared<std::vector<TestColor>>(15);
        SpyBus a(lw::span<TestColor>(owned->data(), 3));
        SpyBus b(lw::span<TestColor>(owned->data() + 3, 5));
        SpyBus c(lw::span<TestColor>(owned->data() + 8, 7));

        std::vector<lw::IAssignableBufferBus<TestColor> *> buses{&a, &b, &c};
        lw::ConcatBus<TestColor> concat(std::move(buses), lw::BufferHolder<TestColor>{owned->size(), owned->data(), false});
        TEST_ASSERT_EQUAL_UINT32(15U, static_cast<uint32_t>(concat.pixelBuffer().size()));
    }

    void test_1_3_3_concat_lifecycle_fan_out(void)
    {
        auto owned = std::make_shared<std::vector<TestColor>>(3);
        SpyBus a(lw::span<TestColor>(owned->data(), 1));
        SpyBus b(lw::span<TestColor>(owned->data() + 1, 1));
        SpyBus c(lw::span<TestColor>(owned->data() + 2, 1));

        std::vector<lw::IAssignableBufferBus<TestColor> *> buses{&a, &b, &c};
        lw::ConcatBus<TestColor> concat(std::move(buses), lw::BufferHolder<TestColor>{owned->size(), owned->data(), false});

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
        auto owned = std::make_shared<std::vector<TestColor>>(7);
        SpyBus a(lw::span<TestColor>(owned->data(), 2));
        SpyBus b(lw::span<TestColor>(owned->data() + 2, 3));
        SpyBus c(lw::span<TestColor>(owned->data() + 5, 2));

        std::vector<lw::IAssignableBufferBus<TestColor> *> buses{};
        buses.emplace_back(&a);
        buses.emplace_back(&b);
        buses.emplace_back(&c);

        lw::ConcatBus<TestColor> concat(std::move(buses), lw::BufferHolder<TestColor>{owned->size(), owned->data(), false});
        auto pixels = concat.pixelBuffer();
        pixels[3] = color_for_index(99);

        TEST_ASSERT_EQUAL_UINT8(99, color_at(b, 1)['R']);
        TEST_ASSERT_EQUAL_UINT8(0, color_at(c, 0)['R']);
    }

    void test_1_3_5_concat_invalid_remove_add_behavior(void)
    {
        auto owned = std::make_shared<std::vector<TestColor>>(4);
        SpyBus a(lw::span<TestColor>(owned->data(), 2));
        SpyBus b(lw::span<TestColor>(owned->data() + 2, 2));
        lw::ConcatBus<TestColor> concat(std::vector<lw::IAssignableBufferBus<TestColor>*>{&a, &b}, lw::BufferHolder<TestColor>{owned->size(), owned->data(), false});

        auto pixels = concat.pixelBuffer();
        pixels[0] = color_for_index(10);
        pixels[3] = color_for_index(20);

        TEST_ASSERT_EQUAL_UINT8(10, color_at(a, 0)['R']);
        TEST_ASSERT_EQUAL_UINT8(20, color_at(b, 1)['R']);
        TEST_ASSERT_EQUAL_UINT32(4U, static_cast<uint32_t>(concat.pixelBuffer().size()));
    }

    void test_1_4_1_mosaic_2d_coordinate_mapping(void)
    {
        lw::MosaicBusSettings cfg{};
        cfg.panelWidth = 2;
        cfg.panelHeight = 2;
        cfg.layout = lw::PanelLayout::RowMajor;
        cfg.tilesWide = 2;
        cfg.tilesHigh = 1;
        cfg.tileLayout = lw::PanelLayout::RowMajor;

        auto owned = std::make_shared<std::vector<TestColor>>(mosaic_pixel_count(cfg));
        const size_t panelPixels = static_cast<size_t>(cfg.panelWidth) * cfg.panelHeight;
        SpyBus p0(lw::span<TestColor>(owned->data(), panelPixels));
        SpyBus p1(lw::span<TestColor>(owned->data() + panelPixels, panelPixels));

        std::vector<lw::IAssignableBufferBus<TestColor> *> buses{};
        buses.emplace_back(&p0);
        buses.emplace_back(&p1);
        lw::MosaicBus<TestColor> mosaic(cfg,
                         std::move(buses),
                         lw::BufferHolder<TestColor>{owned->size(), owned->data(), false});

        auto pixels = mosaic.pixelBuffer();
        pixels[mosaic.topology().getIndex(0, 0)] = color_for_index(10);
        pixels[mosaic.topology().getIndex(1, 1)] = color_for_index(20);
        pixels[mosaic.topology().getIndex(2, 0)] = color_for_index(30);
        pixels[mosaic.topology().getIndex(3, 1)] = color_for_index(40);

        mosaic.show();

        TEST_ASSERT_EQUAL_UINT8(10, color_at(p0, 0)['R']);
        TEST_ASSERT_EQUAL_UINT8(20, color_at(p0, 3)['R']);
        TEST_ASSERT_EQUAL_UINT8(30, color_at(p1, 0)['R']);
        TEST_ASSERT_EQUAL_UINT8(40, color_at(p1, 3)['R']);
    }

    void test_1_4_2_mosaic_linear_flattening_consistency(void)
    {
        lw::MosaicBusSettings cfg{};
        cfg.panelWidth = 2;
        cfg.panelHeight = 2;
        cfg.layout = lw::PanelLayout::RowMajor;
        cfg.tilesWide = 2;
        cfg.tilesHigh = 1;
        cfg.tileLayout = lw::PanelLayout::RowMajor;

        auto owned = std::make_shared<std::vector<TestColor>>(mosaic_pixel_count(cfg));
        const size_t panelPixels = static_cast<size_t>(cfg.panelWidth) * cfg.panelHeight;
        SpyBus p0(lw::span<TestColor>(owned->data(), panelPixels));
        SpyBus p1(lw::span<TestColor>(owned->data() + panelPixels, panelPixels));

        std::vector<lw::IAssignableBufferBus<TestColor> *> buses{};
        buses.emplace_back(&p0);
        buses.emplace_back(&p1);
        lw::MosaicBus<TestColor> mosaic(cfg,
                         std::move(buses),
                         lw::BufferHolder<TestColor>{owned->size(), owned->data(), false});

        const auto linear = make_colors(8, 50);
        auto mosaicBuffer = mosaic.pixelBuffer();
        std::copy(linear.begin(), linear.end(), mosaicBuffer.begin());

        mosaic.show();

        std::vector<TestColor> roundTrip(8, TestColor{});
        std::copy(mosaic.pixelBuffer().begin(), mosaic.pixelBuffer().end(), roundTrip.begin());

        for (size_t idx = 0; idx < linear.size(); ++idx)
        {
            assert_color_equal(linear[idx], roundTrip[idx]);
        }

        assert_color_equal(linear[0], mosaic.pixelBuffer()[mosaic.topology().getIndex(0, 0)]);
        assert_color_equal(linear[3], mosaic.pixelBuffer()[mosaic.topology().getIndex(1, 1)]);
        assert_color_equal(linear[4], mosaic.pixelBuffer()[mosaic.topology().getIndex(2, 0)]);
        assert_color_equal(linear[7], mosaic.pixelBuffer()[mosaic.topology().getIndex(3, 1)]);
    }

    void test_1_4_3_mosaic_can_show_all_children_gate(void)
    {
        lw::MosaicBusSettings cfg{};
        cfg.panelWidth = 1;
        cfg.panelHeight = 1;
        cfg.layout = lw::PanelLayout::RowMajor;
        cfg.tilesWide = 2;
        cfg.tilesHigh = 1;
        cfg.tileLayout = lw::PanelLayout::RowMajor;

        auto owned = std::make_shared<std::vector<TestColor>>(mosaic_pixel_count(cfg));
        SpyBus p0(lw::span<TestColor>(owned->data(), 1));
        SpyBus p1(lw::span<TestColor>(owned->data() + 1, 1));

        std::vector<lw::IAssignableBufferBus<TestColor> *> buses{};
        buses.emplace_back(&p0);
        buses.emplace_back(&p1);
        lw::MosaicBus<TestColor> mosaic(cfg,
                         std::move(buses),
                         lw::BufferHolder<TestColor>{owned->size(), owned->data(), false});

        p0.ready = true;
        p1.ready = false;
        TEST_ASSERT_FALSE(mosaic.canShow());

        p1.ready = true;
        TEST_ASSERT_TRUE(mosaic.canShow());
    }

    void test_1_4_4_mosaic_out_of_bounds_2d_safety(void)
    {
        lw::MosaicBusSettings cfg{};
        cfg.panelWidth = 2;
        cfg.panelHeight = 2;
        cfg.layout = lw::PanelLayout::RowMajor;
        cfg.tilesWide = 1;
        cfg.tilesHigh = 1;
        cfg.tileLayout = lw::PanelLayout::RowMajor;

        auto owned = std::make_shared<std::vector<TestColor>>(mosaic_pixel_count(cfg));
        SpyBus p0(lw::span<TestColor>(owned->data(), owned->size()));

        std::vector<lw::IAssignableBufferBus<TestColor> *> buses{};
        buses.emplace_back(&p0);
        lw::MosaicBus<TestColor> mosaic(cfg,
                         std::move(buses),
                         lw::BufferHolder<TestColor>{owned->size(), owned->data(), false});

        const auto before = color_at(p0, 0);

        const auto idxA = mosaic.topology().getIndex(-1, 0);
        const auto idxB = mosaic.topology().getIndex(0, 5);
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(lw::Topology::InvalidIndex), static_cast<uint32_t>(idxA));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(lw::Topology::InvalidIndex), static_cast<uint32_t>(idxB));

        mosaic.show();
        assert_color_equal(before, color_at(p0, 0));
    }

    void test_1_4_5_mosaic_sparse_tile_safety_and_empty_geometry(void)
    {
        {
            lw::MosaicBusSettings cfg{};
            cfg.panelWidth = 2;
            cfg.panelHeight = 2;
            cfg.layout = lw::PanelLayout::RowMajor;
            cfg.tilesWide = 2;
            cfg.tilesHigh = 2;
            cfg.tileLayout = lw::PanelLayout::RowMajor;

            auto owned = std::make_shared<std::vector<TestColor>>(mosaic_pixel_count(cfg));
            std::fill(owned->begin(), owned->end(), TestColor{0, 0, 0, 0, 0});
            const size_t panelPixels = static_cast<size_t>(cfg.panelWidth) * cfg.panelHeight;
            SpyBus p0(lw::span<TestColor>(owned->data(), panelPixels));
            SpyBus p1(lw::span<TestColor>(owned->data() + panelPixels, panelPixels));
            SpyBus p2(lw::span<TestColor>(owned->data() + (2 * panelPixels), panelPixels));

            std::vector<lw::IAssignableBufferBus<TestColor> *> buses{};
            buses.emplace_back(&p0);
            buses.emplace_back(&p1);
            buses.emplace_back(&p2);
            lw::MosaicBus<TestColor> sparse(cfg,
                                             std::move(buses),
                                             lw::BufferHolder<TestColor>{owned->size(), owned->data(), false});

            const auto beforeP0 = color_at(p0, 0);
            const auto beforeP1 = color_at(p1, 0);
            const auto beforeP2 = color_at(p2, 0);

            auto sparseIndex = sparse.topology().getIndex(3, 3);
            sparse.pixelBuffer()[sparseIndex] = color_for_index(123);
            sparse.show();

            assert_color_equal(beforeP0, color_at(p0, 0));
            assert_color_equal(beforeP1, color_at(p1, 0));
            assert_color_equal(beforeP2, color_at(p2, 0));
        }

        {
            lw::MosaicBusSettings cfg{};
            cfg.panelWidth = 2;
            cfg.panelHeight = 2;
            cfg.layout = lw::PanelLayout::RowMajor;
            cfg.tilesWide = 2;
            cfg.tilesHigh = 2;
            cfg.tileLayout = lw::PanelLayout::RowMajor;

            std::vector<lw::IAssignableBufferBus<TestColor> *> none{};
            lw::MosaicBus<TestColor> empty(cfg,
                                            std::move(none),
                                            lw::BufferHolder<TestColor>{mosaic_pixel_count(cfg), nullptr, true});

            TEST_ASSERT_EQUAL_UINT16(4, empty.width());
            TEST_ASSERT_EQUAL_UINT16(4, empty.height());
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

