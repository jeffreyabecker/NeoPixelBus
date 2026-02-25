#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"
#include "core/ResourceHandle.h"

namespace npb
{

// TLC59711 brightness and control configuration.
//
// Control flags:
//   outtmg  ? true = output on rising edge (default true)
//   extgck  ? true = use external clock on SCKI pin
//   tmgrst  ? true = enable display timer reset (default true)
//   dsprpt  ? true = enable auto display repeat (default true)
//   blank   ? true = outputs blanked
//
// Brightness: 7-bit per channel group (0?127).
//
struct Tlc59711Settings
{
    static constexpr uint8_t MaxBrightness = 127;

    bool outtmg{true};
    bool extgck{false};
    bool tmgrst{true};
    bool dsprpt{true};
    bool blank{false};

    uint8_t bcRed{MaxBrightness};
    uint8_t bcGreen{MaxBrightness};
    uint8_t bcBlue{MaxBrightness};
};

struct Tlc59711ProtocolSettings
{
    ResourceHandle<ITransport> bus;
    Tlc59711Settings config = {};
};

// TLC59711 protocol.
//
// SPI-like two-wire (clock + data), no chip-select.
// 12 channels per chip = 4 RGB pixels per chip.
// 16-bit per channel, big-endian (MSB first).
//
// Per-chip wire format (28 bytes):
//   [4-byte header] [24 bytes channel data]
//
// Header bit layout (32 bits, MSB first on wire):
//   bits [31:26] = 0b100101 (write command)
//   bit  [25]    = OUTTMG
//   bit  [24]    = EXTGCK
//   bit  [23]    = TMGRST
//   bit  [22]    = DSPRPT
//   bit  [21]    = BLANK
//   bits [20:14] = BC_Blue (7-bit)
//   bits [13:7]  = BC_Green (7-bit)
//   bits [6:0]   = BC_Red (7-bit)
//
// Data ordering is REVERSED: last chip transmitted first,
// and within each chip channels go BGR3, BGR2, BGR1, BGR0.
//
// Latch: ~20 ?s guard after transmission.
//
class Tlc59711Protocol : public IProtocol<Rgb8Color>
{
public:
    using SettingsType = Tlc59711ProtocolSettings;
    using TransportCategory = TransportTag;

    Tlc59711Protocol(uint16_t pixelCount,
                    SettingsType settings)
        : _settings{std::move(settings)}
        , _pixelCount{pixelCount}
        , _chipCount{(pixelCount + PixelsPerChip - 1) / PixelsPerChip}
        , _byteBuffer(_chipCount * BytesPerChip)
    {
        encodeHeader(_settings.config);
    }

    void initialize() override
    {
        _settings.bus->begin();
    }

    void update(span<const Rgb8Color> colors) override
    {
        // Serialize: reversed chip order, reversed pixel order within chip
        serialize(colors);

        _settings.bus->beginTransaction();
        _settings.bus->transmitBytes(span<const uint8_t>(_byteBuffer.data(), _byteBuffer.size()));
        _settings.bus->endTransaction();

        // Latch guard
        delayMicroseconds(LatchGuardUs);
    }

    bool isReadyToUpdate() const override
    {
        return true;
    }

    bool alwaysUpdate() const override
    {
        return false;
    }

    void updateSettings(const Tlc59711Settings& settings)
    {
        encodeHeader(settings);
    }

private:
    static constexpr size_t PixelsPerChip = 4;
    static constexpr size_t ChannelsPerChip = 12;
    static constexpr size_t DataBytesPerChip = 24;   // 12 ? 2
    static constexpr size_t HeaderBytesPerChip = 4;
    static constexpr size_t BytesPerChip = 28;        // 4 + 24
    static constexpr uint32_t LatchGuardUs = 20;

    SettingsType _settings;
    size_t _pixelCount;
    size_t _chipCount;
    std::vector<uint8_t> _byteBuffer;
    std::array<uint8_t, HeaderBytesPerChip> _header{};

    void encodeHeader(const Tlc59711Settings& config)
    {
        uint8_t bcR = config.bcRed   & 0x7F;
        uint8_t bcG = config.bcGreen & 0x7F;
        uint8_t bcB = config.bcBlue  & 0x7F;

        // Control bits packed into one byte for convenience
        uint8_t ctrl = 0;
        if (config.outtmg) ctrl |= 0x02;
        if (config.extgck) ctrl |= 0x01;
        if (config.tmgrst) ctrl |= 0x80;
        if (config.dsprpt) ctrl |= 0x40;
        if (config.blank)  ctrl |= 0x20;

        // byte[0] = 0b100101_OE  (write command + OUTTMG + EXTGCK)
        _header[0] = static_cast<uint8_t>(0x94 | (ctrl & 0x03));

        // byte[1] = 0bTDB_bbbbb  (TMGRST, DSPRPT, BLANK, BC_Blue[6:2])
        _header[1] = static_cast<uint8_t>((ctrl & 0xE0) | (bcB >> 2));

        // byte[2] = 0bbb_gggggg  (BC_Blue[1:0], BC_Green[6:1])
        _header[2] = static_cast<uint8_t>((bcB << 6) | (bcG >> 1));

        // byte[3] = 0bg_rrrrrrr  (BC_Green[0], BC_Red[6:0])
        _header[3] = static_cast<uint8_t>((bcG << 7) | bcR);
    }

    void serialize(span<const Rgb8Color> colors)
    {
        // Walk chips in reverse order (last chip first on wire)
        size_t bufOffset = 0;

        for (size_t chip = _chipCount; chip > 0; --chip)
        {
            size_t chipStartPixel = (chip - 1) * PixelsPerChip;

            // Per-chip header (same for all chips)
            _byteBuffer[bufOffset++] = _header[0];
            _byteBuffer[bufOffset++] = _header[1];
            _byteBuffer[bufOffset++] = _header[2];
            _byteBuffer[bufOffset++] = _header[3];

            // Channel data: reversed pixel order within chip, BGR per pixel
            for (size_t px = PixelsPerChip; px > 0; --px)
            {
                size_t pixelIdx = chipStartPixel + (px - 1);

                uint16_t b = 0, g = 0, r = 0;
                if (pixelIdx < colors.size())
                {
                    // 8-bit ? 16-bit: replicate byte into both halves
                    b = static_cast<uint16_t>(
                        (colors[pixelIdx]['B'] << 8) |
                         colors[pixelIdx]['B']);
                    g = static_cast<uint16_t>(
                        (colors[pixelIdx]['G'] << 8) |
                         colors[pixelIdx]['G']);
                    r = static_cast<uint16_t>(
                        (colors[pixelIdx]['R'] << 8) |
                         colors[pixelIdx]['R']);
                }

                // BGR order, big-endian 16-bit each
                _byteBuffer[bufOffset++] = static_cast<uint8_t>(b >> 8);
                _byteBuffer[bufOffset++] = static_cast<uint8_t>(b & 0xFF);
                _byteBuffer[bufOffset++] = static_cast<uint8_t>(g >> 8);
                _byteBuffer[bufOffset++] = static_cast<uint8_t>(g & 0xFF);
                _byteBuffer[bufOffset++] = static_cast<uint8_t>(r >> 8);
                _byteBuffer[bufOffset++] = static_cast<uint8_t>(r & 0xFF);
            }
        }
    }
};

} // namespace npb


