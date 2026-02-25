#pragma once

#include <cstdint>
#include <cstddef>
#include <concepts>
#include <span>
#include <utility>
#include <vector>

#include <Arduino.h>

#include "ITransport.h"
#include "OneWireTiming.h"

namespace npb
{

    enum class EncodedClockDataBitPattern : uint8_t
    {
        ThreeStep = 3,
        FourStep = 4
    };

    template <typename TTransportSettings>
    struct OneWireWrapperSettings : TTransportSettings
    {
        uint32_t clockDataBitRateHz = 0;
        bool manageTransaction = true;
        EncodedClockDataBitPattern bitPattern = EncodedClockDataBitPattern::ThreeStep;
        OneWireTiming timing = timing::Ws2812x;
    };

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag> &&
                     SettingsConstructibleTransportLike<TTransport>
    class OneWireWrapper : public ITransport, public TTransport
    {
    public:
        using TransportSettingsType = OneWireWrapperSettings<typename TTransport::TransportSettingsType>;
        using TransportCategory = OneWireTransportTag;
        static constexpr uint8_t EncodedOne3Step = 0b110;
        static constexpr uint8_t EncodedZero3Step = 0b100;
        static constexpr uint8_t EncodedOne4Step = 0b1110;
        static constexpr uint8_t EncodedZero4Step = 0b1000;

        explicit OneWireWrapper(TransportSettingsType config)
            : TTransport(static_cast<typename TTransport::TransportSettingsType &&>(config)), _config{std::move(config)}
        {
        }

        void begin() override
        {
            TTransport::begin();
            _frameDurationUs = 0;
            _frameEndTimeUs = micros();
        }

        static size_t encode3StepBytes(uint8_t *dest,
                                       const uint8_t *src,
                                       size_t srcSize)
        {
            return encodeStepBytes(dest, src, srcSize, EncodedOne3Step, EncodedZero3Step, 3);
        }

        static size_t encode4StepBytes(uint8_t *dest,
                                       const uint8_t *src,
                                       size_t srcSize)
        {
            return encodeStepBytes(dest, src, srcSize, EncodedOne4Step, EncodedZero4Step, 4);
        }

        static size_t encodeStepBytes(uint8_t *dest,
                                      const uint8_t *src,
                                      size_t srcSize,
                                      uint8_t encodedOne,
                                      uint8_t encodedZero,
                                      uint8_t encodedBitsPerDataBit)
        {
            uint8_t current = 0;
            uint8_t bitsInCurrent = 0;
            size_t outIndex = 0;

            for (size_t i = 0; i < srcSize; ++i)
            {
                uint8_t value = src[i];

                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    const uint8_t encoded = (value & 0x80) ? encodedOne : encodedZero;
                    value <<= 1;

                    current = static_cast<uint8_t>((current << encodedBitsPerDataBit) | encoded);
                    bitsInCurrent += encodedBitsPerDataBit;

                    if (bitsInCurrent >= 8)
                    {
                        dest[outIndex++] = current;
                        current = 0;
                        bitsInCurrent = 0;
                    }
                }
            }

            return outIndex;
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            ensureEncodedCapacity(data.size());
            if (_encoded.empty())
            {
                return;
            }

            const size_t encodedSize = (_config.bitPattern == EncodedClockDataBitPattern::FourStep)
                                           ? encode4StepBytes(_encoded.data(), data.data(), data.size())
                                           : encode3StepBytes(_encoded.data(), data.data(), data.size());

            if (_config.manageTransaction)
            {
                TTransport::beginTransaction();
            }

            TTransport::transmitBytes(std::span<const uint8_t>(_encoded.data(), encodedSize));

            if (_config.manageTransaction)
            {
                TTransport::endTransaction();
            }

            updateFrameTiming(data.size());
        }

        bool isReadyToUpdate() const override
        {
            const bool transportReady = TTransport::isReadyToUpdate();
            const bool resetReady = (micros() - _frameEndTimeUs) >= _frameDurationUs;
            return transportReady && resetReady;
        }

    private:
        TransportSettingsType _config;
        std::vector<uint8_t> _encoded;
        uint32_t _frameDurationUs{0};
        uint32_t _frameEndTimeUs{0};

        static uint8_t encodedBitsPerDataBitFromPattern(EncodedClockDataBitPattern pattern)
        {
            return static_cast<uint8_t>(pattern);
        }

        void ensureEncodedCapacity(size_t sourceBytes)
        {
            const size_t targetSize = sourceBytes * encodedBitsPerDataBitFromPattern(_config.bitPattern);
            if (_encoded.size() != targetSize)
            {
                _encoded.assign(targetSize, 0);
            }
        }

        void updateFrameTiming(size_t sourceBytes)
        {
            if (_config.clockDataBitRateHz == 0)
            {
                _frameDurationUs = _config.timing.resetUs;
            }
            else
            {
                const uint32_t encodedBits = static_cast<uint32_t>(sourceBytes) * 8U * encodedBitsPerDataBitFromPattern(_config.bitPattern);
                const uint32_t encodedUs = static_cast<uint32_t>(
                    (static_cast<uint64_t>(encodedBits) * 1000000ULL) / _config.clockDataBitRateHz);
                _frameDurationUs = (encodedUs > _config.timing.resetUs) ? encodedUs : _config.timing.resetUs;
            }

            _frameEndTimeUs = micros();
        }
    };

    template <typename TTransport>
    using OneWireTransport = OneWireWrapper<TTransport>;

} // namespace npb
