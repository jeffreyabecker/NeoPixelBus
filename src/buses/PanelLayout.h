#pragma once

#include <cstdint>

namespace lw
{

    enum class PanelLayout : uint8_t
    {
        RowMajor = 0,            // Scan rows left-to-right, top-to-bottom.
        RowMajor90 = 1,          // Row-major mapping rotated 90 degrees.
        RowMajor180 = 2,         // Row-major mapping rotated 180 degrees.
        RowMajor270 = 3,         // Row-major mapping rotated 270 degrees.

        RowMajorAlternating = 4,    // Row-major serpentine: odd rows reverse direction.
        RowMajorAlternating90 = 5,  // Serpentine row-major mapping rotated 90 degrees.
        RowMajorAlternating180 = 6, // Serpentine row-major mapping rotated 180 degrees.
        RowMajorAlternating270 = 7, // Serpentine row-major mapping rotated 270 degrees.

        ColumnMajor = 8,         // Scan columns top-to-bottom, left-to-right.
        ColumnMajor90 = 9,       // Column-major mapping rotated 90 degrees.
        ColumnMajor180 = 10,     // Column-major mapping rotated 180 degrees.
        ColumnMajor270 = 11,     // Column-major mapping rotated 270 degrees.

        ColumnMajorAlternating = 12,    // Column-major serpentine: odd columns reverse direction.
        ColumnMajorAlternating90 = 13,  // Serpentine column-major mapping rotated 90 degrees.
        ColumnMajorAlternating180 = 14, // Serpentine column-major mapping rotated 180 degrees.
        ColumnMajorAlternating270 = 15, // Serpentine column-major mapping rotated 270 degrees.
    };

    constexpr uint16_t mapLayout(PanelLayout layout,
                                 uint16_t width, uint16_t height,
                                 uint16_t x, uint16_t y)
    {
        switch (layout)
        {
        case PanelLayout::RowMajor:
            return x + y * width;

        case PanelLayout::RowMajor90:
            return (width - 1 - x) * height + y;

        case PanelLayout::RowMajor180:
            return (width - 1 - x) + (height - 1 - y) * width;

        case PanelLayout::RowMajor270:
            return x * height + (height - 1 - y);

        case PanelLayout::RowMajorAlternating:
        {
            uint16_t index = y * width;
            return (y & 1) ? index + (width - 1 - x) : index + x;
        }

        case PanelLayout::RowMajorAlternating90:
        {
            uint16_t mx = (width - 1) - x;
            uint16_t index = mx * height;
            return (mx & 1) ? index + (height - 1 - y) : index + y;
        }

        case PanelLayout::RowMajorAlternating180:
        {
            uint16_t my = (height - 1) - y;
            uint16_t index = my * width;
            return (my & 1) ? index + x : index + (width - 1 - x);
        }

        case PanelLayout::RowMajorAlternating270:
        {
            uint16_t index = x * height;
            return (x & 1) ? index + y : index + (height - 1 - y);
        }

        case PanelLayout::ColumnMajor:
            return x * height + y;

        case PanelLayout::ColumnMajor90:
            return (width - 1 - x) + y * width;

        case PanelLayout::ColumnMajor180:
            return (width - 1 - x) * height + (height - 1 - y);

        case PanelLayout::ColumnMajor270:
            return x + (height - 1 - y) * width;

        case PanelLayout::ColumnMajorAlternating:
        {
            uint16_t index = x * height;
            return (x & 1) ? index + (height - 1 - y) : index + y;
        }

        case PanelLayout::ColumnMajorAlternating90:
        {
            uint16_t index = y * width;
            return (y & 1) ? index + x : index + (width - 1 - x);
        }

        case PanelLayout::ColumnMajorAlternating180:
        {
            uint16_t mx = (width - 1) - x;
            uint16_t index = mx * height;
            return (mx & 1) ? index + y : index + (height - 1 - y);
        }

        case PanelLayout::ColumnMajorAlternating270:
        {
            uint16_t my = (height - 1) - y;
            uint16_t index = my * width;
            return (my & 1) ? index + (width - 1 - x) : index + x;
        }
        }

        return 0;
    }

    constexpr PanelLayout tilePreferredLayout(PanelLayout baseLayout,
                                              bool oddRow, bool oddColumn)
    {
        auto group = static_cast<uint8_t>(baseLayout) / 4;

        switch (group)
        {
        case 0:
            if (!oddRow && !oddColumn)  return PanelLayout::RowMajor;
            if (!oddRow && oddColumn)   return PanelLayout::RowMajor270;
            if (oddRow && !oddColumn)   return PanelLayout::RowMajor90;
            return PanelLayout::RowMajor180;

        case 1:
            if (!oddRow)  return PanelLayout::RowMajorAlternating270;
            return PanelLayout::RowMajorAlternating90;

        case 2:
            if (!oddRow && !oddColumn)  return PanelLayout::ColumnMajor;
            if (!oddRow && oddColumn)   return PanelLayout::ColumnMajor270;
            if (oddRow && !oddColumn)   return PanelLayout::ColumnMajor90;
            return PanelLayout::ColumnMajor180;

        case 3:
            if (!oddRow)  return PanelLayout::ColumnMajorAlternating;
            return PanelLayout::ColumnMajorAlternating180;
        }

        return baseLayout;
    }

} // namespace lw
