template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Party()
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

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Cloud()
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

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Lava()
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

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Ocean()
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

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Forest()
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

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Rainbow()
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

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> RainbowBands()
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
