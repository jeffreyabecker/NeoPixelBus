#include <Arduino.h>
#include <LumaWave.h>

#if !defined(ARDUINO_ARCH_RP2040)
#error "This example requires an RP2040 target."
#endif

using namespace lw;

namespace
{
    using ColorType = Rgb8Color;
    using DriverType = RpPwmLightDriver<ColorType>;
    using LightType = LightBus<ColorType, DriverType>;

    constexpr int RedPin = 2;
    constexpr int GreenPin = 3;
    constexpr int BluePin = 4;

    RpPwmLightDriverSettings makeDriverSettings()
    {
        RpPwmLightDriverSettings settings{};
        settings.pins[0] = RedPin;
        settings.pins[1] = GreenPin;
        settings.pins[2] = BluePin;
        settings.wrap = 255;
        settings.clockDiv = 4.0f;
        settings.invert = false;
        return settings;
    }

    LightType light(makeDriverSettings());

    uint8_t intensity = 0;
    bool goingUp = true;
}

void setup()
{
    light.begin();
}

void loop()
{
    auto &pixel = light.pixel();
    pixel['R'] = intensity;
    pixel['G'] = static_cast<uint8_t>(255U - intensity);
    pixel['B'] = intensity / 2U;

    light.show();

    if (goingUp)
    {
        if (intensity == 255U)
        {
            goingUp = false;
        }
        else
        {
            ++intensity;
        }
    }
    else
    {
        if (intensity == 0U)
        {
            goingUp = true;
        }
        else
        {
            --intensity;
        }
    }

    delay(8);
}
