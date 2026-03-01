#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "buses/PixelBus.h"
#include "core/Compat.h"

namespace lw
{

    inline Topology normalizeOwningBusTopology(Topology topology, size_t rootLength)
    {
        if (topology.empty())
        {
            return Topology::linear(rootLength);
        }

        return topology;
    }

    template <typename TColor, typename... TArgs>
    class StaticOwningBus : public PixelBus<TColor>
    {
    public:
        using ColorType = TColor;

        static_assert(sizeof...(TArgs) % 4 == 0,
                      "StaticOwningBus requires inline instance groups of 4: protocol instance, transport instance, shader instance, length");

        static constexpr size_t StrandCount = sizeof...(TArgs) / 4;
        using OwnedTuple = std::tuple<lw::remove_cvref_t<TArgs>...>;
        using StrandArray = std::array<StrandExtent<TColor>, StrandCount>;

        StaticOwningBus(BufferHolder<TColor> rootBuffer,
                        BufferHolder<TColor> shaderBuffer,
                        BufferHolder<uint8_t> protocolBuffer,
                        Topology topology,
                        TArgs &&...args)
            : PixelBus<TColor>(std::move(rootBuffer),
                               std::move(shaderBuffer),
                               normalizeOwningBusTopology(std::move(topology), rootBuffer.size))
            , _owned(std::forward<TArgs>(args)...)
            , _protocolBuffer(std::move(protocolBuffer))
        {
            initializeStrands(std::make_index_sequence<StrandCount>{});
            bindProtocolBuffers();
            this->setStrands(span<StrandExtent<TColor>>{_strands.data(), _strands.size()});
        }

        const StrandArray& strands() const
        {
            return _strands;
        }

        auto &protocol()
        {
            static_assert(StrandCount >= 1,
                          "protocol() requires at least one strand");
            return std::get<0>(_owned);
        }

        const auto &protocol() const
        {
            static_assert(StrandCount >= 1,
                          "protocol() requires at least one strand");
            return std::get<0>(_owned);
        }

        auto &transport()
        {
            static_assert(StrandCount >= 1,
                          "transport() requires at least one strand");
            return std::get<1>(_owned);
        }

        const auto &transport() const
        {
            static_assert(StrandCount >= 1,
                          "transport() requires at least one strand");
            return std::get<1>(_owned);
        }

    private:
        void bindProtocolBuffers()
        {
            size_t totalBufferBytes = 0;
            for (const auto &strand : _strands)
            {
                if (strand.protocol == nullptr)
                {
                    continue;
                }

                strand.protocol->bindTransport(strand.transport);

                totalBufferBytes += strand.protocol->requiredBufferSizeBytes();
            }

            if (_protocolBuffer.size < totalBufferBytes)
            {
                if (_protocolBuffer.owns)
                {
                    _protocolBuffer = BufferHolder<uint8_t>{totalBufferBytes, nullptr, true};
                }
            }

            auto protocolArena = _protocolBuffer.getSpan();
            if (protocolArena.size() < totalBufferBytes)
            {
                assert(false && "StaticOwningBus protocol buffer holder is too small and not resizable");
                return;
            }

            size_t runningOffset = 0;
            for (const auto &strand : _strands)
            {
                if (strand.protocol == nullptr)
                {
                    continue;
                }

                const size_t protocolBytes = strand.protocol->requiredBufferSizeBytes();
                if (protocolBytes == 0)
                {
                    strand.protocol->setBuffer(span<uint8_t>{});
                    continue;
                }

                strand.protocol->setBuffer(span<uint8_t>{protocolArena.data() + runningOffset,
                                                         protocolBytes});
                runningOffset += protocolBytes;
            }
        }

        template <size_t TStrandIndex>
        size_t initializeOneStrand(size_t offset)
        {
            using ProtocolType = typename std::tuple_element<TStrandIndex * 4 + 0, OwnedTuple>::type;
            using TransportType = typename std::tuple_element<TStrandIndex * 4 + 1, OwnedTuple>::type;
            using ShaderType = typename std::tuple_element<TStrandIndex * 4 + 2, OwnedTuple>::type;
            using LengthType = typename std::tuple_element<TStrandIndex * 4 + 3, OwnedTuple>::type;

            static_assert(std::is_convertible<ProtocolType*, IProtocol<TColor>*>::value,
                          "Protocol instance type must derive from or be convertible to IProtocol<TColor> (passed as inline instance, not pointer)");
            static_assert(std::is_convertible<TransportType*, ITransport*>::value,
                          "Transport instance type must derive from or be convertible to ITransport (passed as inline instance, not pointer)");
            static_assert(std::is_convertible<ShaderType*, IShader<TColor>*>::value,
                          "Shader instance type must derive from or be convertible to IShader<TColor> (passed as inline instance, not pointer)");
            static_assert(std::is_integral<LengthType>::value,
                          "Strand length must be an integral type");

            size_t length = static_cast<size_t>(std::get<TStrandIndex * 4 + 3>(_owned));

            _strands[TStrandIndex] = StrandExtent<TColor>{
                &std::get<TStrandIndex * 4 + 0>(_owned),
                &std::get<TStrandIndex * 4 + 1>(_owned),
                &std::get<TStrandIndex * 4 + 2>(_owned),
                offset,
                length};

            return length;
        }

        template <size_t... TIndices>
        void initializeStrands(std::index_sequence<TIndices...>)
        {
            size_t runningOffset = 0;
            ((runningOffset += initializeOneStrand<TIndices>(runningOffset)), ...);
        }

        OwnedTuple _owned;
        StrandArray _strands;
        BufferHolder<uint8_t> _protocolBuffer;
    };

    template <typename TColor,
              typename... TArgs>
    auto makeStaticOwningBus(BufferHolder<TColor> rootBuffer,
                             BufferHolder<TColor> shaderBuffer,
                             BufferHolder<uint8_t> protocolBuffer,
                             Topology topology,
                             TArgs &&...args)
        -> StaticOwningBus<TColor, lw::remove_cvref_t<TArgs>...>
    {
        return StaticOwningBus<TColor, lw::remove_cvref_t<TArgs>...>{std::move(rootBuffer),
                                                                      std::move(shaderBuffer),
                                                                      std::move(protocolBuffer),
                                                                      std::move(topology),
                                                                      std::forward<TArgs>(args)...};
    }

    template <typename TColor>
    class DynamicOwningBus : public PixelBus<TColor>
    {
    public:
        DynamicOwningBus(BufferHolder<TColor> rootBuffer,
                         BufferHolder<TColor> shaderBuffer,
                         BufferHolder<uint8_t> protocolBuffer,
                         Topology topology,
                         std::vector<StrandExtent<TColor>> strands)
            : PixelBus<TColor>(std::move(rootBuffer),
                               std::move(shaderBuffer),
                               normalizeOwningBusTopology(std::move(topology), rootBuffer.size))
            , _strands(std::move(strands))
            , _protocolBuffer(std::move(protocolBuffer))
        {
            initializeStrands();
            bindProtocolBuffers();
            this->setStrands(span<StrandExtent<TColor>>{_strands.data(), _strands.size()});
        }

        ~DynamicOwningBus() override
        {
            for (auto& strand : _strands)
            {
                delete strand.protocol;
                delete strand.transport;
                delete strand.shader;
                strand.protocol = nullptr;
                strand.transport = nullptr;
                strand.shader = nullptr;
            }
        }

        const std::vector<StrandExtent<TColor>>& strands() const
        {
            return _strands;
        }

    private:
        void bindProtocolBuffers()
        {
            size_t totalBufferBytes = 0;
            for (const auto &strand : _strands)
            {
                if (strand.protocol == nullptr)
                {
                    continue;
                }

                strand.protocol->bindTransport(strand.transport);

                totalBufferBytes += strand.protocol->requiredBufferSizeBytes();
            }

            if (_protocolBuffer.size < totalBufferBytes)
            {
                if (_protocolBuffer.owns)
                {
                    _protocolBuffer = BufferHolder<uint8_t>{totalBufferBytes, nullptr, true};
                }
            }

            auto protocolArena = _protocolBuffer.getSpan();
            if (protocolArena.size() < totalBufferBytes)
            {
                assert(false && "DynamicOwningBus protocol buffer holder is too small and not resizable");
                return;
            }

            size_t runningOffset = 0;
            for (const auto &strand : _strands)
            {
                if (strand.protocol == nullptr)
                {
                    continue;
                }

                const size_t protocolBytes = strand.protocol->requiredBufferSizeBytes();
                if (protocolBytes == 0)
                {
                    strand.protocol->setBuffer(span<uint8_t>{});
                    continue;
                }

                strand.protocol->setBuffer(span<uint8_t>{protocolArena.data() + runningOffset,
                                                         protocolBytes});
                runningOffset += protocolBytes;
            }
        }

        void initializeStrands()
        {
            size_t runningOffset = 0;
            for (auto& strand : _strands)
            {
                strand.offset = runningOffset;
                runningOffset += strand.length;
            }
        }

        std::vector<StrandExtent<TColor>> _strands;
        BufferHolder<uint8_t> _protocolBuffer;
    };

} // namespace lw
