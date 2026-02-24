#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <vector>
#include <algorithm>
#include <concepts>
#include <utility>

#include "IProtocol.h"
#include "../colors/Color.h"

namespace npb
{

enum class NarrowingComponentMode : uint8_t
{
    Truncate,
    RoundToNearest
};

struct NarrowingProtocolSettings
{
    const char* channelOrder = nullptr;
    NarrowingComponentMode componentMode = NarrowingComponentMode::Truncate;
};

template<typename TBusColor, typename TWireColor, typename TWireProtocol>
    requires std::derived_from<TWireProtocol, IProtocol<TWireColor>>
class NarrowingProtocol : public IProtocol<TBusColor>
{
public:
    using SettingsType = NarrowingProtocolSettings;
    using TransportCategory = typename TWireProtocol::TransportCategory;

    static_assert(std::same_as<typename TBusColor::ComponentType, uint16_t>,
        "NarrowingProtocol expects a 16-bit bus color type.");
    static_assert(std::same_as<typename TWireColor::ComponentType, uint8_t>,
        "NarrowingProtocol expects an 8-bit wire color type.");
    static_assert(TBusColor::ChannelCount >= TWireColor::ChannelCount,
        "NarrowingProtocol requires bus color channels >= wire color channels.");

    template<typename... TWireProtocolArgs>
    NarrowingProtocol(uint16_t pixelCount,
                      NarrowingProtocolSettings settings,
                      TWireProtocolArgs&&... wireProtocolArgs)
        : _wireProtocol(pixelCount, std::forward<TWireProtocolArgs>(wireProtocolArgs)...)
        , _settings{settings}
        , _scratchColors(pixelCount)
    {
    }

    template<typename... TWireProtocolArgs>
    NarrowingProtocol(uint16_t pixelCount,
                      TWireProtocolArgs&&... wireProtocolArgs)
        : NarrowingProtocol(pixelCount,
                            NarrowingProtocolSettings{},
                            std::forward<TWireProtocolArgs>(wireProtocolArgs)...)
    {
    }

    void initialize() override
    {
        _wireProtocol.initialize();
    }

    void update(std::span<const TBusColor> colors) override
    {
        const size_t count = std::min(colors.size(), _scratchColors.size());
        for (size_t idx = 0; idx < count; ++idx)
        {
            for (size_t channel = 0; channel < TWireColor::ChannelCount; ++channel)
            {
                const char mappedChannel = channelAt(channel);
                const uint16_t src = colors[idx][mappedChannel];
                _scratchColors[idx][channel] = narrowComponent(src);
            }
        }

        _wireProtocol.update(std::span<const TWireColor>{_scratchColors.data(), count});
    }

    bool isReadyToUpdate() const override
    {
        return _wireProtocol.isReadyToUpdate();
    }

    bool alwaysUpdate() const override
    {
        return _wireProtocol.alwaysUpdate();
    }

private:
    static constexpr char defaultChannelForIndex(size_t channel)
    {
        switch (channel)
        {
        case 0:
            return 'R';

        case 1:
            return 'G';

        case 2:
            return 'B';

        case 3:
            return 'W';

        case 4:
            return 'C';
        }

        return 'R';
    }

    char channelAt(size_t channel) const
    {
        if (_settings.channelOrder != nullptr && _settings.channelOrder[channel] != '\0')
        {
            return _settings.channelOrder[channel];
        }

        return defaultChannelForIndex(channel);
    }

    uint8_t narrowComponent(uint16_t value) const
    {
        if (_settings.componentMode == NarrowingComponentMode::RoundToNearest)
        {
            return static_cast<uint8_t>((static_cast<uint32_t>(value) + 0x80u) >> 8);
        }

        return static_cast<uint8_t>(value >> 8);
    }

    TWireProtocol _wireProtocol;
    NarrowingProtocolSettings _settings;
    std::vector<TWireColor> _scratchColors;
};

} // namespace npb
