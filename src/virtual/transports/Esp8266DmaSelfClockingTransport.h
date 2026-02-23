#pragma once

#ifdef ARDUINO_ARCH_ESP8266

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>

#include <Arduino.h>

extern "C"
{
    #include "eagle_soc.h"
    #include "i2s_reg.h"
    #include "slc_register.h"
    #include "esp8266_peri.h"
}

#include "ISelfClockingTransport.h"
#include "SelfClockingTransportConfig.h"

namespace npb
{

    struct Esp8266DmaSelfClockingTransportConfig : SelfClockingTransportConfig
    {
    };

    class Esp8266DmaSelfClockingTransport : public ISelfClockingTransport
    {
    public:
        static constexpr uint8_t I2sPin = 3;
        static constexpr size_t DmaBitsPerPixelBit = 3;
        static constexpr size_t DmaBitsPerByte = 8 * DmaBitsPerPixelBit;
        static constexpr size_t MaxDmaBlockSize = 4092;

        explicit Esp8266DmaSelfClockingTransport(Esp8266DmaSelfClockingTransportConfig config)
            : _config{config}
        {
        }

        ~Esp8266DmaSelfClockingTransport() override
        {
            if (_initialised)
            {
                stopI2s();
            }
            freeI2sBuffers();
        }

        void begin() override
        {
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            ensureInitialised(data.size());
            if (!_i2sBuffer)
            {
                return;
            }

            encodeI2sBuffer(data);
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

        Esp8266DmaSelfClockingTransportConfig _config;
        size_t _frameBytes{0};

        uint8_t *_i2sBuffer{nullptr};
        size_t _i2sBufferSize{0};

        uint8_t *_idleData{nullptr};
        size_t _idleDataSize{0};

        SlcDescriptor *_descriptors{nullptr};
        size_t _descriptorCount{0};

        volatile DmaState _dmaState{DmaState::Idle};
        bool _initialised{false};

        static size_t roundUp4(size_t value)
        {
            return (value + 3) & ~static_cast<size_t>(3);
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
                stopI2s();
                _initialised = false;
            }

            freeI2sBuffers();
            _frameBytes = frameBytes;

            allocateI2sBuffers();
            initI2s();
            _initialised = true;
        }

        void allocateI2sBuffers()
        {
            size_t rawBits = static_cast<size_t>(_frameBytes) * DmaBitsPerByte;
            _i2sBufferSize = roundUp4((rawBits + 7) / 8);

            size_t resetBytes = roundUp4(
                (_config.timing.resetUs * 1000) /
                (_config.timing.bitPeriodNs() * DmaBitsPerPixelBit / 8 + 1) + 4);
            _idleDataSize = resetBytes < 256 ? 256 : resetBytes;

            _i2sBuffer = static_cast<uint8_t *>(malloc(_i2sBufferSize));
            _idleData = static_cast<uint8_t *>(malloc(_idleDataSize));

            uint8_t idleFill = _config.invert ? 0xFF : 0x00;
            if (_i2sBuffer)
            {
                std::memset(_i2sBuffer, idleFill, _i2sBufferSize);
            }
            if (_idleData)
            {
                std::memset(_idleData, idleFill, _idleDataSize);
            }

            size_t dataBlockCount =
                (_i2sBufferSize + MaxDmaBlockSize - 1) / MaxDmaBlockSize;

            _descriptorCount = 2 + dataBlockCount;
            _descriptors = static_cast<SlcDescriptor *>(
                calloc(_descriptorCount, sizeof(SlcDescriptor)));

            _descriptors[0].blocksize = 4;
            _descriptors[0].datalen = 4;
            _descriptors[0].buf_ptr = reinterpret_cast<uint32_t>(_idleData);
            _descriptors[0].owner = 1;
            _descriptors[0].next_link_ptr = reinterpret_cast<uint32_t>(&_descriptors[1]);

            _descriptors[1].blocksize = 4;
            _descriptors[1].datalen = 4;
            _descriptors[1].buf_ptr = reinterpret_cast<uint32_t>(_idleData + 4);
            _descriptors[1].owner = 1;
            _descriptors[1].next_link_ptr = reinterpret_cast<uint32_t>(&_descriptors[0]);

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
            pinMode(I2sPin, FUNCTION_1);

            SLCC0 |= SLCRXLR | SLCTXLR;
            SLCC0 &= ~(SLCRXLR | SLCTXLR);
            SLCIC = 0xFFFFFFFF;
            SLCC0 &= ~(SLCMM << SLCM);
            SLCC0 |= (1 << SLCM);

            SLCRXDC |= SLCBINR | SLCBTNR;
            SLCRXDC &= ~(SLCBRXFE | SLCBRXEM | SLCBRXFM);

            SLCTXL &= ~(SLCTXLAM << SLCTXLA);
            SLCTXL |= reinterpret_cast<uint32_t>(&_descriptors[0]) << SLCTXLA;

            SLCIE = SLCIRXEOF;

            ETS_SLC_INTR_ATTACH(slcIsr, reinterpret_cast<void *>(this));
            ETS_SLC_INTR_ENABLE();

            I2SC = 0;
            I2SC |= I2SRST;
            I2SC &= ~I2SRST;

            I2SFC &= ~(I2SDE | (I2STXFMM << I2STXFM) | (I2SRXFMM << I2SRXFM));

            uint32_t targetHz = 1000000000UL /
                (_config.timing.bitPeriodNs() / DmaBitsPerPixelBit);
            configureClock(targetHz);

            I2SC |= I2STXS;
            SLCTXL |= SLCTXLS;
        }

        void configureClock(uint32_t targetRateHz)
        {
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

        void encodeI2sBuffer(std::span<const uint8_t> data)
        {
            static constexpr uint8_t OneBitNormal  = 0b110;
            static constexpr uint8_t ZeroBitNormal = 0b100;
            static constexpr uint8_t OneBitInverted  = 0b001;
            static constexpr uint8_t ZeroBitInverted = 0b011;

            uint8_t oneBit  = _config.invert ? OneBitInverted  : OneBitNormal;
            uint8_t zeroBit = _config.invert ? ZeroBitInverted : ZeroBitNormal;

            uint32_t *pOut = reinterpret_cast<uint32_t *>(_i2sBuffer);
            uint32_t accum = 0;
            uint8_t bitPos = 0;

            for (size_t i = 0; i < data.size(); ++i)
            {
                uint8_t value = data[i];
                for (uint8_t b = 0; b < 8; ++b)
                {
                    uint8_t pattern = (value & 0x80) ? oneBit : zeroBit;
                    value <<= 1;

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

            if (bitPos > 0)
            {
                accum <<= (32 - bitPos);
                *pOut = accum;
            }
        }

        static void IRAM_ATTR slcIsr(void *arg)
        {
            uint32_t status = SLCIS;
            SLCIC = 0xFFFFFFFF;

            if (status & SLCIRXEOF)
            {
                auto *self = static_cast<Esp8266DmaSelfClockingTransport *>(arg);
                self->_descriptors[1].next_link_ptr =
                    reinterpret_cast<uint32_t>(&self->_descriptors[0]);
                self->_dmaState = DmaState::Idle;
            }
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP8266
