#pragma once

// ESP8266 I2S/DMA one-wire emitter.
// Uses the I2S peripheral + SLC DMA engine on the ESP8266.
// Fixed output pin: GPIO3 (I2S data out, shared with Serial RX).
// Only ONE instance may exist at a time (singleton hardware).
//
// 3-step cadence encoding: each NeoPixel bit → 3 I2S bits.
//   1-bit → 110   (normal) / 001 (inverted)
//   0-bit → 100   (normal) / 011 (inverted)

#ifdef ARDUINO_ARCH_ESP8266

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

extern "C"
{
    #include "eagle_soc.h"
    #include "i2s_reg.h"
    #include "slc_register.h"
    #include "esp8266_peri.h"
}

#include "IProtocol.h"
#include "ColorOrderTransform.h"
#include "OneWireTiming.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../colors/Color.h"

namespace npb
{

    /// Construction settings for Esp8266DmaOneWireProtocol.
    struct Esp8266DmaOneWireProtocolSettings
    {
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;
        ColorOrderTransformConfig colorConfig;
        // Pin is always GPIO3 (I2S data out) — not configurable
    };

    /// One-wire NRZ emitter using ESP8266 I2S + SLC DMA.
    ///
    /// Only one instance may exist (the I2S peripheral is a singleton).
    /// Output is always on GPIO3.
    class Esp8266DmaOneWireProtocol : public IProtocol
    {
    public:
        static constexpr uint8_t I2sPin = 3;
        static constexpr size_t DmaBitsPerPixelBit = 3;
        static constexpr size_t DmaBitsPerByte = 8 * DmaBitsPerPixelBit; // 24
        static constexpr size_t DmaBytesAlignment = 4; // I2S sends 4-byte words
        static constexpr size_t MaxDmaBlockSize = 4092; // 4095 rounded down for alignment

        Esp8266DmaOneWireProtocol(uint16_t pixelCount,
                                 ResourceHandle<IShader> shader,
                                 Esp8266DmaOneWireProtocolSettings settings)
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
        }

        ~Esp8266DmaOneWireProtocol()
        {
            if (_initialised)
            {
                stopI2s();
            }
            free(_data);
            freeI2sBuffers();
        }

        Esp8266DmaOneWireProtocol(const Esp8266DmaOneWireProtocol &) = delete;
        Esp8266DmaOneWireProtocol &operator=(const Esp8266DmaOneWireProtocol &) = delete;
        Esp8266DmaOneWireProtocol(Esp8266DmaOneWireProtocol &&) = delete;
        Esp8266DmaOneWireProtocol &operator=(Esp8266DmaOneWireProtocol &&) = delete;

        void initialize() override
        {
            if (_initialised)
            {
                return;
            }

            allocateI2sBuffers();
            initI2s();
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

            // Transform to bytes
            _transform.apply(
                std::span<uint8_t>{_data, _sizeData}, source);

            // Encode into I2S buffer
            encodeI2sBuffer();

            // Start DMA
            writeI2s();
        }

        bool isReadyToUpdate() const override
        {
            return _dmaState == DmaState::Idle;
        }

    private:
        enum class DmaState : uint8_t
        {
            Idle,
            Sending
        };

        Esp8266DmaOneWireProtocolSettings _settings;
        ResourceHandle<IShader> _shader;
        ColorOrderTransform _transform;
        uint16_t _pixelCount;
        size_t _sizeData;

        std::vector<Color> _scratchColors;
        uint8_t *_data{nullptr};

        // I2S / DMA state
        uint8_t *_i2sBuffer{nullptr};
        size_t _i2sBufferSize{0};

        uint8_t *_idleData{nullptr};
        size_t _idleDataSize{0};

        // SLC DMA descriptors
        struct SlcDescriptor
        {
            uint32_t blocksize : 12;
            uint32_t datalen   : 12;
            uint32_t unused    : 5;
            uint32_t sub_sof   : 1;
            uint32_t eof       : 1;
            uint32_t owner     : 1;
            uint32_t buf_ptr;
            uint32_t next_link_ptr;
        };

        SlcDescriptor *_descriptors{nullptr};
        size_t _descriptorCount{0};

        volatile DmaState _dmaState{DmaState::Idle};
        bool _initialised{false};

        // ---- I2S / DMA helpers ------------------------------------------

        static size_t roundUp4(size_t v)
        {
            return (v + 3) & ~static_cast<size_t>(3);
        }

        void allocateI2sBuffers()
        {
            // DMA I2S buffer: 3 bits per pixel bit, 8 pixel bits per byte
            // packed into bytes, then rounded to 4-byte alignment
            size_t rawBits = static_cast<size_t>(_sizeData) * DmaBitsPerByte;
            _i2sBufferSize = roundUp4((rawBits + 7) / 8);

            // Add reset / idle padding
            size_t resetBytes = roundUp4(
                (_settings.timing.resetUs * 1000) /
                (_settings.timing.bitPeriodNs() * DmaBitsPerPixelBit / 8 + 1) + 4);
            _idleDataSize = resetBytes < 256 ? 256 : resetBytes;

            _i2sBuffer = static_cast<uint8_t *>(malloc(_i2sBufferSize));
            _idleData = static_cast<uint8_t *>(malloc(_idleDataSize));

            uint8_t idleFill = _settings.invert ? 0xFF : 0x00;
            if (_i2sBuffer)
            {
                std::memset(_i2sBuffer, idleFill, _i2sBufferSize);
            }
            if (_idleData)
            {
                std::memset(_idleData, idleFill, _idleDataSize);
            }

            // Build descriptor chain
            // Layout: [idle state 0] → [idle state 1] → [data blocks...] → [idle state 0]
            // When idle: loops on state 0 → state 1
            // When writing: state 1.next → data[0], data[last].next → state 0

            size_t dataBlockCount =
                (_i2sBufferSize + MaxDmaBlockSize - 1) / MaxDmaBlockSize;
            size_t idleBlockCount = (_idleDataSize + MaxDmaBlockSize - 1) / MaxDmaBlockSize;

            _descriptorCount = 2 + dataBlockCount;  // 2 state blocks + data

            _descriptors = static_cast<SlcDescriptor *>(
                calloc(_descriptorCount, sizeof(SlcDescriptor)));

            // State block 0 (loops to state 1 by default)
            _descriptors[0].blocksize = 4;
            _descriptors[0].datalen = 4;
            _descriptors[0].buf_ptr = reinterpret_cast<uint32_t>(_idleData);
            _descriptors[0].owner = 1;
            _descriptors[0].next_link_ptr = reinterpret_cast<uint32_t>(&_descriptors[1]);

            // State block 1 (loops back to state 0 by default → idle loop)
            _descriptors[1].blocksize = 4;
            _descriptors[1].datalen = 4;
            _descriptors[1].buf_ptr = reinterpret_cast<uint32_t>(_idleData + 4);
            _descriptors[1].owner = 1;
            _descriptors[1].next_link_ptr = reinterpret_cast<uint32_t>(&_descriptors[0]);

            // Data blocks
            size_t remaining = _i2sBufferSize;
            uint8_t *pBuf = _i2sBuffer;
            for (size_t i = 0; i < dataBlockCount; ++i)
            {
                size_t blockLen = (remaining > MaxDmaBlockSize) ? MaxDmaBlockSize : remaining;
                auto &desc = _descriptors[2 + i];
                desc.blocksize = static_cast<uint32_t>(blockLen);
                desc.datalen   = static_cast<uint32_t>(blockLen);
                desc.buf_ptr   = reinterpret_cast<uint32_t>(pBuf);
                desc.owner     = 1;
                desc.eof       = (i == dataBlockCount - 1) ? 1 : 0;
                desc.next_link_ptr = (i == dataBlockCount - 1)
                    ? reinterpret_cast<uint32_t>(&_descriptors[0])
                    : reinterpret_cast<uint32_t>(&_descriptors[2 + i + 1]);
                pBuf += blockLen;
                remaining -= blockLen;
            }
        }

        void freeI2sBuffers()
        {
            free(_i2sBuffer);
            _i2sBuffer = nullptr;
            free(_idleData);
            _idleData = nullptr;
            free(_descriptors);
            _descriptors = nullptr;
        }

        void initI2s()
        {
            // Set GPIO3 as I2S data output
            pinMode(I2sPin, FUNCTION_1);

            // SLC DMA config
            SLCC0 |= SLCRXLR | SLCTXLR;       // reset SLC
            SLCC0 &= ~(SLCRXLR | SLCTXLR);
            SLCIC = 0xFFFFFFFF;                 // clear all SLC interrupts
            SLCC0 &= ~(SLCMM << SLCM);
            SLCC0 |= (1 << SLCM);              // SLC mode 1

            SLCRXDC |= SLCBINR | SLCBTNR;      // no-change mode
            SLCRXDC &= ~(SLCBRXFE | SLCBRXEM | SLCBRXFM);

            // Set the descriptor link
            SLCTXL &= ~(SLCTXLAM << SLCTXLA);
            SLCTXL |= reinterpret_cast<uint32_t>(&_descriptors[0]) << SLCTXLA;

            SLCIE = SLCIRXEOF;                  // enable RX EOF interrupt

            ETS_SLC_INTR_ATTACH(slcIsr, reinterpret_cast<void *>(this));
            ETS_SLC_INTR_ENABLE();

            // I2S config
            I2SC = 0;
            I2SC |= I2SRST;                    // reset
            I2SC &= ~I2SRST;

            I2SFC &= ~(I2SDE | (I2STXFMM << I2STXFM) | (I2SRXFMM << I2SRXFM));

            // I2S clock: find best divisors for target bit rate
            uint32_t targetHz = 1000000000UL /
                (_settings.timing.bitPeriodNs() / DmaBitsPerPixelBit);
            configureClock(targetHz);

            I2SC |= I2STXS;                    // start transmit

            // Start the idle DMA loop
            SLCTXL |= SLCTXLS;
        }

        void configureClock(uint32_t targetRateHz)
        {
            // ESP8266 I2S clock: base / (bclk_div * clk_div)
            // base = I2SBASEFREQ (160 MHz or 80 MHz depending on CPU)
            static constexpr uint32_t BaseFreq = 160000000UL;
            uint8_t bestBclkDiv = 1;
            uint8_t bestClkDiv = 1;
            int32_t bestError = INT32_MAX;

            for (uint8_t bclk = 1; bclk <= 63; ++bclk)
            {
                for (uint8_t clk = 1; clk <= 63; ++clk)
                {
                    uint32_t rate = BaseFreq / (bclk * clk);
                    int32_t error = static_cast<int32_t>(rate) -
                                    static_cast<int32_t>(targetRateHz);
                    if (error < 0)
                    {
                        error = -error;
                    }
                    if (error < bestError)
                    {
                        bestError = error;
                        bestBclkDiv = bclk;
                        bestClkDiv = clk;
                    }
                }
            }

            I2SC &= ~(I2SBMM << I2SBM);
            I2SC |= (bestBclkDiv << I2SBM);
            I2SC &= ~(I2SCDM << I2SCD);
            I2SC |= (bestClkDiv << I2SCD);
        }

        void writeI2s()
        {
            _dmaState = DmaState::Sending;

            // Re-link: state block 1 → data[0]
            _descriptors[1].next_link_ptr =
                reinterpret_cast<uint32_t>(&_descriptors[2]);
        }

        void stopI2s()
        {
            ETS_SLC_INTR_DISABLE();
            SLCTXL &= ~SLCTXLS;
            I2SC &= ~I2STXS;
            pinMode(I2sPin, INPUT);
        }

        // ---- Encoding ---------------------------------------------------

        void encodeI2sBuffer()
        {
            if (!_i2sBuffer || !_data)
            {
                return;
            }

            // 3-bit patterns per NeoPixel bit
            static constexpr uint8_t OneBitNormal  = 0b110;
            static constexpr uint8_t ZeroBitNormal = 0b100;
            static constexpr uint8_t OneBitInverted  = 0b001;
            static constexpr uint8_t ZeroBitInverted = 0b011;

            uint8_t oneBit  = _settings.invert ? OneBitInverted  : OneBitNormal;
            uint8_t zeroBit = _settings.invert ? ZeroBitInverted : ZeroBitNormal;

            // Pack 3-bit codes into the I2S buffer, MSB first
            uint32_t *pOut = reinterpret_cast<uint32_t *>(_i2sBuffer);
            uint32_t accum = 0;
            uint8_t bitPos = 0; // bits accumulated in current 32-bit word

            for (size_t i = 0; i < _sizeData; ++i)
            {
                uint8_t value = _data[i];
                for (uint8_t b = 0; b < 8; ++b)
                {
                    uint8_t pattern = (value & 0x80) ? oneBit : zeroBit;
                    value <<= 1;

                    // Shift 3 bits into accumulator (MSB-first within word)
                    for (uint8_t p = 0; p < 3; ++p)
                    {
                        accum <<= 1;
                        accum |= (pattern >> (2 - p)) & 1;
                        bitPos++;
                        if (bitPos == 32)
                        {
                            *pOut++ = accum;
                            accum = 0;
                            bitPos = 0;
                        }
                    }
                }
            }

            // Flush remaining bits
            if (bitPos > 0)
            {
                accum <<= (32 - bitPos);
                *pOut = accum;
            }
        }

        // ---- ISR --------------------------------------------------------

        static void IRAM_ATTR slcIsr(void *arg)
        {
            uint32_t status = SLCIS;
            SLCIC = 0xFFFFFFFF; // clear all

            if (status & SLCIRXEOF)
            {
                auto *self = static_cast<Esp8266DmaOneWireProtocol *>(arg);
                // Re-link state 1 → state 0 (idle loop)
                self->_descriptors[1].next_link_ptr =
                    reinterpret_cast<uint32_t>(&self->_descriptors[0]);
                self->_dmaState = DmaState::Idle;
            }
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP8266
