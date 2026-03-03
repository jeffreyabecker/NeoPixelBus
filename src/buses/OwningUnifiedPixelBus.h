#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "buses/PixelBus.h"
#include "core/BufferHolder.h"
#include "core/Compat.h"
#include "core/UnifiedOwningBufferAccessSurface.h"

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

    template <typename TColor>
    struct UnifiedOwningArenaLayout
    {
        BufferHolder<uint8_t> unifiedArena;
        BufferHolder<TColor> rootBuffer;
        BufferHolder<TColor> shaderBuffer;
        BufferHolder<uint8_t> protocolBuffer;
    };

    template <typename TColor>
    UnifiedOwningArenaLayout<TColor> carveUnifiedOwningArena(BufferHolder<uint8_t> unifiedArena,
                                                              size_t rootPixelCount,
                                                              size_t shaderPixelCount,
                                                              size_t protocolByteCount)
    {
        UnifiedOwningArenaLayout<TColor> layout{};

        if (!unifiedArena.owns)
        {
            BufferHolder<uint8_t> ownedArena{unifiedArena.size, nullptr, true};
            ownedArena.init();
            if (ownedArena.buffer != nullptr && unifiedArena.buffer != nullptr && unifiedArena.size > 0)
            {
                std::copy_n(unifiedArena.buffer, unifiedArena.size, ownedArena.buffer);
            }
            unifiedArena = std::move(ownedArena);
        }

        const size_t rootByteCount = rootPixelCount * sizeof(TColor);
        const size_t shaderByteCount = shaderPixelCount * sizeof(TColor);
        const size_t alignmentPadding = (alignof(TColor) > 0) ? (alignof(TColor) - 1) : 0;
        const size_t requiredByteCount = rootByteCount + shaderByteCount + protocolByteCount + (alignmentPadding * 2);

        if (unifiedArena.size < requiredByteCount)
        {
            unifiedArena = BufferHolder<uint8_t>{requiredByteCount, nullptr, true};
        }

        auto arena = unifiedArena.getSpan();
        uint8_t *cursor = arena.data();
        size_t remainingBytes = arena.size();

        auto carveColorBuffer = [&](size_t pixelCount) -> BufferHolder<TColor>
        {
            if (pixelCount == 0)
            {
                return BufferHolder<TColor>::nil();
            }

            const size_t byteCount = pixelCount * sizeof(TColor);
            void *alignedStart = static_cast<void *>(cursor);
            size_t alignedSpace = remainingBytes;

            if (std::align(alignof(TColor), byteCount, alignedStart, alignedSpace) == nullptr)
            {
                assert(false && "Unified arena color slice alignment failed");
                return BufferHolder<TColor>::nil();
            }

            auto *start = static_cast<uint8_t *>(alignedStart);
            if (alignedSpace < byteCount)
            {
                assert(false && "Unified arena color slice is too small");
                return BufferHolder<TColor>::nil();
            }

            cursor = start + byteCount;
            remainingBytes = alignedSpace - byteCount;

            return BufferHolder<TColor>{pixelCount,
                                        reinterpret_cast<TColor *>(start),
                                        false};
        };

        layout.rootBuffer = carveColorBuffer(rootPixelCount);
        layout.shaderBuffer = carveColorBuffer(shaderPixelCount);

        if (protocolByteCount == 0)
        {
            layout.protocolBuffer = BufferHolder<uint8_t>::nil();
        }
        else
        {
            if (remainingBytes < protocolByteCount)
            {
                assert(false && "Unified arena protocol slice is too small");
                layout.protocolBuffer = BufferHolder<uint8_t>::nil();
            }
            else
            {
                layout.protocolBuffer = BufferHolder<uint8_t>{protocolByteCount,
                                                              cursor,
                                                              false};
            }
        }

        layout.unifiedArena = std::move(unifiedArena);
        return layout;
    }

    template <typename TColor>
    class UnifiedOwningBufferAccessContext
    {
    protected:
        UnifiedOwningBufferAccessContext(size_t rootPixelCount,
                                         size_t shaderPixelCount,
                                         std::vector<size_t> protocolSizes)
            : _protocolSizes(std::move(protocolSizes))
            , _bufferAccess(rootPixelCount,
                            shaderPixelCount,
                            span<size_t>{_protocolSizes.data(), _protocolSizes.size()})
        {
        }

        UnifiedOwningBufferAccessSurface<TColor> &bufferAccess()
        {
            return _bufferAccess;
        }

        const UnifiedOwningBufferAccessSurface<TColor> &bufferAccess() const
        {
            return _bufferAccess;
        }

    private:
        std::vector<size_t> _protocolSizes;
        UnifiedOwningBufferAccessSurface<TColor> _bufferAccess;
    };

    template <typename TColor, typename... TArgs>
    class StaticOwningBus : protected UnifiedOwningBufferAccessContext<TColor>, public PixelBus<TColor>
    {
    public:
        using ColorType = TColor;

        static_assert(sizeof...(TArgs) % 4 == 0,
                      "StaticOwningBus requires inline instance groups of 4: protocol instance, transport instance, shader instance, length");

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

        StaticOwningBus(BufferHolder<TColor> rootBuffer,
                        BufferHolder<TColor> shaderBuffer,
                        BufferHolder<uint8_t> protocolBuffer,
                        Topology topology,
                        TArgs &&...args)
            : UnifiedOwningBufferAccessContext<TColor>(rootBuffer.size,
                                                       shaderBuffer.size,
                                                       protocolSizesFromTuple(std::forward_as_tuple(args...),
                                                                             std::make_index_sequence<StrandCount>{}))
            , PixelBus<TColor>(this->bufferAccess(),
                               normalizeOwningBusTopology(std::move(topology), rootBuffer.size),
                               span<StrandExtent<TColor>>{})
            , _owned(std::forward<TArgs>(args)...)
        {
            (void)protocolBuffer;
            initializeStrands(std::make_index_sequence<StrandCount>{});
            auto root = this->bufferAccess().rootPixels();
            auto inputRoot = rootBuffer.getSpan();
            if (root.size() > 0 && inputRoot.size() > 0)
            {
                std::copy_n(inputRoot.begin(), std::min(root.size(), inputRoot.size()), root.begin());
            }
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

    protected:
        void bindProtocolBuffers()
        {
            for (const auto &strand : _strands)
            {
                if (strand.protocol == nullptr)
                {
                    continue;
                }

                strand.protocol->bindTransport(strand.transport);
            }

            for (size_t strandIndex = 0; strandIndex < _strands.size(); ++strandIndex)
            {
                const auto &strand = _strands[strandIndex];
                if (strand.protocol == nullptr)
                {
                    continue;
                }

                strand.protocol->setBuffer(this->bufferAccess().protocolSlice(strandIndex));
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

    private:

        OwnedTuple _owned;
        StrandArray _strands;
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

    template <typename TColor,
              typename... TArgs>
    class UnifiedStaticOwningBus : public StaticOwningBus<TColor, TArgs...>
    {
    public:
        using BaseBus = StaticOwningBus<TColor, TArgs...>;
        using ArenaLayout = UnifiedOwningArenaLayout<TColor>;
        static constexpr size_t StrandCount = BaseBus::StrandCount;

        UnifiedStaticOwningBus(BufferHolder<uint8_t> unifiedArena,
                               size_t rootPixelCount,
                               size_t shaderPixelCount,
                               Topology topology,
                               TArgs &&...args)
            : UnifiedStaticOwningBus(buildLayout(std::move(unifiedArena),
                                                 rootPixelCount,
                                                 shaderPixelCount,
                                                 std::forward_as_tuple(args...)),
                                     std::move(topology),
                                     std::forward<TArgs>(args)...)
        {
        }

    private:
        template <typename TTuple, size_t... TIndices>
        static size_t protocolBytesFromTuple(const TTuple &values,
                                             std::index_sequence<TIndices...>)
        {
            return (static_cast<size_t>(std::get<TIndices * 4>(values).requiredBufferSizeBytes()) + ... + 0u);
        }

        template <typename TTuple>
        static ArenaLayout buildLayout(BufferHolder<uint8_t> unifiedArena,
                                       size_t rootPixelCount,
                                       size_t shaderPixelCount,
                                       const TTuple &values)
        {
            const size_t protocolByteCount = protocolBytesFromTuple(values,
                                                                    std::make_index_sequence<StrandCount>{});
            return carveUnifiedOwningArena<TColor>(std::move(unifiedArena),
                                                   rootPixelCount,
                                                   shaderPixelCount,
                                                   protocolByteCount);
        }

        UnifiedStaticOwningBus(ArenaLayout layout,
                               Topology topology,
                               TArgs &&...args)
            : BaseBus(std::move(layout.rootBuffer),
                      std::move(layout.shaderBuffer),
                      std::move(layout.protocolBuffer),
                      std::move(topology),
                      std::forward<TArgs>(args)...)
            , _unifiedArena(std::move(layout.unifiedArena))
        {
        }

        BufferHolder<uint8_t> _unifiedArena;
    };

    template <typename TColor,
              typename... TArgs>
    auto makeUnifiedStaticOwningBus(BufferHolder<uint8_t> unifiedArena,
                                    size_t rootPixelCount,
                                    size_t shaderPixelCount,
                                    Topology topology,
                                    TArgs &&...args)
        -> UnifiedStaticOwningBus<TColor, lw::remove_cvref_t<TArgs>...>
    {
        return UnifiedStaticOwningBus<TColor, lw::remove_cvref_t<TArgs>...>{std::move(unifiedArena),
                                                                             rootPixelCount,
                                                                             shaderPixelCount,
                                                                             std::move(topology),
                                                                             std::forward<TArgs>(args)...};
    }

    template <typename TColor>
    class DynamicOwningBus : protected UnifiedOwningBufferAccessContext<TColor>, public PixelBus<TColor>
    {
    public:
        static std::vector<size_t> protocolSizesFromStrands(const std::vector<StrandExtent<TColor>> &strands)
        {
            std::vector<size_t> sizes{};
            sizes.reserve(strands.size());
            for (const auto &strand : strands)
            {
                if (strand.protocol == nullptr)
                {
                    sizes.push_back(0);
                    continue;
                }

                sizes.push_back(strand.protocol->requiredBufferSizeBytes());
            }

            return sizes;
        }

        DynamicOwningBus(BufferHolder<TColor> rootBuffer,
                         BufferHolder<TColor> shaderBuffer,
                         BufferHolder<uint8_t> protocolBuffer,
                         Topology topology,
                         std::vector<StrandExtent<TColor>> strands)
            : DynamicOwningBus(std::move(rootBuffer),
                               std::move(shaderBuffer),
                               std::move(protocolBuffer),
                               std::move(topology),
                               std::move(strands),
                               true)
        {
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

    protected:
        DynamicOwningBus(BufferHolder<TColor> rootBuffer,
                         BufferHolder<TColor> shaderBuffer,
                         BufferHolder<uint8_t> protocolBuffer,
                         Topology topology,
                         std::vector<StrandExtent<TColor>> strands,
                         bool initializeNow)
            : UnifiedOwningBufferAccessContext<TColor>(rootBuffer.size,
                                                       shaderBuffer.size,
                                                       protocolSizesFromStrands(strands))
            , PixelBus<TColor>(this->bufferAccess(),
                               normalizeOwningBusTopology(std::move(topology), rootBuffer.size),
                               span<StrandExtent<TColor>>{})
            , _strands(std::move(strands))
        {
            (void)protocolBuffer;
            auto root = this->bufferAccess().rootPixels();
            auto inputRoot = rootBuffer.getSpan();
            if (root.size() > 0 && inputRoot.size() > 0)
            {
                std::copy_n(inputRoot.begin(), std::min(root.size(), inputRoot.size()), root.begin());
            }

            if (initializeNow)
            {
                initializeStrands();
                bindProtocolBuffers();
            }

            this->setStrands(span<StrandExtent<TColor>>{_strands.data(), _strands.size()});
        }

        std::vector<StrandExtent<TColor>>& mutableStrands()
        {
            return _strands;
        }

    protected:
        void bindProtocolBuffers()
        {
            for (const auto &strand : _strands)
            {
                if (strand.protocol == nullptr)
                {
                    continue;
                }

                strand.protocol->bindTransport(strand.transport);
            }

            for (size_t strandIndex = 0; strandIndex < _strands.size(); ++strandIndex)
            {
                const auto &strand = _strands[strandIndex];
                if (strand.protocol == nullptr)
                {
                    continue;
                }

                strand.protocol->setBuffer(this->bufferAccess().protocolSlice(strandIndex));
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

    private:

        std::vector<StrandExtent<TColor>> _strands;
    };

    template <typename TColor>
    class UnifiedDynamicOwningBus : public DynamicOwningBus<TColor>
    {
    public:
        using BaseBus = DynamicOwningBus<TColor>;

        UnifiedDynamicOwningBus(BufferHolder<uint8_t> unifiedArena,
                                size_t rootPixelCount,
                                size_t shaderPixelCount,
                                Topology topology,
                                std::vector<StrandExtent<TColor>> strands)
            : BaseBus(BufferHolder<TColor>{rootPixelCount, nullptr, false},
                      BufferHolder<TColor>{shaderPixelCount, nullptr, false},
                      BufferHolder<uint8_t>::nil(),
                      std::move(topology),
                      std::move(strands),
                      true)
        {
            (void)unifiedArena;
        }
    };

    template <typename TColor>
    auto makeUnifiedDynamicOwningBus(BufferHolder<uint8_t> unifiedArena,
                                     size_t rootPixelCount,
                                     size_t shaderPixelCount,
                                     Topology topology,
                                     std::vector<StrandExtent<TColor>> strands)
        -> UnifiedDynamicOwningBus<TColor>
    {
        return UnifiedDynamicOwningBus<TColor>{std::move(unifiedArena),
                                               rootPixelCount,
                                               shaderPixelCount,
                                               std::move(topology),
                                               std::move(strands)};
    }

} // namespace lw
