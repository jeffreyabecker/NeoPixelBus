#pragma once

#include <cstdint>
#include <cstddef>
#include <concepts>
#include <span>
#include <utility>
#include <cstring>

#include <Arduino.h>

#include "ITransport.h"
#include "OneWireWrapper.h"
#include "../Writable.h"
namespace npb
{

    struct NilTransportSettings
    {
        bool invert = false;
    };

    class NilTransport : public ITransport
    {
    public:
        using TransportSettingsType = NilTransportSettings;
        using TransportCategory = TransportTag;
        explicit NilTransport(NilTransportSettings = {})
        {
        }

        void begin() override
        {
        }

        void beginTransaction() override
        {
        }

        void endTransaction() override
        {
        }

        void transmitBytes(std::span<const uint8_t>) override
        {
        }
    };

    template <typename TTransportSettings,
              Writable TWritable = Print>
    struct DebugTransportSettingsT : TTransportSettings
    {
        TWritable *output = nullptr;
        bool invert = false;
    };

    template <typename TTransport,
              typename TTransportSettings,
              Writable TWritable = Print>
        requires(TaggedTransportLike<TTransport, TransportTag> &&
                 std::constructible_from<TTransport, TTransportSettings>)
    class DebugTransportT : public TTransport
    {
    public:
        using TransportSettingsType = DebugTransportSettingsT<TTransportSettings, TWritable>;
        using TransportCategory = typename TTransport::TransportCategory;

        explicit DebugTransportT(DebugTransportSettingsT<TTransportSettings, TWritable> config)
            : TTransport(static_cast<TTransportSettings>(config)), _output{config.output}, _invert{config.invert}
        {
        }

        explicit DebugTransportT(TTransportSettings config)
            : DebugTransportT(DebugTransportSettingsT<TTransportSettings, TWritable>{std::move(config)})
        {
        }

        explicit DebugTransportT(TWritable &output,
                                 bool invert = false)
            requires std::default_initializable<TTransportSettings>
            : TTransport(TTransportSettings{}), _output{&output}, _invert{invert}
        {
        }

        void begin() override
        {
            if (_output != nullptr)
            {
                writeLine("[BUS] begin");
            }

            TTransport::begin();
        }

        void beginTransaction() override
        {
            if (_output != nullptr)
            {
                writeLine("[BUS] beginTransaction");
            }

            TTransport::beginTransaction();
        }

        void endTransaction() override
        {
            if (_output != nullptr)
            {
                writeLine("[BUS] endTransaction");
            }

            TTransport::endTransaction();
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            static constexpr char Hex[] = "0123456789ABCDEF";
            if (_output != nullptr)
            {
                writeText("[BUS] bytes(");

                char countBuffer[3 * sizeof(unsigned long)]{};
                const size_t countLength = formatUnsignedDecimal(countBuffer, sizeof(countBuffer), static_cast<unsigned long>(data.size()));
                if (countLength > 0)
                {
                    writeBytes(reinterpret_cast<const uint8_t *>(countBuffer), countLength);
                }

                writeText("): ");
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (i > 0)
                    {
                        static constexpr char Space[] = " ";
                        writeBytes(reinterpret_cast<const uint8_t *>(Space), sizeof(Space) - 1);
                    }
                    uint8_t byte = data[i];
                    if (_invert)
                    {
                        byte = ~byte;
                    }

                    char byteBuffer[2] = {
                        Hex[byte >> 4],
                        Hex[byte & 0x0F]};
                    writeBytes(reinterpret_cast<const uint8_t *>(byteBuffer), sizeof(byteBuffer));
                }

                writeNewline();
            }

            TTransport::transmitBytes(data);
        }

    private:
        void writeBytes(const uint8_t *data, size_t length)
        {
            if (_output == nullptr || data == nullptr || length == 0)
            {
                return;
            }

            _output->write(data, length);
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

        TWritable *_output = nullptr;
        bool _invert = false;
    };

    template <typename TTransportSettings,
              Writable TWritable = Print>
    using DebugOneWireTransportSettingsT = OneWireWrapperSettings<DebugTransportSettingsT<TTransportSettings, TWritable>>;

    template <typename TTransport = NilTransport,
              typename TTransportSettings = NilTransportSettings,
              Writable TWritable = Print>
        requires(TaggedTransportLike<TTransport, TransportTag> &&
                 std::constructible_from<TTransport, TTransportSettings>)
    class DebugOneWireTransportT : public ITransport
    {
    public:
        using WrappedTransport = DebugTransportT<TTransport, TTransportSettings, TWritable>;
        using WrappedOneWireTransport = OneWireWrapper<WrappedTransport>;
        using TransportSettingsType = DebugOneWireTransportSettingsT<TTransportSettings, TWritable>;
        using TransportCategory = OneWireTransportTag;

        explicit DebugOneWireTransportT(DebugOneWireTransportSettingsT<TTransportSettings, TWritable> config)
            : _transport(static_cast<OneWireWrapperSettings<DebugTransportSettingsT<TTransportSettings, TWritable>> &&>(config))
        {
        }

        void begin() override
        {
            _transport.begin();
        }

        void beginTransaction() override
        {
            static_cast<WrappedTransport &>(_transport).beginTransaction();
        }

        void endTransaction() override
        {
            static_cast<WrappedTransport &>(_transport).endTransaction();
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            _transport.transmitBytes(data);
        }

        bool isReadyToUpdate() const override
        {
            return _transport.isReadyToUpdate();
        }

    private:
        WrappedOneWireTransport _transport;
    };

    using DebugTransportSettings = DebugTransportSettingsT<NilTransportSettings, Print>;
    using DebugTransport = DebugTransportT<NilTransport, NilTransportSettings, Print>;

    using DebugOneWireTransportSettings = DebugOneWireTransportSettingsT<NilTransportSettings, Print>;
    using DebugOneWireTransport = DebugOneWireTransportT<NilTransport, NilTransportSettings, Print>;
} // namespace npb
