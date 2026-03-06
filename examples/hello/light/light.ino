#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with platform-default light driver support.
Requires: Platform default light driver pin assignment.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Uses `lw::busses::LightBus<lw::Rgbcw16Color, Driver>` with default-constructed driver settings.
*/

namespace
{
using ColorType = lw::Rgbcw16Color;
using DriverType = lw::transports::PlatformDefaultLightDriver<ColorType>;
using LightType = lw::busses::LightBus<ColorType, DriverType>;

constexpr int RedPin = 2;
constexpr int GreenPin = 3;
constexpr int BluePin = 4;
constexpr int WarmPin = 5;
constexpr int CoolPin = 6;

LightType::DriverSettingsType makeDriverSettings()
{
    LightType::DriverSettingsType settings{};
    settings.pins[0] = RedPin;
    settings.pins[1] = GreenPin;
    settings.pins[2] = BluePin;
    settings.pins[3] = WarmPin;
    settings.pins[4] = CoolPin;
    settings.invert = false;
    return settings;
}

LightType light(makeDriverSettings());
uint16_t phase = 0;
} // namespace

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
