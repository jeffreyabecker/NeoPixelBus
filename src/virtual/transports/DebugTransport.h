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

    struct NilTransportConfig
    {
    };

    class NilTransport : public ITransport
    {
    public:
        using TransportConfigType = NilTransportConfig;
        using TransportCategory = TransportTag;
        explicit NilTransport(NilTransportConfig = {})
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

    template <typename TTransportConfig>
    struct DebugTransportConfigT : TTransportConfig
    {
        ResourceHandle<Print> output = nullptr;
        bool invert = false;
    };

    template <typename TTransport,
              typename TTransportConfig>
        requires(TaggedTransportLike<TTransport, TransportTag> &&
                 std::constructible_from<TTransport, TTransportConfig>)
    class DebugTransportT : public TTransport
    {
    public:
        explicit DebugTransportT(DebugTransportConfigT<TTransportConfig> config)
            : TTransport(static_cast<TTransportConfig>(config)), _output{std::move(config.output)}, _invert{config.invert}
        {
        }

        explicit DebugTransportT(Print &output,
                                 bool invert = false)
            requires std::default_initializable<TTransportConfig>
            : TTransport(TTransportConfig{}), _output{output}, _invert{invert}
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

    template <typename TTransportConfig>
    using DebugOneWireTransportConfigT = OneWireWrapperConfig<DebugTransportConfigT<TTransportConfig>>;

    template <typename TTransport = NilTransport,
              typename TTransportConfig = NilTransportConfig>
        requires(TaggedTransportLike<TTransport, TransportTag> &&
                 std::constructible_from<TTransport, TTransportConfig>)
    class DebugOneWireTransportT : public OneWireWrapper<DebugTransportT<TTransport, TTransportConfig>>
    {
    public:
        using Base = OneWireWrapper<DebugTransportT<TTransport, TTransportConfig>>;

        explicit DebugOneWireTransportT(DebugOneWireTransportConfigT<TTransportConfig> config)
            : Base(static_cast<OneWireWrapperConfig<DebugTransportConfigT<TTransportConfig>>>(config))
        {
        }
    };

    using DebugTransportConfig = DebugTransportConfigT<NilTransportConfig>;
    using DebugTransport = DebugTransportT<NilTransport, NilTransportConfig>;

    using DebugOneWireTransportConfig = DebugOneWireTransportConfigT<NilTransportConfig>;
    using DebugOneWireTransport = DebugOneWireTransportT<NilTransport, NilTransportConfig>;
} // namespace npb
