#include <Arduino.h>
#include <LumaWave.h>
#include <memory>
#include <vector>

/*
Target: Arduino platforms with one-wire output support.
Requires: Two valid data pins and heap allocation for owned child strips.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Aggregate strip composition owns child strips and takes them by `std::unique_ptr`.
*/

constexpr pixel_count_t leftCount = 12;
constexpr pixel_count_t rightCount = 12;
constexpr int leftDataPin = 2;
constexpr int rightDataPin = 3;

AggregateStrip<Color> aggregate(
    []
    {
        std::vector<std::unique_ptr<IStrip<Color>>> childBuses{};
        childBuses.emplace_back(std::make_unique<Strip<Protocols::Ws2812>>(
            leftCount, Transport::DefaultSettings{{.dataPin = leftDataPin}}));
        childBuses.emplace_back(std::make_unique<Strip<Protocols::Ws2812>>(
            rightCount, Transport::DefaultSettings{{.dataPin = rightDataPin}}));
        return childBuses;
    }());

uint16_t frame = 0;

void setup()
{
    aggregate.begin();
}

void loop()
{
    auto& pixels = aggregate.pixels();
    const size_t count = pixels.size();

    while (true)
    {
        for (size_t i = 0; i < count; ++i)
        {
            pixels[i] = Color(static_cast<uint8_t>((i + frame) & 0x3F), static_cast<uint8_t>((2U * i + frame) & 0x3F),
                              static_cast<uint8_t>((3U * i + frame) & 0x3F));
        }

        aggregate.show();
        ++frame;
        delay(20);
    }
