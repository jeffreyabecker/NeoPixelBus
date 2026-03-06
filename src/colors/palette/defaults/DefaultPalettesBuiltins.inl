namespace detail
{

template <typename TColor>
constexpr colors::palettes::PaletteStop<TColor> stop(size_t index, uint8_t r, uint8_t g, uint8_t b)
{
    return colors::palettes::PaletteStop<TColor>{index, TColor{r, g, b}};
}

template <typename TColor, size_t N>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor>
makeGenerator(const std::array<colors::palettes::PaletteStop<TColor>, N>& stops)
{
    return colors::palettes::StaticStopsPaletteGenerator<TColor>(
        span<const colors::palettes::PaletteStop<TColor>>(stops.data(), stops.size()));
}
} // namespace detail

template <typename TColor = colors::DefaultColorType> struct NamedPalette
{
    const char* name;
    colors::palettes::StaticStopsPaletteGenerator<TColor> (*create)();
};

template <typename TColor = colors::DefaultColorType> constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Default()
{
    return colors::palettes::StaticStopsPaletteGenerator<TColor>(span<const colors::palettes::PaletteStop<TColor>>());
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Color1(const TColor& primary)
{
    const std::array<colors::palettes::PaletteStop<TColor>, 2> stops = {
        colors::palettes::PaletteStop<TColor>{0, primary},
        colors::palettes::PaletteStop<TColor>{255, primary},
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Colors1And2(const TColor& primary, const TColor& secondary)
{
    const std::array<colors::palettes::PaletteStop<TColor>, 4> stops = {
        colors::palettes::PaletteStop<TColor>{0, primary},
        colors::palettes::PaletteStop<TColor>{127, primary},
        colors::palettes::PaletteStop<TColor>{128, secondary},
        colors::palettes::PaletteStop<TColor>{255, secondary},
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> ColorGradient(const TColor& primary, const TColor& secondary,
                                                          const TColor& tertiary)
{
    const std::array<colors::palettes::PaletteStop<TColor>, 3> stops = {
        colors::palettes::PaletteStop<TColor>{0, tertiary},
        colors::palettes::PaletteStop<TColor>{127, secondary},
        colors::palettes::PaletteStop<TColor>{255, primary},
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> ColorsOnly(const TColor& primary, const TColor& secondary,
                                                       const TColor& tertiary)
{
    const std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        colors::palettes::PaletteStop<TColor>{0, primary},     colors::palettes::PaletteStop<TColor>{16, primary},
        colors::palettes::PaletteStop<TColor>{32, primary},    colors::palettes::PaletteStop<TColor>{48, primary},
        colors::palettes::PaletteStop<TColor>{64, primary},    colors::palettes::PaletteStop<TColor>{80, secondary},
        colors::palettes::PaletteStop<TColor>{96, secondary},  colors::palettes::PaletteStop<TColor>{112, secondary},
        colors::palettes::PaletteStop<TColor>{128, secondary}, colors::palettes::PaletteStop<TColor>{144, secondary},
        colors::palettes::PaletteStop<TColor>{160, tertiary},  colors::palettes::PaletteStop<TColor>{176, tertiary},
        colors::palettes::PaletteStop<TColor>{192, tertiary},  colors::palettes::PaletteStop<TColor>{208, tertiary},
        colors::palettes::PaletteStop<TColor>{224, tertiary},  colors::palettes::PaletteStop<TColor>{255, primary},
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType, size_t TStopCount = 8>
using RandomSmoothGenerator = colors::palettes::RandomSmoothPaletteGenerator<TColor, TStopCount>;

template <typename TColor = colors::DefaultColorType, size_t TStopCount = 8>
using RandomCycleGenerator = colors::palettes::RandomCyclePaletteGenerator<TColor, TStopCount>;

template <typename TColor = colors::DefaultColorType> constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Party()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        detail::stop<TColor>(0, 0x55, 0x00, 0xAB),   detail::stop<TColor>(17, 0x84, 0x00, 0x7C),
        detail::stop<TColor>(34, 0xB5, 0x00, 0x4B),  detail::stop<TColor>(51, 0xE5, 0x00, 0x1B),
        detail::stop<TColor>(68, 0xE8, 0x17, 0x00),  detail::stop<TColor>(85, 0xB8, 0x47, 0x00),
        detail::stop<TColor>(102, 0xAB, 0x77, 0x00), detail::stop<TColor>(119, 0xAB, 0xAB, 0x00),
        detail::stop<TColor>(136, 0xAB, 0x55, 0x00), detail::stop<TColor>(153, 0xDD, 0x22, 0x00),
        detail::stop<TColor>(170, 0xF2, 0x00, 0x0E), detail::stop<TColor>(187, 0xC2, 0x00, 0x3E),
        detail::stop<TColor>(204, 0x8F, 0x00, 0x71), detail::stop<TColor>(221, 0x5F, 0x00, 0xA1),
        detail::stop<TColor>(238, 0x2F, 0x00, 0xD0), detail::stop<TColor>(255, 0x00, 0x07, 0xF9),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType> constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Cloud()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        detail::stop<TColor>(0, 0x00, 0x00, 0xFF),   detail::stop<TColor>(17, 0x00, 0x00, 0x8B),
        detail::stop<TColor>(34, 0x00, 0x00, 0x8B),  detail::stop<TColor>(51, 0x00, 0x00, 0x8B),
        detail::stop<TColor>(68, 0x00, 0x00, 0x8B),  detail::stop<TColor>(85, 0x00, 0x00, 0x8B),
        detail::stop<TColor>(102, 0x00, 0x00, 0x8B), detail::stop<TColor>(119, 0x00, 0x00, 0x8B),
        detail::stop<TColor>(136, 0x00, 0x00, 0xFF), detail::stop<TColor>(153, 0x00, 0x00, 0x8B),
        detail::stop<TColor>(170, 0x87, 0xCE, 0xEB), detail::stop<TColor>(187, 0x87, 0xCE, 0xEB),
        detail::stop<TColor>(204, 0xAD, 0xD8, 0xE6), detail::stop<TColor>(221, 0xFF, 0xFF, 0xFF),
        detail::stop<TColor>(238, 0xAD, 0xD8, 0xE6), detail::stop<TColor>(255, 0x87, 0xCE, 0xEB),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType> constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Lava()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        detail::stop<TColor>(0, 0x00, 0x00, 0x00),   detail::stop<TColor>(17, 0x80, 0x00, 0x00),
        detail::stop<TColor>(34, 0x00, 0x00, 0x00),  detail::stop<TColor>(51, 0x80, 0x00, 0x00),
        detail::stop<TColor>(68, 0x8B, 0x00, 0x00),  detail::stop<TColor>(85, 0x8B, 0x00, 0x00),
        detail::stop<TColor>(102, 0x80, 0x00, 0x00), detail::stop<TColor>(119, 0x8B, 0x00, 0x00),
        detail::stop<TColor>(136, 0x8B, 0x00, 0x00), detail::stop<TColor>(153, 0x8B, 0x00, 0x00),
        detail::stop<TColor>(170, 0xFF, 0x00, 0x00), detail::stop<TColor>(187, 0xFF, 0xA5, 0x00),
        detail::stop<TColor>(204, 0xFF, 0xFF, 0xFF), detail::stop<TColor>(221, 0xFF, 0xA5, 0x00),
        detail::stop<TColor>(238, 0xFF, 0x00, 0x00), detail::stop<TColor>(255, 0x8B, 0x00, 0x00),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType> constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Ocean()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        detail::stop<TColor>(0, 0x19, 0x19, 0x70),   detail::stop<TColor>(17, 0x00, 0x00, 0x8B),
        detail::stop<TColor>(34, 0x19, 0x19, 0x70),  detail::stop<TColor>(51, 0x00, 0x00, 0x80),
        detail::stop<TColor>(68, 0x00, 0x00, 0x8B),  detail::stop<TColor>(85, 0x00, 0x00, 0xCD),
        detail::stop<TColor>(102, 0x2E, 0x8B, 0x57), detail::stop<TColor>(119, 0x00, 0x80, 0x80),
        detail::stop<TColor>(136, 0x5F, 0x9E, 0xA0), detail::stop<TColor>(153, 0x00, 0x00, 0xFF),
        detail::stop<TColor>(170, 0x00, 0x8B, 0x8B), detail::stop<TColor>(187, 0x64, 0x95, 0xED),
        detail::stop<TColor>(204, 0x7F, 0xFF, 0xD4), detail::stop<TColor>(221, 0x2E, 0x8B, 0x57),
        detail::stop<TColor>(238, 0x00, 0xFF, 0xFF), detail::stop<TColor>(255, 0x87, 0xCE, 0xFA),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType> constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Forest()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        detail::stop<TColor>(0, 0x00, 0x64, 0x00),   detail::stop<TColor>(17, 0x00, 0x64, 0x00),
        detail::stop<TColor>(34, 0x55, 0x6B, 0x2F),  detail::stop<TColor>(51, 0x00, 0x64, 0x00),
        detail::stop<TColor>(68, 0x00, 0x80, 0x00),  detail::stop<TColor>(85, 0x22, 0x8B, 0x22),
        detail::stop<TColor>(102, 0x6B, 0x8E, 0x23), detail::stop<TColor>(119, 0x00, 0x80, 0x00),
        detail::stop<TColor>(136, 0x2E, 0x8B, 0x57), detail::stop<TColor>(153, 0x66, 0xCD, 0xAA),
        detail::stop<TColor>(170, 0x32, 0xCD, 0x32), detail::stop<TColor>(187, 0x9A, 0xCD, 0x32),
        detail::stop<TColor>(204, 0x90, 0xEE, 0x90), detail::stop<TColor>(221, 0x7C, 0xFC, 0x00),
        detail::stop<TColor>(238, 0x66, 0xCD, 0xAA), detail::stop<TColor>(255, 0x22, 0x8B, 0x22),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType> constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Rainbow()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        detail::stop<TColor>(0, 0xFF, 0x00, 0x00),   detail::stop<TColor>(17, 0xD5, 0x2A, 0x00),
        detail::stop<TColor>(34, 0xAB, 0x55, 0x00),  detail::stop<TColor>(51, 0xAB, 0x7F, 0x00),
        detail::stop<TColor>(68, 0xAB, 0xAB, 0x00),  detail::stop<TColor>(85, 0x56, 0xD5, 0x00),
        detail::stop<TColor>(102, 0x00, 0xFF, 0x00), detail::stop<TColor>(119, 0x00, 0xD5, 0x2A),
        detail::stop<TColor>(136, 0x00, 0xAB, 0x55), detail::stop<TColor>(153, 0x00, 0x56, 0xAA),
        detail::stop<TColor>(170, 0x00, 0x00, 0xFF), detail::stop<TColor>(187, 0x2A, 0x00, 0xD5),
        detail::stop<TColor>(204, 0x55, 0x00, 0xAB), detail::stop<TColor>(221, 0x7F, 0x00, 0x81),
        detail::stop<TColor>(238, 0xAB, 0x00, 0x55), detail::stop<TColor>(255, 0xD5, 0x00, 0x2B),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType> constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> RainbowBands()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        detail::stop<TColor>(0, 0xFF, 0x00, 0x00),   detail::stop<TColor>(17, 0x00, 0x00, 0x00),
        detail::stop<TColor>(34, 0xAB, 0x55, 0x00),  detail::stop<TColor>(51, 0x00, 0x00, 0x00),
        detail::stop<TColor>(68, 0xAB, 0xAB, 0x00),  detail::stop<TColor>(85, 0x00, 0x00, 0x00),
        detail::stop<TColor>(102, 0x00, 0xFF, 0x00), detail::stop<TColor>(119, 0x00, 0x00, 0x00),
        detail::stop<TColor>(136, 0x00, 0xAB, 0x55), detail::stop<TColor>(153, 0x00, 0x00, 0x00),
        detail::stop<TColor>(170, 0x00, 0x00, 0xFF), detail::stop<TColor>(187, 0x00, 0x00, 0x00),
        detail::stop<TColor>(204, 0x55, 0x00, 0xAB), detail::stop<TColor>(221, 0x00, 0x00, 0x00),
        detail::stop<TColor>(238, 0xAB, 0x00, 0x55), detail::stop<TColor>(255, 0x00, 0x00, 0x00),
    };
    return detail::makeGenerator(stops);
}
