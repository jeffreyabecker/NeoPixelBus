#include <LumaWave.h>

constexpr uint16_t PixelCount = 20;
using Color = lw::Rgb8Color;
#if defined(ARDUINO_ARCH_RP2040)
using WrappedTransport = lw::OneWireWrapper<lw::RpSpiTransport>;
#endif

static std::unique_ptr<lw::IPixelBus<Color>> gBus;

static std::unique_ptr<lw::IPixelBus<Color>> makeBus()
{
#if defined(ARDUINO_ARCH_RP2040)
    auto settings = WrappedTransport::TransportSettingsType{};
    settings.spiIndex = 0;
    settings.dataPin = 3;
    settings.clockPin = 2;
    settings.clockRateHz = 0;
    settings.timing = lw::timing::Ws2812x;

    auto *transport = new WrappedTransport(settings);
    auto *protocol = new lw::Ws2812xProtocol<Color>(
        PixelCount,
        lw::Ws2812xProtocolSettings{transport, lw::ChannelOrder::GRB::value, lw::timing::Ws2812x});
    return std::make_unique<lw::OwningPixelBusT<Color>>(protocol, transport);
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
        gBus->setPixelColor(index, Color(8, 0, 24));
    }

    gBus->show();
}

void loop()
{
    delay(1000);
}
