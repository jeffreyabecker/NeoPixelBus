#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "core/Compat.h"
#include "colors/Colors.h"
#include "colors/palette/Generators.h"

namespace lw::palettes
{
template <typename TColor = colors::DefaultColorType> struct NamedPalette
{
    const char* name;
    colors::palettes::Palette<TColor> (*create)();
};

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Party()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x55, 0x00, 0xAB),   colors::palettes::PaletteStop<TColor>::fromRgb8(17, 0x84, 0x00, 0x7C),
        colors::palettes::PaletteStop<TColor>::fromRgb8(34, 0xB5, 0x00, 0x4B),  colors::palettes::PaletteStop<TColor>::fromRgb8(51, 0xE5, 0x00, 0x1B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(68, 0xE8, 0x17, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(85, 0xB8, 0x47, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(102, 0xAB, 0x77, 0x00), colors::palettes::PaletteStop<TColor>::fromRgb8(119, 0xAB, 0xAB, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(136, 0xAB, 0x55, 0x00), colors::palettes::PaletteStop<TColor>::fromRgb8(153, 0xDD, 0x22, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0xF2, 0x00, 0x0E), colors::palettes::PaletteStop<TColor>::fromRgb8(187, 0xC2, 0x00, 0x3E),
        colors::palettes::PaletteStop<TColor>::fromRgb8(204, 0x8F, 0x00, 0x71), colors::palettes::PaletteStop<TColor>::fromRgb8(221, 0x5F, 0x00, 0xA1),
        colors::palettes::PaletteStop<TColor>::fromRgb8(238, 0x2F, 0x00, 0xD0), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x07, 0xF9),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Cloud()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x00, 0xFF),   colors::palettes::PaletteStop<TColor>::fromRgb8(17, 0x00, 0x00, 0x8B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(34, 0x00, 0x00, 0x8B),  colors::palettes::PaletteStop<TColor>::fromRgb8(51, 0x00, 0x00, 0x8B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(68, 0x00, 0x00, 0x8B),  colors::palettes::PaletteStop<TColor>::fromRgb8(85, 0x00, 0x00, 0x8B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(102, 0x00, 0x00, 0x8B), colors::palettes::PaletteStop<TColor>::fromRgb8(119, 0x00, 0x00, 0x8B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(136, 0x00, 0x00, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(153, 0x00, 0x00, 0x8B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0x87, 0xCE, 0xEB), colors::palettes::PaletteStop<TColor>::fromRgb8(187, 0x87, 0xCE, 0xEB),
        colors::palettes::PaletteStop<TColor>::fromRgb8(204, 0xAD, 0xD8, 0xE6), colors::palettes::PaletteStop<TColor>::fromRgb8(221, 0xFF, 0xFF, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(238, 0xAD, 0xD8, 0xE6), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x87, 0xCE, 0xEB),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Lava()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(17, 0x80, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(34, 0x00, 0x00, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(51, 0x80, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(68, 0x8B, 0x00, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(85, 0x8B, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(102, 0x80, 0x00, 0x00), colors::palettes::PaletteStop<TColor>::fromRgb8(119, 0x8B, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(136, 0x8B, 0x00, 0x00), colors::palettes::PaletteStop<TColor>::fromRgb8(153, 0x8B, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0xFF, 0x00, 0x00), colors::palettes::PaletteStop<TColor>::fromRgb8(187, 0xFF, 0xA5, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(204, 0xFF, 0xFF, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(221, 0xFF, 0xA5, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(238, 0xFF, 0x00, 0x00), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x8B, 0x00, 0x00),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Ocean()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x19, 0x19, 0x70),   colors::palettes::PaletteStop<TColor>::fromRgb8(17, 0x00, 0x00, 0x8B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(34, 0x19, 0x19, 0x70),  colors::palettes::PaletteStop<TColor>::fromRgb8(51, 0x00, 0x00, 0x80),
        colors::palettes::PaletteStop<TColor>::fromRgb8(68, 0x00, 0x00, 0x8B),  colors::palettes::PaletteStop<TColor>::fromRgb8(85, 0x00, 0x00, 0xCD),
        colors::palettes::PaletteStop<TColor>::fromRgb8(102, 0x2E, 0x8B, 0x57), colors::palettes::PaletteStop<TColor>::fromRgb8(119, 0x00, 0x80, 0x80),
        colors::palettes::PaletteStop<TColor>::fromRgb8(136, 0x5F, 0x9E, 0xA0), colors::palettes::PaletteStop<TColor>::fromRgb8(153, 0x00, 0x00, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0x00, 0x8B, 0x8B), colors::palettes::PaletteStop<TColor>::fromRgb8(187, 0x64, 0x95, 0xED),
        colors::palettes::PaletteStop<TColor>::fromRgb8(204, 0x7F, 0xFF, 0xD4), colors::palettes::PaletteStop<TColor>::fromRgb8(221, 0x2E, 0x8B, 0x57),
        colors::palettes::PaletteStop<TColor>::fromRgb8(238, 0x00, 0xFF, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x87, 0xCE, 0xFA),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Forest()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x64, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(17, 0x00, 0x64, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(34, 0x55, 0x6B, 0x2F),  colors::palettes::PaletteStop<TColor>::fromRgb8(51, 0x00, 0x64, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(68, 0x00, 0x80, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(85, 0x22, 0x8B, 0x22),
        colors::palettes::PaletteStop<TColor>::fromRgb8(102, 0x6B, 0x8E, 0x23), colors::palettes::PaletteStop<TColor>::fromRgb8(119, 0x00, 0x80, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(136, 0x2E, 0x8B, 0x57), colors::palettes::PaletteStop<TColor>::fromRgb8(153, 0x66, 0xCD, 0xAA),
        colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0x32, 0xCD, 0x32), colors::palettes::PaletteStop<TColor>::fromRgb8(187, 0x9A, 0xCD, 0x32),
        colors::palettes::PaletteStop<TColor>::fromRgb8(204, 0x90, 0xEE, 0x90), colors::palettes::PaletteStop<TColor>::fromRgb8(221, 0x7C, 0xFC, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(238, 0x66, 0xCD, 0xAA), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x22, 0x8B, 0x22),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Rainbow()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xFF, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(17, 0xD5, 0x2A, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(34, 0xAB, 0x55, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(51, 0xAB, 0x7F, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(68, 0xAB, 0xAB, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(85, 0x56, 0xD5, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(102, 0x00, 0xFF, 0x00), colors::palettes::PaletteStop<TColor>::fromRgb8(119, 0x00, 0xD5, 0x2A),
        colors::palettes::PaletteStop<TColor>::fromRgb8(136, 0x00, 0xAB, 0x55), colors::palettes::PaletteStop<TColor>::fromRgb8(153, 0x00, 0x56, 0xAA),
        colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0x00, 0x00, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(187, 0x2A, 0x00, 0xD5),
        colors::palettes::PaletteStop<TColor>::fromRgb8(204, 0x55, 0x00, 0xAB), colors::palettes::PaletteStop<TColor>::fromRgb8(221, 0x7F, 0x00, 0x81),
        colors::palettes::PaletteStop<TColor>::fromRgb8(238, 0xAB, 0x00, 0x55), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xD5, 0x00, 0x2B),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> RainbowBands()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xFF, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(17, 0x00, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(34, 0xAB, 0x55, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(51, 0x00, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(68, 0xAB, 0xAB, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(85, 0x00, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(102, 0x00, 0xFF, 0x00), colors::palettes::PaletteStop<TColor>::fromRgb8(119, 0x00, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(136, 0x00, 0xAB, 0x55), colors::palettes::PaletteStop<TColor>::fromRgb8(153, 0x00, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0x00, 0x00, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(187, 0x00, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(204, 0x55, 0x00, 0xAB), colors::palettes::PaletteStop<TColor>::fromRgb8(221, 0x00, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(238, 0xAB, 0x00, 0x55), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x00, 0x00),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Sunset()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x78, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(22, 0xB3, 0x16, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(51, 0xFF, 0x68, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(85, 0xA7, 0x16, 0x12),
        colors::palettes::PaletteStop<TColor>::fromRgb8(135, 0x64, 0x00, 0x67), colors::palettes::PaletteStop<TColor>::fromRgb8(198, 0x10, 0x00, 0x82),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x00, 0xA0),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Rivendell()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x01, 0x0E, 0x05),   colors::palettes::PaletteStop<TColor>::fromRgb8(101, 0x10, 0x24, 0x0E),
        colors::palettes::PaletteStop<TColor>::fromRgb8(165, 0x38, 0x44, 0x1E), colors::palettes::PaletteStop<TColor>::fromRgb8(242, 0x96, 0x9C, 0x63),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x96, 0x9C, 0x63),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Breeze()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 4> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x01, 0x06, 0x07),
        colors::palettes::PaletteStop<TColor>::fromRgb8(89, 0x01, 0x63, 0x6F),
        colors::palettes::PaletteStop<TColor>::fromRgb8(153, 0x90, 0xD1, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x49, 0x52),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> RedBlue()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 9> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x04, 0x01, 0x46),   colors::palettes::PaletteStop<TColor>::fromRgb8(31, 0x37, 0x01, 0x1E),
        colors::palettes::PaletteStop<TColor>::fromRgb8(63, 0xFF, 0x04, 0x07),  colors::palettes::PaletteStop<TColor>::fromRgb8(95, 0x3B, 0x02, 0x1D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0x0B, 0x03, 0x32), colors::palettes::PaletteStop<TColor>::fromRgb8(159, 0x27, 0x08, 0x3C),
        colors::palettes::PaletteStop<TColor>::fromRgb8(191, 0x70, 0x13, 0x28), colors::palettes::PaletteStop<TColor>::fromRgb8(223, 0x4E, 0x0B, 0x27),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x1D, 0x08, 0x3B),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Yellowout()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 2> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xBC, 0x87, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x2E, 0x07, 0x01),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Analogous()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x03, 0x00, 0xFF),   colors::palettes::PaletteStop<TColor>::fromRgb8(63, 0x17, 0x00, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0x43, 0x00, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(191, 0x8E, 0x00, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xFF, 0x00, 0x00),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Splash()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x7E, 0x0B, 0xFF),   colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0xC5, 0x01, 0x16),
        colors::palettes::PaletteStop<TColor>::fromRgb8(175, 0xD2, 0x9D, 0xAC), colors::palettes::PaletteStop<TColor>::fromRgb8(221, 0x9D, 0x03, 0x70),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x9D, 0x03, 0x70),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Pastel()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 11> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x0A, 0x3E, 0x7B),   colors::palettes::PaletteStop<TColor>::fromRgb8(36, 0x38, 0x82, 0x67),
        colors::palettes::PaletteStop<TColor>::fromRgb8(87, 0x99, 0xE1, 0x55),  colors::palettes::PaletteStop<TColor>::fromRgb8(100, 0xC7, 0xD9, 0x44),
        colors::palettes::PaletteStop<TColor>::fromRgb8(107, 0xFF, 0xCF, 0x36), colors::palettes::PaletteStop<TColor>::fromRgb8(115, 0xF7, 0x98, 0x39),
        colors::palettes::PaletteStop<TColor>::fromRgb8(120, 0xEF, 0x6B, 0x3D), colors::palettes::PaletteStop<TColor>::fromRgb8(128, 0xF7, 0x98, 0x39),
        colors::palettes::PaletteStop<TColor>::fromRgb8(180, 0xFF, 0xCF, 0x36), colors::palettes::PaletteStop<TColor>::fromRgb8(223, 0xFF, 0xE3, 0x30),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xFF, 0xF8, 0x2A),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Sunset2()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x6E, 0x31, 0x0B),   colors::palettes::PaletteStop<TColor>::fromRgb8(29, 0x37, 0x22, 0x0A),
        colors::palettes::PaletteStop<TColor>::fromRgb8(68, 0x16, 0x16, 0x09),  colors::palettes::PaletteStop<TColor>::fromRgb8(68, 0xEF, 0x7C, 0x08),
        colors::palettes::PaletteStop<TColor>::fromRgb8(97, 0xDC, 0x9C, 0x1B),  colors::palettes::PaletteStop<TColor>::fromRgb8(124, 0xCB, 0xC1, 0x3D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(178, 0x21, 0x35, 0x38), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x01, 0x34),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Beach()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 15> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xFF, 0xFC, 0xD6),   colors::palettes::PaletteStop<TColor>::fromRgb8(12, 0xFF, 0xFC, 0xD6),
        colors::palettes::PaletteStop<TColor>::fromRgb8(22, 0xFF, 0xFC, 0xD6),  colors::palettes::PaletteStop<TColor>::fromRgb8(26, 0xBE, 0xBF, 0x73),
        colors::palettes::PaletteStop<TColor>::fromRgb8(28, 0x89, 0x8D, 0x34),  colors::palettes::PaletteStop<TColor>::fromRgb8(28, 0x70, 0xFF, 0xCD),
        colors::palettes::PaletteStop<TColor>::fromRgb8(50, 0x33, 0xF6, 0xD6),  colors::palettes::PaletteStop<TColor>::fromRgb8(71, 0x11, 0xEB, 0xE2),
        colors::palettes::PaletteStop<TColor>::fromRgb8(93, 0x02, 0xC1, 0xC7),  colors::palettes::PaletteStop<TColor>::fromRgb8(120, 0x00, 0x9C, 0xAE),
        colors::palettes::PaletteStop<TColor>::fromRgb8(133, 0x01, 0x65, 0x73), colors::palettes::PaletteStop<TColor>::fromRgb8(136, 0x01, 0x3B, 0x47),
        colors::palettes::PaletteStop<TColor>::fromRgb8(136, 0x07, 0x83, 0xAA), colors::palettes::PaletteStop<TColor>::fromRgb8(208, 0x01, 0x5A, 0x97),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x38, 0x85),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Vintage()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x04, 0x01, 0x01),   colors::palettes::PaletteStop<TColor>::fromRgb8(51, 0x10, 0x00, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(76, 0x61, 0x68, 0x03),  colors::palettes::PaletteStop<TColor>::fromRgb8(101, 0xFF, 0x83, 0x13),
        colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0x43, 0x09, 0x04), colors::palettes::PaletteStop<TColor>::fromRgb8(153, 0x10, 0x00, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(229, 0x04, 0x01, 0x01), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x04, 0x01, 0x01),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Departure()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 12> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x08, 0x03, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(42, 0x17, 0x07, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(63, 0x4B, 0x26, 0x06),  colors::palettes::PaletteStop<TColor>::fromRgb8(84, 0xA9, 0x63, 0x26),
        colors::palettes::PaletteStop<TColor>::fromRgb8(106, 0xD5, 0xA9, 0x77), colors::palettes::PaletteStop<TColor>::fromRgb8(116, 0xFF, 0xFF, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(138, 0x87, 0xFF, 0x8A), colors::palettes::PaletteStop<TColor>::fromRgb8(148, 0x16, 0xFF, 0x18),
        colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0x00, 0xFF, 0x00), colors::palettes::PaletteStop<TColor>::fromRgb8(191, 0x00, 0x88, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(212, 0x00, 0x37, 0x00), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x37, 0x00),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Landscape()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 9> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(37, 0x02, 0x19, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(76, 0x0F, 0x73, 0x05),  colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0x4F, 0xD5, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(128, 0x7E, 0xD3, 0x2F), colors::palettes::PaletteStop<TColor>::fromRgb8(130, 0xBC, 0xD1, 0xF7),
        colors::palettes::PaletteStop<TColor>::fromRgb8(153, 0x90, 0xB6, 0xCD), colors::palettes::PaletteStop<TColor>::fromRgb8(204, 0x3B, 0x75, 0xFA),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x01, 0x25, 0xC0),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Beech()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 6> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x01, 0x05, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(19, 0x20, 0x17, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(38, 0xA1, 0x37, 0x01), colors::palettes::PaletteStop<TColor>::fromRgb8(63, 0xE5, 0x90, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(66, 0x27, 0x8E, 0x4A), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x01, 0x04, 0x01),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Sherbet()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xFF, 0x21, 0x04),   colors::palettes::PaletteStop<TColor>::fromRgb8(43, 0xFF, 0x44, 0x19),
        colors::palettes::PaletteStop<TColor>::fromRgb8(86, 0xFF, 0x07, 0x19),  colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0xFF, 0x52, 0x67),
        colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0xFF, 0xFF, 0xF2), colors::palettes::PaletteStop<TColor>::fromRgb8(209, 0x2A, 0xFF, 0x16),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x57, 0xFF, 0x41),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Hult()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 6> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xF7, 0xB0, 0xF7),   colors::palettes::PaletteStop<TColor>::fromRgb8(48, 0xFF, 0x88, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(89, 0xDC, 0x1D, 0xE2),  colors::palettes::PaletteStop<TColor>::fromRgb8(160, 0x07, 0x52, 0xB2),
        colors::palettes::PaletteStop<TColor>::fromRgb8(216, 0x01, 0x7C, 0x6D), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x01, 0x7C, 0x6D),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Hult64()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x01, 0x7C, 0x6D),   colors::palettes::PaletteStop<TColor>::fromRgb8(66, 0x01, 0x5D, 0x4F),
        colors::palettes::PaletteStop<TColor>::fromRgb8(104, 0x34, 0x41, 0x01), colors::palettes::PaletteStop<TColor>::fromRgb8(130, 0x73, 0x7F, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(150, 0x34, 0x41, 0x01), colors::palettes::PaletteStop<TColor>::fromRgb8(201, 0x01, 0x56, 0x48),
        colors::palettes::PaletteStop<TColor>::fromRgb8(239, 0x00, 0x37, 0x2D), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x37, 0x2D),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Drywet()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x2F, 0x1E, 0x02),   colors::palettes::PaletteStop<TColor>::fromRgb8(42, 0xD5, 0x93, 0x18),
        colors::palettes::PaletteStop<TColor>::fromRgb8(84, 0x67, 0xDB, 0x34),  colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0x03, 0xDB, 0xCF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0x01, 0x30, 0xD6), colors::palettes::PaletteStop<TColor>::fromRgb8(212, 0x01, 0x01, 0x6F),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x01, 0x07, 0x21),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Jul()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 4> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xC2, 0x01, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(94, 0x01, 0x1D, 0x12),
        colors::palettes::PaletteStop<TColor>::fromRgb8(132, 0x39, 0x83, 0x1C),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x71, 0x01, 0x01),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Grintage()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x02, 0x01, 0x01),   colors::palettes::PaletteStop<TColor>::fromRgb8(53, 0x12, 0x01, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(104, 0x45, 0x1D, 0x01), colors::palettes::PaletteStop<TColor>::fromRgb8(153, 0xA7, 0x87, 0x0A),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x2E, 0x38, 0x04),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Rewhi()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 6> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x71, 0x5B, 0x93),   colors::palettes::PaletteStop<TColor>::fromRgb8(72, 0x9D, 0x58, 0x4E),
        colors::palettes::PaletteStop<TColor>::fromRgb8(89, 0xD0, 0x55, 0x21),  colors::palettes::PaletteStop<TColor>::fromRgb8(107, 0xFF, 0x1D, 0x0B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(141, 0x89, 0x1F, 0x27), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x3B, 0x21, 0x59),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Tertiary()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x01, 0xFF),   colors::palettes::PaletteStop<TColor>::fromRgb8(63, 0x03, 0x44, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0x17, 0xFF, 0x00), colors::palettes::PaletteStop<TColor>::fromRgb8(191, 0x64, 0x44, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xFF, 0x01, 0x04),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Fire()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 13> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(46, 0x12, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(96, 0x71, 0x00, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(108, 0x8E, 0x03, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(119, 0xAF, 0x11, 0x01), colors::palettes::PaletteStop<TColor>::fromRgb8(146, 0xD5, 0x2C, 0x02),
        colors::palettes::PaletteStop<TColor>::fromRgb8(174, 0xFF, 0x52, 0x04), colors::palettes::PaletteStop<TColor>::fromRgb8(188, 0xFF, 0x73, 0x04),
        colors::palettes::PaletteStop<TColor>::fromRgb8(202, 0xFF, 0x9C, 0x04), colors::palettes::PaletteStop<TColor>::fromRgb8(218, 0xFF, 0xCB, 0x04),
        colors::palettes::PaletteStop<TColor>::fromRgb8(234, 0xFF, 0xFF, 0x04), colors::palettes::PaletteStop<TColor>::fromRgb8(244, 0xFF, 0xFF, 0x47),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xFF, 0xFF, 0xFF),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Icefire()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(59, 0x00, 0x09, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(119, 0x00, 0x26, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(149, 0x03, 0x64, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(180, 0x17, 0xC7, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(217, 0x64, 0xEB, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xFF, 0xFF, 0xFF),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Cyane()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 11> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x0A, 0x55, 0x05),   colors::palettes::PaletteStop<TColor>::fromRgb8(25, 0x1D, 0x6D, 0x12),
        colors::palettes::PaletteStop<TColor>::fromRgb8(60, 0x3B, 0x8A, 0x2A),  colors::palettes::PaletteStop<TColor>::fromRgb8(93, 0x53, 0x63, 0x34),
        colors::palettes::PaletteStop<TColor>::fromRgb8(106, 0x6E, 0x42, 0x40), colors::palettes::PaletteStop<TColor>::fromRgb8(109, 0x7B, 0x31, 0x41),
        colors::palettes::PaletteStop<TColor>::fromRgb8(113, 0x8B, 0x23, 0x42), colors::palettes::PaletteStop<TColor>::fromRgb8(116, 0xC0, 0x75, 0x62),
        colors::palettes::PaletteStop<TColor>::fromRgb8(124, 0xFF, 0xFF, 0x89), colors::palettes::PaletteStop<TColor>::fromRgb8(168, 0x64, 0xB4, 0x9B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x16, 0x79, 0xAE),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> LightPink()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 11> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x13, 0x02, 0x27),   colors::palettes::PaletteStop<TColor>::fromRgb8(25, 0x1A, 0x04, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(51, 0x21, 0x06, 0x34),  colors::palettes::PaletteStop<TColor>::fromRgb8(76, 0x44, 0x3E, 0x7D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(102, 0x76, 0xBB, 0xF0), colors::palettes::PaletteStop<TColor>::fromRgb8(109, 0xA3, 0xD7, 0xF7),
        colors::palettes::PaletteStop<TColor>::fromRgb8(114, 0xD9, 0xF4, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(122, 0x9F, 0x95, 0xDD),
        colors::palettes::PaletteStop<TColor>::fromRgb8(149, 0x71, 0x4E, 0xBC), colors::palettes::PaletteStop<TColor>::fromRgb8(183, 0x80, 0x39, 0x9B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x92, 0x28, 0x7B),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Autumn()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 13> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x1A, 0x01, 0x01),   colors::palettes::PaletteStop<TColor>::fromRgb8(51, 0x43, 0x04, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(84, 0x76, 0x0E, 0x01),  colors::palettes::PaletteStop<TColor>::fromRgb8(104, 0x89, 0x98, 0x34),
        colors::palettes::PaletteStop<TColor>::fromRgb8(112, 0x71, 0x41, 0x01), colors::palettes::PaletteStop<TColor>::fromRgb8(122, 0x85, 0x95, 0x3B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(124, 0x89, 0x98, 0x34), colors::palettes::PaletteStop<TColor>::fromRgb8(135, 0x71, 0x41, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(142, 0x8B, 0x9A, 0x2E), colors::palettes::PaletteStop<TColor>::fromRgb8(163, 0x71, 0x0D, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(204, 0x37, 0x03, 0x01), colors::palettes::PaletteStop<TColor>::fromRgb8(249, 0x11, 0x01, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x11, 0x01, 0x01),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Magenta()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(42, 0x00, 0x00, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(84, 0x00, 0x00, 0xFF),  colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0x2A, 0x00, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0xFF, 0x00, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(212, 0xFF, 0x37, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xFF, 0xFF, 0xFF),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Magred()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(63, 0x2A, 0x00, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0xFF, 0x00, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(191, 0xFF, 0x00, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xFF, 0x00, 0x00),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Yelmag()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(42, 0x2A, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(84, 0xFF, 0x00, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0xFF, 0x00, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0xFF, 0x00, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(212, 0xFF, 0x37, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xFF, 0xFF, 0x00),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Yelblu()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x00, 0xFF),   colors::palettes::PaletteStop<TColor>::fromRgb8(63, 0x00, 0x37, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0x00, 0xFF, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(191, 0x2A, 0xFF, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xFF, 0xFF, 0x00),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> OrangeTeal()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 4> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x96, 0x5C),
        colors::palettes::PaletteStop<TColor>::fromRgb8(55, 0x00, 0x96, 0x5C),
        colors::palettes::PaletteStop<TColor>::fromRgb8(200, 0xFF, 0x48, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xFF, 0x48, 0x00),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Tiamat()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 17> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x01, 0x02, 0x0E),   colors::palettes::PaletteStop<TColor>::fromRgb8(33, 0x02, 0x05, 0x23),
        colors::palettes::PaletteStop<TColor>::fromRgb8(47, 0x3D, 0x7E, 0x64),  colors::palettes::PaletteStop<TColor>::fromRgb8(13, 0x87, 0x5C, 0x58),
        colors::palettes::PaletteStop<TColor>::fromRgb8(242, 0xF7, 0x78, 0x2B), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xC1, 0x87, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(253, 0x8C, 0xF7, 0x07), colors::palettes::PaletteStop<TColor>::fromRgb8(249, 0xFC, 0x45, 0xFD),
        colors::palettes::PaletteStop<TColor>::fromRgb8(160, 0xC1, 0x11, 0xD0), colors::palettes::PaletteStop<TColor>::fromRgb8(231, 0x60, 0xED, 0xB4),
        colors::palettes::PaletteStop<TColor>::fromRgb8(39, 0xFF, 0x9A, 0x82),  colors::palettes::PaletteStop<TColor>::fromRgb8(77, 0xD5, 0xC8, 0x04),
        colors::palettes::PaletteStop<TColor>::fromRgb8(213, 0xEC, 0x39, 0x7A), colors::palettes::PaletteStop<TColor>::fromRgb8(248, 0xDC, 0x27, 0xFC),
        colors::palettes::PaletteStop<TColor>::fromRgb8(135, 0xB1, 0xFE, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(240, 0xC1, 0xD5, 0xFD),
        colors::palettes::PaletteStop<TColor>::fromRgb8(203, 0xEF, 0xFD, 0xFF),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> AprilNight()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 17> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x01, 0x05, 0x2D),   colors::palettes::PaletteStop<TColor>::fromRgb8(10, 0x01, 0x05, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(25, 0x05, 0xA9, 0xAF),  colors::palettes::PaletteStop<TColor>::fromRgb8(40, 0x01, 0x05, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(61, 0x01, 0x05, 0x2D),  colors::palettes::PaletteStop<TColor>::fromRgb8(76, 0x2D, 0xAF, 0x1F),
        colors::palettes::PaletteStop<TColor>::fromRgb8(91, 0x01, 0x05, 0x2D),  colors::palettes::PaletteStop<TColor>::fromRgb8(112, 0x01, 0x05, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0xF9, 0x96, 0x05), colors::palettes::PaletteStop<TColor>::fromRgb8(143, 0x01, 0x05, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(162, 0x01, 0x05, 0x2D), colors::palettes::PaletteStop<TColor>::fromRgb8(178, 0xFF, 0x5C, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(193, 0x01, 0x05, 0x2D), colors::palettes::PaletteStop<TColor>::fromRgb8(214, 0x01, 0x05, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(229, 0xDF, 0x2D, 0x48), colors::palettes::PaletteStop<TColor>::fromRgb8(244, 0x01, 0x05, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x01, 0x05, 0x2D),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Orangery()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 9> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xFF, 0x5F, 0x17),   colors::palettes::PaletteStop<TColor>::fromRgb8(30, 0xFF, 0x52, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(60, 0xDF, 0x0D, 0x08),  colors::palettes::PaletteStop<TColor>::fromRgb8(90, 0x90, 0x2C, 0x02),
        colors::palettes::PaletteStop<TColor>::fromRgb8(120, 0xFF, 0x6E, 0x11), colors::palettes::PaletteStop<TColor>::fromRgb8(150, 0xFF, 0x45, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(180, 0x9E, 0x0D, 0x0B), colors::palettes::PaletteStop<TColor>::fromRgb8(210, 0xF1, 0x52, 0x11),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xD5, 0x25, 0x04),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> C9()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xB8, 0x04, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(60, 0xB8, 0x04, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(65, 0x90, 0x2C, 0x02),  colors::palettes::PaletteStop<TColor>::fromRgb8(125, 0x90, 0x2C, 0x02),
        colors::palettes::PaletteStop<TColor>::fromRgb8(130, 0x04, 0x60, 0x02), colors::palettes::PaletteStop<TColor>::fromRgb8(190, 0x04, 0x60, 0x02),
        colors::palettes::PaletteStop<TColor>::fromRgb8(195, 0x07, 0x07, 0x58), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x07, 0x07, 0x58),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Sakura()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xC4, 0x13, 0x0A),   colors::palettes::PaletteStop<TColor>::fromRgb8(65, 0xFF, 0x45, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(130, 0xDF, 0x2D, 0x48), colors::palettes::PaletteStop<TColor>::fromRgb8(195, 0xFF, 0x52, 0x67),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xDF, 0x0D, 0x11),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Aurora()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 6> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x01, 0x05, 0x2D),   colors::palettes::PaletteStop<TColor>::fromRgb8(64, 0x00, 0xC8, 0x17),
        colors::palettes::PaletteStop<TColor>::fromRgb8(128, 0x00, 0xFF, 0x00), colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0x00, 0xF3, 0x2D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(200, 0x00, 0x87, 0x07), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x01, 0x05, 0x2D),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Atlantica()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x1C, 0x70),   colors::palettes::PaletteStop<TColor>::fromRgb8(1, 0x46, 0x32, 0x20),
        colors::palettes::PaletteStop<TColor>::fromRgb8(96, 0xFF, 0x80, 0xC6),  colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xF3, 0x2D, 0x96),
        colors::palettes::PaletteStop<TColor>::fromRgb8(12, 0x5F, 0x52, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(5, 0x34, 0xC8, 0x19),
        colors::palettes::PaletteStop<TColor>::fromRgb8(190, 0x5F, 0x13, 0x05), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x28, 0xAA, 0x50),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> C92()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 10> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x06, 0x7E, 0x02),   colors::palettes::PaletteStop<TColor>::fromRgb8(45, 0x06, 0x7E, 0x02),
        colors::palettes::PaletteStop<TColor>::fromRgb8(45, 0x04, 0x1E, 0x72),  colors::palettes::PaletteStop<TColor>::fromRgb8(90, 0x04, 0x1E, 0x72),
        colors::palettes::PaletteStop<TColor>::fromRgb8(90, 0xFF, 0x05, 0x00),  colors::palettes::PaletteStop<TColor>::fromRgb8(135, 0xFF, 0x05, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(135, 0xC4, 0x39, 0x02), colors::palettes::PaletteStop<TColor>::fromRgb8(180, 0xC4, 0x39, 0x02),
        colors::palettes::PaletteStop<TColor>::fromRgb8(180, 0x89, 0x55, 0x02), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x89, 0x55, 0x02),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> C9New()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xFF, 0x05, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(60, 0xFF, 0x05, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(60, 0xC4, 0x39, 0x02),  colors::palettes::PaletteStop<TColor>::fromRgb8(61, 0x78, 0xC4, 0x39),
        colors::palettes::PaletteStop<TColor>::fromRgb8(2, 0x78, 0x06, 0x7E),   colors::palettes::PaletteStop<TColor>::fromRgb8(2, 0x7E, 0xB4, 0x06),
        colors::palettes::PaletteStop<TColor>::fromRgb8(126, 0x02, 0xB4, 0x04), colors::palettes::PaletteStop<TColor>::fromRgb8(30, 0x72, 0xBF, 0xFF),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Temperature()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 18> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x01, 0x1B, 0x69),   colors::palettes::PaletteStop<TColor>::fromRgb8(14, 0x01, 0x28, 0x7F),
        colors::palettes::PaletteStop<TColor>::fromRgb8(28, 0x01, 0x46, 0xA8),  colors::palettes::PaletteStop<TColor>::fromRgb8(42, 0x01, 0x5C, 0xC5),
        colors::palettes::PaletteStop<TColor>::fromRgb8(56, 0x01, 0x77, 0xDD),  colors::palettes::PaletteStop<TColor>::fromRgb8(70, 0x03, 0x82, 0x97),
        colors::palettes::PaletteStop<TColor>::fromRgb8(84, 0x17, 0x9C, 0x95),  colors::palettes::PaletteStop<TColor>::fromRgb8(99, 0x43, 0xB6, 0x70),
        colors::palettes::PaletteStop<TColor>::fromRgb8(113, 0x79, 0xC9, 0x34), colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0x8E, 0xCB, 0x0B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(141, 0xE0, 0xDF, 0x01), colors::palettes::PaletteStop<TColor>::fromRgb8(155, 0xFC, 0xBB, 0x02),
        colors::palettes::PaletteStop<TColor>::fromRgb8(170, 0xF7, 0x93, 0x01), colors::palettes::PaletteStop<TColor>::fromRgb8(184, 0xED, 0x57, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(198, 0xE5, 0x2B, 0x01), colors::palettes::PaletteStop<TColor>::fromRgb8(226, 0xAB, 0x02, 0x02),
        colors::palettes::PaletteStop<TColor>::fromRgb8(240, 0x50, 0x03, 0x03), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x50, 0x03, 0x03),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Aurora2()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x11, 0xB1, 0x0D),   colors::palettes::PaletteStop<TColor>::fromRgb8(64, 0x79, 0xF2, 0x05),
        colors::palettes::PaletteStop<TColor>::fromRgb8(128, 0x19, 0xAD, 0x79), colors::palettes::PaletteStop<TColor>::fromRgb8(192, 0xFA, 0x4D, 0x7F),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xAB, 0x65, 0xDD),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> RetroClown()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 3> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xE3, 0x65, 0x03),
        colors::palettes::PaletteStop<TColor>::fromRgb8(117, 0xC2, 0x12, 0x13),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x5C, 0x08, 0xC0),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Candy()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xE5, 0xE3, 0x01),   colors::palettes::PaletteStop<TColor>::fromRgb8(15, 0xE3, 0x65, 0x03),
        colors::palettes::PaletteStop<TColor>::fromRgb8(142, 0x28, 0x01, 0x50), colors::palettes::PaletteStop<TColor>::fromRgb8(198, 0x11, 0x01, 0x4F),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x00, 0x2D),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> ToxyReaf()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 2> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x01, 0xDD, 0x35),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x49, 0x03, 0xB2),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> FairyReaf()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 4> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xB8, 0x01, 0x80),
        colors::palettes::PaletteStop<TColor>::fromRgb8(160, 0x01, 0xC1, 0xB6),
        colors::palettes::PaletteStop<TColor>::fromRgb8(219, 0x99, 0xE3, 0xBE),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xFF, 0xFF, 0xFF),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> SemiBlue()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 9> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(12, 0x01, 0x01, 0x03),
        colors::palettes::PaletteStop<TColor>::fromRgb8(53, 0x08, 0x01, 0x16),  colors::palettes::PaletteStop<TColor>::fromRgb8(80, 0x04, 0x06, 0x59),
        colors::palettes::PaletteStop<TColor>::fromRgb8(119, 0x02, 0x19, 0xD8), colors::palettes::PaletteStop<TColor>::fromRgb8(145, 0x07, 0x0A, 0x63),
        colors::palettes::PaletteStop<TColor>::fromRgb8(186, 0x0F, 0x02, 0x1F), colors::palettes::PaletteStop<TColor>::fromRgb8(233, 0x02, 0x01, 0x05),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x00, 0x00),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> PinkCandy()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xFF, 0xFF, 0xFF),   colors::palettes::PaletteStop<TColor>::fromRgb8(45, 0x07, 0x0C, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(112, 0xE3, 0x01, 0x7F), colors::palettes::PaletteStop<TColor>::fromRgb8(112, 0xE3, 0x01, 0x7F),
        colors::palettes::PaletteStop<TColor>::fromRgb8(140, 0xFF, 0xFF, 0xFF), colors::palettes::PaletteStop<TColor>::fromRgb8(155, 0xE3, 0x01, 0x7F),
        colors::palettes::PaletteStop<TColor>::fromRgb8(196, 0x2D, 0x01, 0x63), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xFF, 0xFF, 0xFF),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> RedReaf()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 4> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x03, 0x0D, 0x2B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(104, 0x4E, 0x8D, 0xF0),
        colors::palettes::PaletteStop<TColor>::fromRgb8(188, 0xFF, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x1C, 0x01, 0x01),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> AquaFlash()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(66, 0x39, 0xE3, 0xE9),
        colors::palettes::PaletteStop<TColor>::fromRgb8(96, 0xFF, 0xFF, 0x08),  colors::palettes::PaletteStop<TColor>::fromRgb8(124, 0xFF, 0xFF, 0xFF),
        colors::palettes::PaletteStop<TColor>::fromRgb8(153, 0xFF, 0xFF, 0x08), colors::palettes::PaletteStop<TColor>::fromRgb8(188, 0x39, 0xE3, 0xE9),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x00, 0x00),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> YelbluHot()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x04, 0x02, 0x09),   colors::palettes::PaletteStop<TColor>::fromRgb8(58, 0x10, 0x00, 0x2F),
        colors::palettes::PaletteStop<TColor>::fromRgb8(122, 0x18, 0x00, 0x10), colors::palettes::PaletteStop<TColor>::fromRgb8(158, 0x90, 0x09, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(183, 0xB3, 0x2D, 0x01), colors::palettes::PaletteStop<TColor>::fromRgb8(219, 0xDC, 0x72, 0x02),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0xEA, 0xED, 0x01),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> LiteLight()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 6> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(9, 0x01, 0x01, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(40, 0x05, 0x05, 0x06),  colors::palettes::PaletteStop<TColor>::fromRgb8(66, 0x05, 0x05, 0x06),
        colors::palettes::PaletteStop<TColor>::fromRgb8(101, 0x0A, 0x01, 0x0C), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x00, 0x00),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> RedFlash()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 5> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x00, 0x00, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(99, 0xE3, 0x01, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(130, 0xF9, 0xC7, 0x5F), colors::palettes::PaletteStop<TColor>::fromRgb8(155, 0xE3, 0x01, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x00, 0x00),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> BlinkRed()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 8> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x01, 0x01, 0x01),   colors::palettes::PaletteStop<TColor>::fromRgb8(43, 0x04, 0x01, 0x0B),
        colors::palettes::PaletteStop<TColor>::fromRgb8(76, 0x0A, 0x01, 0x03),  colors::palettes::PaletteStop<TColor>::fromRgb8(109, 0xA1, 0x04, 0x1D),
        colors::palettes::PaletteStop<TColor>::fromRgb8(127, 0xFF, 0x56, 0x7B), colors::palettes::PaletteStop<TColor>::fromRgb8(165, 0x7D, 0x10, 0xA0),
        colors::palettes::PaletteStop<TColor>::fromRgb8(204, 0x23, 0x0D, 0xDF), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x12, 0x02, 0x12),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> RedShift()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 7> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x1F, 0x01, 0x1B),   colors::palettes::PaletteStop<TColor>::fromRgb8(45, 0x22, 0x01, 0x10),
        colors::palettes::PaletteStop<TColor>::fromRgb8(99, 0x89, 0x05, 0x09),  colors::palettes::PaletteStop<TColor>::fromRgb8(132, 0xD5, 0x80, 0x0A),
        colors::palettes::PaletteStop<TColor>::fromRgb8(175, 0xC7, 0x16, 0x01), colors::palettes::PaletteStop<TColor>::fromRgb8(201, 0xC7, 0x09, 0x06),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x01, 0x00, 0x01),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> RedTide()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 11> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xF7, 0x05, 0x00),   colors::palettes::PaletteStop<TColor>::fromRgb8(28, 0xFF, 0x43, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(43, 0xEA, 0x58, 0x0B),  colors::palettes::PaletteStop<TColor>::fromRgb8(58, 0xEA, 0xB0, 0x33),
        colors::palettes::PaletteStop<TColor>::fromRgb8(84, 0xE5, 0x1C, 0x01),  colors::palettes::PaletteStop<TColor>::fromRgb8(114, 0x71, 0x0C, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(140, 0xFF, 0xE1, 0x2C), colors::palettes::PaletteStop<TColor>::fromRgb8(168, 0x71, 0x0C, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(196, 0xF4, 0xD1, 0x58), colors::palettes::PaletteStop<TColor>::fromRgb8(216, 0xFF, 0x1C, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x35, 0x01, 0x01),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> Candy2()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 10> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0x27, 0x21, 0x22),   colors::palettes::PaletteStop<TColor>::fromRgb8(25, 0x04, 0x06, 0x0F),
        colors::palettes::PaletteStop<TColor>::fromRgb8(48, 0x31, 0x1D, 0x16),  colors::palettes::PaletteStop<TColor>::fromRgb8(73, 0xE0, 0xAD, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(89, 0xB1, 0x23, 0x05),  colors::palettes::PaletteStop<TColor>::fromRgb8(130, 0x04, 0x06, 0x0F),
        colors::palettes::PaletteStop<TColor>::fromRgb8(163, 0xFF, 0x72, 0x06), colors::palettes::PaletteStop<TColor>::fromRgb8(186, 0xE0, 0xAD, 0x01),
        colors::palettes::PaletteStop<TColor>::fromRgb8(211, 0x27, 0x21, 0x22), colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x01, 0x01, 0x01),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> AudioResponsiveRatio()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 3> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xFF, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(125, 0x00, 0xFF, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x00, 0xFF),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> AudioResponsiveHue()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 3> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xFF, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(125, 0x00, 0xFF, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x00, 0xFF),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::Palette<TColor> AudioResponsiveRamp()
{
    constexpr std::array<colors::palettes::PaletteStop<TColor>, 3> stops = {
        colors::palettes::PaletteStop<TColor>::fromRgb8(0, 0xFF, 0x00, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(125, 0x00, 0xFF, 0x00),
        colors::palettes::PaletteStop<TColor>::fromRgb8(255, 0x00, 0x00, 0xFF),
    };
    return colors::palettes::Palette<TColor>(stops);
}

template <typename TColor = colors::DefaultColorType> inline span<const NamedPalette<TColor>> StaticPalettes()
{
    static constexpr std::array<NamedPalette<TColor>, 68> palettes = {
        NamedPalette<TColor>{"Party", &Party<TColor>},
        NamedPalette<TColor>{"Cloud", &Cloud<TColor>},
        NamedPalette<TColor>{"Lava", &Lava<TColor>},
        NamedPalette<TColor>{"Ocean", &Ocean<TColor>},
        NamedPalette<TColor>{"Forest", &Forest<TColor>},
        NamedPalette<TColor>{"Rainbow", &Rainbow<TColor>},
        NamedPalette<TColor>{"Rainbow Bands", &RainbowBands<TColor>},
        NamedPalette<TColor>{"Sunset", &Sunset<TColor>},
        NamedPalette<TColor>{"Rivendell", &Rivendell<TColor>},
        NamedPalette<TColor>{"Breeze", &Breeze<TColor>},
        NamedPalette<TColor>{"Red & Blue", &RedBlue<TColor>},
        NamedPalette<TColor>{"Yellowout", &Yellowout<TColor>},
        NamedPalette<TColor>{"Analogous", &Analogous<TColor>},
        NamedPalette<TColor>{"Splash", &Splash<TColor>},
        NamedPalette<TColor>{"Pastel", &Pastel<TColor>},
        NamedPalette<TColor>{"Sunset 2", &Sunset2<TColor>},
        NamedPalette<TColor>{"Beach", &Beach<TColor>},
        NamedPalette<TColor>{"Vintage", &Vintage<TColor>},
        NamedPalette<TColor>{"Departure", &Departure<TColor>},
        NamedPalette<TColor>{"Landscape", &Landscape<TColor>},
        NamedPalette<TColor>{"Beech", &Beech<TColor>},
        NamedPalette<TColor>{"Sherbet", &Sherbet<TColor>},
        NamedPalette<TColor>{"Hult", &Hult<TColor>},
        NamedPalette<TColor>{"Hult 64", &Hult64<TColor>},
        NamedPalette<TColor>{"Drywet", &Drywet<TColor>},
        NamedPalette<TColor>{"Jul", &Jul<TColor>},
        NamedPalette<TColor>{"Grintage", &Grintage<TColor>},
        NamedPalette<TColor>{"Rewhi", &Rewhi<TColor>},
        NamedPalette<TColor>{"Tertiary", &Tertiary<TColor>},
        NamedPalette<TColor>{"Fire", &Fire<TColor>},
        NamedPalette<TColor>{"Icefire", &Icefire<TColor>},
        NamedPalette<TColor>{"Cyane", &Cyane<TColor>},
        NamedPalette<TColor>{"Light Pink", &LightPink<TColor>},
        NamedPalette<TColor>{"Autumn", &Autumn<TColor>},
        NamedPalette<TColor>{"Magenta", &Magenta<TColor>},
        NamedPalette<TColor>{"Magred", &Magred<TColor>},
        NamedPalette<TColor>{"Yelmag", &Yelmag<TColor>},
        NamedPalette<TColor>{"Yelblu", &Yelblu<TColor>},
        NamedPalette<TColor>{"Orange & Teal", &OrangeTeal<TColor>},
        NamedPalette<TColor>{"Tiamat", &Tiamat<TColor>},
        NamedPalette<TColor>{"April Night", &AprilNight<TColor>},
        NamedPalette<TColor>{"Orangery", &Orangery<TColor>},
        NamedPalette<TColor>{"C9", &C9<TColor>},
        NamedPalette<TColor>{"Sakura", &Sakura<TColor>},
        NamedPalette<TColor>{"Aurora", &Aurora<TColor>},
        NamedPalette<TColor>{"Atlantica", &Atlantica<TColor>},
        NamedPalette<TColor>{"C9 2", &C92<TColor>},
        NamedPalette<TColor>{"C9 New", &C9New<TColor>},
        NamedPalette<TColor>{"Temperature", &Temperature<TColor>},
        NamedPalette<TColor>{"Aurora 2", &Aurora2<TColor>},
        NamedPalette<TColor>{"Retro Clown", &RetroClown<TColor>},
        NamedPalette<TColor>{"Candy", &Candy<TColor>},
        NamedPalette<TColor>{"Toxy Reaf", &ToxyReaf<TColor>},
        NamedPalette<TColor>{"Fairy Reaf", &FairyReaf<TColor>},
        NamedPalette<TColor>{"Semi Blue", &SemiBlue<TColor>},
        NamedPalette<TColor>{"Pink Candy", &PinkCandy<TColor>},
        NamedPalette<TColor>{"Red Reaf", &RedReaf<TColor>},
        NamedPalette<TColor>{"Aqua Flash", &AquaFlash<TColor>},
        NamedPalette<TColor>{"Yelblu Hot", &YelbluHot<TColor>},
        NamedPalette<TColor>{"Lite Light", &LiteLight<TColor>},
        NamedPalette<TColor>{"Red Flash", &RedFlash<TColor>},
        NamedPalette<TColor>{"Blink Red", &BlinkRed<TColor>},
        NamedPalette<TColor>{"Red Shift", &RedShift<TColor>},
        NamedPalette<TColor>{"Red Tide", &RedTide<TColor>},
        NamedPalette<TColor>{"Candy2", &Candy2<TColor>},
        NamedPalette<TColor>{"Audio Responsive Ratio", &AudioResponsiveRatio<TColor>},
        NamedPalette<TColor>{"Audio Responsive Hue", &AudioResponsiveHue<TColor>},
        NamedPalette<TColor>{"Audio Responsive Ramp", &AudioResponsiveRamp<TColor>},
    };
    return span<const NamedPalette<TColor>>(palettes.data(), palettes.size());
}

} // namespace lw::palettes


