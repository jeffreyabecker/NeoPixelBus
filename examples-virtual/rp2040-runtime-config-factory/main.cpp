#include <Arduino.h>
#include <NeoPixelBus.h>

#include <array>
#include <cctype>
#include <cstring>
#include <memory>

#ifdef ARDUINO_ARCH_RP2040

namespace
{

    enum class RuntimeProtocolId : uint8_t
    {
        Ws2812,
        Debug,
    };

    struct RuntimeBusConfig
    {
        uint16_t pixelCount = 8;
        RuntimeProtocolId protocol = RuntimeProtocolId::Ws2812;
        const char *channelOrder = npb::ChannelOrder::GRB;
        uint8_t dataPin = 16;
        uint8_t pioIndex = 1;
        bool invert = false;
    };

    using RuntimeBusPtr = npb::factory::Ws2812BusPtr;

    RuntimeBusPtr g_bus;

    RuntimeProtocolId readProtocolChoice(Print &output,
                                         uint32_t timeoutMs)
    {
        output.println("Type 'debug' to use DebugProtocol+PrintTransport; press Enter for Ws2812+RpPioOneWire.");

        std::array<char, 16> buffer{};
        size_t length = 0;
        const uint32_t start = millis();

        while ((millis() - start) < timeoutMs)
        {
            while (Serial.available() > 0)
            {
                const int raw = Serial.read();
                if (raw < 0)
                {
                    continue;
                }

                const char c = static_cast<char>(raw);
                if (c == '\r' || c == '\n')
                {
                    if (length == 0)
                    {
                        return RuntimeProtocolId::Ws2812;
                    }

                    buffer[length] = '\0';
                    if (std::tolower(static_cast<unsigned char>(buffer[0])) == 'd')
                    {
                        return RuntimeProtocolId::Debug;
                    }

                    return RuntimeProtocolId::Ws2812;
                }

                if (length + 1 < buffer.size())
                {
                    buffer[length++] = c;
                }
            }

            delay(5);
        }

        return RuntimeProtocolId::Ws2812;
    }

    RuntimeBusPtr makeRuntimeBus(const RuntimeBusConfig &config,
                                 Print &debugOutput)
    {
        if (config.protocol == RuntimeProtocolId::Ws2812)
        {
            const size_t channelCount = strlen(config.channelOrder);
            const Ws2812 protocolConfig{
                .colorOrder = config.channelOrder};

            const RpPioOneWire transportConfig{
                .settings = {
                    .pin = config.dataPin,
                    .pioIndex = config.pioIndex,
                    .frameBytes = config.pixelCount * channelCount,
                    .invert = config.invert,
                    .timing = npb::timing::Ws2812x}};

            TransportPtr myTransport = makeTransport(transportConfig);
            ProtocolPtr<Ws2812> myProtocol = makeProtocol(config.pixelCount, protocolConfig, myTransport);
            return makeBus(std::move(myProtocol), std::move(myTransport));
        }

        if (config.protocol == RuntimeProtocolId::Debug)
        {
            const DebugProtocolConfig<npb::Rgb8Color> protocolConfig{
                .settings = {
                    .output = &debugOutput,
                    .invert = config.invert,
                    .protocol = nullptr}};

            const PrintTransportConfig transportConfig{
                .settings = {
                    .output = &debugOutput,
                    .invert = false}};

            TransportPtr myTransport = makeTransport(transportConfig);
            ProtocolPtr<DebugProtocolConfig<npb::Rgb8Color>> myProtocol = makeProtocol(config.pixelCount, protocolConfig, myTransport);
            return makeBus(std::move(myProtocol), std::move(myTransport));
        }

        return nullptr;
    }

} // namespace

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    RuntimeBusConfig config{};
    config.protocol = readProtocolChoice(Serial, 5000);

    g_bus = makeRuntimeBus(config, Serial);
    if (!g_bus)
    {
        Serial.println("Unsupported runtime protocol/transport configuration.");
        return;
    }

    g_bus->begin();

    Serial.println("Runtime factory bus created.");
}

void loop()
{
    if (!g_bus)
    {
        delay(1000);
        return;
    }

    static uint8_t value = 0;

    for (size_t i = 0; i < g_bus->pixelCount(); ++i)
    {
        g_bus->setPixelColor(i, npb::Rgb8Color{0, 0, 0});
    }

    const size_t index = (value / 16) % g_bus->pixelCount();
    g_bus->setPixelColor(index, npb::Rgb8Color{value, static_cast<uint8_t>(255 - value), 32});
    g_bus->show();

    value += 4;
    delay(40);
}

#else

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    Serial.println("This example requires ARDUINO_ARCH_RP2040");
}

void loop()
{
    delay(1000);
}

#endif