#pragma once

// nRF52840 PWM one-wire emitter.
// Uses one of the nRF52 PWM peripherals (PWM0–PWM3) with DMA to generate
// NRZ-encoded waveforms for one-wire LED protocols.
//
// The PWM runs at 16 MHz (1 tick = 0.0625 µs).  Each pixel bit becomes one
// PWM cycle whose duty cycle encodes a 0- or 1-bit.  The nRF52 PWM sample
// format uses bit 15 (0x8000) to select output polarity:
//   0x8000 | duty → output starts LOW, goes HIGH after `duty` ticks
//                    (normal for WS2812x — idle LOW)
//   0x0000 | duty → output starts HIGH, goes LOW after `duty` ticks
//                    (normal for TM1814 — idle HIGH)

#if defined(ARDUINO_ARCH_NRF52840)

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>
#include <nrf_pwm.h>

#include "IEmitPixels.h"
#include "ColorOrderTransform.h"
#include "OneWireTiming.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../colors/Color.h"

namespace npb
{

    /// Construction settings for Nrf52PwmOneWireEmitter.
    struct Nrf52PwmOneWireEmitterSettings
    {
        uint8_t pin;
        uint8_t pwmIndex = 2;              // 0–3 (PWM0–PWM3)
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;
        ColorOrderTransformConfig colorConfig;
    };

    /// One-wire NRZ emitter using nRF52840 PWM + DMA.
    class Nrf52PwmOneWireEmitter : public IEmitPixels
    {
    public:
        static constexpr uint32_t PwmClockHz = 16000000UL;
        static constexpr double TickUs = 1.0 / 16.0; // 0.0625 µs per tick
        static constexpr size_t BytesPerSample = sizeof(uint16_t); // nrf_pwm_values_common_t
        static constexpr size_t SamplesPerByte = 8; // 8 PWM cycles per pixel byte

        Nrf52PwmOneWireEmitter(uint16_t pixelCount,
                               ResourceHandle<IShader> shader,
                               Nrf52PwmOneWireEmitterSettings settings)
            : _settings{settings}
            , _shader{std::move(shader)}
            , _transform{settings.colorConfig}
            , _pixelCount{pixelCount}
            , _sizeData{_transform.bytesNeeded(pixelCount)}
            , _scratchColors(pixelCount)
        {
            _data = static_cast<uint8_t *>(malloc(_sizeData));
            if (_data)
            {
                std::memset(_data, 0, _sizeData);
            }

            computeTimingConstants();
        }

        ~Nrf52PwmOneWireEmitter()
        {
            if (_initialised)
            {
                NRF_PWM_Type *pwm = getPwm();
                // Wait for completion
                while (!pwm->EVENTS_STOPPED)
                {
                    yield();
                }
                pwm->PSEL.OUT[0] = 0xFFFFFFFF; // NC
                pwm->ENABLE = 0;
                pinMode(_settings.pin, INPUT);
            }
            free(_data);
            free(_dmaBuffer);
        }

        Nrf52PwmOneWireEmitter(const Nrf52PwmOneWireEmitter &) = delete;
        Nrf52PwmOneWireEmitter &operator=(const Nrf52PwmOneWireEmitter &) = delete;
        Nrf52PwmOneWireEmitter(Nrf52PwmOneWireEmitter &&) = delete;
        Nrf52PwmOneWireEmitter &operator=(Nrf52PwmOneWireEmitter &&) = delete;

        void initialize() override
        {
            if (_initialised)
            {
                return;
            }

            allocateDmaBuffer();

            // Set idle level before connecting PWM
            pinMode(_settings.pin, OUTPUT);
            digitalWrite(_settings.pin,
                         _settings.invert ? HIGH : LOW);

            NRF_PWM_Type *pwm = getPwm();

            // Connect pin to channel 0 only
            pwm->PSEL.OUT[0] = digitalPinToPinName(_settings.pin);
            pwm->PSEL.OUT[1] = 0xFFFFFFFF; // NC
            pwm->PSEL.OUT[2] = 0xFFFFFFFF;
            pwm->PSEL.OUT[3] = 0xFFFFFFFF;

            pwm->MODE = PWM_MODE_UPDOWN_Up;
            pwm->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_1; // 16 MHz
            pwm->COUNTERTOP = _countTop;
            pwm->LOOP = 1; // single-shot: SEQ[0] → SEQ[1] → LOOPSDONE

            pwm->DECODER = PWM_DECODER_LOAD_Common |
                           PWM_DECODER_MODE_RefreshCount;

            // SEQ[0] = pixel data + trailing BitReset
            pwm->SEQ[0].PTR = reinterpret_cast<uint32_t>(_dmaBuffer);
            pwm->SEQ[0].CNT = _dmaBufferCount;
            pwm->SEQ[0].REFRESH = 0;
            pwm->SEQ[0].ENDDELAY = _countReset;

            // SEQ[1] = single BitReset sample (idle during reset gap)
            pwm->SEQ[1].PTR = reinterpret_cast<uint32_t>(
                &_dmaBuffer[_dmaBufferCount - 1]);
            pwm->SEQ[1].CNT = 1;
            pwm->SEQ[1].REFRESH = 0;
            pwm->SEQ[1].ENDDELAY = 0;

            // Short: LOOPSDONE → STOP
            pwm->SHORTS = PWM_SHORTS_LOOPSDONE_STOP_Msk;
            pwm->INTEN = 0; // no interrupts — we poll

            pwm->ENABLE = PWM_ENABLE_ENABLE_Enabled;

            _initialised = true;
        }

        void update(std::span<const Color> colors) override
        {
            while (!isReadyToUpdate())
            {
                yield();
            }

            // Shade
            std::span<const Color> source = colors;
            if (nullptr != _shader)
            {
                std::copy(colors.begin(), colors.end(), _scratchColors.begin());
                _shader->apply(_scratchColors);
                source = _scratchColors;
            }

            // Transform
            _transform.apply(
                std::span<uint8_t>{_data, _sizeData}, source);

            // Fill DMA buffer
            fillDmaBuffer();

            // Start PWM
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
        Nrf52PwmOneWireEmitterSettings _settings;
        ResourceHandle<IShader> _shader;
        ColorOrderTransform _transform;
        uint16_t _pixelCount;
        size_t _sizeData;

        std::vector<Color> _scratchColors;
        uint8_t *_data{nullptr};

        uint16_t *_dmaBuffer{nullptr};
        size_t _dmaBufferCount{0}; // number of uint16_t samples

        // Precomputed timing
        uint16_t _countTop{0};
        uint16_t _bit0{0};
        uint16_t _bit1{0};
        uint16_t _bitReset{0};
        uint16_t _countReset{0};

        bool _initialised{false};

        // ---- Timing computation -----------------------------------------

        void computeTimingConstants()
        {
            const auto &t = _settings.timing;

            // CountTop = bit period in 16 MHz ticks
            _countTop = static_cast<uint16_t>(
                (t.bitPeriodNs() * PwmClockHz + 500000000ULL) / 1000000000ULL);

            // Bit0 duty = T0H in ticks
            uint16_t bit0Duty = static_cast<uint16_t>(
                (static_cast<uint64_t>(t.t0hNs) * PwmClockHz + 500000000ULL)
                / 1000000000ULL);

            // Bit1 duty = T1H in ticks
            uint16_t bit1Duty = static_cast<uint16_t>(
                (static_cast<uint64_t>(t.t1hNs) * PwmClockHz + 500000000ULL)
                / 1000000000ULL);

            // Polarity flag: 0x8000 means "start LOW, compare goes HIGH"
            // For normal (idle LOW) NeoPixel: set 0x8000
            // For inverted (idle HIGH): clear 0x8000
            uint16_t polarityFlag = _settings.invert
                ? static_cast<uint16_t>(0x0000)
                : static_cast<uint16_t>(0x8000);

            _bit0 = bit0Duty | polarityFlag;
            _bit1 = bit1Duty | polarityFlag;
            _bitReset = polarityFlag; // duty = 0, just the polarity (idle level)

            // CountReset = number of bit periods for reset gap
            uint32_t bitPeriodUs = t.bitPeriodNs() / 1000;
            if (bitPeriodUs == 0)
            {
                bitPeriodUs = 1;
            }
            _countReset = static_cast<uint16_t>(t.resetUs / bitPeriodUs);
        }

        // ---- DMA buffer -------------------------------------------------

        void allocateDmaBuffer()
        {
            // 8 samples per data byte + 1 trailing BitReset
            _dmaBufferCount = _sizeData * SamplesPerByte + 1;
            _dmaBuffer = static_cast<uint16_t *>(
                malloc(_dmaBufferCount * BytesPerSample));
            if (_dmaBuffer)
            {
                // Fill with BitReset
                for (size_t i = 0; i < _dmaBufferCount; ++i)
                {
                    _dmaBuffer[i] = _bitReset;
                }
            }
        }

        void fillDmaBuffer()
        {
            if (!_dmaBuffer || !_data)
            {
                return;
            }

            uint16_t *pOut = _dmaBuffer;

            for (size_t i = 0; i < _sizeData; ++i)
            {
                uint8_t value = _data[i];
                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    *pOut++ = (value & 0x80) ? _bit1 : _bit0;
                    value <<= 1;
                }
            }

            // Trailing BitReset
            *pOut = _bitReset;
        }

        // ---- PWM peripheral lookup --------------------------------------

        NRF_PWM_Type *getPwm() const
        {
            static NRF_PWM_Type *const pwms[] =
            {
                NRF_PWM0,
                NRF_PWM1,
                NRF_PWM2,
#if defined(NRF_PWM3)
                NRF_PWM3,
#endif
            };
            uint8_t idx = _settings.pwmIndex;
            if (idx >= (sizeof(pwms) / sizeof(pwms[0])))
            {
                idx = 0;
            }
            return pwms[idx];
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_NRF52840
