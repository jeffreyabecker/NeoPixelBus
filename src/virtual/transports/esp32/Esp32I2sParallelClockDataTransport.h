#pragma once

#if defined(ARDUINO_ARCH_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S3) && !defined(CONFIG_IDF_TARGET_ESP32C3)

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <array>

#include <Arduino.h>
#include "esp_heap_caps.h"

extern "C"
{
    #include "../../../original/internal/methods/platform/esp32/Esp32_i2s.h"
}

#include "../ISelfClockingTransport.h"
#include "../SelfClockingTransportConfig.h"
#include "../IClockDataTransport.h"
#include "../IParallelDataTransport.h"

namespace npb
{

    struct Esp32I2sParallelSelfClockingTransportConfig : SelfClockingTransportConfig
    {
        uint8_t busNumber = 1;
    };

    class Esp32I2sParallelContext
    {
    public:
        static constexpr size_t MaxChannels = 8;
        static constexpr size_t DmaBitsPerPixelBit = 3;

        uint8_t registerChannel(size_t channelDataSize)
        {
            uint8_t id = _nextMuxId++;
            if (channelDataSize > _maxDataSize)
            {
                _maxDataSize = channelDataSize;
            }
            _registeredMask |= (1u << id);
            return id;
        }

        void unregisterChannel(uint8_t muxId, uint8_t pin, uint8_t busNumber)
        {
            _registeredMask &= ~(1u << muxId);

            gpio_matrix_out(pin, SIG_GPIO_OUT_IDX, false, false);
            pinMode(pin, INPUT);

            if (_registeredMask == 0 && _initialised)
            {
                while (!i2sWriteDone(busNumber))
                {
                    yield();
                }
                i2sDeinit(busNumber);
                if (_dmaBuffer)
                {
                    heap_caps_free(_dmaBuffer);
                    _dmaBuffer = nullptr;
                }
                _initialised = false;
            }
        }

        void initialize(uint8_t busNumber, uint16_t bitSendTimeNs,
                        uint8_t pin, uint8_t muxId, bool invert)
        {
            if (!_initialised)
            {
                _dmaBufferSize = roundUp4(
                    MaxChannels * DmaBitsPerPixelBit * _maxDataSize);
                _dmaBuffer = static_cast<uint8_t *>(
                    heap_caps_malloc(_dmaBufferSize, MALLOC_CAP_DMA));
                if (_dmaBuffer)
                {
                    std::memset(_dmaBuffer, 0, _dmaBufferSize);
                }

                size_t dmaBlockCount =
                    (_dmaBufferSize + I2S_DMA_MAX_DATA_LEN - 1) / I2S_DMA_MAX_DATA_LEN;

                i2sInit(busNumber,
                        true,
                        1,
                        DmaBitsPerPixelBit,
                        bitSendTimeNs,
                        I2S_CHAN_RIGHT_TO_LEFT,
                        I2S_FIFO_16BIT_SINGLE,
                        dmaBlockCount,
                        _dmaBuffer,
                        _dmaBufferSize);

                _initialised = true;
            }

            i2sSetPins(busNumber, pin, muxId, 1, invert);
        }

        void clearIfNeeded()
        {
            if (_updatedMask == 0 && _dmaBuffer)
            {
                std::memset(_dmaBuffer, 0, _dmaBufferSize);
            }
        }

        void encodeChannel(const uint8_t *data, size_t sizeData, uint8_t muxId)
        {
            if (!_dmaBuffer)
            {
                return;
            }

            uint8_t muxBit = (1u << muxId);
            uint8_t *pDma = _dmaBuffer;

            for (size_t i = 0; i < sizeData; ++i)
            {
                uint8_t value = data[i];
                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    bool isOne = (value & 0x80) != 0;
                    value <<= 1;

                    pDma[0] |= muxBit;
                    if (isOne)
                    {
                        pDma[1] |= muxBit;
                    }

                    pDma += DmaBitsPerPixelBit;
                }
            }

            _updatedMask |= (1u << muxId);
        }

        bool allChannelsUpdated() const
        {
            return (_updatedMask & _registeredMask) == _registeredMask;
        }

        void startWrite(uint8_t busNumber)
        {
            _updatedMask = 0;
            i2sWrite(busNumber);
        }

        bool isWriteDone(uint8_t busNumber) const
        {
            return i2sWriteDone(busNumber);
        }

    private:
        uint8_t *_dmaBuffer{nullptr};
        size_t _dmaBufferSize{0};
        size_t _maxDataSize{0};
        uint8_t _nextMuxId{0};
        uint8_t _registeredMask{0};
        uint8_t _updatedMask{0};
        bool _initialised{false};

        static size_t roundUp4(size_t value)
        {
            return (value + 3) & ~static_cast<size_t>(3);
        }
    };

    class Esp32I2sParallelSelfClockingTransport : public ISelfClockingTransport
    {
    public:
        explicit Esp32I2sParallelSelfClockingTransport(Esp32I2sParallelSelfClockingTransportConfig config)
            : _config{config}
        {
        }

        ~Esp32I2sParallelSelfClockingTransport() override
        {
            if (_registered)
            {
                context(_config.busNumber).unregisterChannel(_muxId, _config.pin, _config.busNumber);
            }
        }

        void begin() override
        {
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            ensureChannelReady(data.size());

            auto &ctx = context(_config.busNumber);
            ctx.clearIfNeeded();
            ctx.encodeChannel(data.data(), data.size(), _muxId);

            if (ctx.allChannelsUpdated())
            {
                ctx.startWrite(_config.busNumber);
            }
        }

        bool isReadyToUpdate() const override
        {
            return context(_config.busNumber).isWriteDone(_config.busNumber);
        }

    private:
        Esp32I2sParallelSelfClockingTransportConfig _config;
        uint8_t _muxId{0};
        bool _registered{false};

        void ensureChannelReady(size_t frameBytes)
        {
            auto &ctx = context(_config.busNumber);

            if (!_registered)
            {
                _muxId = ctx.registerChannel(frameBytes);
                _registered = true;

                ctx.initialize(
                    _config.busNumber,
                    static_cast<uint16_t>(_config.timing.bitPeriodNs()),
                    _config.pin,
                    _muxId,
                    _config.invert);
            }
        }

        static Esp32I2sParallelContext &context(uint8_t busNumber)
        {
            static Esp32I2sParallelContext SharedContext[2];
            return SharedContext[busNumber & 1];
        }
    };

    struct Esp32I2sParallelClockDataLaneConfig
    {
        int8_t pin = -1;
        bool invert = false;
    };

    struct Esp32I2sParallelClockDataTransportConfig
    {
        uint8_t busNumber = 1;
        uint16_t bitSendTimeNs = 400;
        std::array<Esp32I2sParallelClockDataLaneConfig, Esp32I2sParallelContext::MaxChannels> lanes{};
        uint8_t laneMask = 0xFF;
    };

    class Esp32I2sParallelClockDataContext
    {
    public:
        static constexpr size_t MaxChannels = Esp32I2sParallelContext::MaxChannels;
        static constexpr size_t DmaBitsPerClockDataBit = 1;

        void registerLane(uint8_t lane,
                          int8_t pin,
                          bool invert,
                          size_t frameBytes,
                          uint8_t busNumber,
                          uint16_t bitSendTimeNs)
        {
            if (lane >= MaxChannels || pin < 0)
            {
                return;
            }

            _lanePins[lane] = pin;
            _laneInvert[lane] = invert;
            _registeredMask |= static_cast<uint8_t>(1u << lane);

            bool needsReinit = false;
            if (frameBytes > _maxFrameBytes)
            {
                _maxFrameBytes = frameBytes;
                needsReinit = true;
            }

            if (!_initialised || needsReinit)
            {
                reinitialize(busNumber, bitSendTimeNs);
            }

            i2sSetPins(busNumber, static_cast<uint8_t>(pin), lane, 1, invert);
        }

        void unregisterLane(uint8_t lane, uint8_t busNumber)
        {
            if (lane >= MaxChannels)
            {
                return;
            }

            _registeredMask &= static_cast<uint8_t>(~(1u << lane));

            if (_lanePins[lane] >= 0)
            {
                gpio_matrix_out(_lanePins[lane], SIG_GPIO_OUT_IDX, false, false);
                pinMode(_lanePins[lane], INPUT);
                _lanePins[lane] = -1;
            }

            if (_registeredMask == 0)
            {
                teardown(busNumber);
            }
        }

        void beginLaneTransaction(uint8_t lane)
        {
            if (!_frameOpen)
            {
                _expectedMask = _registeredMask;
                _begunMask = 0;
                _writtenMask = 0;
                _endedMask = 0;
                _frameOpen = true;
                if (_dmaBuffer)
                {
                    std::memset(_dmaBuffer, 0, _dmaBufferSize);
                }
            }

            _begunMask |= static_cast<uint8_t>(1u << lane);
        }

        void writeLane(uint8_t lane, std::span<const uint8_t> data)
        {
            if (!_dmaBuffer || lane >= MaxChannels)
            {
                return;
            }

            uint8_t muxBit = static_cast<uint8_t>(1u << lane);
            uint8_t* pDma = _dmaBuffer;

            for (size_t i = 0; i < data.size(); ++i)
            {
                uint8_t value = data[i];
                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    if ((value & 0x80) != 0)
                    {
                        pDma[0] |= muxBit;
                    }
                    value <<= 1;
                    pDma += DmaBitsPerClockDataBit;
                }
            }

            _writtenMask |= static_cast<uint8_t>(1u << lane);
        }

        void endLaneTransaction(uint8_t lane, uint8_t busNumber)
        {
            _endedMask |= static_cast<uint8_t>(1u << lane);

            if (!_frameOpen)
            {
                return;
            }

            const uint8_t expected = _expectedMask;
            const bool allEnded = (_endedMask & expected) == expected;
            const bool allWritten = (_writtenMask & expected) == expected;

            if (allEnded && allWritten)
            {
                _frameOpen = false;
                _begunMask = 0;
                _writtenMask = 0;
                _endedMask = 0;
                i2sWrite(busNumber);
            }
        }

        bool isWriteDone(uint8_t busNumber) const
        {
            if (!_initialised)
            {
                return true;
            }
            return i2sWriteDone(busNumber);
        }

        bool isReady(uint8_t busNumber) const
        {
            if (_frameOpen)
            {
                return false;
            }
            return isWriteDone(busNumber);
        }

    private:
        uint8_t* _dmaBuffer{nullptr};
        size_t _dmaBufferSize{0};
        size_t _maxFrameBytes{0};
        uint8_t _registeredMask{0};
        uint8_t _expectedMask{0};
        uint8_t _begunMask{0};
        uint8_t _writtenMask{0};
        uint8_t _endedMask{0};
        bool _frameOpen{false};
        bool _initialised{false};
        std::array<int8_t, MaxChannels> _lanePins{ { -1, -1, -1, -1, -1, -1, -1, -1 } };
        std::array<bool, MaxChannels> _laneInvert{ { false, false, false, false, false, false, false, false } };

        static size_t roundUp4(size_t value)
        {
            return (value + 3) & ~static_cast<size_t>(3);
        }

        void reinitialize(uint8_t busNumber, uint16_t bitSendTimeNs)
        {
            if (_initialised)
            {
                while (!i2sWriteDone(busNumber))
                {
                    yield();
                }
                i2sDeinit(busNumber);
                _initialised = false;
            }

            if (_dmaBuffer)
            {
                heap_caps_free(_dmaBuffer);
                _dmaBuffer = nullptr;
            }

            const size_t frameBytes = (_maxFrameBytes == 0) ? 1 : _maxFrameBytes;
            _dmaBufferSize = roundUp4(MaxChannels * DmaBitsPerClockDataBit * frameBytes * 8);
            _dmaBuffer = static_cast<uint8_t*>(
                heap_caps_malloc(_dmaBufferSize, MALLOC_CAP_DMA));
            if (_dmaBuffer)
            {
                std::memset(_dmaBuffer, 0, _dmaBufferSize);
            }

            size_t dmaBlockCount =
                (_dmaBufferSize + I2S_DMA_MAX_DATA_LEN - 1) / I2S_DMA_MAX_DATA_LEN;

            i2sInit(busNumber,
                    true,
                    1,
                    DmaBitsPerClockDataBit,
                    bitSendTimeNs,
                    I2S_CHAN_RIGHT_TO_LEFT,
                    I2S_FIFO_16BIT_SINGLE,
                    dmaBlockCount,
                    _dmaBuffer,
                    _dmaBufferSize);

            for (uint8_t lane = 0; lane < MaxChannels; ++lane)
            {
                if ((_registeredMask & static_cast<uint8_t>(1u << lane)) && _lanePins[lane] >= 0)
                {
                    i2sSetPins(busNumber,
                               static_cast<uint8_t>(_lanePins[lane]),
                               lane,
                               1,
                               _laneInvert[lane]);
                }
            }

            _initialised = true;
        }

        void teardown(uint8_t busNumber)
        {
            if (_initialised)
            {
                while (!i2sWriteDone(busNumber))
                {
                    yield();
                }
                i2sDeinit(busNumber);
                _initialised = false;
            }

            if (_dmaBuffer)
            {
                heap_caps_free(_dmaBuffer);
                _dmaBuffer = nullptr;
            }

            _dmaBufferSize = 0;
            _maxFrameBytes = 0;
            _frameOpen = false;
            _expectedMask = 0;
            _begunMask = 0;
            _writtenMask = 0;
            _endedMask = 0;
        }
    };

    class Esp32I2sParallelClockDataTransport : public IParallelDataTransport
    {
    public:
        static constexpr size_t MaxChannels = Esp32I2sParallelClockDataContext::MaxChannels;

        class LaneTransport : public IClockDataTransport
        {
        public:
            LaneTransport() = default;

            void bind(Esp32I2sParallelClockDataTransport* parent, uint8_t lane)
            {
                _parent = parent;
                _lane = lane;
            }

            void begin() override
            {
            }

            void beginTransaction() override
            {
                if (!_parent)
                {
                    return;
                }

                ensureRegistered((_lastFrameBytes == 0) ? 1 : _lastFrameBytes);
                _parent->context().beginLaneTransaction(_lane);
            }

            void transmitBytes(std::span<const uint8_t> data) override
            {
                if (!_parent)
                {
                    return;
                }

                _lastFrameBytes = data.size();
                ensureRegistered(_lastFrameBytes);
                _parent->context().writeLane(_lane, data);
            }

            void endTransaction() override
            {
                if (_parent)
                {
                    _parent->context().endLaneTransaction(_lane, _parent->_config.busNumber);
                }
            }

            bool isReadyToUpdate() const override
            {
                if (!_parent)
                {
                    return true;
                }
                return _parent->isReadyToUpdate();
            }

        private:
            Esp32I2sParallelClockDataTransport* _parent{nullptr};
            uint8_t _lane{0};
            size_t _lastFrameBytes{0};
            bool _registered{false};

            void ensureRegistered(size_t frameBytes)
            {
                if (!_parent)
                {
                    return;
                }

                const auto& laneConfig = _parent->_config.lanes[_lane];
                _parent->context().registerLane(_lane,
                                                laneConfig.pin,
                                                laneConfig.invert,
                                                frameBytes,
                                                _parent->_config.busNumber,
                                                _parent->_config.bitSendTimeNs);
                _registered = true;
            }

            friend class Esp32I2sParallelClockDataTransport;
        };

        explicit Esp32I2sParallelClockDataTransport(Esp32I2sParallelClockDataTransportConfig config)
            : _config{config}
        {
            for (uint8_t lane = 0; lane < MaxChannels; ++lane)
            {
                _lanes[lane].bind(this, lane);
            }
        }

        ~Esp32I2sParallelClockDataTransport() override
        {
            for (uint8_t lane = 0; lane < MaxChannels; ++lane)
            {
                if ((_config.laneMask & static_cast<uint8_t>(1u << lane)) &&
                    _config.lanes[lane].pin >= 0)
                {
                    context().unregisterLane(lane, _config.busNumber);
                }
            }
        }

        void begin() override
        {
        }

        ResourceHandle<IClockDataTransport> getLane(uint8_t lane) override
        {
            if (lane >= MaxChannels)
            {
                return ResourceHandle<IClockDataTransport>{};
            }

            if ((_config.laneMask & static_cast<uint8_t>(1u << lane)) == 0)
            {
                return ResourceHandle<IClockDataTransport>{};
            }

            if (_config.lanes[lane].pin < 0)
            {
                return ResourceHandle<IClockDataTransport>{};
            }

            return ResourceHandle<IClockDataTransport>{_lanes[lane]};
        }

        bool isReadyToUpdate() const override
        {
            return context().isReady(_config.busNumber);
        }

    private:
        Esp32I2sParallelClockDataTransportConfig _config;
        std::array<LaneTransport, MaxChannels> _lanes{};

        static Esp32I2sParallelClockDataContext& context(uint8_t busNumber)
        {
            static Esp32I2sParallelClockDataContext SharedContext[2];
            return SharedContext[busNumber & 1];
        }

        Esp32I2sParallelClockDataContext& context() const
        {
            return context(_config.busNumber);
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP32 && !ESP32S3 && !ESP32C3
