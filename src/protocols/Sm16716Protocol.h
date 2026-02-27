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

namespace npb
{

struct Sm16716ProtocolSettings
{
    ITransport *bus = nullptr;
    const char* channelOrder = ChannelOrder::RGB;
};

// SM16716 protocol.
//
// Bit-level protocol ? NOT byte-aligned ? pre-packed into a byte buffer.
//
// Bit stream layout:
//   Start frame: 50 zero-bits
//   Per pixel:   1 HIGH bit (separator) + 3 ? 8-bit channel data = 25 bits
//
// Total bits = 50 + pixelCount ? 25
// Pre-packed into ceil(totalBits / 8) bytes, MSB-first.
//
// No end frame. Entire stream transmitted as bytes via transmitBytes().
//
class Sm16716Protocol : public IProtocol<Rgb8Color>
{
public:
    using SettingsType = Sm16716ProtocolSettings;
    using TransportCategory = TransportTag;

    Sm16716Protocol(uint16_t pixelCount,
                   SettingsType settings)
        : IProtocol<Rgb8Color>(pixelCount)
        , _settings{std::move(settings)}
        , _byteBuffer((StartFrameBits + pixelCount * BitsPerPixel + 7) / 8)
    {
    }

    void initialize() override
    {
        _settings.bus->begin();
    }

    void update(span<const Rgb8Color> colors) override
    {
        // Pack entire bit stream into byte buffer
        serialize(colors);

        _settings.bus->beginTransaction();
        _settings.bus->transmitBytes(span<uint8_t>(_byteBuffer.data(), _byteBuffer.size()));
        _settings.bus->endTransaction();
    }

    bool isReadyToUpdate() const override
    {
        return _settings.bus->isReadyToUpdate();
    }

    bool alwaysUpdate() const override
    {
        return false;
    }

private:
    static constexpr size_t StartFrameBits = 50;
    static constexpr size_t ChannelCount = ChannelOrder::LengthRGB;
    static constexpr size_t BitsPerPixel = 1 + (ChannelCount * 8);

    SettingsType _settings;
    std::vector<uint8_t> _byteBuffer;

    // Set a single bit in the buffer (MSB-first ordering)
    void setBit(size_t bitPos)
    {
        _byteBuffer[bitPos / 8] |= (0x80 >> (bitPos % 8));
    }

    // Pack an 8-bit value at an arbitrary bit position (MSB-first)
    void packByte(uint8_t val, size_t& bitPos)
    {
        size_t byteIdx = bitPos / 8;
        uint8_t shift = bitPos % 8;

        // Value may span two output bytes
        _byteBuffer[byteIdx] |= (val >> shift);
        if (shift > 0 && byteIdx + 1 < _byteBuffer.size())
        {
            _byteBuffer[byteIdx + 1] |= (val << (8 - shift));
        }
        bitPos += 8;
    }

    void serialize(span<const Rgb8Color> colors)
    {
        // Clear buffer ? start frame is 50 zero-bits, so zeros are default
        std::fill(_byteBuffer.begin(), _byteBuffer.end(), 0);

        size_t bitPos = StartFrameBits;  // skip 50 zero-bits

        const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(this->pixelCount()));
        for (size_t index = 0; index < pixelLimit; ++index)
        {
            const auto& color = colors[index];
            // 1-bit HIGH separator
            setBit(bitPos++);

            // Channel bytes
            for (size_t channel = 0; channel < ChannelCount; ++channel)
            {
                packByte(color[_settings.channelOrder[channel]], bitPos);
            }
        }
    }
};

} // namespace npb


