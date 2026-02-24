#pragma once

#include <cstdint>
#include <cstddef>
#include <concepts>
#include <span>
#include <utility>

#include <Print.h>

#include "ITransport.h"
#include "../ResourceHandle.h"
#include "SelfClockingWrapperTransport.h"
namespace npb
{

    struct NilClockDataTransportConfig
    {
    };

    class NilClockDataTransport : public ITransport
    {
    public:
        using TransportCategory = ClockDataTransportTag;
        explicit NilClockDataTransport(NilClockDataTransportConfig = {})
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
    struct DebugClockDataTransportConfigT : TTransportConfig
    {
        ResourceHandle<Print> output = nullptr;
        bool invert = false;
    };

    template <typename TClockDataTransport,
              typename TTransportConfig>
        requires(TaggedTransportLike<TClockDataTransport, ClockDataTransportTag> &&
                 std::constructible_from<TClockDataTransport, TTransportConfig>)
    class DebugClockDataTransportT : public TClockDataTransport
    {
    public:
        explicit DebugClockDataTransportT(DebugClockDataTransportConfigT<TTransportConfig> config)
            : TClockDataTransport(static_cast<TTransportConfig>(config)), _output{std::move(config.output)}, _invert{config.invert}
        {
        }

        explicit DebugClockDataTransportT(Print &output,
                                          bool invert = false)
            requires std::default_initializable<TTransportConfig>
            : TClockDataTransport(TTransportConfig{}), _output{output}, _invert{invert}
        {
        }

        void begin() override
        {
            if (_output != nullptr)
            {
                _output->println("[BUS] begin");
            }

            TClockDataTransport::begin();
        }

        void beginTransaction() override
        {
            if (_output != nullptr)
            {
                _output->println("[BUS] beginTransaction");
            }

            TClockDataTransport::beginTransaction();
        }

        void endTransaction() override
        {
            if (_output != nullptr)
            {
                _output->println("[BUS] endTransaction");
            }

            TClockDataTransport::endTransaction();
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            static constexpr char Hex[] = "0123456789ABCDEF";
            if (_output != nullptr)
            {
                _output->print("[BUS] bytes(");
                _output->print(data.size());
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

            TClockDataTransport::transmitBytes(data);
        }

    private:
        ResourceHandle<Print> _output = nullptr;
        bool _invert = false;
    };

    template <typename TTransportConfig>
    using DebugSelfClockingTransportConfigT = SelfClockingWrapperTransportConfig<DebugClockDataTransportConfigT<TTransportConfig>>;

    template <typename TClockDataTransport = NilClockDataTransport,
              typename TTransportConfig = NilClockDataTransportConfig>
        requires(TaggedTransportLike<TClockDataTransport, ClockDataTransportTag> &&
                 std::constructible_from<TClockDataTransport, TTransportConfig>)
    class DebugSelfClockingTransportT : public SelfClockingWrapperTransport<DebugClockDataTransportT<TClockDataTransport, TTransportConfig>>
    {
    public:
        using Base = SelfClockingWrapperTransport<DebugClockDataTransportT<TClockDataTransport, TTransportConfig>>;

        explicit DebugSelfClockingTransportT(DebugSelfClockingTransportConfigT<TTransportConfig> config)
            : Base(static_cast<SelfClockingWrapperTransportConfig<DebugClockDataTransportConfigT<TTransportConfig>>>(config))
        {
        }
    };

    using DebugClockDataTransportConfig = DebugClockDataTransportConfigT<NilClockDataTransportConfig>;
    using DebugClockDataTransport = DebugClockDataTransportT<NilClockDataTransport, NilClockDataTransportConfig>;

    using DebugSelfClockingTransportConfig = DebugSelfClockingTransportConfigT<NilClockDataTransportConfig>;
    using DebugSelfClockingTransport = DebugSelfClockingTransportT<NilClockDataTransport, NilClockDataTransportConfig>;
} // namespace npb
