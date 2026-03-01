#pragma once

#include <array>
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

    template <typename TColor, typename... TArgs>
    class StaticOwningBus : public PixelBus<TColor>
    {
    public:
        static_assert(sizeof...(TArgs) % 5 == 0,
                      "StaticOwningBus requires strand argument groups of 5: protocol, transport, shader, offset, length");

        static constexpr size_t StrandCount = sizeof...(TArgs) / 5;
        using OwnedTuple = std::tuple<lw::remove_cvref_t<TArgs>...>;
        using StrandArray = std::array<StrandExtent<TColor>, StrandCount>;

        StaticOwningBus(BufferHolder<TColor> rootBuffer,
                        Topology topology,
                        TArgs &&...args)
            : PixelBus<TColor>(std::move(rootBuffer), std::move(topology))
            , _owned(std::forward<TArgs>(args)...)
        {
            initializeStrands(std::make_index_sequence<StrandCount>{});
            this->setStrands(span<StrandExtent<TColor>>{_strands.data(), _strands.size()});
        }

        const StrandArray& strands() const
        {
            return _strands;
        }

    private:
        template <size_t TStrandIndex>
        void initializeOneStrand()
        {
            using ProtocolType = typename std::tuple_element<TStrandIndex * 5 + 0, OwnedTuple>::type;
            using TransportType = typename std::tuple_element<TStrandIndex * 5 + 1, OwnedTuple>::type;
            using ShaderType = typename std::tuple_element<TStrandIndex * 5 + 2, OwnedTuple>::type;
            using OffsetType = typename std::tuple_element<TStrandIndex * 5 + 3, OwnedTuple>::type;
            using LengthType = typename std::tuple_element<TStrandIndex * 5 + 4, OwnedTuple>::type;

            static_assert(std::is_convertible<ProtocolType*, IProtocol<TColor>*>::value,
                          "Protocol type must be convertible to IProtocol<TColor>*");
            static_assert(std::is_convertible<TransportType*, ITransport*>::value,
                          "Transport type must be convertible to ITransport*");
            static_assert(std::is_convertible<ShaderType*, IShader<TColor>*>::value,
                          "Shader type must be convertible to IShader<TColor>*");
            static_assert(std::is_integral<OffsetType>::value,
                          "Strand offset must be an integral type");
            static_assert(std::is_integral<LengthType>::value,
                          "Strand length must be an integral type");

            _strands[TStrandIndex] = StrandExtent<TColor>{
                &std::get<TStrandIndex * 5 + 0>(_owned),
                &std::get<TStrandIndex * 5 + 1>(_owned),
                &std::get<TStrandIndex * 5 + 2>(_owned),
                static_cast<size_t>(std::get<TStrandIndex * 5 + 3>(_owned)),
                static_cast<size_t>(std::get<TStrandIndex * 5 + 4>(_owned))};
        }

        template <size_t... TIndices>
        void initializeStrands(std::index_sequence<TIndices...>)
        {
            (initializeOneStrand<TIndices>(), ...);
        }

        OwnedTuple _owned;
        StrandArray _strands;
    };

    template <typename TColor,
              typename... TArgs>
    auto makeStaticOwningBus(BufferHolder<TColor> rootBuffer,
                             Topology topology,
                             TArgs &&...args)
        -> StaticOwningBus<TColor, lw::remove_cvref_t<TArgs>...>
    {
        return StaticOwningBus<TColor, lw::remove_cvref_t<TArgs>...>{std::move(rootBuffer),
                                                                      std::move(topology),
                                                                      std::forward<TArgs>(args)...};
    }

    template <typename TColor>
    class DynamicOwningBus : public PixelBus<TColor>
    {
    public:
        DynamicOwningBus(BufferHolder<TColor> rootBuffer,
                         Topology topology,
                         std::vector<StrandExtent<TColor>> strands,
                         std::vector<std::unique_ptr<IProtocol<TColor>>> ownedProtocols,
                         std::vector<std::unique_ptr<ITransport>> ownedTransports,
                         std::vector<std::unique_ptr<IShader<TColor>>> ownedShaders)
            : PixelBus<TColor>(std::move(rootBuffer), std::move(topology))
            , _strands(std::move(strands))
            , _ownedProtocols(std::move(ownedProtocols))
            , _ownedTransports(std::move(ownedTransports))
            , _ownedShaders(std::move(ownedShaders))
        {
            this->setStrands(span<StrandExtent<TColor>>{_strands.data(), _strands.size()});
        }

        const std::vector<StrandExtent<TColor>>& strands() const
        {
            return _strands;
        }

    private:
        std::vector<StrandExtent<TColor>> _strands;
        std::vector<std::unique_ptr<IProtocol<TColor>>> _ownedProtocols;
        std::vector<std::unique_ptr<ITransport>> _ownedTransports;
        std::vector<std::unique_ptr<IShader<TColor>>> _ownedShaders;
    };

} // namespace lw
