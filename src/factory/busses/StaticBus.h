#pragma once

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "factory/busses/OwningBufferContext.h"
#include "core/Compat.h"

namespace lw
{

    template <typename TColor, typename... TArgs>
    class StaticBus : protected OwningBufferContext<TColor>, public PixelBus<TColor>
    {
    public:
        using ColorType = TColor;

        static_assert(sizeof...(TArgs) % 4 == 0,
                      "StaticBus requires inline instance groups of 4: protocol instance, transport instance, shader instance, length");

        static constexpr size_t StrandCount = sizeof...(TArgs) / 4;
        using OwnedTuple = std::tuple<lw::remove_cvref_t<TArgs>...>;
        using StrandArray = std::array<StrandExtent<TColor>, StrandCount>;

    private:
        template <typename TTuple, size_t... TIndices>
        static std::vector<size_t> protocolSizesFromTuple(const TTuple &values,
                                                          std::index_sequence<TIndices...>)
        {
            return std::vector<size_t>{static_cast<size_t>(std::get<TIndices * 4>(values).requiredBufferSizeBytes())...};
        }

    public:
        StaticBus(size_t rootPixelCount,
                  size_t shaderPixelCount,
                  Topology topology,
                  uint8_t *buffer,
                  ssize_t bufferSize,
                  bool owns,
                  TArgs &&...args)
            : OwningBufferContext<TColor>(rootPixelCount,
                                          shaderPixelCount,
                                          protocolSizesFromTuple(std::forward_as_tuple(args...),
                                                                 std::make_index_sequence<StrandCount>{}),
                                          buffer,
                                          bufferSize,
                                          owns)
            , PixelBus<TColor>(this->bufferAccess(),
                               normalizeOwningBusTopology(std::move(topology), rootPixelCount),
                               span<StrandExtent<TColor>>{})
            , _owned(std::forward<TArgs>(args)...)
        {
            initializeStrands(std::make_index_sequence<StrandCount>{});
            this->setStrands(span<StrandExtent<TColor>>{_strands.data(), _strands.size()});
        }

        const StrandArray &strands() const
        {
            return _strands;
        }

        size_t getBufferSize() const
        {
            return this->bufferAccess().totalBytes();
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

    protected:
        template <size_t TStrandIndex>
        size_t initializeOneStrand(size_t offset)
        {
            using ProtocolType = typename std::tuple_element<TStrandIndex * 4 + 0, OwnedTuple>::type;
            using TransportType = typename std::tuple_element<TStrandIndex * 4 + 1, OwnedTuple>::type;
            using ShaderType = typename std::tuple_element<TStrandIndex * 4 + 2, OwnedTuple>::type;
            using LengthType = typename std::tuple_element<TStrandIndex * 4 + 3, OwnedTuple>::type;

            static_assert(std::is_convertible<ProtocolType *, IProtocol<TColor> *>::value,
                          "Protocol instance type must derive from or be convertible to IProtocol<TColor> (passed as inline instance, not pointer)");
            static_assert(std::is_convertible<TransportType *, ITransport *>::value,
                          "Transport instance type must derive from or be convertible to ITransport (passed as inline instance, not pointer)");
            static_assert(std::is_convertible<ShaderType *, IShader<TColor> *>::value,
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

    private:
        OwnedTuple _owned;
        StrandArray _strands;
    };

    template <typename TColor, typename... TArgs>
    auto makeStaticBus(size_t rootPixelCount,
                       size_t shaderPixelCount,
                       Topology topology,
                       TArgs &&...args)
        -> StaticBus<TColor, lw::remove_cvref_t<TArgs>...>
    {
        return StaticBus<TColor, lw::remove_cvref_t<TArgs>...>{rootPixelCount,
                                                                shaderPixelCount,
                                                                std::move(topology),
                                                                nullptr,
                                                                -1,
                                                                true,
                                                                std::forward<TArgs>(args)...};
    }

    template <typename TColor, typename... TArgs>
    auto makeStaticBus(size_t rootPixelCount,
                       size_t shaderPixelCount,
                       Topology topology,
                       uint8_t *buffer,
                       ssize_t bufferSize,
                       bool owns,
                       TArgs &&...args)
        -> StaticBus<TColor, lw::remove_cvref_t<TArgs>...>
    {
        return StaticBus<TColor, lw::remove_cvref_t<TArgs>...>{rootPixelCount,
                                                                shaderPixelCount,
                                                                std::move(topology),
                                                                buffer,
                                                                bufferSize,
                                                                owns,
                                                                std::forward<TArgs>(args)...};
    }

    template <typename TColor, typename... TArgs>
    class UnifiedStaticBus : public StaticBus<TColor, TArgs...>
    {
    public:
        using BaseBus = StaticBus<TColor, TArgs...>;

        UnifiedStaticBus(size_t rootPixelCount,
                         size_t shaderPixelCount,
                         Topology topology,
                         uint8_t *buffer,
                         ssize_t bufferSize,
                         bool owns,
                         TArgs &&...args)
            : BaseBus(rootPixelCount,
                      shaderPixelCount,
                      std::move(topology),
                      buffer,
                      bufferSize,
                      owns,
                      std::forward<TArgs>(args)...)
        {
        }
    };

    template <typename TColor, typename... TArgs>
    auto makeUnifiedStaticBus(size_t rootPixelCount,
                              size_t shaderPixelCount,
                              Topology topology,
                              TArgs &&...args)
        -> UnifiedStaticBus<TColor, lw::remove_cvref_t<TArgs>...>
    {
        return UnifiedStaticBus<TColor, lw::remove_cvref_t<TArgs>...>{rootPixelCount,
                                                                       shaderPixelCount,
                                                                       std::move(topology),
                                                                       nullptr,
                                                                       -1,
                                                                       true,
                                                                       std::forward<TArgs>(args)...};
    }

    template <typename TColor, typename... TArgs>
    auto makeUnifiedStaticBus(size_t rootPixelCount,
                              size_t shaderPixelCount,
                              Topology topology,
                              uint8_t *buffer,
                              ssize_t bufferSize,
                              bool owns,
                              TArgs &&...args)
        -> UnifiedStaticBus<TColor, lw::remove_cvref_t<TArgs>...>
    {
        return UnifiedStaticBus<TColor, lw::remove_cvref_t<TArgs>...>{rootPixelCount,
                                                                       shaderPixelCount,
                                                                       std::move(topology),
                                                                       buffer,
                                                                       bufferSize,
                                                                       owns,
                                                                       std::forward<TArgs>(args)...};
    }

} // namespace lw
