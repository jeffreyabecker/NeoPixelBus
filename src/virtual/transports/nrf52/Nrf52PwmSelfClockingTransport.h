#pragma once

#if defined(ARDUINO_ARCH_NRF52840)

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <span>

#include <Arduino.h>
#include <nrf_pwm.h>

#include "../ITransport.h"
#include "../OneWireTiming.h"

namespace npb
{

    struct Nrf52PwmOneWireTransportConfig
    {
        uint8_t pin = 0;
        bool invert = false;
        OneWireTiming timing = timing::Ws2812x;
        uint8_t pwmIndex = 2;
    };

    class Nrf52PwmOneWireTransport : public ITransport
    {
    public:
        using TransportCategory = SelfClockingTransportTag;
        static constexpr uint32_t PwmClockHz = 16000000UL;
        static constexpr size_t BytesPerSample = sizeof(uint16_t);
        static constexpr size_t SamplesPerByte = 8;

        explicit Nrf52PwmOneWireTransport(Nrf52PwmOneWireTransportConfig config)
            : _config{config}
        {
            computeTimingConstants();
        }

        ~Nrf52PwmOneWireTransport() override
        {
            if (_initialised)
            {
                NRF_PWM_Type *pwm = getPwm();
                while (!pwm->EVENTS_STOPPED)
                {
                    yield();
                }
                pwm->PSEL.OUT[0] = 0xFFFFFFFF;
                pwm->ENABLE = 0;
                pinMode(_config.pin, INPUT);
            }

            free(_dmaBuffer);
        }

        void begin() override
        {
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            ensureInitialised(data.size());
            if (!_dmaBuffer)
            {
                return;
            }

            fillDmaBuffer(data);

            NRF_PWM_Type *pwm = getPwm();
            pwm->EVENTS_LOOPSDONE = 0;
            pwm->EVENTS_SEQEND[0] = 0;
            pwm->EVENTS_SEQEND[1] = 0;
            pwm->EVENTS_STOPPED = 0;
            pwm->TASKS_SEQSTART[0] = 1;
        }

        bool isReadyToUpdate() const override
        {
            if (!_initialised)
            {
                return true;
            }
            return getPwm()->EVENTS_STOPPED != 0;
        }

    private:
        Nrf52PwmOneWireTransportConfig _config;

        uint16_t *_dmaBuffer{nullptr};
        size_t _dmaBufferCount{0};
        size_t _frameBytes{0};

        uint16_t _countTop{0};
        uint16_t _bit0{0};
        uint16_t _bit1{0};
        uint16_t _bitReset{0};
        uint16_t _countReset{0};

        bool _initialised{false};

        void computeTimingConstants()
        {
            const auto &t = _config.timing;

            _countTop = static_cast<uint16_t>(
                (t.bitPeriodNs() * PwmClockHz + 500000000ULL) / 1000000000ULL);

            uint16_t bit0Duty = static_cast<uint16_t>(
                (static_cast<uint64_t>(t.t0hNs) * PwmClockHz + 500000000ULL) / 1000000000ULL);

            uint16_t bit1Duty = static_cast<uint16_t>(
                (static_cast<uint64_t>(t.t1hNs) * PwmClockHz + 500000000ULL) / 1000000000ULL);

            uint16_t polarityFlag = _config.invert
                                        ? static_cast<uint16_t>(0x0000)
                                        : static_cast<uint16_t>(0x8000);

            _bit0 = bit0Duty | polarityFlag;
            _bit1 = bit1Duty | polarityFlag;
            _bitReset = polarityFlag;

            uint32_t bitPeriodUs = t.bitPeriodNs() / 1000;
            if (bitPeriodUs == 0)
            {
                bitPeriodUs = 1;
            }
            _countReset = static_cast<uint16_t>(t.resetUs / bitPeriodUs);
        }

        void ensureInitialised(size_t frameBytes)
        {
            if (_initialised && _frameBytes == frameBytes)
            {
                return;
            }

            if (_initialised)
            {
                while (!isReadyToUpdate())
                {
                    yield();
                }
                getPwm()->ENABLE = 0;
                _initialised = false;
            }

            _frameBytes = frameBytes;
            allocateDmaBuffer();

            pinMode(_config.pin, OUTPUT);
            digitalWrite(_config.pin, _config.invert ? HIGH : LOW);

            NRF_PWM_Type *pwm = getPwm();
            pwm->PSEL.OUT[0] = digitalPinToPinName(_config.pin);
            pwm->PSEL.OUT[1] = 0xFFFFFFFF;
            pwm->PSEL.OUT[2] = 0xFFFFFFFF;
            pwm->PSEL.OUT[3] = 0xFFFFFFFF;

            pwm->MODE = PWM_MODE_UPDOWN_Up;
            pwm->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_1;
            pwm->COUNTERTOP = _countTop;
            pwm->LOOP = 1;

            pwm->DECODER = PWM_DECODER_LOAD_Common |
                           PWM_DECODER_MODE_RefreshCount;

            pwm->SEQ[0].PTR = reinterpret_cast<uint32_t>(_dmaBuffer);
            pwm->SEQ[0].CNT = _dmaBufferCount;
            pwm->SEQ[0].REFRESH = 0;
            pwm->SEQ[0].ENDDELAY = _countReset;

            pwm->SEQ[1].PTR = reinterpret_cast<uint32_t>(
                &_dmaBuffer[_dmaBufferCount - 1]);
            pwm->SEQ[1].CNT = 1;
            pwm->SEQ[1].REFRESH = 0;
            pwm->SEQ[1].ENDDELAY = 0;

            pwm->SHORTS = PWM_SHORTS_LOOPSDONE_STOP_Msk;
            pwm->INTEN = 0;
            pwm->ENABLE = PWM_ENABLE_ENABLE_Enabled;

            _initialised = true;
        }

        void allocateDmaBuffer()
        {
            free(_dmaBuffer);
            _dmaBuffer = nullptr;

            _dmaBufferCount = _frameBytes * SamplesPerByte + 1;
            _dmaBuffer = static_cast<uint16_t *>(
                malloc(_dmaBufferCount * BytesPerSample));
            if (_dmaBuffer)
            {
                for (size_t i = 0; i < _dmaBufferCount; ++i)
                {
                    _dmaBuffer[i] = _bitReset;
                }
            }
        }

        void fillDmaBuffer(std::span<const uint8_t> data)
        {
            uint16_t *pOut = _dmaBuffer;
            for (size_t i = 0; i < data.size(); ++i)
            {
                uint8_t value = data[i];
                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    *pOut++ = (value & 0x80) ? _bit1 : _bit0;
                    value <<= 1;
                }
            }

            *pOut = _bitReset;
        }

        NRF_PWM_Type *getPwm() const
        {
            static NRF_PWM_Type *const PwmTable[] =
                {
                    NRF_PWM0,
                    NRF_PWM1,
                    NRF_PWM2,
#if defined(NRF_PWM3)
                    NRF_PWM3,
#endif
                };

            uint8_t index = _config.pwmIndex;
            if (index >= (sizeof(PwmTable) / sizeof(PwmTable[0])))
            {
                index = 0;
            }
            return PwmTable[index];
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_NRF52840
