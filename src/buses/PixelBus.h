#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <algorithm>

#include "buses/Topology.h"
#include "colors/IShader.h"
#include "core/BufferHolder.h"
#include "core/IPixelBus.h"
#include "protocols/IProtocol.h"
#include "transports/ITransport.h"

namespace lw
{

    template <typename TColor>
    struct StrandExtent
    {
        IProtocol<TColor>* protocol = nullptr;
        ITransport* transport = nullptr;
        IShader<TColor>* shader = nullptr;
        size_t offset = 0;
        size_t length = 0;
    };

    template <typename TColor>
    class PixelBus : public IPixelBus<TColor>
    {
    public:
        PixelBus(BufferHolder<TColor> rootBuffer,
                 Topology topology,
                 span<StrandExtent<TColor>> strands)
            : _rootBuffer(std::move(rootBuffer))
            , _topology(std::move(topology))
            , _strands(strands)
            , _valid(validateStrands(_strands, _rootBuffer.size, false))
        {
        }

        PixelBus(BufferHolder<TColor> rootBuffer,
                 Topology topology)
            : _rootBuffer(std::move(rootBuffer))
            , _topology(std::move(topology))
            , _strands{}
            , _valid(true)
        {
        }

        void begin() override
        {
            _rootBuffer.init();
            for (const auto& strand : _strands)
            {
                if (strand.length == 0)
                {
                    continue;
                }

                if (strand.transport != nullptr)
                {
                    strand.transport->begin();
                }

                if (strand.protocol != nullptr)
                {
                    strand.protocol->initialize();
                }
            }
        }

        void show() override
        {
            if (!_valid)
            {
                return;
            }

            if (!_dirty && !anyAlwaysUpdate())
            {
                return;
            }

            auto root = _rootBuffer.getSpan(0, _rootBuffer.size);

            for (const auto& strand : _strands)
            {
                if (strand.protocol == nullptr || strand.length == 0)
                {
                    continue;
                }

                if (strand.offset + strand.length > root.size())
                {
                    continue;
                }

                if (!strand.protocol->isReadyToUpdate())
                {
                    continue;
                }

                span<TColor> segment{root.data() + strand.offset, strand.length};
                if (strand.shader != nullptr)
                {
                    strand.shader->apply(segment);
                }

                strand.protocol->update(span<const TColor>{segment.data(), segment.size()});
            }

            _dirty = false;
        }

        bool canShow() const override
        {
            if (!_valid)
            {
                return false;
            }

            return std::all_of(_strands.begin(),
                               _strands.end(),
                               [](const StrandExtent<TColor>& strand)
                               {
                                   if (strand.length == 0)
                                   {
                                       return true;
                                   }

                                   if (strand.protocol == nullptr)
                                   {
                                       return false;
                                   }

                                   if (strand.transport != nullptr && !strand.transport->isReadyToUpdate())
                                   {
                                       return false;
                                   }

                                   return strand.protocol->isReadyToUpdate();
                               });
        }

        span<TColor> pixelBuffer() override
        {
            _dirty = true;
            return _rootBuffer.getSpan();
        }

        span<const TColor> pixelBuffer() const override
        {
            return _rootBuffer.getSpan();
        }

        const Topology* topologyOrNull() const override
        {
            return &_topology;
        }

    protected:
        void setStrands(span<StrandExtent<TColor>> strands,
                        bool allowOverlap = false)
        {
            _strands = strands;
            _valid = validateStrands(_strands, _rootBuffer.size, allowOverlap);
            _dirty = true;
        }

    private:
        static bool validateStrands(span<StrandExtent<TColor>> strands,
                                    size_t rootSize,
                                    bool allowOverlap)
        {
            for (const auto& strand : strands)
            {
                if (strand.length == 0)
                {
                    continue;
                }

                if (strand.protocol == nullptr)
                {
                    return false;
                }

                if (strand.offset > rootSize || strand.length > rootSize - strand.offset)
                {
                    return false;
                }
            }

            if (allowOverlap || strands.size() < 2)
            {
                return true;
            }

            std::vector<std::pair<size_t, size_t>> ranges;
            ranges.reserve(strands.size());
            for (const auto& strand : strands)
            {
                if (strand.length == 0)
                {
                    continue;
                }

                ranges.emplace_back(strand.offset, strand.offset + strand.length);
            }

            std::sort(ranges.begin(), ranges.end(), [](const auto& left, const auto& right)
            {
                return left.first < right.first;
            });

            for (size_t i = 1; i < ranges.size(); ++i)
            {
                if (ranges[i].first < ranges[i - 1].second)
                {
                    return false;
                }
            }

            return true;
        }

        bool anyAlwaysUpdate() const
        {
            return std::any_of(_strands.begin(),
                               _strands.end(),
                               [](const StrandExtent<TColor>& strand)
                               {
                                   return strand.protocol != nullptr && strand.protocol->alwaysUpdate();
                               });
        }

        BufferHolder<TColor> _rootBuffer;
        Topology _topology;
        span<StrandExtent<TColor>> _strands;
        bool _valid{false};
        bool _dirty{true};
    };

    template <typename TColor>
    class OwningPixelBusT : public IAssignableBufferBus<TColor>
    {
    public:
        explicit OwningPixelBusT(IProtocol<TColor> *protocol,
                                 ITransport *transport = nullptr)
            : _ownedProtocol(protocol)
            , _ownedTransport(transport)
            , _protocol(_ownedProtocol.get())
            , _pixelCount(protocol != nullptr ? protocol->pixelCount() : 0)
            , _ownedColors(protocol != nullptr ? protocol->pixelCount() : 0)
            , _colors(_ownedColors.data(), _ownedColors.size())
        {
        }

        OwningPixelBusT(const OwningPixelBusT &) = delete;
        OwningPixelBusT &operator=(const OwningPixelBusT &) = delete;
        OwningPixelBusT(OwningPixelBusT &&) = default;
        OwningPixelBusT &operator=(OwningPixelBusT &&) = default;

        void begin() override
        {
            if (_protocol != nullptr)
            {
                _protocol->initialize();
            }
        }

        void show() override
        {
            if (_protocol == nullptr)
            {
                return;
            }

            if (!_dirty && !_protocol->alwaysUpdate())
            {
                return;
            }

            _protocol->update(span<const TColor>{_colors.data(), _colors.size()});
            _dirty = false;
        }

        bool canShow() const override
        {
            return _protocol != nullptr && _protocol->isReadyToUpdate();
        }

        void setBuffer(span<TColor> buffer) override
        {
            _colors = buffer;
            _dirty = true;
        }

        uint16_t pixelCount() const override
        {
            return _pixelCount;
        }

        span<TColor> pixelBuffer() override
        {
            _dirty = true;
            return span<TColor>{_colors.data(), _colors.size()};
        }

        span<const TColor> pixelBuffer() const override
        {
            return span<const TColor>{_colors.data(), _colors.size()};
        }

        span<TColor> colors()
        {
            return pixelBuffer();
        }

        span<const TColor> colors() const
        {
            return pixelBuffer();
        }

        IProtocol<TColor> *protocol()
        {
            return _ownedProtocol.get();
        }

        const IProtocol<TColor> *protocol() const
        {
            return _ownedProtocol.get();
        }

        ITransport *transport()
        {
            return _ownedTransport.get();
        }

        const ITransport *transport() const
        {
            return _ownedTransport.get();
        }

    private:
        std::unique_ptr<IProtocol<TColor>> _ownedProtocol;
        std::unique_ptr<ITransport> _ownedTransport;
        IProtocol<TColor> *_protocol{nullptr};
        uint16_t _pixelCount{0};
        std::vector<TColor> _ownedColors;
        span<TColor> _colors;
        bool _dirty{false};
    };

} // namespace lw


