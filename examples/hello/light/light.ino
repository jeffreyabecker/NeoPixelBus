#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with platform-default light driver support.
Requires: Platform default light driver pin assignment.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Uses `lw::busses::LightBus<lw::Rgbcw16Color, Driver>` with default-constructed driver settings.
*/

constexpr int redPin = 2;
constexpr int greenPin = 3;
constexpr int bluePin = 4;
constexpr int warmPin = 5;
constexpr int coolPin = 6;

Light<Rgbcw16Color, Driver::PlatformDefault<Rgbcw16Color>> light({.pins = {redPin, greenPin, bluePin, warmPin, coolPin},
                                                                  .invert = false});
uint16_t phase = 0;

void setup()
{
    light.begin();
}

void loop()
{
    auto& pixel = light.pixel();

    const uint16_t ramp = static_cast<uint16_t>((phase & 0xFF) * 257U);
    pixel['R'] = ramp;
    pixel['G'] = static_cast<uint16_t>(65535U - ramp);
    pixel['B'] = static_cast<uint16_t>(ramp / 2U);
    pixel['W'] = static_cast<uint16_t>((phase * 113U) & 0xFFFF);
    pixel['C'] = static_cast<uint16_t>((phase * 197U) & 0xFFFF);

    light.show();
    ++phase;
    delay(12);
}
