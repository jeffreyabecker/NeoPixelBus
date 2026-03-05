#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include "buses/Topology.h"
#include "colors/IShader.h"
#include "colors/NilShader.h"
#include "core/IPixelBus.h"
#include "protocols/IProtocol.h"
#include "transports/ITransport.h"

namespace lw
{

    template <typename TProtocol,
              typename TTransport,
              typename TShader = NilShader<typename TProtocol::ColorType>>
    class StaticBusDriverPixelBusT : public IPixelBus<typename TProtocol::ColorType>
    {
    public:
        using ProtocolType = TProtocol;
        using TransportType = TTransport;
        using ShaderType = TShader;
        using ColorType = typename ProtocolType::ColorType;
        using ProtocolSettingsType = typename ProtocolType::SettingsType;
        using TransportSettingsType = typename TransportType::TransportSettingsType;

        static_assert(!std::is_same<ProtocolSettingsType, void>::value,
                      "Protocol settings type must not be void.");
        static_assert(std::is_convertible<ProtocolType *, IProtocol<ColorType> *>::value,
                      "Protocol type must derive from IProtocol<ColorType>.");
        static_assert(std::is_convertible<TransportType *, ITransport *>::value,
                      "Transport type must derive from ITransport.");
        static_assert(std::is_convertible<ShaderType *, IShader<ColorType> *>::value,
                      "Shader type must derive from IShader<ColorType>.");

        StaticBusDriverPixelBusT(size_t pixelCount,
                                 ProtocolSettingsType protocolSettings,
                                 TransportSettingsType transportSettings,
                                 ShaderType shaderInstance)
            : _pixelCount(normalizePixelCount(pixelCount))
            , _topology(Topology::linear(_pixelCount))
            , _transport(normalizeTransportSettings(std::move(transportSettings), _pixelCount))
            , _protocol(makeProtocol(_pixelCount,
                                     _transport,
                                     normalizeProtocolSettings(std::move(protocolSettings))))
            , _shader(std::move(shaderInstance))
            , _rootPixels(_pixelCount)
            , _shaderScratch(_pixelCount)
            , _protocolBuffer(_protocol.requiredBufferSizeBytes(), static_cast<uint8_t>(0))
        {
        }

        void begin() override
        {
            _transport.begin();
            _protocol.begin();
        }

        void show() override
        {
            if (!_dirty && !_protocol.alwaysUpdate())
            {
                return;
            }

            if (!_transport.isReadyToUpdate())
            {
                return;
            }

            span<const ColorType> protocolInput{};
            if (!_rootPixels.empty())
            {
                std::copy(_rootPixels.begin(),
                          _rootPixels.end(),
                          _shaderScratch.begin());

                span<ColorType> shaderSpan{_shaderScratch.data(), _shaderScratch.size()};
                _shader.apply(shaderSpan);
                protocolInput = shaderSpan;
            }

            span<uint8_t> protocolBytes{};
            if (!_protocolBuffer.empty())
            {
                protocolBytes = span<uint8_t>{_protocolBuffer.data(), _protocolBuffer.size()};
            }

            _protocol.update(protocolInput, protocolBytes);

            if (!protocolBytes.empty())
            {
                _transport.beginTransaction();
                _transport.transmitBytes(protocolBytes);
                _transport.endTransaction();
            }

            _dirty = false;
        }

        bool isReadyToUpdate() const override
        {
            return _transport.isReadyToUpdate();
        }

        span<ColorType> pixelBuffer() override
        {
            _dirty = true;
            return rootPixels();
        }

        span<const ColorType> pixelBuffer() const override
        {
            return rootPixels();
        }

        const Topology *topologyOrNull() const override
        {
            return &_topology;
        }

        size_t pixelCount() const
        {
            return _pixelCount;
        }

        span<ColorType> rootPixels()
        {
            return span<ColorType>{_rootPixels.data(), _rootPixels.size()};
        }

        span<const ColorType> rootPixels() const
        {
            return span<const ColorType>{_rootPixels.data(), _rootPixels.size()};
        }

        span<ColorType> shaderScratch()
        {
            return span<ColorType>{_shaderScratch.data(), _shaderScratch.size()};
        }

        span<const ColorType> shaderScratch() const
        {
            return span<const ColorType>{_shaderScratch.data(), _shaderScratch.size()};
        }

        span<uint8_t> protocolBuffer()
        {
            return span<uint8_t>{_protocolBuffer.data(), _protocolBuffer.size()};
        }

        span<const uint8_t> protocolBuffer() const
        {
            return span<const uint8_t>{_protocolBuffer.data(), _protocolBuffer.size()};
        }

        ProtocolType &protocol()
        {
            return _protocol;
        }

        const ProtocolType &protocol() const
        {
            return _protocol;
        }

        TransportType &transport()
        {
            return _transport;
        }

        const TransportType &transport() const
        {
            return _transport;
        }

        ShaderType &shader()
        {
            return _shader;
        }

        const ShaderType &shader() const
        {
            return _shader;
        }

    private:
        static size_t normalizePixelCount(size_t pixelCount)
        {
            const size_t maxPixelCount = static_cast<size_t>(std::numeric_limits<uint16_t>::max());
            return (pixelCount <= maxPixelCount) ? pixelCount : maxPixelCount;
        }

        static ProtocolType makeProtocol(size_t pixelCount,
                                         TransportType &transport,
                                         ProtocolSettingsType settings)
        {
            const uint16_t protocolPixelCount = static_cast<uint16_t>(pixelCount);
            if constexpr (std::is_constructible<ProtocolType,
                                                uint16_t,
                                                ProtocolSettingsType>::value)
            {
                return ProtocolType{protocolPixelCount, std::move(settings)};
            }
            else if constexpr (std::is_constructible<ProtocolType,
                                                     uint16_t,
                                                     ProtocolSettingsType,
                                                     TransportType &>::value)
            {
                return ProtocolType{protocolPixelCount, std::move(settings), transport};
            }
            else
            {
                static_assert(std::is_constructible<ProtocolType,
                                                    uint16_t,
                                                    ProtocolSettingsType>::value ||
                                  std::is_constructible<ProtocolType,
                                                        uint16_t,
                                                        ProtocolSettingsType,
                                                        TransportType &>::value,
                              "Protocol must be constructible with settings (with or without transport reference).");
                return ProtocolType{protocolPixelCount, std::move(settings)};
            }
        }

        template <typename TSettings,
                  typename = void>
        struct ProtocolSettingsHasNormalizeForColor : std::false_type
        {
        };

        template <typename TSettings>
        struct ProtocolSettingsHasNormalizeForColor<TSettings,
                                                    std::void_t<decltype(TSettings::template normalizeForColor<ColorType>(std::declval<TSettings>()))>> : std::true_type
        {
        };

        static ProtocolSettingsType normalizeProtocolSettings(ProtocolSettingsType settings)
        {
            if constexpr (ProtocolSettingsHasNormalizeForColor<ProtocolSettingsType>::value)
            {
                return ProtocolSettingsType::template normalizeForColor<ColorType>(std::move(settings));
            }

            return settings;
        }

        template <typename TSettings,
                  typename = void>
        struct TransportSettingsHasNormalizePixelCount : std::false_type
        {
        };

        template <typename TSettings>
        struct TransportSettingsHasNormalizePixelCount<TSettings,
                                                       std::void_t<decltype(TSettings::normalize(std::declval<TSettings>(), std::declval<uint16_t>()))>> : std::true_type
        {
        };

        template <typename TSettings,
                  typename = void>
        struct TransportSettingsHasNormalize : std::false_type
        {
        };

        template <typename TSettings>
        struct TransportSettingsHasNormalize<TSettings,
                                             std::void_t<decltype(TSettings::normalize(std::declval<TSettings>()))>> : std::true_type
        {
        };

        static TransportSettingsType normalizeTransportSettings(TransportSettingsType settings,
                                                                size_t pixelCount)
        {
            const uint16_t protocolPixelCount = static_cast<uint16_t>(pixelCount);
            if constexpr (TransportSettingsHasNormalizePixelCount<TransportSettingsType>::value)
            {
                return TransportSettingsType::normalize(std::move(settings), protocolPixelCount);
            }

            if constexpr (TransportSettingsHasNormalize<TransportSettingsType>::value)
            {
                return TransportSettingsType::normalize(std::move(settings));
            }

            return settings;
        }

        size_t _pixelCount{0};
        Topology _topology{};
        TransportType _transport;
        ProtocolType _protocol;
        ShaderType _shader;
        std::vector<ColorType> _rootPixels;
        std::vector<ColorType> _shaderScratch;
        std::vector<uint8_t> _protocolBuffer;
        bool _dirty{true};
    };

    template <typename TProtocol,
              typename TTransport,
              typename TShader = NilShader<typename TProtocol::ColorType>>
    auto makeStaticDriverPixelBus(size_t pixelCount,
                                  typename TProtocol::SettingsType protocolSettings,
                                  typename TTransport::TransportSettingsType transportSettings,
                                  TShader shaderInstance = TShader{})
        -> StaticBusDriverPixelBusT<TProtocol,
                                    TTransport,
                                    lw::remove_cvref_t<TShader>>
    {
        return StaticBusDriverPixelBusT<TProtocol,
                                        TTransport,
                                        lw::remove_cvref_t<TShader>>{pixelCount,
                                                                      std::move(protocolSettings),
                                                                      std::move(transportSettings),
                                                                      std::move(shaderInstance)};
    }

} // namespace lw
