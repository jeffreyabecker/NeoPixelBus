template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Sunset()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        detail::stop<TColor>(0, 0x78, 0x00, 0x00),   detail::stop<TColor>(22, 0xB3, 0x16, 0x00),
        detail::stop<TColor>(51, 0xFF, 0x68, 0x00),  detail::stop<TColor>(85, 0xA7, 0x16, 0x12),
        detail::stop<TColor>(135, 0x64, 0x00, 0x67), detail::stop<TColor>(198, 0x10, 0x00, 0x82),
        detail::stop<TColor>(255, 0x00, 0x00, 0xA0),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Rivendell()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        detail::stop<TColor>(0, 0x01, 0x0E, 0x05),   detail::stop<TColor>(101, 0x10, 0x24, 0x0E),
        detail::stop<TColor>(165, 0x38, 0x44, 0x1E), detail::stop<TColor>(242, 0x96, 0x9C, 0x63),
        detail::stop<TColor>(255, 0x96, 0x9C, 0x63),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Breeze()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 4> stops = {
        detail::stop<TColor>(0, 0x01, 0x06, 0x07),
        detail::stop<TColor>(89, 0x01, 0x63, 0x6F),
        detail::stop<TColor>(153, 0x90, 0xD1, 0xFF),
        detail::stop<TColor>(255, 0x00, 0x49, 0x52),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> RedBlue()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 9> stops = {
        detail::stop<TColor>(0, 0x04, 0x01, 0x46),   detail::stop<TColor>(31, 0x37, 0x01, 0x1E),
        detail::stop<TColor>(63, 0xFF, 0x04, 0x07),  detail::stop<TColor>(95, 0x3B, 0x02, 0x1D),
        detail::stop<TColor>(127, 0x0B, 0x03, 0x32), detail::stop<TColor>(159, 0x27, 0x08, 0x3C),
        detail::stop<TColor>(191, 0x70, 0x13, 0x28), detail::stop<TColor>(223, 0x4E, 0x0B, 0x27),
        detail::stop<TColor>(255, 0x1D, 0x08, 0x3B),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Yellowout()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 2> stops = {
        detail::stop<TColor>(0, 0xBC, 0x87, 0x01),
        detail::stop<TColor>(255, 0x2E, 0x07, 0x01),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Analogous()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        detail::stop<TColor>(0, 0x03, 0x00, 0xFF),   detail::stop<TColor>(63, 0x17, 0x00, 0xFF),
        detail::stop<TColor>(127, 0x43, 0x00, 0xFF), detail::stop<TColor>(191, 0x8E, 0x00, 0x2D),
        detail::stop<TColor>(255, 0xFF, 0x00, 0x00),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Splash()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        detail::stop<TColor>(0, 0x7E, 0x0B, 0xFF),   detail::stop<TColor>(127, 0xC5, 0x01, 0x16),
        detail::stop<TColor>(175, 0xD2, 0x9D, 0xAC), detail::stop<TColor>(221, 0x9D, 0x03, 0x70),
        detail::stop<TColor>(255, 0x9D, 0x03, 0x70),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Pastel()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 11> stops = {
        detail::stop<TColor>(0, 0x0A, 0x3E, 0x7B),   detail::stop<TColor>(36, 0x38, 0x82, 0x67),
        detail::stop<TColor>(87, 0x99, 0xE1, 0x55),  detail::stop<TColor>(100, 0xC7, 0xD9, 0x44),
        detail::stop<TColor>(107, 0xFF, 0xCF, 0x36), detail::stop<TColor>(115, 0xF7, 0x98, 0x39),
        detail::stop<TColor>(120, 0xEF, 0x6B, 0x3D), detail::stop<TColor>(128, 0xF7, 0x98, 0x39),
        detail::stop<TColor>(180, 0xFF, 0xCF, 0x36), detail::stop<TColor>(223, 0xFF, 0xE3, 0x30),
        detail::stop<TColor>(255, 0xFF, 0xF8, 0x2A),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Sunset2()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        detail::stop<TColor>(0, 0x6E, 0x31, 0x0B),   detail::stop<TColor>(29, 0x37, 0x22, 0x0A),
        detail::stop<TColor>(68, 0x16, 0x16, 0x09),  detail::stop<TColor>(68, 0xEF, 0x7C, 0x08),
        detail::stop<TColor>(97, 0xDC, 0x9C, 0x1B),  detail::stop<TColor>(124, 0xCB, 0xC1, 0x3D),
        detail::stop<TColor>(178, 0x21, 0x35, 0x38), detail::stop<TColor>(255, 0x00, 0x01, 0x34),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Beach()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 15> stops = {
        detail::stop<TColor>(0, 0xFF, 0xFC, 0xD6),   detail::stop<TColor>(12, 0xFF, 0xFC, 0xD6),
        detail::stop<TColor>(22, 0xFF, 0xFC, 0xD6),  detail::stop<TColor>(26, 0xBE, 0xBF, 0x73),
        detail::stop<TColor>(28, 0x89, 0x8D, 0x34),  detail::stop<TColor>(28, 0x70, 0xFF, 0xCD),
        detail::stop<TColor>(50, 0x33, 0xF6, 0xD6),  detail::stop<TColor>(71, 0x11, 0xEB, 0xE2),
        detail::stop<TColor>(93, 0x02, 0xC1, 0xC7),  detail::stop<TColor>(120, 0x00, 0x9C, 0xAE),
        detail::stop<TColor>(133, 0x01, 0x65, 0x73), detail::stop<TColor>(136, 0x01, 0x3B, 0x47),
        detail::stop<TColor>(136, 0x07, 0x83, 0xAA), detail::stop<TColor>(208, 0x01, 0x5A, 0x97),
        detail::stop<TColor>(255, 0x00, 0x38, 0x85),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Vintage()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        detail::stop<TColor>(0, 0x04, 0x01, 0x01),   detail::stop<TColor>(51, 0x10, 0x00, 0x01),
        detail::stop<TColor>(76, 0x61, 0x68, 0x03),  detail::stop<TColor>(101, 0xFF, 0x83, 0x13),
        detail::stop<TColor>(127, 0x43, 0x09, 0x04), detail::stop<TColor>(153, 0x10, 0x00, 0x01),
        detail::stop<TColor>(229, 0x04, 0x01, 0x01), detail::stop<TColor>(255, 0x04, 0x01, 0x01),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Departure()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 12> stops = {
        detail::stop<TColor>(0, 0x08, 0x03, 0x00),   detail::stop<TColor>(42, 0x17, 0x07, 0x00),
        detail::stop<TColor>(63, 0x4B, 0x26, 0x06),  detail::stop<TColor>(84, 0xA9, 0x63, 0x26),
        detail::stop<TColor>(106, 0xD5, 0xA9, 0x77), detail::stop<TColor>(116, 0xFF, 0xFF, 0xFF),
        detail::stop<TColor>(138, 0x87, 0xFF, 0x8A), detail::stop<TColor>(148, 0x16, 0xFF, 0x18),
        detail::stop<TColor>(170, 0x00, 0xFF, 0x00), detail::stop<TColor>(191, 0x00, 0x88, 0x00),
        detail::stop<TColor>(212, 0x00, 0x37, 0x00), detail::stop<TColor>(255, 0x00, 0x37, 0x00),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Landscape()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 9> stops = {
        detail::stop<TColor>(0, 0x00, 0x00, 0x00),   detail::stop<TColor>(37, 0x02, 0x19, 0x01),
        detail::stop<TColor>(76, 0x0F, 0x73, 0x05),  detail::stop<TColor>(127, 0x4F, 0xD5, 0x01),
        detail::stop<TColor>(128, 0x7E, 0xD3, 0x2F), detail::stop<TColor>(130, 0xBC, 0xD1, 0xF7),
        detail::stop<TColor>(153, 0x90, 0xB6, 0xCD), detail::stop<TColor>(204, 0x3B, 0x75, 0xFA),
        detail::stop<TColor>(255, 0x01, 0x25, 0xC0),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Beech()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 6> stops = {
        detail::stop<TColor>(0, 0x01, 0x05, 0x00),  detail::stop<TColor>(19, 0x20, 0x17, 0x01),
        detail::stop<TColor>(38, 0xA1, 0x37, 0x01), detail::stop<TColor>(63, 0xE5, 0x90, 0x01),
        detail::stop<TColor>(66, 0x27, 0x8E, 0x4A), detail::stop<TColor>(255, 0x01, 0x04, 0x01),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Sherbet()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        detail::stop<TColor>(0, 0xFF, 0x21, 0x04),   detail::stop<TColor>(43, 0xFF, 0x44, 0x19),
        detail::stop<TColor>(86, 0xFF, 0x07, 0x19),  detail::stop<TColor>(127, 0xFF, 0x52, 0x67),
        detail::stop<TColor>(170, 0xFF, 0xFF, 0xF2), detail::stop<TColor>(209, 0x2A, 0xFF, 0x16),
        detail::stop<TColor>(255, 0x57, 0xFF, 0x41),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Hult()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 6> stops = {
        detail::stop<TColor>(0, 0xF7, 0xB0, 0xF7),   detail::stop<TColor>(48, 0xFF, 0x88, 0xFF),
        detail::stop<TColor>(89, 0xDC, 0x1D, 0xE2),  detail::stop<TColor>(160, 0x07, 0x52, 0xB2),
        detail::stop<TColor>(216, 0x01, 0x7C, 0x6D), detail::stop<TColor>(255, 0x01, 0x7C, 0x6D),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Hult64()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        detail::stop<TColor>(0, 0x01, 0x7C, 0x6D),   detail::stop<TColor>(66, 0x01, 0x5D, 0x4F),
        detail::stop<TColor>(104, 0x34, 0x41, 0x01), detail::stop<TColor>(130, 0x73, 0x7F, 0x01),
        detail::stop<TColor>(150, 0x34, 0x41, 0x01), detail::stop<TColor>(201, 0x01, 0x56, 0x48),
        detail::stop<TColor>(239, 0x00, 0x37, 0x2D), detail::stop<TColor>(255, 0x00, 0x37, 0x2D),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Drywet()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        detail::stop<TColor>(0, 0x2F, 0x1E, 0x02),   detail::stop<TColor>(42, 0xD5, 0x93, 0x18),
        detail::stop<TColor>(84, 0x67, 0xDB, 0x34),  detail::stop<TColor>(127, 0x03, 0xDB, 0xCF),
        detail::stop<TColor>(170, 0x01, 0x30, 0xD6), detail::stop<TColor>(212, 0x01, 0x01, 0x6F),
        detail::stop<TColor>(255, 0x01, 0x07, 0x21),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Jul()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 4> stops = {
        detail::stop<TColor>(0, 0xC2, 0x01, 0x01),
        detail::stop<TColor>(94, 0x01, 0x1D, 0x12),
        detail::stop<TColor>(132, 0x39, 0x83, 0x1C),
        detail::stop<TColor>(255, 0x71, 0x01, 0x01),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Grintage()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        detail::stop<TColor>(0, 0x02, 0x01, 0x01),   detail::stop<TColor>(53, 0x12, 0x01, 0x00),
        detail::stop<TColor>(104, 0x45, 0x1D, 0x01), detail::stop<TColor>(153, 0xA7, 0x87, 0x0A),
        detail::stop<TColor>(255, 0x2E, 0x38, 0x04),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Rewhi()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 6> stops = {
        detail::stop<TColor>(0, 0x71, 0x5B, 0x93),   detail::stop<TColor>(72, 0x9D, 0x58, 0x4E),
        detail::stop<TColor>(89, 0xD0, 0x55, 0x21),  detail::stop<TColor>(107, 0xFF, 0x1D, 0x0B),
        detail::stop<TColor>(141, 0x89, 0x1F, 0x27), detail::stop<TColor>(255, 0x3B, 0x21, 0x59),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Tertiary()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        detail::stop<TColor>(0, 0x00, 0x01, 0xFF),   detail::stop<TColor>(63, 0x03, 0x44, 0x2D),
        detail::stop<TColor>(127, 0x17, 0xFF, 0x00), detail::stop<TColor>(191, 0x64, 0x44, 0x01),
        detail::stop<TColor>(255, 0xFF, 0x01, 0x04),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Fire()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 13> stops = {
        detail::stop<TColor>(0, 0x00, 0x00, 0x00),   detail::stop<TColor>(46, 0x12, 0x00, 0x00),
        detail::stop<TColor>(96, 0x71, 0x00, 0x00),  detail::stop<TColor>(108, 0x8E, 0x03, 0x01),
        detail::stop<TColor>(119, 0xAF, 0x11, 0x01), detail::stop<TColor>(146, 0xD5, 0x2C, 0x02),
        detail::stop<TColor>(174, 0xFF, 0x52, 0x04), detail::stop<TColor>(188, 0xFF, 0x73, 0x04),
        detail::stop<TColor>(202, 0xFF, 0x9C, 0x04), detail::stop<TColor>(218, 0xFF, 0xCB, 0x04),
        detail::stop<TColor>(234, 0xFF, 0xFF, 0x04), detail::stop<TColor>(244, 0xFF, 0xFF, 0x47),
        detail::stop<TColor>(255, 0xFF, 0xFF, 0xFF),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Icefire()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        detail::stop<TColor>(0, 0x00, 0x00, 0x00),   detail::stop<TColor>(59, 0x00, 0x09, 0x2D),
        detail::stop<TColor>(119, 0x00, 0x26, 0xFF), detail::stop<TColor>(149, 0x03, 0x64, 0xFF),
        detail::stop<TColor>(180, 0x17, 0xC7, 0xFF), detail::stop<TColor>(217, 0x64, 0xEB, 0xFF),
        detail::stop<TColor>(255, 0xFF, 0xFF, 0xFF),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Cyane()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 11> stops = {
        detail::stop<TColor>(0, 0x0A, 0x55, 0x05),   detail::stop<TColor>(25, 0x1D, 0x6D, 0x12),
        detail::stop<TColor>(60, 0x3B, 0x8A, 0x2A),  detail::stop<TColor>(93, 0x53, 0x63, 0x34),
        detail::stop<TColor>(106, 0x6E, 0x42, 0x40), detail::stop<TColor>(109, 0x7B, 0x31, 0x41),
        detail::stop<TColor>(113, 0x8B, 0x23, 0x42), detail::stop<TColor>(116, 0xC0, 0x75, 0x62),
        detail::stop<TColor>(124, 0xFF, 0xFF, 0x89), detail::stop<TColor>(168, 0x64, 0xB4, 0x9B),
        detail::stop<TColor>(255, 0x16, 0x79, 0xAE),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> LightPink()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 11> stops = {
        detail::stop<TColor>(0, 0x13, 0x02, 0x27),   detail::stop<TColor>(25, 0x1A, 0x04, 0x2D),
        detail::stop<TColor>(51, 0x21, 0x06, 0x34),  detail::stop<TColor>(76, 0x44, 0x3E, 0x7D),
        detail::stop<TColor>(102, 0x76, 0xBB, 0xF0), detail::stop<TColor>(109, 0xA3, 0xD7, 0xF7),
        detail::stop<TColor>(114, 0xD9, 0xF4, 0xFF), detail::stop<TColor>(122, 0x9F, 0x95, 0xDD),
        detail::stop<TColor>(149, 0x71, 0x4E, 0xBC), detail::stop<TColor>(183, 0x80, 0x39, 0x9B),
        detail::stop<TColor>(255, 0x92, 0x28, 0x7B),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Autumn()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 13> stops = {
        detail::stop<TColor>(0, 0x1A, 0x01, 0x01),   detail::stop<TColor>(51, 0x43, 0x04, 0x01),
        detail::stop<TColor>(84, 0x76, 0x0E, 0x01),  detail::stop<TColor>(104, 0x89, 0x98, 0x34),
        detail::stop<TColor>(112, 0x71, 0x41, 0x01), detail::stop<TColor>(122, 0x85, 0x95, 0x3B),
        detail::stop<TColor>(124, 0x89, 0x98, 0x34), detail::stop<TColor>(135, 0x71, 0x41, 0x01),
        detail::stop<TColor>(142, 0x8B, 0x9A, 0x2E), detail::stop<TColor>(163, 0x71, 0x0D, 0x01),
        detail::stop<TColor>(204, 0x37, 0x03, 0x01), detail::stop<TColor>(249, 0x11, 0x01, 0x01),
        detail::stop<TColor>(255, 0x11, 0x01, 0x01),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Magenta()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        detail::stop<TColor>(0, 0x00, 0x00, 0x00),   detail::stop<TColor>(42, 0x00, 0x00, 0x2D),
        detail::stop<TColor>(84, 0x00, 0x00, 0xFF),  detail::stop<TColor>(127, 0x2A, 0x00, 0xFF),
        detail::stop<TColor>(170, 0xFF, 0x00, 0xFF), detail::stop<TColor>(212, 0xFF, 0x37, 0xFF),
        detail::stop<TColor>(255, 0xFF, 0xFF, 0xFF),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Magred()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        detail::stop<TColor>(0, 0x00, 0x00, 0x00),   detail::stop<TColor>(63, 0x2A, 0x00, 0x2D),
        detail::stop<TColor>(127, 0xFF, 0x00, 0xFF), detail::stop<TColor>(191, 0xFF, 0x00, 0x2D),
        detail::stop<TColor>(255, 0xFF, 0x00, 0x00),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Yelmag()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        detail::stop<TColor>(0, 0x00, 0x00, 0x00),   detail::stop<TColor>(42, 0x2A, 0x00, 0x00),
        detail::stop<TColor>(84, 0xFF, 0x00, 0x00),  detail::stop<TColor>(127, 0xFF, 0x00, 0x2D),
        detail::stop<TColor>(170, 0xFF, 0x00, 0xFF), detail::stop<TColor>(212, 0xFF, 0x37, 0x2D),
        detail::stop<TColor>(255, 0xFF, 0xFF, 0x00),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Yelblu()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        detail::stop<TColor>(0, 0x00, 0x00, 0xFF),   detail::stop<TColor>(63, 0x00, 0x37, 0xFF),
        detail::stop<TColor>(127, 0x00, 0xFF, 0xFF), detail::stop<TColor>(191, 0x2A, 0xFF, 0x2D),
        detail::stop<TColor>(255, 0xFF, 0xFF, 0x00),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> OrangeTeal()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 4> stops = {
        detail::stop<TColor>(0, 0x00, 0x96, 0x5C),
        detail::stop<TColor>(55, 0x00, 0x96, 0x5C),
        detail::stop<TColor>(200, 0xFF, 0x48, 0x00),
        detail::stop<TColor>(255, 0xFF, 0x48, 0x00),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Tiamat()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 17> stops = {
        detail::stop<TColor>(0, 0x01, 0x02, 0x0E),   detail::stop<TColor>(33, 0x02, 0x05, 0x23),
        detail::stop<TColor>(47, 0x3D, 0x7E, 0x64),  detail::stop<TColor>(13, 0x87, 0x5C, 0x58),
        detail::stop<TColor>(242, 0xF7, 0x78, 0x2B), detail::stop<TColor>(255, 0xC1, 0x87, 0xFF),
        detail::stop<TColor>(253, 0x8C, 0xF7, 0x07), detail::stop<TColor>(249, 0xFC, 0x45, 0xFD),
        detail::stop<TColor>(160, 0xC1, 0x11, 0xD0), detail::stop<TColor>(231, 0x60, 0xED, 0xB4),
        detail::stop<TColor>(39, 0xFF, 0x9A, 0x82),  detail::stop<TColor>(77, 0xD5, 0xC8, 0x04),
        detail::stop<TColor>(213, 0xEC, 0x39, 0x7A), detail::stop<TColor>(248, 0xDC, 0x27, 0xFC),
        detail::stop<TColor>(135, 0xB1, 0xFE, 0xFF), detail::stop<TColor>(240, 0xC1, 0xD5, 0xFD),
        detail::stop<TColor>(203, 0xEF, 0xFD, 0xFF),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> AprilNight()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 17> stops = {
        detail::stop<TColor>(0, 0x01, 0x05, 0x2D),   detail::stop<TColor>(10, 0x01, 0x05, 0x2D),
        detail::stop<TColor>(25, 0x05, 0xA9, 0xAF),  detail::stop<TColor>(40, 0x01, 0x05, 0x2D),
        detail::stop<TColor>(61, 0x01, 0x05, 0x2D),  detail::stop<TColor>(76, 0x2D, 0xAF, 0x1F),
        detail::stop<TColor>(91, 0x01, 0x05, 0x2D),  detail::stop<TColor>(112, 0x01, 0x05, 0x2D),
        detail::stop<TColor>(127, 0xF9, 0x96, 0x05), detail::stop<TColor>(143, 0x01, 0x05, 0x2D),
        detail::stop<TColor>(162, 0x01, 0x05, 0x2D), detail::stop<TColor>(178, 0xFF, 0x5C, 0x00),
        detail::stop<TColor>(193, 0x01, 0x05, 0x2D), detail::stop<TColor>(214, 0x01, 0x05, 0x2D),
        detail::stop<TColor>(229, 0xDF, 0x2D, 0x48), detail::stop<TColor>(244, 0x01, 0x05, 0x2D),
        detail::stop<TColor>(255, 0x01, 0x05, 0x2D),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Orangery()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 9> stops = {
        detail::stop<TColor>(0, 0xFF, 0x5F, 0x17),   detail::stop<TColor>(30, 0xFF, 0x52, 0x00),
        detail::stop<TColor>(60, 0xDF, 0x0D, 0x08),  detail::stop<TColor>(90, 0x90, 0x2C, 0x02),
        detail::stop<TColor>(120, 0xFF, 0x6E, 0x11), detail::stop<TColor>(150, 0xFF, 0x45, 0x00),
        detail::stop<TColor>(180, 0x9E, 0x0D, 0x0B), detail::stop<TColor>(210, 0xF1, 0x52, 0x11),
        detail::stop<TColor>(255, 0xD5, 0x25, 0x04),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> C9()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        detail::stop<TColor>(0, 0xB8, 0x04, 0x00),   detail::stop<TColor>(60, 0xB8, 0x04, 0x00),
        detail::stop<TColor>(65, 0x90, 0x2C, 0x02),  detail::stop<TColor>(125, 0x90, 0x2C, 0x02),
        detail::stop<TColor>(130, 0x04, 0x60, 0x02), detail::stop<TColor>(190, 0x04, 0x60, 0x02),
        detail::stop<TColor>(195, 0x07, 0x07, 0x58), detail::stop<TColor>(255, 0x07, 0x07, 0x58),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Sakura()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        detail::stop<TColor>(0, 0xC4, 0x13, 0x0A),   detail::stop<TColor>(65, 0xFF, 0x45, 0x2D),
        detail::stop<TColor>(130, 0xDF, 0x2D, 0x48), detail::stop<TColor>(195, 0xFF, 0x52, 0x67),
        detail::stop<TColor>(255, 0xDF, 0x0D, 0x11),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Aurora()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 6> stops = {
        detail::stop<TColor>(0, 0x01, 0x05, 0x2D),   detail::stop<TColor>(64, 0x00, 0xC8, 0x17),
        detail::stop<TColor>(128, 0x00, 0xFF, 0x00), detail::stop<TColor>(170, 0x00, 0xF3, 0x2D),
        detail::stop<TColor>(200, 0x00, 0x87, 0x07), detail::stop<TColor>(255, 0x01, 0x05, 0x2D),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Atlantica()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        detail::stop<TColor>(0, 0x00, 0x1C, 0x70),   detail::stop<TColor>(1, 0x46, 0x32, 0x20),
        detail::stop<TColor>(96, 0xFF, 0x80, 0xC6),  detail::stop<TColor>(0, 0xF3, 0x2D, 0x96),
        detail::stop<TColor>(12, 0x5F, 0x52, 0x00),  detail::stop<TColor>(5, 0x34, 0xC8, 0x19),
        detail::stop<TColor>(190, 0x5F, 0x13, 0x05), detail::stop<TColor>(255, 0x28, 0xAA, 0x50),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> C92()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 10> stops = {
        detail::stop<TColor>(0, 0x06, 0x7E, 0x02),   detail::stop<TColor>(45, 0x06, 0x7E, 0x02),
        detail::stop<TColor>(45, 0x04, 0x1E, 0x72),  detail::stop<TColor>(90, 0x04, 0x1E, 0x72),
        detail::stop<TColor>(90, 0xFF, 0x05, 0x00),  detail::stop<TColor>(135, 0xFF, 0x05, 0x00),
        detail::stop<TColor>(135, 0xC4, 0x39, 0x02), detail::stop<TColor>(180, 0xC4, 0x39, 0x02),
        detail::stop<TColor>(180, 0x89, 0x55, 0x02), detail::stop<TColor>(255, 0x89, 0x55, 0x02),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> C9New()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        detail::stop<TColor>(0, 0xFF, 0x05, 0x00),   detail::stop<TColor>(60, 0xFF, 0x05, 0x00),
        detail::stop<TColor>(60, 0xC4, 0x39, 0x02),  detail::stop<TColor>(61, 0x78, 0xC4, 0x39),
        detail::stop<TColor>(2, 0x78, 0x06, 0x7E),   detail::stop<TColor>(2, 0x7E, 0xB4, 0x06),
        detail::stop<TColor>(126, 0x02, 0xB4, 0x04), detail::stop<TColor>(30, 0x72, 0xBF, 0xFF),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Temperature()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 18> stops = {
        detail::stop<TColor>(0, 0x01, 0x1B, 0x69),   detail::stop<TColor>(14, 0x01, 0x28, 0x7F),
        detail::stop<TColor>(28, 0x01, 0x46, 0xA8),  detail::stop<TColor>(42, 0x01, 0x5C, 0xC5),
        detail::stop<TColor>(56, 0x01, 0x77, 0xDD),  detail::stop<TColor>(70, 0x03, 0x82, 0x97),
        detail::stop<TColor>(84, 0x17, 0x9C, 0x95),  detail::stop<TColor>(99, 0x43, 0xB6, 0x70),
        detail::stop<TColor>(113, 0x79, 0xC9, 0x34), detail::stop<TColor>(127, 0x8E, 0xCB, 0x0B),
        detail::stop<TColor>(141, 0xE0, 0xDF, 0x01), detail::stop<TColor>(155, 0xFC, 0xBB, 0x02),
        detail::stop<TColor>(170, 0xF7, 0x93, 0x01), detail::stop<TColor>(184, 0xED, 0x57, 0x01),
        detail::stop<TColor>(198, 0xE5, 0x2B, 0x01), detail::stop<TColor>(226, 0xAB, 0x02, 0x02),
        detail::stop<TColor>(240, 0x50, 0x03, 0x03), detail::stop<TColor>(255, 0x50, 0x03, 0x03),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Aurora2()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        detail::stop<TColor>(0, 0x11, 0xB1, 0x0D),   detail::stop<TColor>(64, 0x79, 0xF2, 0x05),
        detail::stop<TColor>(128, 0x19, 0xAD, 0x79), detail::stop<TColor>(192, 0xFA, 0x4D, 0x7F),
        detail::stop<TColor>(255, 0xAB, 0x65, 0xDD),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> RetroClown()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 3> stops = {
        detail::stop<TColor>(0, 0xE3, 0x65, 0x03),
        detail::stop<TColor>(117, 0xC2, 0x12, 0x13),
        detail::stop<TColor>(255, 0x5C, 0x08, 0xC0),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Candy()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        detail::stop<TColor>(0, 0xE5, 0xE3, 0x01),   detail::stop<TColor>(15, 0xE3, 0x65, 0x03),
        detail::stop<TColor>(142, 0x28, 0x01, 0x50), detail::stop<TColor>(198, 0x11, 0x01, 0x4F),
        detail::stop<TColor>(255, 0x00, 0x00, 0x2D),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> ToxyReaf()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 2> stops = {
        detail::stop<TColor>(0, 0x01, 0xDD, 0x35),
        detail::stop<TColor>(255, 0x49, 0x03, 0xB2),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> FairyReaf()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 4> stops = {
        detail::stop<TColor>(0, 0xB8, 0x01, 0x80),
        detail::stop<TColor>(160, 0x01, 0xC1, 0xB6),
        detail::stop<TColor>(219, 0x99, 0xE3, 0xBE),
        detail::stop<TColor>(255, 0xFF, 0xFF, 0xFF),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> SemiBlue()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 9> stops = {
        detail::stop<TColor>(0, 0x00, 0x00, 0x00),   detail::stop<TColor>(12, 0x01, 0x01, 0x03),
        detail::stop<TColor>(53, 0x08, 0x01, 0x16),  detail::stop<TColor>(80, 0x04, 0x06, 0x59),
        detail::stop<TColor>(119, 0x02, 0x19, 0xD8), detail::stop<TColor>(145, 0x07, 0x0A, 0x63),
        detail::stop<TColor>(186, 0x0F, 0x02, 0x1F), detail::stop<TColor>(233, 0x02, 0x01, 0x05),
        detail::stop<TColor>(255, 0x00, 0x00, 0x00),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> PinkCandy()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        detail::stop<TColor>(0, 0xFF, 0xFF, 0xFF),   detail::stop<TColor>(45, 0x07, 0x0C, 0xFF),
        detail::stop<TColor>(112, 0xE3, 0x01, 0x7F), detail::stop<TColor>(112, 0xE3, 0x01, 0x7F),
        detail::stop<TColor>(140, 0xFF, 0xFF, 0xFF), detail::stop<TColor>(155, 0xE3, 0x01, 0x7F),
        detail::stop<TColor>(196, 0x2D, 0x01, 0x63), detail::stop<TColor>(255, 0xFF, 0xFF, 0xFF),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> RedReaf()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 4> stops = {
        detail::stop<TColor>(0, 0x03, 0x0D, 0x2B),
        detail::stop<TColor>(104, 0x4E, 0x8D, 0xF0),
        detail::stop<TColor>(188, 0xFF, 0x00, 0x00),
        detail::stop<TColor>(255, 0x1C, 0x01, 0x01),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> AquaFlash()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        detail::stop<TColor>(0, 0x00, 0x00, 0x00),   detail::stop<TColor>(66, 0x39, 0xE3, 0xE9),
        detail::stop<TColor>(96, 0xFF, 0xFF, 0x08),  detail::stop<TColor>(124, 0xFF, 0xFF, 0xFF),
        detail::stop<TColor>(153, 0xFF, 0xFF, 0x08), detail::stop<TColor>(188, 0x39, 0xE3, 0xE9),
        detail::stop<TColor>(255, 0x00, 0x00, 0x00),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> YelbluHot()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        detail::stop<TColor>(0, 0x04, 0x02, 0x09),   detail::stop<TColor>(58, 0x10, 0x00, 0x2F),
        detail::stop<TColor>(122, 0x18, 0x00, 0x10), detail::stop<TColor>(158, 0x90, 0x09, 0x01),
        detail::stop<TColor>(183, 0xB3, 0x2D, 0x01), detail::stop<TColor>(219, 0xDC, 0x72, 0x02),
        detail::stop<TColor>(255, 0xEA, 0xED, 0x01),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> LiteLight()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 6> stops = {
        detail::stop<TColor>(0, 0x00, 0x00, 0x00),   detail::stop<TColor>(9, 0x01, 0x01, 0x01),
        detail::stop<TColor>(40, 0x05, 0x05, 0x06),  detail::stop<TColor>(66, 0x05, 0x05, 0x06),
        detail::stop<TColor>(101, 0x0A, 0x01, 0x0C), detail::stop<TColor>(255, 0x00, 0x00, 0x00),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> RedFlash()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        detail::stop<TColor>(0, 0x00, 0x00, 0x00),   detail::stop<TColor>(99, 0xE3, 0x01, 0x01),
        detail::stop<TColor>(130, 0xF9, 0xC7, 0x5F), detail::stop<TColor>(155, 0xE3, 0x01, 0x01),
        detail::stop<TColor>(255, 0x00, 0x00, 0x00),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> BlinkRed()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        detail::stop<TColor>(0, 0x01, 0x01, 0x01),   detail::stop<TColor>(43, 0x04, 0x01, 0x0B),
        detail::stop<TColor>(76, 0x0A, 0x01, 0x03),  detail::stop<TColor>(109, 0xA1, 0x04, 0x1D),
        detail::stop<TColor>(127, 0xFF, 0x56, 0x7B), detail::stop<TColor>(165, 0x7D, 0x10, 0xA0),
        detail::stop<TColor>(204, 0x23, 0x0D, 0xDF), detail::stop<TColor>(255, 0x12, 0x02, 0x12),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> RedShift()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        detail::stop<TColor>(0, 0x1F, 0x01, 0x1B),   detail::stop<TColor>(45, 0x22, 0x01, 0x10),
        detail::stop<TColor>(99, 0x89, 0x05, 0x09),  detail::stop<TColor>(132, 0xD5, 0x80, 0x0A),
        detail::stop<TColor>(175, 0xC7, 0x16, 0x01), detail::stop<TColor>(201, 0xC7, 0x09, 0x06),
        detail::stop<TColor>(255, 0x01, 0x00, 0x01),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> RedTide()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 11> stops = {
        detail::stop<TColor>(0, 0xF7, 0x05, 0x00),   detail::stop<TColor>(28, 0xFF, 0x43, 0x01),
        detail::stop<TColor>(43, 0xEA, 0x58, 0x0B),  detail::stop<TColor>(58, 0xEA, 0xB0, 0x33),
        detail::stop<TColor>(84, 0xE5, 0x1C, 0x01),  detail::stop<TColor>(114, 0x71, 0x0C, 0x01),
        detail::stop<TColor>(140, 0xFF, 0xE1, 0x2C), detail::stop<TColor>(168, 0x71, 0x0C, 0x01),
        detail::stop<TColor>(196, 0xF4, 0xD1, 0x58), detail::stop<TColor>(216, 0xFF, 0x1C, 0x01),
        detail::stop<TColor>(255, 0x35, 0x01, 0x01),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Candy2()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 10> stops = {
        detail::stop<TColor>(0, 0x27, 0x21, 0x22),   detail::stop<TColor>(25, 0x04, 0x06, 0x0F),
        detail::stop<TColor>(48, 0x31, 0x1D, 0x16),  detail::stop<TColor>(73, 0xE0, 0xAD, 0x01),
        detail::stop<TColor>(89, 0xB1, 0x23, 0x05),  detail::stop<TColor>(130, 0x04, 0x06, 0x0F),
        detail::stop<TColor>(163, 0xFF, 0x72, 0x06), detail::stop<TColor>(186, 0xE0, 0xAD, 0x01),
        detail::stop<TColor>(211, 0x27, 0x21, 0x22), detail::stop<TColor>(255, 0x01, 0x01, 0x01),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> AudioResponsiveRatio()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 3> stops = {
        detail::stop<TColor>(0, 0xFF, 0x00, 0x00),
        detail::stop<TColor>(125, 0x00, 0xFF, 0x00),
        detail::stop<TColor>(255, 0x00, 0x00, 0xFF),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> AudioResponsiveHue()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 3> stops = {
        detail::stop<TColor>(0, 0xFF, 0x00, 0x00),
        detail::stop<TColor>(125, 0x00, 0xFF, 0x00),
        detail::stop<TColor>(255, 0x00, 0x00, 0xFF),
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> AudioResponsiveRamp()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 3> stops = {
        detail::stop<TColor>(0, 0xFF, 0x00, 0x00),
        detail::stop<TColor>(125, 0x00, 0xFF, 0x00),
        detail::stop<TColor>(255, 0x00, 0x00, 0xFF),
    };
    return detail::makeGenerator(stops);
}
