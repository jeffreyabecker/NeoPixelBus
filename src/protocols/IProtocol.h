#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <utility>

#include "colors/Color.h"
#include "core/Compat.h"
#include "transports/ITransport.h"
namespace lw
{

    template <typename TColor>
    class IProtocol
    {
    public:
        using ColorType = TColor;
        using SettingsType = void;
        static constexpr bool RequiresExternalBuffer = true;
        explicit IProtocol(uint16_t pixelCount = 0)
            : _pixelCount{pixelCount}
        {
        }

        virtual ~IProtocol() = default;

        uint16_t pixelCount() const
        {
            return _pixelCount;
        }

        virtual void initialize() = 0;
        virtual void update(span<const TColor> colors) = 0;
        virtual void setBuffer(span<uint8_t> buffer)
        {
            (void)buffer;
        }
        virtual void bindTransport(ITransport *transport)
        {
            (void)transport;
        }
        virtual size_t requiredBufferSizeBytes() const
        {
            return 0;
        }
        virtual bool isReadyToUpdate() const = 0;
        virtual bool alwaysUpdate() const = 0;

    protected:
        uint16_t _pixelCount;
    };

    template <typename TProtocol, typename = void>
    struct ProtocolTypeImpl : std::false_type
    {
    };

    template <typename TProtocol>
    struct ProtocolTypeImpl<TProtocol,
                            std::void_t<typename TProtocol::SettingsType>> : std::true_type
    {
    };

    template <typename TProtocol>
    static constexpr bool ProtocolType = ProtocolTypeImpl<TProtocol>::value;

    template <typename TProtocol>
    static constexpr bool ProtocolMoveConstructible =
        ProtocolType<TProtocol> &&
        std::is_move_constructible<TProtocol>::value;

    template <typename TProtocol, typename = void>
    struct ProtocolExternalBufferRequiredImpl : std::false_type
    {
    };

    template <typename TProtocol>
    struct ProtocolExternalBufferRequiredImpl<TProtocol,
                                              std::void_t<decltype(TProtocol::RequiresExternalBuffer)>>
        : std::integral_constant<bool, static_cast<bool>(TProtocol::RequiresExternalBuffer)>
    {
    };

    template <typename TProtocol>
    static constexpr bool ProtocolExternalBufferRequired =
        ProtocolType<TProtocol> &&
        ProtocolExternalBufferRequiredImpl<TProtocol>::value;

    template <typename TProtocol, typename = void>
    struct ProtocolRequiredBufferSizeComputableImpl : std::false_type
    {
    };

    template <typename TProtocol>
    struct ProtocolRequiredBufferSizeComputableImpl<TProtocol,
                                                    std::void_t<decltype(TProtocol::requiredBufferSize(std::declval<uint16_t>(),
                                                                                                        std::declval<const typename TProtocol::SettingsType &>()))>>
        : std::integral_constant<bool,
                                 std::is_convertible<decltype(TProtocol::requiredBufferSize(std::declval<uint16_t>(),
                                                                                           std::declval<const typename TProtocol::SettingsType &>())),
                                                     size_t>::value>
    {
    };

    template <typename TProtocol>
    static constexpr bool ProtocolRequiredBufferSizeComputable =
        ProtocolType<TProtocol> &&
        ProtocolRequiredBufferSizeComputableImpl<TProtocol>::value;

    template <typename TProtocol>
    static constexpr bool ProtocolPixelSettingsConstructible =
        ProtocolType<TProtocol> &&
        ProtocolMoveConstructible<TProtocol> &&
        ProtocolExternalBufferRequired<TProtocol> &&
        ProtocolRequiredBufferSizeComputable<TProtocol> &&
        !std::is_same<typename TProtocol::SettingsType, void>::value &&
        std::is_move_constructible<typename TProtocol::SettingsType>::value &&
        std::is_constructible<TProtocol,
                              uint16_t,
                              typename TProtocol::SettingsType>::value;

    template <typename TSettings, typename = void>
    struct HasBusMember : std::false_type
    {
    };

    template <typename TSettings>
    struct HasBusMember<TSettings,
                        std::void_t<decltype(std::declval<TSettings &>().bus)>> : std::true_type
    {
    };

    template <typename TProtocol>
    static constexpr bool ProtocolSettingsTransportBindable =
        ProtocolType<TProtocol> &&
        HasBusMember<typename TProtocol::SettingsType>::value;

    template <typename TProtocol, typename TTransport>
    static constexpr bool ProtocolTransportCompatible =
        ProtocolType<TProtocol> &&
        TransportLike<TTransport>;

    template <typename TProtocol,
              typename TTransport,
              typename = std::enable_if_t<ProtocolTransportCompatible<TProtocol, TTransport> &&
                                          !std::is_same<typename TProtocol::SettingsType, void>::value>>
    struct ProtocolTransportSettings : public TProtocol::SettingsType
    {
        using SettingsType = typename TProtocol::SettingsType;

        template <typename... TTransportArgs,
                  typename = std::enable_if_t<std::is_constructible<TTransport, TTransportArgs...>::value &&
                                              std::is_constructible<SettingsType, std::unique_ptr<TTransport>>::value>>
        explicit ProtocolTransportSettings(TTransportArgs &&...transportArgs)
            : SettingsType{std::make_unique<TTransport>(std::forward<TTransportArgs>(transportArgs)...)}
        {
        }

        template <typename... TTransportArgs,
                  typename = std::enable_if_t<std::is_constructible<TTransport, TTransportArgs...>::value &&
                                              HasBusMember<SettingsType>::value>>
        explicit ProtocolTransportSettings(SettingsType settings,
                                           TTransportArgs &&...transportArgs)
            : SettingsType{std::move(settings)}
            , _transportOwner(std::make_unique<TTransport>(std::forward<TTransportArgs>(transportArgs)...))
        {
            this->bus = _transportOwner.get();
        }

    private:
        std::unique_ptr<TTransport> _transportOwner;
    };

} // namespace lw


