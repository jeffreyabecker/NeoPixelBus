#include <NeoPixelBus.h>

constexpr uint16_t PixelCount = 20;
using ColorType = npb::Rgb8Color;
using WrappedTransport = npb::OneWireWrapper<npb::Esp32I2sTransport>;

static npb::Ws2812xProtocol<ColorType>* gProtocol = nullptr;
static npb::IPixelBus<ColorType>* gBus = nullptr;

static void initializeBus()
{
    if (gBus != nullptr)
    {
        return;
    }

#if defined(ARDUINO_ARCH_ESP32)
    auto settings = WrappedTransport::TransportSettingsType{};
    settings.dataPin = 13;
    settings.clockPin = 14;
    settings.busNumber = 0;
    settings.clockRateHz = 0;
    settings.timing = npb::timing::Ws2812x;

    auto* transport = new WrappedTransport(settings);
    gProtocol = new npb::Ws2812xProtocol<ColorType>(
        PixelCount,
        npb::Ws2812xProtocolSettings{transport, npb::ChannelOrder::GRB, npb::timing::Ws2812x});
    gBus = new npb::PixelBusT<ColorType>(*gProtocol);
#endif
}

void setup()
{
    initializeBus();

    if (gBus == nullptr)
    {
        return;
    }

    gBus->begin();

    for (size_t index = 0; index < gBus->pixelCount(); ++index)
    {
        gBus->setPixelColor(index, ColorType(32, 0, 0));
    }

    gBus->show();
}

void loop()
{
    delay(1000);
}
