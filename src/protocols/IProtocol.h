#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <utility>

#include "colors/Color.h"
#include "core/Compat.h"
#include "transports/ITransport.h"
namespace npb
{

    template <typename TColor>
    class IProtocol
    {
    public:
        using ColorType = TColor;
        using SettingsType = void;
        using TransportCategory = AnyTransportTag;
        virtual ~IProtocol() = default;

        virtual void initialize() = 0;
        virtual void update(span<const TColor> colors) = 0;
        virtual bool isReadyToUpdate() const = 0;
        virtual bool alwaysUpdate() const = 0;
    };

    template <typename TProtocol, typename = void>
    struct ProtocolTypeImpl : std::false_type
    {
    };

    template <typename TProtocol>
    struct ProtocolTypeImpl<TProtocol,
                            std::void_t<typename TProtocol::SettingsType,
                                        typename TProtocol::TransportCategory>> : std::true_type
    {
    };

    template <typename TProtocol>
    static constexpr bool ProtocolType = ProtocolTypeImpl<TProtocol>::value;

    template <typename TProtocol>
    static constexpr bool ProtocolPixelSettingsConstructible =
        ProtocolType<TProtocol> &&
        !std::is_same<typename TProtocol::SettingsType, void>::value &&
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
        TransportLike<TTransport> &&
        TransportCategoryCompatible<typename TProtocol::TransportCategory,
                                    typename TTransport::TransportCategory>;

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

} // namespace npb


