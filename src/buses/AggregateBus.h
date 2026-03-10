#pragma once

#include <memory>
#include <vector>

#include "core/IPixelBus.h"

namespace lw::busses
{

template <typename TColor> class AggregateBus : public IPixelBus<TColor>
{
  public:
    using BusType = IPixelBus<TColor>;
    using ChunkType = typename PixelView<TColor>::ChunkType;

    explicit AggregateBus(std::vector<std::unique_ptr<BusType>> buses)
        : _buses(std::move(buses)), _pixelChunks(collectChunks(_buses)),
          _pixels(span<ChunkType>{_pixelChunks.data(), _pixelChunks.size()})
    {
    }

    void begin() override
    {
        for (const auto& bus : _buses)
        {
            if (bus)
            {
                bus->begin();
            }
        }
    }

    void show() override
    {
        for (const auto& bus : _buses)
        {
            if (bus)
            {
                bus->show();
            }
        }
    }

    bool isReadyToUpdate() const override
    {
        for (const auto& bus : _buses)
        {
            if (!bus || !bus->isReadyToUpdate())
            {
                return false;
            }
        }

        return true;
    }

    PixelView<TColor>& pixels() override { return _pixels; }

    const PixelView<TColor>& pixels() const override { return _pixels; }

  private:
    static std::vector<ChunkType> collectChunks(const std::vector<std::unique_ptr<BusType>>& buses)
    {
        std::vector<ChunkType> chunks;

        for (const auto& bus : buses)
        {
            if (!bus)
            {
                continue;
            }

            const PixelView<TColor>& view = bus->pixels();

            for (auto chunk : view.chunks())
            {
                chunks.push_back(chunk);
            }
        }

        return chunks;
    }

    std::vector<std::unique_ptr<BusType>> _buses;
    std::vector<ChunkType> _pixelChunks;
    PixelView<TColor> _pixels;
};

} // namespace lw::busses
