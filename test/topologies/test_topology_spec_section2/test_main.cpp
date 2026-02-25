#include <unity.h>

#include <array>
#include <cstdint>

#include "topologies/PanelLayout.h"
#include "topologies/PanelTopology.h"
#include "topologies/TiledTopology.h"

namespace
{
    using npb::PanelLayout;

    struct LayoutGolden
    {
        PanelLayout layout;
        std::array<uint16_t, 16> values;
    };

    void test_2_1_1_panel_layout_all_layout_golden_mapping_4x4(void)
    {
        const std::array<LayoutGolden, 16> goldens{{
            {PanelLayout::RowMajor, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}},
            {PanelLayout::RowMajor90, {12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3}},
            {PanelLayout::RowMajor180, {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0}},
            {PanelLayout::RowMajor270, {3, 7, 11, 15, 2, 6, 10, 14, 1, 5, 9, 13, 0, 4, 8, 12}},

            {PanelLayout::RowMajorAlternating, {0, 1, 2, 3, 7, 6, 5, 4, 8, 9, 10, 11, 15, 14, 13, 12}},
            {PanelLayout::RowMajorAlternating90, {15, 8, 7, 0, 14, 9, 6, 1, 13, 10, 5, 2, 12, 11, 4, 3}},
            {PanelLayout::RowMajorAlternating180, {12, 13, 14, 15, 11, 10, 9, 8, 4, 5, 6, 7, 3, 2, 1, 0}},
            {PanelLayout::RowMajorAlternating270, {3, 4, 11, 12, 2, 5, 10, 13, 1, 6, 9, 14, 0, 7, 8, 15}},

            {PanelLayout::ColumnMajor, {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15}},
            {PanelLayout::ColumnMajor90, {3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12}},
            {PanelLayout::ColumnMajor180, {15, 11, 7, 3, 14, 10, 6, 2, 13, 9, 5, 1, 12, 8, 4, 0}},
            {PanelLayout::ColumnMajor270, {12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3}},

            {PanelLayout::ColumnMajorAlternating, {0, 7, 8, 15, 1, 6, 9, 14, 2, 5, 10, 13, 3, 4, 11, 12}},
            {PanelLayout::ColumnMajorAlternating90, {3, 2, 1, 0, 4, 5, 6, 7, 11, 10, 9, 8, 12, 13, 14, 15}},
            {PanelLayout::ColumnMajorAlternating180, {12, 11, 4, 3, 13, 10, 5, 2, 14, 9, 6, 1, 15, 8, 7, 0}},
            {PanelLayout::ColumnMajorAlternating270, {15, 14, 13, 12, 8, 9, 10, 11, 7, 6, 5, 4, 0, 1, 2, 3}},
        }};

        for (const auto &golden : goldens)
        {
            for (uint16_t y = 0; y < 4; ++y)
            {
                for (uint16_t x = 0; x < 4; ++x)
                {
                    const uint16_t actual = npb::mapLayout(golden.layout, 4, 4, x, y);
                    const uint16_t expected = golden.values[static_cast<size_t>(y) * 4U + x];
                    TEST_ASSERT_EQUAL_UINT16(expected, actual);
                }
            }
        }
    }

    void test_2_2_1_tile_preferred_layout_parity_selection(void)
    {
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::RowMajor),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::RowMajor180, false, false)));
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::RowMajor270),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::RowMajor180, false, true)));
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::RowMajor90),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::RowMajor180, true, false)));
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::RowMajor180),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::RowMajor180, true, true)));

        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::RowMajorAlternating270),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::RowMajorAlternating90, false, false)));
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::RowMajorAlternating90),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::RowMajorAlternating90, true, true)));

        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::ColumnMajor),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::ColumnMajor270, false, false)));
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::ColumnMajor270),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::ColumnMajor270, false, true)));
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::ColumnMajor90),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::ColumnMajor270, true, false)));
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::ColumnMajor180),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::ColumnMajor270, true, true)));

        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::ColumnMajorAlternating),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::ColumnMajorAlternating90, false, false)));
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::ColumnMajorAlternating180),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::ColumnMajorAlternating90, true, false)));
    }

    void test_2_3_1_panel_topology_in_bounds_probe_mapping(void)
    {
        npb::PanelTopology topology(4, 3, PanelLayout::RowMajorAlternating);

        const auto a = topology.mapProbe(0, 0);
        const auto b = topology.mapProbe(3, 0);
        const auto c = topology.mapProbe(0, 1);
        const auto d = topology.mapProbe(3, 1);

        TEST_ASSERT_TRUE(a.has_value());
        TEST_ASSERT_TRUE(b.has_value());
        TEST_ASSERT_TRUE(c.has_value());
        TEST_ASSERT_TRUE(d.has_value());

        TEST_ASSERT_EQUAL_UINT16(0, *a);
        TEST_ASSERT_EQUAL_UINT16(3, *b);
        TEST_ASSERT_EQUAL_UINT16(7, *c);
        TEST_ASSERT_EQUAL_UINT16(4, *d);
    }

    void test_2_3_2_panel_topology_clamped_map_behavior(void)
    {
        npb::PanelTopology topology(4, 3, PanelLayout::RowMajor);

        TEST_ASSERT_EQUAL_UINT16(0, topology.map(-5, -9));
        TEST_ASSERT_EQUAL_UINT16(11, topology.map(99, 99));
        TEST_ASSERT_EQUAL_UINT16(8, topology.map(-1, 2));
    }

    void test_2_3_3_panel_topology_pixel_count_invariant(void)
    {
        npb::PanelTopology topology(7, 5, PanelLayout::ColumnMajor);
        TEST_ASSERT_EQUAL_UINT16(35, topology.pixelCount());
    }

    void test_2_3_4_panel_topology_out_of_bounds_probe_nullopt(void)
    {
        npb::PanelTopology topology(3, 3, PanelLayout::RowMajor);

        TEST_ASSERT_FALSE(topology.mapProbe(-1, 0).has_value());
        TEST_ASSERT_FALSE(topology.mapProbe(0, -1).has_value());
        TEST_ASSERT_FALSE(topology.mapProbe(3, 1).has_value());
        TEST_ASSERT_FALSE(topology.mapProbe(1, 3).has_value());
    }

    void test_2_4_1_tiled_topology_cross_tile_probe_correctness(void)
    {
        npb::TiledTopology topology({
            .panelWidth = 2,
            .panelHeight = 2,
            .tilesWide = 2,
            .tilesHigh = 2,
            .panelLayout = PanelLayout::RowMajor,
            .tileLayout = PanelLayout::RowMajor,
            .mosaicRotation = false,
        });

        TEST_ASSERT_EQUAL_UINT16(0, *topology.mapProbe(0, 0));
        TEST_ASSERT_EQUAL_UINT16(3, *topology.mapProbe(1, 1));
        TEST_ASSERT_EQUAL_UINT16(4, *topology.mapProbe(2, 0));
        TEST_ASSERT_EQUAL_UINT16(8, *topology.mapProbe(0, 2));
        TEST_ASSERT_EQUAL_UINT16(15, *topology.mapProbe(3, 3));
    }

    void test_2_4_2_tiled_topology_global_edge_clamp_behavior(void)
    {
        npb::TiledTopology topology({
            .panelWidth = 2,
            .panelHeight = 2,
            .tilesWide = 2,
            .tilesHigh = 2,
            .panelLayout = PanelLayout::RowMajor,
            .tileLayout = PanelLayout::RowMajor,
            .mosaicRotation = false,
        });

        TEST_ASSERT_EQUAL_UINT16(0, topology.map(-9, -3));
        TEST_ASSERT_EQUAL_UINT16(15, topology.map(999, 999));
    }

    void test_2_4_3_tiled_topology_hint_classification(void)
    {
        npb::TiledTopology topology({
            .panelWidth = 2,
            .panelHeight = 2,
            .tilesWide = 2,
            .tilesHigh = 1,
            .panelLayout = PanelLayout::RowMajor,
            .tileLayout = PanelLayout::RowMajor,
            .mosaicRotation = false,
        });

        TEST_ASSERT_EQUAL_INT(static_cast<int>(npb::TiledTopology::TopologyHint::FirstOnPanel),
                              static_cast<int>(topology.topologyHint(0, 0)));
        TEST_ASSERT_EQUAL_INT(static_cast<int>(npb::TiledTopology::TopologyHint::InPanel),
                              static_cast<int>(topology.topologyHint(1, 0)));
        TEST_ASSERT_EQUAL_INT(static_cast<int>(npb::TiledTopology::TopologyHint::LastOnPanel),
                              static_cast<int>(topology.topologyHint(1, 1)));
        TEST_ASSERT_EQUAL_INT(static_cast<int>(npb::TiledTopology::TopologyHint::OutOfBounds),
                              static_cast<int>(topology.topologyHint(-1, 0)));
    }

    void test_2_4_4_tiled_topology_out_of_bounds_probe_safety(void)
    {
        npb::TiledTopology topology({
            .panelWidth = 2,
            .panelHeight = 2,
            .tilesWide = 2,
            .tilesHigh = 2,
            .panelLayout = PanelLayout::RowMajor,
            .tileLayout = PanelLayout::RowMajor,
            .mosaicRotation = false,
        });

        TEST_ASSERT_FALSE(topology.mapProbe(-1, 0).has_value());
        TEST_ASSERT_FALSE(topology.mapProbe(0, -1).has_value());
        TEST_ASSERT_FALSE(topology.mapProbe(4, 0).has_value());
        TEST_ASSERT_FALSE(topology.mapProbe(0, 4).has_value());
    }

    void test_2_4_5_tiled_topology_zero_dimension_config_guard(void)
    {
        npb::TiledTopology zeroWidth({
            .panelWidth = 0,
            .panelHeight = 2,
            .tilesWide = 2,
            .tilesHigh = 2,
            .panelLayout = PanelLayout::RowMajor,
            .tileLayout = PanelLayout::RowMajor,
            .mosaicRotation = false,
        });

        npb::TiledTopology zeroHeight({
            .panelWidth = 2,
            .panelHeight = 0,
            .tilesWide = 2,
            .tilesHigh = 2,
            .panelLayout = PanelLayout::RowMajor,
            .tileLayout = PanelLayout::RowMajor,
            .mosaicRotation = false,
        });

        TEST_ASSERT_EQUAL_UINT16(0, zeroWidth.map(5, 5));
        TEST_ASSERT_EQUAL_UINT16(0, zeroHeight.map(-1, -1));
        TEST_ASSERT_FALSE(zeroWidth.mapProbe(0, 0).has_value());
        TEST_ASSERT_FALSE(zeroHeight.mapProbe(0, 0).has_value());
    }

    void test_2_4_6_tiled_topology_non_existent_tile_probe_boundedness(void)
    {
        npb::TiledTopology topology({
            .panelWidth = 2,
            .panelHeight = 2,
            .tilesWide = 2,
            .tilesHigh = 1,
            .panelLayout = PanelLayout::RowMajor,
            .tileLayout = PanelLayout::RowMajor,
            .mosaicRotation = false,
        });

        TEST_ASSERT_FALSE(topology.mapProbe(4, 0).has_value());
        TEST_ASSERT_FALSE(topology.mapProbe(100, 1).has_value());
        TEST_ASSERT_EQUAL_UINT16(5, topology.map(100, 0));
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
    RUN_TEST(test_2_1_1_panel_layout_all_layout_golden_mapping_4x4);
    RUN_TEST(test_2_2_1_tile_preferred_layout_parity_selection);
    RUN_TEST(test_2_3_1_panel_topology_in_bounds_probe_mapping);
    RUN_TEST(test_2_3_2_panel_topology_clamped_map_behavior);
    RUN_TEST(test_2_3_3_panel_topology_pixel_count_invariant);
    RUN_TEST(test_2_3_4_panel_topology_out_of_bounds_probe_nullopt);
    RUN_TEST(test_2_4_1_tiled_topology_cross_tile_probe_correctness);
    RUN_TEST(test_2_4_2_tiled_topology_global_edge_clamp_behavior);
    RUN_TEST(test_2_4_3_tiled_topology_hint_classification);
    RUN_TEST(test_2_4_4_tiled_topology_out_of_bounds_probe_safety);
    RUN_TEST(test_2_4_5_tiled_topology_zero_dimension_config_guard);
    RUN_TEST(test_2_4_6_tiled_topology_non_existent_tile_probe_boundedness);
    return UNITY_END();
}

