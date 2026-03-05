#pragma once

#include <vector>

#include "core/IPixelBus.h"

namespace lw
{

    template <typename TColor>
    class AggregateBus : public IPixelBus<TColor>
    {
    public:
        using BusType = IPixelBus<TColor>;
        using ChunkType = typename PixelView<TColor>::ChunkType;

        explicit AggregateBus(span<BusType *> buses)
            : _buses(buses.begin(), buses.end())
            , _pixels(span<ChunkType>{_pixelChunks.data(), _pixelChunks.size()})
        {
            rebuildPixels(false);
        }

        void begin() override
        {
            for (auto *bus : _buses)
            {
                if (bus != nullptr)
                {
                    bus->begin();
                }
            }
        }

        void show() override
        {
            for (auto *bus : _buses)
            {
                if (bus != nullptr)
                {
                    bus->show();
                }
            }
        }

        bool isReadyToUpdate() const override
        {
            for (const auto *bus : _buses)
            {
                if (bus == nullptr || !bus->isReadyToUpdate())
                {
                    return false;
                }
            }

            return true;
        }

        PixelView<TColor> &pixels() override
        {
            rebuildPixels(true);
            return _pixels;
        }

        const PixelView<TColor> &pixels() const override
        {
            rebuildPixels(false);
            return _pixels;
        }

    private:
        void rebuildPixels(bool markChildrenDirty) const
        {
            _pixelChunks.clear();

            for (auto *bus : _buses)
            {
                if (bus == nullptr)
                {
                    continue;
                }

                const PixelView<TColor> &view = markChildrenDirty
                    ? bus->pixels()
                    : static_cast<const BusType &>(*bus).pixels();

                for (auto chunk : view.chunks())
                {
                    _pixelChunks.push_back(chunk);
                }
            }

            _pixels = PixelView<TColor>(span<ChunkType>{_pixelChunks.data(), _pixelChunks.size()});
        }

        std::vector<BusType *> _buses;
        mutable std::vector<ChunkType> _pixelChunks;
        mutable PixelView<TColor> _pixels;
    };

} // namespace lw
