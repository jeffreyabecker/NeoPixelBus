#include <unity.h>

#include <array>
#include <cstdint>

#include "buses/MosaicBusSettings.h"
#include "buses/PanelLayout.h"
#include "buses/Topology.h"

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

        for (const auto& golden : goldens)
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

        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::ColumnMajor),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::ColumnMajor270, false, false)));
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PanelLayout::ColumnMajor180),
                                static_cast<uint8_t>(npb::tilePreferredLayout(PanelLayout::ColumnMajor270, true, true)));
    }

    void test_2_3_1_topology_dimensions(void)
    {
        npb::MosaicBusSettings settings{2, 3, PanelLayout::RowMajor, 4, 5, PanelLayout::RowMajor, false};
        npb::Topology topology(settings);

        TEST_ASSERT_EQUAL_UINT16(8, topology.width());
        TEST_ASSERT_EQUAL_UINT16(15, topology.height());
        TEST_ASSERT_EQUAL_UINT32(120, static_cast<uint32_t>(topology.pixelCount()));
    }

    void test_2_3_2_topology_global_index_mapping_no_rotation(void)
    {
        npb::MosaicBusSettings settings{2, 2, PanelLayout::RowMajor, 2, 2, PanelLayout::RowMajor, false};
        npb::Topology topology(settings);

        TEST_ASSERT_EQUAL_UINT16(0, static_cast<uint16_t>(topology.getIndex(0, 0)));
        TEST_ASSERT_EQUAL_UINT16(3, static_cast<uint16_t>(topology.getIndex(1, 1)));
        TEST_ASSERT_EQUAL_UINT16(4, static_cast<uint16_t>(topology.getIndex(2, 0)));
        TEST_ASSERT_EQUAL_UINT16(8, static_cast<uint16_t>(topology.getIndex(0, 2)));
        TEST_ASSERT_EQUAL_UINT16(15, static_cast<uint16_t>(topology.getIndex(3, 3)));
    }

    void test_2_3_3_topology_out_of_bounds_and_zero_dimension_guard(void)
    {
        npb::MosaicBusSettings normal{2, 2, PanelLayout::RowMajor, 2, 2, PanelLayout::RowMajor, false};
        npb::Topology topology(normal);

        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::Topology::InvalidIndex), static_cast<uint32_t>(topology.getIndex(-1, 0)));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::Topology::InvalidIndex), static_cast<uint32_t>(topology.getIndex(0, -1)));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::Topology::InvalidIndex), static_cast<uint32_t>(topology.getIndex(4, 0)));
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::Topology::InvalidIndex), static_cast<uint32_t>(topology.getIndex(0, 4)));

        npb::MosaicBusSettings zeroWidth{0, 2, PanelLayout::RowMajor, 2, 2, PanelLayout::RowMajor, false};
        npb::Topology invalid(zeroWidth);
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(npb::Topology::InvalidIndex), static_cast<uint32_t>(invalid.getIndex(0, 0)));
    }

    void test_2_3_4_topology_rotation_preference_integration(void)
    {
        npb::MosaicBusSettings settings{2, 2, PanelLayout::RowMajor180, 2, 1, PanelLayout::RowMajor, true};
        npb::Topology topology(settings);

        TEST_ASSERT_EQUAL_UINT16(0, static_cast<uint16_t>(topology.getIndex(0, 0)));
        TEST_ASSERT_EQUAL_UINT16(5, static_cast<uint16_t>(topology.getIndex(2, 0)));
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
    RUN_TEST(test_2_1_1_panel_layout_all_layout_golden_mapping_4x4);
    RUN_TEST(test_2_2_1_tile_preferred_layout_parity_selection);
    RUN_TEST(test_2_3_1_topology_dimensions);
    RUN_TEST(test_2_3_2_topology_global_index_mapping_no_rotation);
    RUN_TEST(test_2_3_3_topology_out_of_bounds_and_zero_dimension_guard);
    RUN_TEST(test_2_3_4_topology_rotation_preference_integration);
    return UNITY_END();
}
