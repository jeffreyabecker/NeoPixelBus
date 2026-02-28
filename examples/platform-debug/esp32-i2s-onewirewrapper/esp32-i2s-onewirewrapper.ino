#include <NeoPixelBus.h>

constexpr uint16_t PixelCount = 20;
using Color = npb::Rgb8Color;
#if defined(ARDUINO_ARCH_ESP32)
using WrappedTransport = npb::OneWireWrapper<npb::Esp32I2sTransport>;
#endif

static std::unique_ptr<npb::IPixelBus<Color>> gBus;

static std::unique_ptr<npb::IPixelBus<Color>> makeBus()
{
#if defined(ARDUINO_ARCH_ESP32)
    auto settings = WrappedTransport::TransportSettingsType{};
    settings.dataPin = 13;
    settings.clockPin = 14;
    settings.busNumber = 0;
    settings.clockRateHz = 0;
    settings.timing = npb::timing::Ws2812x;

    auto *transport = new WrappedTransport(settings);
    auto *protocol = new npb::Ws2812xProtocol<Color>(
        PixelCount,
        npb::Ws2812xProtocolSettings{transport, npb::ChannelOrder::GRB::value, npb::timing::Ws2812x});
    return std::make_unique<npb::OwningPixelBusT<Color>>(protocol, transport);
#endif

    return {};
}

void setup()
{
    gBus = makeBus();

    if (gBus == nullptr)
    {
        return;
    }

    gBus->begin();

    for (size_t index = 0; index < gBus->pixelCount(); ++index)
    {
        gBus->setPixelColor(index, Color(32, 0, 0));
    }

    gBus->show();
}

void loop()
{
    delay(1000);
}
