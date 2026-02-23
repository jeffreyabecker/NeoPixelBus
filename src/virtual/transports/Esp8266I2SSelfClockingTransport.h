#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "ISelfClockingTransport.h"
#include "SelfClockingTransportConfig.h"

namespace npb
{

#ifndef NEOPIXELBUS_ESP8266_DMX_BREAK_DEFAULT_US
#define NEOPIXELBUS_ESP8266_DMX_BREAK_DEFAULT_US 96U
#endif
    static constexpr uint16_t Esp8266DmxBreakDefaultUs = NEOPIXELBUS_ESP8266_DMX_BREAK_DEFAULT_US;

#ifndef NEOPIXELBUS_ESP8266_DMX_MAB_DEFAULT_US
#define NEOPIXELBUS_ESP8266_DMX_MAB_DEFAULT_US 12U
#endif
    static constexpr uint16_t Esp8266DmxMabDefaultUs = NEOPIXELBUS_ESP8266_DMX_MAB_DEFAULT_US;

    struct Esp8266I2SSelfClockingTransportConfig : SelfClockingTransportConfig
    {
        uint16_t breakUs = Esp8266DmxBreakDefaultUs;
        uint16_t markAfterBreakUs = Esp8266DmxMabDefaultUs;
    };

    class Esp8266I2SSelfClockingTransport : public ISelfClockingTransport
    {
    public:
        explicit Esp8266I2SSelfClockingTransport(Esp8266I2SSelfClockingTransportConfig config = {})
            : _config{config}
        {
        }

        void begin() override
        {
            _lastFrame.clear();
            _frameDurationUs = 0;
            _frameEndTimeUs = micros();
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            _lastFrame.assign(data.begin(), data.end());

            const float bitsPerSecond = _config.timing.bitRateHz();
            const uint32_t payloadBits = static_cast<uint32_t>(_lastFrame.size()) * 10U; // 8N2 style slot framing
            const uint32_t payloadUs = (bitsPerSecond > 0.0f)
                ? static_cast<uint32_t>((static_cast<float>(payloadBits) * 1000000.0f) / bitsPerSecond)
                : 0U;

            _frameDurationUs = std::max<uint32_t>(
                _config.timing.resetUs,
                static_cast<uint32_t>(_config.breakUs) + static_cast<uint32_t>(_config.markAfterBreakUs) + payloadUs);
            _frameEndTimeUs = micros();
        }

        bool isReadyToUpdate() const override
        {
            return (micros() - _frameEndTimeUs) >= _frameDurationUs;
        }

    private:
        Esp8266I2SSelfClockingTransportConfig _config;
        std::vector<uint8_t> _lastFrame;
        uint32_t _frameDurationUs{0};
        uint32_t _frameEndTimeUs{0};
    };

} // namespace npb
