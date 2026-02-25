#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <type_traits>
#include <utility>
#include <cstring>

#include <Arduino.h>

#include "IProtocol.h"
#include "core/ResourceHandle.h"
#include "core/Writable.h"

namespace npb
{

    template <typename TColor,
              Writable TWritable = Print>
    struct DebugProtocolSettingsT
    {
        ResourceHandle<ITransport> bus = nullptr;
        TWritable *output = nullptr;
        bool invert = false;
        ResourceHandle<IProtocol<TColor>> protocol = nullptr;
    };

    template <typename TColor,
              Writable TWritable = Print>
    class DebugProtocol : public IProtocol<TColor>
    {
    public:
        using SettingsType = DebugProtocolSettingsT<TColor, TWritable>;
        using TransportCategory = AnyTransportTag;

        DebugProtocol(uint16_t pixelCount,
                      SettingsType settings)
            : _settings{std::move(settings)}
            , _pixelCount{pixelCount}
        {
        }

        DebugProtocol(uint16_t pixelCount,
                      TWritable &output,
                      bool invert = false)
            : DebugProtocol(pixelCount,
                            SettingsType{.output = &output, .invert = invert})
        {
        }

        DebugProtocol(uint16_t pixelCount,
                      TWritable &output,
                      ResourceHandle<IProtocol<TColor>> protocol,
                      bool invert = false)
            : DebugProtocol(pixelCount,
                            SettingsType{.output = &output, .invert = invert, .protocol = std::move(protocol)})
        {
        }

        void initialize() override
        {
            if (_settings.output != nullptr)
            {
                writeText("[PROTOCOL] begin pixelCount=");

                char countBuffer[8]{};
                const size_t countLength = formatUnsignedDecimal(countBuffer, sizeof(countBuffer), static_cast<unsigned long>(_pixelCount));
                if (countLength > 0)
                {
                    writeBytes(reinterpret_cast<const uint8_t *>(countBuffer), countLength);
                }

                writeNewline();
            }

            if (_settings.protocol != nullptr)
            {
                _settings.protocol->initialize();
            }
        }

        void update(std::span<const TColor> colors) override
        {
            if (_settings.output == nullptr)
            {
                return;
            }

            static constexpr char HexDigits[] = "0123456789ABCDEF";

            writeText("[PROTOCOL] colors(");

            char countBuffer[3 * sizeof(unsigned long)]{};
            const size_t countLength = formatUnsignedDecimal(countBuffer, sizeof(countBuffer), static_cast<unsigned long>(colors.size()));
            if (countLength > 0)
            {
                writeBytes(reinterpret_cast<const uint8_t *>(countBuffer), countLength);
            }

            writeText("): ");

            for (size_t colorIndex = 0; colorIndex < colors.size(); ++colorIndex)
            {
                if (colorIndex > 0)
                {
                    static constexpr char Space[] = " ";
                    writeBytes(reinterpret_cast<const uint8_t *>(Space), sizeof(Space) - 1);
                }

                const auto &color = colors[colorIndex];
                for (size_t channelIndex = 0; channelIndex < TColor::ChannelCount; ++channelIndex)
                {
                    using ComponentType = typename TColor::ComponentType;
                    using UnsignedComponentType = std::make_unsigned_t<ComponentType>;

                    UnsignedComponentType value = static_cast<UnsignedComponentType>(color[channelIndex]);
                    if (_settings.invert)
                    {
                        value = static_cast<UnsignedComponentType>(~value);
                    }

                    int shift = static_cast<int>(sizeof(UnsignedComponentType) * 8) - 4;
                    while (shift >= 0)
                    {
                        const uint8_t nibble = static_cast<uint8_t>((value >> shift) & 0x0F);
                        char hexCharBuffer[1] = {
                            HexDigits[nibble]};
                        writeBytes(reinterpret_cast<const uint8_t *>(hexCharBuffer), sizeof(hexCharBuffer));
                        shift -= 4;
                    }
                }
            }

            writeNewline();

            if (_settings.protocol != nullptr)
            {
                _settings.protocol->update(colors);
            }
        }

        bool isReadyToUpdate() const override
        {
            if (_settings.protocol != nullptr)
            {
                return _settings.protocol->isReadyToUpdate();
            }

            return true;
        }

        bool alwaysUpdate() const override
        {
            if (_settings.protocol != nullptr)
            {
                return _settings.protocol->alwaysUpdate();
            }

            return false;
        }

    private:
        void writeBytes(const uint8_t *data, size_t length)
        {
            if (_settings.output == nullptr || data == nullptr || length == 0)
            {
                return;
            }

            _settings.output->write(data, length);
        }

        void writeText(const char *text)
        {
            if (text == nullptr)
            {
                return;
            }

            writeBytes(reinterpret_cast<const uint8_t *>(text), std::strlen(text));
        }

        void writeNewline()
        {
            static constexpr char Newline[] = "\r\n";
            writeBytes(reinterpret_cast<const uint8_t *>(Newline), sizeof(Newline) - 1);
        }

        static size_t formatUnsignedDecimal(char *buffer, size_t capacity, unsigned long value)
        {
            if (buffer == nullptr || capacity == 0)
            {
                return 0;
            }

            size_t index = 0;
            do
            {
                if (index >= capacity)
                {
                    return 0;
                }

                const unsigned long digit = value % 10UL;
                buffer[index++] = static_cast<char>('0' + digit);
                value /= 10UL;
            } while (value > 0UL);

            for (size_t left = 0, right = index - 1; left < right; ++left, --right)
            {
                const char tmp = buffer[left];
                buffer[left] = buffer[right];
                buffer[right] = tmp;
            }

            return index;
        }

        SettingsType _settings;
        uint16_t _pixelCount{0};
    };

} // namespace npb


