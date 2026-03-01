#pragma once

#include <cstdint>
#include <cstddef>
#include <utility>
#include <vector>

#include "ITransport.h"
#include "OneWireTiming.h"

namespace lw
{

    template <typename TSettings, typename = void>
    struct OneWireSettingsHasclockRateHz : std::false_type
    {
    };

    template <typename TSettings>
    struct OneWireSettingsHasclockRateHz<TSettings,
                                         std::void_t<decltype(std::declval<TSettings &>().clockRateHz)>>
        : std::true_type
    {
    };

    template <typename TSettings, typename = void>
    struct OneWireSettingsHasBaudRate : std::false_type
    {
    };

    template <typename TSettings>
    struct OneWireSettingsHasBaudRate<TSettings,
                                      std::void_t<decltype(std::declval<TSettings &>().baudRate)>>
        : std::true_type
    {
    };

    template <typename TTransportSettings>
    void applyOneWireEncodedRateIfUnset(uint32_t encodedRateHz,
                                        TTransportSettings &transportSettings)
    {
        if constexpr (OneWireSettingsHasclockRateHz<TTransportSettings>::value)
        {
            if (transportSettings.clockRateHz == 0)
            {
                transportSettings.clockRateHz = encodedRateHz;
            }
        }

        if constexpr (OneWireSettingsHasBaudRate<TTransportSettings>::value)
        {
            if (transportSettings.baudRate == 0)
            {
                transportSettings.baudRate = encodedRateHz;
            }
        }
    }

    template <typename TTransportSettings>
    void normalizeOneWireTransportClockDataBitRate(const OneWireTiming &timing,
                                                   TTransportSettings &transportSettings)
    {
        applyOneWireEncodedRateIfUnset(timing.encodedDataRateHz(), transportSettings);
    }

    template <typename TTransportSettings>
    struct OneWireWrapperSettings : TTransportSettings
    {
        OneWireTiming timing = timing::Ws2812x;
    };

    template <typename TTransport,
              uint8_t PrefixReset = 0,
              uint8_t SuffixReset = 1,
              bool ProtocolIdleHigh = false,
              typename = std::enable_if_t<TaggedTransportLike<TTransport, TransportTag> &&
                                          SettingsConstructibleTransportLike<TTransport>>>
    class OneWireWrapper : public TTransport
    {
    public:
        using TransportSettingsType = OneWireWrapperSettings<typename TTransport::TransportSettingsType>;
        using TransportCategory = OneWireTransportTag;
        static constexpr uint8_t EncodedOne3Step = 0b110;
        static constexpr uint8_t EncodedZero3Step = 0b100;
        static constexpr uint8_t EncodedOne4Step = 0b1110;
        static constexpr uint8_t EncodedZero4Step = 0b1000;

        

        explicit OneWireWrapper(TransportSettingsType config)
                        : TTransport(toTransportSettings(config)),
                            _bitPattern{config.timing.bitPattern()},
                            _prefixResetBytes{computePrefixResetBytes(config)},
                            _suffixResetBytes{computeResetBytes(config, SuffixReset)}
        {
        }

        void begin() override
        {
            TTransport::begin();
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

        void transmitBytes(span<uint8_t> data) override
        {
            const size_t payloadCapacity = data.size() * encodedBitsPerDataBitFromPattern(_bitPattern);
            const size_t targetSize = _prefixResetBytes + payloadCapacity + _suffixResetBytes;

            ensureEncodedCapacity(targetSize);
            if (_encoded.empty())
            {
                return;
            }

            for (size_t i = 0; i < _prefixResetBytes; ++i)
            {
                _encoded[i] = resetFillByte();
            }

            const size_t encodedSize = (_bitPattern == EncodedClockDataBitPattern::FourStep)
                                           ? encodeStepBytes(_encoded.data() + _prefixResetBytes,
                                                             data.data(),
                                                             data.size(),
                                                             encodedOne4Step(),
                                                             encodedZero4Step(),
                                                             4)
                                           : encodeStepBytes(_encoded.data() + _prefixResetBytes,
                                                             data.data(),
                                                             data.size(),
                                                             encodedOne3Step(),
                                                             encodedZero3Step(),
                                                             3);

            const size_t suffixOffset = _prefixResetBytes + encodedSize;
            for (size_t i = 0; i < _suffixResetBytes; ++i)
            {
                _encoded[suffixOffset + i] = resetFillByte();
            }

            TTransport::transmitBytes(span<uint8_t>(_encoded.data(), suffixOffset + _suffixResetBytes));
        }

        bool isReadyToUpdate() const override
        {
            return TTransport::isReadyToUpdate();
        }

    private:
        EncodedClockDataBitPattern _bitPattern;
        size_t _prefixResetBytes;
        size_t _suffixResetBytes;
        std::vector<uint8_t> _encoded;

        static constexpr uint64_t NsPerSecond = 1000000000ULL;

        

        static void normalizeConfig(TransportSettingsType &config)
        {
            normalizeTransportClockDataBitRate(config);
        }

        static typename TTransport::TransportSettingsType toTransportSettings(TransportSettingsType &config)
        {
            normalizeConfig(config);
            return static_cast<typename TTransport::TransportSettingsType>(config);
        }

        static uint32_t defaultclockRateHz(const TransportSettingsType &config)
        {
            return config.timing.encodedDataRateHz();
        }

        static uint32_t effectiveclockRateHz(const TransportSettingsType &config)
        {
            if constexpr (OneWireSettingsHasclockRateHz<typename TTransport::TransportSettingsType>::value)
            {
                const auto &transportSettings = static_cast<const typename TTransport::TransportSettingsType &>(config);
                if (transportSettings.clockRateHz != 0)
                {
                    return transportSettings.clockRateHz;
                }
            }

            return defaultclockRateHz(config);
        }

        static size_t computeResetBytes(const TransportSettingsType &config,
                                        uint8_t resetMultiplier)
        {
            if (resetMultiplier == 0)
            {
                return 0;
            }

            const uint64_t clockRateHz = static_cast<uint64_t>(effectiveclockRateHz(config));
            if (clockRateHz == 0)
            {
                return 0;
            }

            const uint64_t resetNs = static_cast<uint64_t>(config.timing.resetNs) * static_cast<uint64_t>(resetMultiplier);
            const uint64_t resetBits = (resetNs * clockRateHz + (NsPerSecond - 1ULL)) / NsPerSecond;
            return static_cast<size_t>((resetBits + 7ULL) / 8ULL);
        }

        static size_t computePrefixResetBytes(const TransportSettingsType &config)
        {
            size_t prefixResetBytes = computeResetBytes(config, PrefixReset);
            if constexpr (ProtocolIdleHigh)
            {
                if (prefixResetBytes < 1)
                {
                    prefixResetBytes = 1;
                }
            }

            return prefixResetBytes;
        }

        static void normalizeTransportClockDataBitRate(TransportSettingsType &config)
        {
            auto &transportSettings = static_cast<typename TTransport::TransportSettingsType &>(config);
            normalizeOneWireTransportClockDataBitRate(config.timing, transportSettings);
        }

        static uint8_t encodedBitsPerDataBitFromPattern(EncodedClockDataBitPattern pattern)
        {
            return static_cast<uint8_t>(pattern);
        }

        static constexpr uint8_t resetFillByte()
        {
            return ProtocolIdleHigh ? 0xFF : 0x00;
        }

        static constexpr uint8_t invertEncodedPattern(uint8_t pattern,
                                                      uint8_t bits)
        {
            return static_cast<uint8_t>((~pattern) & ((1u << bits) - 1u));
        }

        static constexpr uint8_t encodedOne3Step()
        {
            if constexpr (ProtocolIdleHigh)
            {
                return invertEncodedPattern(EncodedOne3Step, 3);
            }
            return EncodedOne3Step;
        }

        static constexpr uint8_t encodedZero3Step()
        {
            if constexpr (ProtocolIdleHigh)
            {
                return invertEncodedPattern(EncodedZero3Step, 3);
            }
            return EncodedZero3Step;
        }

        static constexpr uint8_t encodedOne4Step()
        {
            if constexpr (ProtocolIdleHigh)
            {
                return invertEncodedPattern(EncodedOne4Step, 4);
            }
            return EncodedOne4Step;
        }

        static constexpr uint8_t encodedZero4Step()
        {
            if constexpr (ProtocolIdleHigh)
            {
                return invertEncodedPattern(EncodedZero4Step, 4);
            }
            return EncodedZero4Step;
        }

        void ensureEncodedCapacity(size_t targetSize)
        {
            if (_encoded.size() != targetSize)
            {
                _encoded.assign(targetSize, 0);
            }
        }

    };

} // namespace lw

