#pragma once

#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "Color.h"
#include "IShader.h"
#include "KelvinToRgbStrategies.h"

namespace lw
{

    enum class CCTColorInterlock : uint8_t
    {
        None,
        ForceOff,
        ForceOn,
        MatchWhite
    };

    template<typename TColor,
             template<typename> class TKelvinToRgbStrategy = KelvinToRgbLut64Strategy,
             typename = std::enable_if_t<ColorChannelsAtLeast<TColor, 5>>>
    struct CCTWhiteBalanceShaderSettings
    {
        template<typename TComponent>
        using KelvinToRgbStrategy = TKelvinToRgbStrategy<TComponent>;

        uint16_t lowKelvin = 2700;
        uint16_t highKelvin = 6500;
        CCTColorInterlock colorInterlock = CCTColorInterlock::None;
    };

    template<typename TColor,
             template<typename> class TKelvinToRgbStrategy = KelvinToRgbLut64Strategy,
             typename = std::enable_if_t<ColorChannelsAtLeast<TColor, 5>>>
    class CCTWhiteBalanceShader : public IShader<TColor>
    {
    public:
        using ColorType = TColor;
        using SettingsType = CCTWhiteBalanceShaderSettings<TColor, TKelvinToRgbStrategy>;
        using ComponentType = typename TColor::ComponentType;

        explicit CCTWhiteBalanceShader(SettingsType settings)
            : _lowKelvin{clampKelvin(settings.lowKelvin)}
            , _highKelvin{clampKelvin(settings.highKelvin)}
            , _colorInterlock{settings.colorInterlock}
        {
        }

        void apply(span<TColor> colors) override
        {
            for (auto &color : colors)
            {
                const ComponentType brightness = color['C'];
                const ComponentType balance = color['W'];

                // Interpret incoming C/W as controls: C is white brightness, W is warm/cool balance.
                const ComponentType warm = scaleByUnit(brightness, inverseUnit(balance));
                const ComponentType cool = scaleByUnit(brightness, balance);
                color['W'] = warm;
                color['C'] = cool;

                switch (_colorInterlock)
                {
                case CCTColorInterlock::None:
                    break;

                case CCTColorInterlock::ForceOff:
                    color['R'] = static_cast<ComponentType>(0);
                    color['G'] = static_cast<ComponentType>(0);
                    color['B'] = static_cast<ComponentType>(0);
                    break;

                case CCTColorInterlock::ForceOn:
                    color['R'] = MaxComponent;
                    color['G'] = MaxComponent;
                    color['B'] = MaxComponent;
                    break;

                case CCTColorInterlock::MatchWhite:
                {
                    const auto rgbApproximation = kelvinToRgb(lerpKelvin(balance));
                    color['R'] = scaleByUnit(rgbApproximation[0], brightness);
                    color['G'] = scaleByUnit(rgbApproximation[1], brightness);
                    color['B'] = scaleByUnit(rgbApproximation[2], brightness);
                    break;
                }
                }
            }
        }

    private:
        static constexpr ComponentType MaxComponent = TColor::MaxComponent;
        using KelvinToRgbStrategy = typename SettingsType::template KelvinToRgbStrategy<ComponentType>;

        static constexpr uint16_t clampKelvin(uint16_t kelvin)
        {
            return KelvinToRgbExactStrategy<ComponentType>::clampKelvin(kelvin);
        }

        static constexpr ComponentType inverseUnit(ComponentType value)
        {
            return static_cast<ComponentType>(MaxComponent - value);
        }

        static ComponentType scaleByUnit(ComponentType value, ComponentType unit)
        {
            const uint64_t numerator = static_cast<uint64_t>(value) * static_cast<uint64_t>(unit)
                + static_cast<uint64_t>(MaxComponent / 2u);
            return static_cast<ComponentType>(numerator / static_cast<uint64_t>(MaxComponent));
        }

        uint16_t lerpKelvin(ComponentType balance) const
        {
            const uint32_t range = static_cast<uint32_t>(_highKelvin - _lowKelvin);
            const uint64_t blended = static_cast<uint64_t>(_lowKelvin)
                + ((static_cast<uint64_t>(range) * static_cast<uint64_t>(balance)
                    + static_cast<uint64_t>(MaxComponent / 2u))
                   / static_cast<uint64_t>(MaxComponent));
            return static_cast<uint16_t>(blended);
        }

        static std::array<ComponentType, 3> kelvinToRgb(uint16_t kelvin)
        {
            return KelvinToRgbStrategy::convert(kelvin);
        }

        uint16_t _lowKelvin;
        uint16_t _highKelvin;
        CCTColorInterlock _colorInterlock;
    };

} // namespace lw
