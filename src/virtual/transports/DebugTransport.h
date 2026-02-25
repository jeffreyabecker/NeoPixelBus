#pragma once

#include <cstdint>
#include <cstddef>
#include <concepts>
#include <span>
#include <utility>

#include <Arduino.h>

#include "ITransport.h"
#include "../ResourceHandle.h"
#include "OneWireWrapper.h"
namespace npb
{

    struct NilTransportSettings
    {
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

    template <typename TTransportSettings>
    struct DebugTransportSettingsT : TTransportSettings
    {
        ResourceHandle<Print> output = nullptr;
        bool invert = false;
    };

    template <typename TTransport,
              typename TTransportSettings>
        requires(TaggedTransportLike<TTransport, TransportTag> &&
                 std::constructible_from<TTransport, TTransportSettings>)
    class DebugTransportT : public TTransport
    {
    public:
        using TransportSettingsType = DebugTransportSettingsT<TTransportSettings>;
        using TransportCategory = typename TTransport::TransportCategory;

        explicit DebugTransportT(DebugTransportSettingsT<TTransportSettings> config)
            : TTransport(static_cast<TTransportSettings>(config)), _output{std::move(config.output)}, _invert{config.invert}
        {
        }

        explicit DebugTransportT(TTransportSettings config)
            : DebugTransportT(DebugTransportSettingsT<TTransportSettings>{std::move(config)})
        {
        }

        explicit DebugTransportT(Print &output,
                                 bool invert = false)
            requires std::default_initializable<TTransportSettings>
            : TTransport(TTransportSettings{}), _output{output}, _invert{invert}
        {
        }

        void begin() override
        {
            if (_output != nullptr)
            {
                _output->println("[BUS] begin");
            }

            TTransport::begin();
        }

        void beginTransaction() override
        {
            if (_output != nullptr)
            {
                _output->println("[BUS] beginTransaction");
            }

            TTransport::beginTransaction();
        }

        void endTransaction() override
        {
            if (_output != nullptr)
            {
                _output->println("[BUS] endTransaction");
            }

            TTransport::endTransaction();
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            static constexpr char Hex[] = "0123456789ABCDEF";
            if (_output != nullptr)
            {
                _output->print("[BUS] bytes(");
                _output->print(static_cast<unsigned long>(data.size()));
                _output->print("): ");
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (i > 0)
                    {
                        _output->print(' ');
                    }
                    uint8_t byte = data[i];
                    if (_invert)
                    {
                        byte = ~byte;
                    }
                    _output->print(Hex[byte >> 4]);
                    _output->print(Hex[byte & 0x0F]);
                }
                _output->println();
            }

            TTransport::transmitBytes(data);
        }

    private:
        ResourceHandle<Print> _output = nullptr;
        bool _invert = false;
    };

    template <typename TTransportSettings>
    using DebugOneWireTransportSettingsT = OneWireWrapperSettings<DebugTransportSettingsT<TTransportSettings>>;

    template <typename TTransport = NilTransport,
              typename TTransportSettings = NilTransportSettings>
        requires(TaggedTransportLike<TTransport, TransportTag> &&
                 std::constructible_from<TTransport, TTransportSettings>)
    class DebugOneWireTransportT : public ITransport
    {
    public:
        using WrappedTransport = DebugTransportT<TTransport, TTransportSettings>;
        using WrappedOneWireTransport = OneWireWrapper<WrappedTransport>;
        using TransportSettingsType = DebugOneWireTransportSettingsT<TTransportSettings>;
        using TransportCategory = OneWireTransportTag;

        explicit DebugOneWireTransportT(DebugOneWireTransportSettingsT<TTransportSettings> config)
            : _transport(static_cast<OneWireWrapperSettings<DebugTransportSettingsT<TTransportSettings>> &&>(config))
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

    using DebugTransportSettings = DebugTransportSettingsT<NilTransportSettings>;
    using DebugTransport = DebugTransportT<NilTransport, NilTransportSettings>;

    using DebugOneWireTransportSettings = DebugOneWireTransportSettingsT<NilTransportSettings>;
    using DebugOneWireTransport = DebugOneWireTransportT<NilTransport, NilTransportSettings>;
} // namespace npb
