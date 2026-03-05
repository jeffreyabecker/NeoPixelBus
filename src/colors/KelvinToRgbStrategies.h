#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace lw
{

    template<typename TComponent>
    struct KelvinToRgbExactStrategy
    {
        static constexpr uint16_t MinKelvin = 1200;
        static constexpr uint16_t MaxKelvin = 65000;

        static constexpr uint16_t clampKelvin(uint16_t kelvin)
        {
            return std::clamp(kelvin, MinKelvin, MaxKelvin);
        }

        static TComponent scale255ToComponent(int32_t value)
        {
            const uint64_t numerator = static_cast<uint64_t>(value) * static_cast<uint64_t>(std::numeric_limits<TComponent>::max())
                + 127u;
            return static_cast<TComponent>(numerator / 255u);
        }

        static std::array<TComponent, 3> convert(uint16_t kelvin)
        {
            const float temp = static_cast<float>(clampKelvin(kelvin)) / 100.0f;

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
                scale255ToComponent(std::clamp(red, int32_t{0}, int32_t{255})),
                scale255ToComponent(std::clamp(green, int32_t{0}, int32_t{255})),
                scale255ToComponent(std::clamp(blue, int32_t{0}, int32_t{255}))};
        }
    };


    // Approximate Kelvin to RGB conversion using a 64-entry lookup table with linear interpolation.
    // This strategy is designed for use in shaders where performance is critical and some approximation is acceptable.
    // The math is valid for a range of approximately 2000K to 6800K @ < 0.75% error, which covers most common white light sources. Values outside this range will be 
    // clamped and converted using the exact strategy.
    template<typename TComponent>
    struct KelvinToRgbLut64Strategy
    {
        static constexpr uint16_t TableMinKelvin = 2000;
        static constexpr uint16_t TableMaxKelvin = 6800;
        static constexpr uint16_t TableRangeKelvin = TableMaxKelvin - TableMinKelvin;
        static constexpr size_t TablePointCount = 64;
        static constexpr size_t TableSegmentCount = TablePointCount - 1;

        static std::array<TComponent, 3> convert(uint16_t kelvin)
        {
            const uint16_t clampedKelvin = KelvinToRgbExactStrategy<TComponent>::clampKelvin(kelvin);

            if (clampedKelvin < TableMinKelvin || clampedKelvin > TableMaxKelvin)
            {
                return KelvinToRgbExactStrategy<TComponent>::convert(clampedKelvin);
            }

            const uint32_t offsetKelvin = static_cast<uint32_t>(clampedKelvin - TableMinKelvin);
            const uint32_t scaledOffset = offsetKelvin * static_cast<uint32_t>(TableSegmentCount);
            const size_t segmentIndex = static_cast<size_t>(scaledOffset / TableRangeKelvin);
            const uint32_t segmentFraction = scaledOffset % TableRangeKelvin;

            if (segmentIndex >= TableSegmentCount)
            {
                return scaleEntry(Table[TablePointCount - 1]);
            }

            const auto &left = Table[segmentIndex];
            const auto &right = Table[segmentIndex + 1];
            return {
                KelvinToRgbExactStrategy<TComponent>::scale255ToComponent(interpolate8(left[0], right[0], segmentFraction)),
                KelvinToRgbExactStrategy<TComponent>::scale255ToComponent(interpolate8(left[1], right[1], segmentFraction)),
                KelvinToRgbExactStrategy<TComponent>::scale255ToComponent(interpolate8(left[2], right[2], segmentFraction))};
        }

    private:
        using Rgb8 = std::array<uint8_t, 3>;

        static int32_t interpolate8(uint8_t left, uint8_t right, uint32_t segmentFraction)
        {
            const int32_t delta = static_cast<int32_t>(right) - static_cast<int32_t>(left);
            const int32_t numerator = static_cast<int32_t>(left) * static_cast<int32_t>(TableRangeKelvin)
                + delta * static_cast<int32_t>(segmentFraction)
                + static_cast<int32_t>(TableRangeKelvin / 2u);
            return numerator / static_cast<int32_t>(TableRangeKelvin);
        }

        static std::array<TComponent, 3> scaleEntry(const Rgb8 &entry)
        {
            return {
                KelvinToRgbExactStrategy<TComponent>::scale255ToComponent(entry[0]),
                KelvinToRgbExactStrategy<TComponent>::scale255ToComponent(entry[1]),
                KelvinToRgbExactStrategy<TComponent>::scale255ToComponent(entry[2])};
        }

        static constexpr std::array<Rgb8, TablePointCount> Table = {
            Rgb8{255, 137, 14},
            Rgb8{255, 141, 24},
            Rgb8{255, 144, 34},
            Rgb8{255, 148, 42},
            Rgb8{255, 151, 51},
            Rgb8{255, 154, 59},
            Rgb8{255, 157, 66},
            Rgb8{255, 160, 73},
            Rgb8{255, 163, 80},
            Rgb8{255, 166, 86},
            Rgb8{255, 169, 92},
            Rgb8{255, 172, 98},
            Rgb8{255, 174, 104},
            Rgb8{255, 177, 109},
            Rgb8{255, 179, 114},
            Rgb8{255, 182, 119},
            Rgb8{255, 184, 124},
            Rgb8{255, 187, 129},
            Rgb8{255, 189, 134},
            Rgb8{255, 191, 138},
            Rgb8{255, 193, 142},
            Rgb8{255, 195, 146},
            Rgb8{255, 197, 150},
            Rgb8{255, 199, 154},
            Rgb8{255, 201, 158},
            Rgb8{255, 203, 162},
            Rgb8{255, 205, 165},
            Rgb8{255, 207, 169},
            Rgb8{255, 209, 172},
            Rgb8{255, 211, 175},
            Rgb8{255, 213, 179},
            Rgb8{255, 214, 182},
            Rgb8{255, 216, 185},
            Rgb8{255, 218, 188},
            Rgb8{255, 220, 191},
            Rgb8{255, 221, 194},
            Rgb8{255, 223, 197},
            Rgb8{255, 224, 200},
            Rgb8{255, 226, 202},
            Rgb8{255, 227, 205},
            Rgb8{255, 229, 208},
            Rgb8{255, 230, 210},
            Rgb8{255, 232, 213},
            Rgb8{255, 233, 215},
            Rgb8{255, 235, 218},
            Rgb8{255, 236, 220},
            Rgb8{255, 238, 222},
            Rgb8{255, 239, 225},
            Rgb8{255, 240, 227},
            Rgb8{255, 242, 229},
            Rgb8{255, 243, 231},
            Rgb8{255, 244, 234},
            Rgb8{255, 246, 236},
            Rgb8{255, 247, 238},
            Rgb8{255, 248, 240},
            Rgb8{255, 249, 242},
            Rgb8{255, 250, 244},
            Rgb8{255, 252, 246},
            Rgb8{255, 253, 248},
            Rgb8{255, 254, 250},
            Rgb8{255, 255, 252},
            Rgb8{255, 250, 255},
            Rgb8{253, 248, 255},
            Rgb8{250, 246, 255}};
    };

} // namespace lw
