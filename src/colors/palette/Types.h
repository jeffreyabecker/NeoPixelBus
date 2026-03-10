#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/Compat.h"
#include "colors/Color.h"
#include "colors/palette/ModeEnums.h"
#include "colors/palette/WrapModes.h"

namespace lw::colors::palettes
{
template <typename TColor, typename = std::enable_if_t<ColorType<TColor>>> struct PaletteSampleOptions
{
    typename TColor::ComponentType brightnessScale{std::numeric_limits<typename TColor::ComponentType>::max()};
    TColor outOfRangeColor{};
    WrapMode wrapMode{WrapMode::Clamp};
    BlendMode blendMode{BlendMode::Linear};
    TieBreakPolicy tieBreakPolicy{TieBreakPolicy::Stable};
    uint8_t quantizedLevels{8};
};

template <typename TColor, typename = std::enable_if_t<ColorType<TColor>>> struct PaletteStop
{
    using ColorType = TColor;

    static constexpr PaletteStop fromRgb8(size_t index, uint32_t rgb)
    {
        return fromRgb8(index, static_cast<uint8_t>((rgb >> 16U) & 0xFFU), static_cast<uint8_t>((rgb >> 8U) & 0xFFU),
                        static_cast<uint8_t>(rgb & 0xFFU));
    }

    static constexpr PaletteStop fromRgb8(size_t index, uint8_t r, uint8_t g, uint8_t b)
    {
        return PaletteStop{index, TColor{r, g, b}};
    }

    size_t index{0};
    TColor color{};
};

template <typename TColor, typename = std::enable_if_t<ColorType<TColor>>> class IPalette
{
  public:
    using ColorType = TColor;
    using StopsView = span<const PaletteStop<TColor>>;

    virtual ~IPalette() = default;

    virtual StopsView stops() const = 0;
    virtual void update(uint8_t step = 0) = 0;
};

template <typename TColor, typename = std::enable_if_t<ColorType<TColor>>> class Palette : public IPalette<TColor>
{
  public:
    using StopsView = typename IPalette<TColor>::StopsView;
    using StorageType = std::vector<PaletteStop<TColor>>;

    Palette() = default;

    explicit Palette(StorageType stops) : _stops(std::move(stops)) {}

    explicit Palette(StopsView stops) : _stops(stops.begin(), stops.end()) {}

    template <size_t N>
    explicit Palette(const std::array<PaletteStop<TColor>, N>& stops) : _stops(stops.begin(), stops.end())
    {
    }

    Palette(std::initializer_list<PaletteStop<TColor>> stops) : _stops(stops) {}

    static Palette parse(const char* stops)
    {
        StorageType parsedStops;
        if (!tryParseStops(stops, parsedStops))
        {
            return Palette();
        }

        return Palette(std::move(parsedStops));
    }

    static Palette color1(const TColor& primary)
    {
        const std::array<PaletteStop<TColor>, 2> stops = {
            PaletteStop<TColor>{0, primary},
            PaletteStop<TColor>{255, primary},
        };
        return Palette(stops);
    }

    static Palette colors1And2(const TColor& primary, const TColor& secondary)
    {
        const std::array<PaletteStop<TColor>, 4> stops = {
            PaletteStop<TColor>{0, primary},
            PaletteStop<TColor>{127, primary},
            PaletteStop<TColor>{128, secondary},
            PaletteStop<TColor>{255, secondary},
        };
        return Palette(stops);
    }

    static Palette colorGradient(const TColor& primary, const TColor& secondary, const TColor& tertiary)
    {
        const std::array<PaletteStop<TColor>, 3> stops = {
            PaletteStop<TColor>{0, tertiary},
            PaletteStop<TColor>{127, secondary},
            PaletteStop<TColor>{255, primary},
        };
        return Palette(stops);
    }

    static Palette colorsOnly(const TColor& primary, const TColor& secondary, const TColor& tertiary)
    {
        const std::array<PaletteStop<TColor>, 16> stops = {
            PaletteStop<TColor>{0, primary},     PaletteStop<TColor>{16, primary},
            PaletteStop<TColor>{32, primary},    PaletteStop<TColor>{48, primary},
            PaletteStop<TColor>{64, primary},    PaletteStop<TColor>{80, secondary},
            PaletteStop<TColor>{96, secondary},  PaletteStop<TColor>{112, secondary},
            PaletteStop<TColor>{128, secondary}, PaletteStop<TColor>{144, secondary},
            PaletteStop<TColor>{160, tertiary},  PaletteStop<TColor>{176, tertiary},
            PaletteStop<TColor>{192, tertiary},  PaletteStop<TColor>{208, tertiary},
            PaletteStop<TColor>{224, tertiary},  PaletteStop<TColor>{255, primary},
        };
        return Palette(stops);
    }

    StopsView stops() const override { return StopsView(_stops.data(), _stops.size()); }

    void update(uint8_t = 0) override {}

    StorageType& storage() { return _stops; }

    const StorageType& storage() const { return _stops; }

  private:
    static bool tryParseStops(const char* text, StorageType& parsedStops)
    {
        if (text == nullptr)
        {
            return false;
        }

        const char* cursor = skipWhitespace(text);
        if (*cursor == '\0')
        {
            return false;
        }

        while (*cursor != '\0')
        {
            size_t index = 0;
            TColor color{};
            if (!tryParseStop(cursor, index, color))
            {
                return false;
            }

            parsedStops.push_back(PaletteStop<TColor>{index, color});

            cursor = skipWhitespace(cursor);
            if (*cursor == '\0')
            {
                return true;
            }

            if (*cursor != '|')
            {
                return false;
            }

            cursor = skipWhitespace(cursor + 1);
            if (*cursor == '\0')
            {
                return false;
            }
        }

        return !parsedStops.empty();
    }

    static bool tryParseStop(const char*& cursor, size_t& index, TColor& color)
    {
        if (!tryParseIndex(cursor, index))
        {
            return false;
        }

        cursor = skipWhitespace(cursor);
        if (*cursor != ',')
        {
            return false;
        }

        cursor = skipWhitespace(cursor + 1);
        return tryParseRgb(cursor, color);
    }

    static bool tryParseIndex(const char*& cursor, size_t& index)
    {
        cursor = skipWhitespace(cursor);
        if (!std::isdigit(static_cast<unsigned char>(*cursor)))
        {
            return false;
        }

        size_t parsed = 0;
        while (std::isdigit(static_cast<unsigned char>(*cursor)))
        {
            parsed = (parsed * 10u) + static_cast<size_t>(*cursor - '0');
            ++cursor;
        }

        index = parsed;
        return true;
    }

    static bool tryParseRgb(const char*& cursor, TColor& color)
    {
        uint8_t components[3] = {};
        for (size_t componentIndex = 0; componentIndex < 3; ++componentIndex)
        {
            int highNibble = hexNibble(*cursor);
            if (highNibble < 0)
            {
                return false;
            }
            ++cursor;

            int lowNibble = hexNibble(*cursor);
            if (lowNibble < 0)
            {
                return false;
            }
            ++cursor;

            components[componentIndex] =
                static_cast<uint8_t>((static_cast<uint8_t>(highNibble) << 4) | static_cast<uint8_t>(lowNibble));
        }

        color = TColor{};
        color['R'] = static_cast<typename TColor::ComponentType>(components[0]);
        color['G'] = static_cast<typename TColor::ComponentType>(components[1]);
        color['B'] = static_cast<typename TColor::ComponentType>(components[2]);
        return true;
    }

    static const char* skipWhitespace(const char* cursor)
    {
        while (*cursor != '\0' && std::isspace(static_cast<unsigned char>(*cursor)))
        {
            ++cursor;
        }

        return cursor;
    }

    static int hexNibble(char value)
    {
        if (value >= '0' && value <= '9')
        {
            return value - '0';
        }

        if (value >= 'a' && value <= 'f')
        {
            return 10 + (value - 'a');
        }

        if (value >= 'A' && value <= 'F')
        {
            return 10 + (value - 'A');
        }

        return -1;
    }

    StorageType _stops{};
};

} // namespace lw::colors::palettes
