#pragma once

#include <cstdint>

namespace npb
{

    // -------------------------------------------------------------------
    // PanelLayout ? 16 wiring patterns for 2D LED panels
    //
    // 4 base layouts ? 4 rotations (0?, 90?, 180?, 270?).
    // Each layout defines how physical pixel wiring maps a 2D (x, y)
    // coordinate to a linear 1D strip index.
    //
    // The enum values are grouped so that integer division by 4 yields
    // the base group (RowMajor, RowMajorAlternating, ColumnMajor,
    // ColumnMajorAlternating) and the remainder yields the rotation.
    // -------------------------------------------------------------------
    enum class PanelLayout : uint8_t
    {
        // Group 0: RowMajor ? left-to-right, top-to-bottom
        RowMajor              = 0,
        RowMajor90            = 1,
        RowMajor180           = 2,
        RowMajor270           = 3,

        // Group 1: RowMajorAlternating ? serpentine (zig-zag) rows
        RowMajorAlternating       = 4,
        RowMajorAlternating90     = 5,
        RowMajorAlternating180    = 6,
        RowMajorAlternating270    = 7,

        // Group 2: ColumnMajor ? top-to-bottom, left-to-right
        ColumnMajor               = 8,
        ColumnMajor90             = 9,
        ColumnMajor180            = 10,
        ColumnMajor270            = 11,

        // Group 3: ColumnMajorAlternating ? serpentine columns
        ColumnMajorAlternating       = 12,
        ColumnMajorAlternating90     = 13,
        ColumnMajorAlternating180    = 14,
        ColumnMajorAlternating270    = 15,
    };

    // -------------------------------------------------------------------
    // mapLayout ? pure (x, y) ? linear index mapping
    //
    // No bounds checking.  Caller must ensure 0 <= x < width and
    // 0 <= y < height.  All formulas are taken directly from the
    // original NeoPixelBus layout classes.
    // -------------------------------------------------------------------
    constexpr uint16_t mapLayout(PanelLayout layout,
                                 uint16_t width, uint16_t height,
                                 uint16_t x, uint16_t y)
    {
        switch (layout)
        {
        // -- RowMajor ----------------------------------------------------
        //  00  01  02  03
        //  04  05  06  07
        //  08  09  10  11
        //  12  13  14  15
        case PanelLayout::RowMajor:
            return x + y * width;

        //  12  08  04  00
        //  13  09  05  01
        //  14  10  06  02
        //  15  11  07  03
        case PanelLayout::RowMajor90:
            return (width - 1 - x) * height + y;

        //  15  14  13  12
        //  11  10  09  08
        //  07  06  05  04
        //  03  02  01  00
        case PanelLayout::RowMajor180:
            return (width - 1 - x) + (height - 1 - y) * width;

        //  03  07  11  15
        //  02  06  10  14
        //  01  05  09  13
        //  00  04  08  12
        case PanelLayout::RowMajor270:
            return x * height + (height - 1 - y);

        // -- RowMajorAlternating -----------------------------------------
        //  00  01  02  03
        //  07  06  05  04
        //  08  09  10  11
        //  15  14  13  12
        case PanelLayout::RowMajorAlternating:
        {
            uint16_t index = y * width;
            return (y & 1) ? index + (width - 1 - x) : index + x;
        }

        //  15  08  07  00
        //  14  09  06  01
        //  13  10  05  02
        //  12  11  04  03
        case PanelLayout::RowMajorAlternating90:
        {
            uint16_t mx = (width - 1) - x;
            uint16_t index = mx * height;
            return (mx & 1) ? index + (height - 1 - y) : index + y;
        }

        //  12  13  14  15
        //  11  10  09  08
        //  04  05  06  07
        //  03  02  01  00
        case PanelLayout::RowMajorAlternating180:
        {
            uint16_t my = (height - 1) - y;
            uint16_t index = my * width;
            return (my & 1) ? index + x : index + (width - 1 - x);
        }

        //  03  04  11  12
        //  02  05  10  13
        //  01  06  09  14
        //  00  07  08  15
        case PanelLayout::RowMajorAlternating270:
        {
            uint16_t index = x * height;
            return (x & 1) ? index + y : index + (height - 1 - y);
        }

        // -- ColumnMajor -------------------------------------------------
        //  00  04  08  12
        //  01  05  09  13
        //  02  06  10  14
        //  03  07  11  15
        case PanelLayout::ColumnMajor:
            return x * height + y;

        //  03  02  01  00
        //  07  06  05  04
        //  11  10  09  08
        //  15  14  13  12
        case PanelLayout::ColumnMajor90:
            return (width - 1 - x) + y * width;

        //  15  11  07  03
        //  14  10  06  02
        //  13  09  05  01
        //  12  08  04  00
        case PanelLayout::ColumnMajor180:
            return (width - 1 - x) * height + (height - 1 - y);

        //  12  13  14  15
        //  08  09  10  11
        //  04  05  06  07
        //  00  01  02  03
        case PanelLayout::ColumnMajor270:
            return x + (height - 1 - y) * width;

        // -- ColumnMajorAlternating --------------------------------------
        //  00  07  08  15
        //  01  06  09  14
        //  02  05  10  13
        //  03  04  11  12
        case PanelLayout::ColumnMajorAlternating:
        {
            uint16_t index = x * height;
            return (x & 1) ? index + (height - 1 - y) : index + y;
        }

        //  03  02  01  00
        //  04  05  06  07
        //  11  10  09  08
        //  12  13  14  15
        case PanelLayout::ColumnMajorAlternating90:
        {
            uint16_t index = y * width;
            return (y & 1) ? index + x : index + (width - 1 - x);
        }

        //  12  11  04  03
        //  13  10  05  02
        //  14  09  06  01
        //  15  08  07  00
        case PanelLayout::ColumnMajorAlternating180:
        {
            uint16_t mx = (width - 1) - x;
            uint16_t index = mx * height;
            return (mx & 1) ? index + y : index + (height - 1 - y);
        }

        //  15  14  13  12
        //  08  09  10  11
        //  07  06  05  04
        //  00  01  02  03
        case PanelLayout::ColumnMajorAlternating270:
        {
            uint16_t my = (height - 1) - y;
            uint16_t index = my * width;
            return (my & 1) ? index + (width - 1 - x) : index + x;
        }
        }

        // unreachable ? all 16 cases covered
        return 0;
    }

    // -------------------------------------------------------------------
    // tilePreferredLayout ? mosaic rotation logic
    //
    // Given a base panel layout (any rotation ? the rotation is ignored,
    // only the base group matters) and the tile's row/column parity,
    // returns the rotated layout that minimizes inter-tile wiring.
    //
    // This reproduces the TilePreference typedef tables from the original
    // layout classes.
    //
    //   oddRow=false, oddColumn=false  ?  EvenRowEvenColumn
    //   oddRow=false, oddColumn=true   ?  EvenRowOddColumn
    //   oddRow=true,  oddColumn=false  ?  OddRowEvenColumn
    //   oddRow=true,  oddColumn=true   ?  OddRowOddColumn
    // -------------------------------------------------------------------
    constexpr PanelLayout tilePreferredLayout(PanelLayout baseLayout,
                                              bool oddRow, bool oddColumn)
    {
        // Determine which base group the layout belongs to
        auto group = static_cast<uint8_t>(baseLayout) / 4;

        switch (group)
        {
        case 0: // RowMajor group
            if (!oddRow && !oddColumn)  return PanelLayout::RowMajor;
            if (!oddRow &&  oddColumn)  return PanelLayout::RowMajor270;
            if ( oddRow && !oddColumn)  return PanelLayout::RowMajor90;
            return PanelLayout::RowMajor180;    // oddRow && oddColumn

        case 1: // RowMajorAlternating group
            if (!oddRow)  return PanelLayout::RowMajorAlternating270;
            return PanelLayout::RowMajorAlternating90;

        case 2: // ColumnMajor group
            if (!oddRow && !oddColumn)  return PanelLayout::ColumnMajor;
            if (!oddRow &&  oddColumn)  return PanelLayout::ColumnMajor270;
            if ( oddRow && !oddColumn)  return PanelLayout::ColumnMajor90;
            return PanelLayout::ColumnMajor180;

        case 3: // ColumnMajorAlternating group
            if (!oddRow)  return PanelLayout::ColumnMajorAlternating;
            return PanelLayout::ColumnMajorAlternating180;
        }

        return baseLayout; // unreachable
    }

} // namespace npb

