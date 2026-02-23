#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "../shaders/IShader.h"
#include "../buses/IClockDataTransport.h"
#include "../ResourceHandle.h"

namespace npb
{

static constexpr int8_t PinNotUsed = -1;

struct Tlc5947ProtocolSettings
{
    ResourceHandle<IClockDataTransport> bus;
    int8_t latchPin;
    int8_t oePin = PinNotUsed;
};

template<typename TClockDataTransport>
    requires std::derived_from<TClockDataTransport, IClockDataTransport>
struct Tlc5947ProtocolSettingsOfT : Tlc5947ProtocolSettings
{
    template<typename... BusArgs>
    explicit Tlc5947ProtocolSettingsOfT(int8_t latchPin,
                                      BusArgs&&... busArgs)
        : Tlc5947ProtocolSettings{
            std::make_unique<TClockDataTransport>(std::forward<BusArgs>(busArgs)...),
            latchPin}
    {
    }

    template<typename... BusArgs>
    explicit Tlc5947ProtocolSettingsOfT(int8_t latchPin,
                                      int8_t oePin,
                                      BusArgs&&... busArgs)
        : Tlc5947ProtocolSettings{
            std::make_unique<TClockDataTransport>(std::forward<BusArgs>(busArgs)...),
            latchPin,
            oePin}
    {
    }
};

// TLC5947 emitter.
//
// SPI-like two-wire (clock + data) + GPIO latch pin + optional output-enable pin.
// 24 PWM channels per module (= 8 RGB pixels per module).
// 12-bit per channel on the wire.
//
// No in-band settings — pure channel data with external latch.
//
// Transmission order:
//   - Within each module, channels are sent in REVERSE order
//   - 8-bit input is scaled to 12-bit (value << 4 | value >> 4)
//   - Two 12-bit channels are packed into 3 bytes
//
// Latch sequence:
//   1. OE = HIGH (disable outputs)
//   2. LATCH = LOW
//   3. SPI transmit all modules
//   4. LATCH = HIGH → LOW pulse (rising edge latches)
//   5. OE = LOW (enable outputs)
//
class Tlc5947Protocol : public IProtocol
{
public:
    Tlc5947Protocol(uint16_t pixelCount,
                   ResourceHandle<IShader> shader,
                   Tlc5947ProtocolSettings settings)
        : _settings{std::move(settings)}
        , _shader{std::move(shader)}
        , _pixelCount{pixelCount}
        , _moduleCount{(pixelCount + PixelsPerModule - 1) / PixelsPerModule}
        , _scratchColors(pixelCount)
        , _byteBuffer(_moduleCount * BytesPerModule)
    {
    }

    void initialize() override
    {
        _settings.bus->begin();

        if (_settings.latchPin != PinNotUsed)
        {
            pinMode(_settings.latchPin, OUTPUT);
            digitalWrite(_settings.latchPin, LOW);
        }
        if (_settings.oePin != PinNotUsed)
        {
            pinMode(_settings.oePin, OUTPUT);
            digitalWrite(_settings.oePin, LOW);  // outputs enabled
        }
    }

    void update(std::span<const Color> colors) override
    {
        // Apply shader
        std::span<const Color> source = colors;
        if (nullptr != _shader)
        {
            std::copy(colors.begin(), colors.end(), _scratchColors.begin());
            _shader->apply(_scratchColors);
            source = _scratchColors;
        }

        // Serialize: 12-bit channels, reversed order within each module
        serialize(source);

        // Disable outputs during update
        if (_settings.oePin != PinNotUsed)
        {
            digitalWrite(_settings.oePin, HIGH);
        }

        // Latch low before data
        if (_settings.latchPin != PinNotUsed)
        {
            digitalWrite(_settings.latchPin, LOW);
        }

        _settings.bus->beginTransaction();
        _settings.bus->transmitBytes(_byteBuffer);
        _settings.bus->endTransaction();

        // Pulse latch: rising edge latches data
        if (_settings.latchPin != PinNotUsed)
        {
            digitalWrite(_settings.latchPin, HIGH);
            digitalWrite(_settings.latchPin, LOW);
        }

        // Re-enable outputs
        if (_settings.oePin != PinNotUsed)
        {
            digitalWrite(_settings.oePin, LOW);
        }
    }

    bool isReadyToUpdate() const override
    {
        return true;
    }

    bool alwaysUpdate() const override
    {
        return false;
    }

private:
    static constexpr size_t ChannelsPerModule = 24;
    static constexpr size_t PixelsPerModule = 8;     // 24 channels / 3 RGB
    static constexpr size_t BytesPerModule = 36;     // 24 × 12 bits / 8

    Tlc5947ProtocolSettings _settings;
    ResourceHandle<IShader> _shader;
    size_t _pixelCount;
    size_t _moduleCount;
    std::vector<Color> _scratchColors;
    std::vector<uint8_t> _byteBuffer;

    void serialize(std::span<const Color> colors)
    {
        size_t bufOffset = 0;

        for (size_t mod = 0; mod < _moduleCount; ++mod)
        {
            size_t modStartPixel = mod * PixelsPerModule;

            // Collect 24 channels in forward order, then reverse
            uint16_t channels[ChannelsPerModule]{};

            for (size_t px = 0; px < PixelsPerModule; ++px)
            {
                size_t pixelIdx = modStartPixel + px;
                size_t chBase = px * 3;

                if (pixelIdx < colors.size())
                {
                    // 8-bit → 12-bit: val << 4 | val >> 4
                    uint8_t r = colors[pixelIdx][Color::IdxR];
                    uint8_t g = colors[pixelIdx][Color::IdxG];
                    uint8_t b = colors[pixelIdx][Color::IdxB];

                    channels[chBase + 0] = static_cast<uint16_t>((r << 4) | (r >> 4));
                    channels[chBase + 1] = static_cast<uint16_t>((g << 4) | (g >> 4));
                    channels[chBase + 2] = static_cast<uint16_t>((b << 4) | (b >> 4));
                }
            }

            // Pack 12-bit channels in REVERSE order, 2 channels per 3 bytes
            for (size_t i = ChannelsPerModule; i >= 2; i -= 2)
            {
                uint16_t ch1 = channels[i - 2];  // earlier channel
                uint16_t ch2 = channels[i - 1];  // later channel

                _byteBuffer[bufOffset++] = static_cast<uint8_t>(ch2 >> 4);
                _byteBuffer[bufOffset++] = static_cast<uint8_t>(
                    ((ch2 & 0x0F) << 4) | (ch1 >> 8));
                _byteBuffer[bufOffset++] = static_cast<uint8_t>(ch1 & 0xFF);
            }
        }
    }
};

} // namespace npb
