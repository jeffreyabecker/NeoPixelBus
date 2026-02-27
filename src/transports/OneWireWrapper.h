#pragma once

#include <cstdint>
#include <cstddef>
#include <utility>
#include <vector>

#include "ITransport.h"
#include "OneWireTiming.h"

namespace npb
{

    template <typename TTransportSettings>
    struct OneWireWrapperSettings : TTransportSettings
    {
        OneWireTiming timing = timing::Ws2812x;
    };

    template <typename TTransport,
              typename = std::enable_if_t<TaggedTransportLike<TTransport, TransportTag> &&
                                          SettingsConstructibleTransportLike<TTransport>>>
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
                        : TTransport(toTransportSettings(config)),
                            _bitPattern{config.timing.bitPattern()}
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

        void transmitBytes(span<const uint8_t> data) override
        {
            ensureEncodedCapacity(data.size());
            if (_encoded.empty())
            {
                return;
            }

            const size_t encodedSize = (_bitPattern == EncodedClockDataBitPattern::FourStep)
                                           ? encode4StepBytes(_encoded.data(), data.data(), data.size())
                                           : encode3StepBytes(_encoded.data(), data.data(), data.size());

            TTransport::transmitBytes(span<const uint8_t>(_encoded.data(), encodedSize));
        }

        bool isReadyToUpdate() const override
        {
            return TTransport::isReadyToUpdate();
        }

    private:
        EncodedClockDataBitPattern _bitPattern;
        std::vector<uint8_t> _encoded;

        template <typename TSettings, typename = void>
        struct HasclockRateHz : std::false_type
        {
        };

        template <typename TSettings>
        struct HasclockRateHz<TSettings,
                                     std::void_t<decltype(std::declval<TSettings &>().clockRateHz)>>
            : std::true_type
        {
        };

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
            const uint32_t bitRateHz = static_cast<uint32_t>(config.timing.bitRateHz());
            return bitRateHz * encodedBitsPerDataBitFromPattern(config.timing.bitPattern());
        }

        static void normalizeTransportClockDataBitRate(TransportSettingsType &config)
        {
            auto &transportSettings = static_cast<typename TTransport::TransportSettingsType &>(config);
            if constexpr (HasclockRateHz<typename TTransport::TransportSettingsType>::value)
            {
                if (transportSettings.clockRateHz == 0)
                {
                    transportSettings.clockRateHz = defaultclockRateHz(config);
                }
            }
        }

        static uint8_t encodedBitsPerDataBitFromPattern(EncodedClockDataBitPattern pattern)
        {
            return static_cast<uint8_t>(pattern);
        }

        void ensureEncodedCapacity(size_t sourceBytes)
        {
            const size_t targetSize = sourceBytes * encodedBitsPerDataBitFromPattern(_bitPattern);
            if (_encoded.size() != targetSize)
            {
                _encoded.assign(targetSize, 0);
            }
        }

    };

} // namespace npb

