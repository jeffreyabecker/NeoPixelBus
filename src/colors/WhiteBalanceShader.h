#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <cmath>
#include <algorithm>
#include <limits>

#include "Color.h"
#include "IShader.h"

namespace npb
{

    template<typename TColor,
             typename = std::enable_if_t<ColorChannelsAtLeast<TColor, 4>>>
    struct WhiteBalanceShaderSettings
    {
        bool dualWhite = false;
        uint16_t whiteKelvin = 6500;
        uint16_t warmWhiteKelvin = 2700;
        uint16_t coolWhiteKelvin = 6500;
    };

    // White-balance and Kelvin-to-RGB correction logic adapted from WLED / WLED-MM.
    // Source: https://github.com/MoonModules/WLED-MM
    template<typename TColor,
             typename = std::enable_if_t<ColorChannelsAtLeast<TColor, 4>>>
    class WhiteBalanceShader : public IShader<TColor>
    {
    public:
        using ColorType = TColor;
        using SettingsType = WhiteBalanceShaderSettings<TColor>;
        using ComponentType = typename TColor::ComponentType;

        explicit WhiteBalanceShader(SettingsType settings)
            : _dualWhite{settings.dualWhite}
            , _warmCorrection{kelvinToRgbCorrection(settings.dualWhite ? settings.warmWhiteKelvin : settings.whiteKelvin)}
            , _coolCorrection{settings.dualWhite ? kelvinToRgbCorrection(settings.coolWhiteKelvin) : _warmCorrection}
        {
        }

        void apply(span<TColor> colors) override
        {
            for (auto &color : colors)
            {
                uint16_t warmWeight = 255;
                uint16_t coolWeight = 0;

                if (_dualWhite)
                {
                    uint16_t warm = color['W'];
                    uint16_t cool = color['C'];
                    uint16_t total = warm + cool;

                    if (total > 0)
                    {
                        warmWeight = static_cast<uint16_t>((warm * 255u) / total);
                        coolWeight = static_cast<uint16_t>(255u - warmWeight);
                    }
                    else
                    {
                        warmWeight = 128;
                        coolWeight = 127;
                    }
                }

                for (size_t channel = 0; channel < 3; ++channel)
                {
                    const char channelTag = ChannelOrder::RGB::value[channel];
                    uint16_t correction = _warmCorrection[channel];

                    if (_dualWhite)
                    {
                        correction = static_cast<uint16_t>(
                            (_warmCorrection[channel] * warmWeight + _coolCorrection[channel] * coolWeight + 127u) / 255u);
                    }

                    color[channelTag] = static_cast<ComponentType>(
                        (static_cast<uint64_t>(color[channelTag]) * correction + (MaxCorrection / 2u)) / MaxCorrection);
                }
            }
        }

    private:
        static constexpr uint16_t MinKelvin = 1200;
        static constexpr uint16_t MaxKelvin = 65000;
        static constexpr uint16_t MaxCorrection = static_cast<uint16_t>(std::numeric_limits<ComponentType>::max());

        // Kelvin-to-RGB conversion coefficients follow the implementation used in WLED / WLED-MM.
        static std::array<ComponentType, 3> kelvinToRgbCorrection(uint16_t kelvin)
        {
            if (kelvin < MinKelvin || kelvin > MaxKelvin)
            {
                return {MaxCorrection, MaxCorrection, MaxCorrection};
            }

            float temp = static_cast<float>(kelvin) / 100.0f;

            int32_t red = 0;
            int32_t green = 0;
            int32_t blue = 0;

            if (temp <= 66.0f)
            {
                red = 255;
                green = static_cast<int32_t>(roundf(99.4708025861f * logf(temp) - 161.1195681661f));

                if (temp <= 19.0f)
                {
                    blue = 0;
                }
                else
                {
                    blue = static_cast<int32_t>(roundf(138.5177312231f * logf(temp - 10.0f) - 305.0447927307f));
                }
            }
            else
            {
                red = static_cast<int32_t>(roundf(329.698727446f * powf(temp - 60.0f, -0.1332047592f)));
                green = static_cast<int32_t>(roundf(288.1221695283f * powf(temp - 60.0f, -0.0755148492f)));
                blue = 255;
            }

            return {
                static_cast<ComponentType>((static_cast<uint32_t>(std::clamp(red, int32_t{0}, int32_t{255})) * MaxCorrection + 127u) / 255u),
                static_cast<ComponentType>((static_cast<uint32_t>(std::clamp(green, int32_t{0}, int32_t{255})) * MaxCorrection + 127u) / 255u),
                static_cast<ComponentType>((static_cast<uint32_t>(std::clamp(blue, int32_t{0}, int32_t{255})) * MaxCorrection + 127u) / 255u)};
        }

        bool _dualWhite;
        std::array<ComponentType, 3> _warmCorrection;
        std::array<ComponentType, 3> _coolCorrection;
    };

} // namespace npb

