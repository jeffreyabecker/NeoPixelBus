#pragma once

#include <cstdint>
#include <array>
#include <cmath>

#include "Color.h"
#include "IShader.h"

namespace npb
{

    template<typename TColor,
             typename = std::enable_if_t<ColorComponentTypeIs<TColor, uint8_t>>>
    struct GammaShaderSettings
    {
        float gamma = 2.6f;
        bool enableColorGamma = true;
        bool enableBrightnessGamma = false;
    };

    template<typename TColor,
             typename = std::enable_if_t<ColorComponentTypeIs<TColor, uint8_t>>>
    class GammaShader : public IShader<TColor>
    {
    public:
        using ColorType = TColor;
        static constexpr float MinGamma = 0.999f;
        static constexpr float MaxGamma = 3.0f;
        static constexpr float DefaultGamma = 2.6f;

        using SettingsType = GammaShaderSettings<TColor>;

        explicit GammaShader(SettingsType settings = {})
            : gammaCorrectCol{settings.enableColorGamma},
              gammaCorrectBri{settings.enableBrightnessGamma},
              gammaCorrectVal{settings.gamma}
        {
            recalculateTables();
        }

        void apply(span<TColor> colors) override
        {
            if (!gammaCorrectCol)
            {
                return;
            }

            for (auto &color : colors)
            {
                const size_t maxChannels = (TColor::ChannelCount < 4) ? TColor::ChannelCount : 4;
                for (size_t channel = 0; channel < maxChannels; ++channel)
                {
                    color[channel] = gamma8(color[channel]);
                }
            }
        }

        void setGamma(float gamma)
        {
            gammaCorrectVal = gamma;
            recalculateTables();
        }

        uint8_t gamma8_cal(uint8_t b, float gamma) const
        {
            if (b == 0)
            {
                return 0;
            }

            if (b == 255)
            {
                return 255;
            }

            return static_cast<uint8_t>(
                static_cast<int>(powf(static_cast<float>(b) / 255.0f, gamma) * 255.0f + 0.5f));
        }

        void calcGammaTable(float gamma)
        {
            for (uint16_t i = 1; i < 256; ++i)
            {
                gammaT[i] = gamma8_cal(static_cast<uint8_t>(i), gamma);
            }

            gammaT[0] = 0;
            gammaT[255] = 255;
        }

        uint8_t gamma8_slow(uint8_t b) const
        {
            return gammaT[b];
        }

        uint8_t gamma8(uint8_t value) const
        {
            return gammaT[value];
        }

        uint32_t gamma32(uint32_t color) const
        {
            if (!gammaCorrectCol)
            {
                return color;
            }

            const uint8_t w = gamma8(wFromColor(color));
            const uint8_t r = gamma8(rFromColor(color));
            const uint8_t g = gamma8(gFromColor(color));
            const uint8_t b = gamma8(bFromColor(color));
            return rgbw32(r, g, b, w);
        }

        std::array<uint8_t, 256> gammaT{};

        bool gammaCorrectCol;
        bool gammaCorrectBri;
        float gammaCorrectVal;

    private:
        static constexpr uint32_t rgbw32(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
        {
            return (static_cast<uint32_t>(w) << 24)
                | (static_cast<uint32_t>(r) << 16)
                | (static_cast<uint32_t>(g) << 8)
                | static_cast<uint32_t>(b);
        }

        static constexpr uint8_t rFromColor(uint32_t color)
        {
            return static_cast<uint8_t>(color >> 16);
        }

        static constexpr uint8_t gFromColor(uint32_t color)
        {
            return static_cast<uint8_t>(color >> 8);
        }

        static constexpr uint8_t bFromColor(uint32_t color)
        {
            return static_cast<uint8_t>(color);
        }

        static constexpr uint8_t wFromColor(uint32_t color)
        {
            return static_cast<uint8_t>(color >> 24);
        }

        void recalculateTables()
        {
            if ((gammaCorrectVal < MinGamma) || (gammaCorrectVal > MaxGamma))
            {
                calcGammaTable(1.0f);
            }
            else
            {
                calcGammaTable(gammaCorrectVal);
            }
        }
    };

    template<typename TColor>
    using WledGammaShader = GammaShader<TColor>;

} // namespace npb
