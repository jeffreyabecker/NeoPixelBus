#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <utility>

#include <Arduino.h>

#include "ITransport.h"
#include "core/Writable.h"

namespace npb
{

    template <typename TWritable = Print,
              typename = std::enable_if_t<Writable<TWritable>>>
    struct PrintTransportSettingsT
        : TransportSettingsBase
    {
        TWritable *output = nullptr;
        bool asciiOutput = false;
        bool debugOutput = false;
    };

    template <typename TWritable = Print,
              typename = std::enable_if_t<Writable<TWritable>>>
    class PrintTransportT : public ITransport
    {
    public:
        using TransportSettingsType = PrintTransportSettingsT<TWritable>;
        using TransportCategory = AnyTransportTag;
        explicit PrintTransportT(PrintTransportSettingsT<TWritable> config)
            : _config{std::move(config)}
        {
        }

        explicit PrintTransportT(TWritable &output)
            : _config{.output = &output}
        {
        }

        void begin() override
        {
            if (_config.debugOutput)
            {
                writeLine("[BUS] begin");
            }
        }

        void beginTransaction() override
        {
            if (_config.debugOutput)
            {
                writeLine("[BUS] beginTransaction");
            }
        }

        void transmitBytes(span<uint8_t> data) override
        {
            if (_config.output == nullptr)
            {
                return;
            }

            if (_config.debugOutput)
            {
                writeText("[BUS] bytes(");

                char countBuffer[3 * sizeof(unsigned long)]{};
                const size_t countLength = formatUnsignedDecimal(
                    countBuffer,
                    sizeof(countBuffer),
                    static_cast<unsigned long>(data.size()));
                if (countLength > 0)
                {
                    writeBytes(reinterpret_cast<const uint8_t *>(countBuffer), countLength);
                }

                writeLine(")");
            }

            if (!_config.asciiOutput)
            {
                writeBytes(data.data(), data.size());
                return;
            }

            static constexpr char Hex[] = "0123456789ABCDEF";
            for (size_t index = 0; index < data.size(); ++index)
            {
                const uint8_t byte = data[index];
                char byteBuffer[2] = {
                    Hex[byte >> 4],
                    Hex[byte & 0x0F]};
                writeBytes(reinterpret_cast<const uint8_t *>(byteBuffer), sizeof(byteBuffer));
            }
        }

        void endTransaction() override
        {
            if (_config.debugOutput)
            {
                writeLine("[BUS] endTransaction");
            }
        }

    private:
        void writeBytes(const uint8_t *data, size_t length)
        {
            if (_config.output == nullptr || data == nullptr || length == 0)
            {
                return;
            }

            _config.output->write(data, length);
        }

        void writeText(const char *text)
        {
            if (text == nullptr)
            {
                return;
            }

            writeBytes(reinterpret_cast<const uint8_t *>(text), std::strlen(text));
        }

        void writeLine(const char *text)
        {
            writeText(text);
            writeNewline();
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

        PrintTransportSettingsT<TWritable> _config;
    };

    using PrintTransportSettings = PrintTransportSettingsT<Print>;
    using PrintTransport = PrintTransportT<Print>;

} // namespace npb


