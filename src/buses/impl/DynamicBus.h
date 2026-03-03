#pragma once

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "buses/impl/OwningBufferContext.h"

namespace lw
{

    template <typename TColor>
    class DynamicBus : protected OwningBufferContext<TColor>, public PixelBus<TColor>
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

        DynamicBus(size_t rootPixelCount,
                   size_t shaderPixelCount,
                   Topology topology,
                   std::vector<StrandExtent<TColor>> strands)
            : DynamicBus(rootPixelCount,
                         shaderPixelCount,
                         std::move(topology),
                         std::move(strands),
                         true)
        {
        }

        ~DynamicBus() override
        {
            for (auto &strand : _strands)
            {
                delete strand.protocol;
                delete strand.transport;
                delete strand.shader;
                strand.protocol = nullptr;
                strand.transport = nullptr;
                strand.shader = nullptr;
            }
        }

        const std::vector<StrandExtent<TColor>> &strands() const
        {
            return _strands;
        }

    protected:
        DynamicBus(size_t rootPixelCount,
                   size_t shaderPixelCount,
                   Topology topology,
                   std::vector<StrandExtent<TColor>> strands,
                   bool initializeNow)
            : OwningBufferContext<TColor>(rootPixelCount,
                                          shaderPixelCount,
                                          protocolSizesFromStrands(strands))
            , PixelBus<TColor>(this->bufferAccess(),
                               normalizeOwningBusTopology(std::move(topology), rootPixelCount),
                               span<StrandExtent<TColor>>{})
            , _strands(std::move(strands))
        {
            if (initializeNow)
            {
                initializeStrands();
                bindProtocolBuffers();
            }

            this->setStrands(span<StrandExtent<TColor>>{_strands.data(), _strands.size()});
        }

        std::vector<StrandExtent<TColor>> &mutableStrands()
        {
            return _strands;
        }

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
        }

        void initializeStrands()
        {
            size_t runningOffset = 0;
            for (auto &strand : _strands)
            {
                strand.offset = runningOffset;
                runningOffset += strand.length;
            }
        }

    private:
        std::vector<StrandExtent<TColor>> _strands;
    };

    template <typename TColor>
    class UnifiedDynamicBus : public DynamicBus<TColor>
    {
    public:
        using BaseBus = DynamicBus<TColor>;

        UnifiedDynamicBus(size_t rootPixelCount,
                          size_t shaderPixelCount,
                          Topology topology,
                          std::vector<StrandExtent<TColor>> strands)
            : BaseBus(rootPixelCount,
                      shaderPixelCount,
                      std::move(topology),
                      std::move(strands),
                      true)
        {
        }
    };

    template <typename TColor>
    auto makeUnifiedDynamicBus(size_t rootPixelCount,
                               size_t shaderPixelCount,
                               Topology topology,
                               std::vector<StrandExtent<TColor>> strands)
        -> UnifiedDynamicBus<TColor>
    {
        return UnifiedDynamicBus<TColor>{rootPixelCount,
                                         shaderPixelCount,
                                         std::move(topology),
                                         std::move(strands)};
    }

} // namespace lw
