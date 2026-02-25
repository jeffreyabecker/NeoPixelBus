#pragma once

#include <cstdint>
#include <cstddef>
#include <concepts>
#include <memory>
#include <span>
#include <utility>

#include "../colors/Color.h"
#include "../ResourceHandle.h"
#include "../transports/ITransport.h"
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
        virtual void update(std::span<const TColor> colors) = 0;
        virtual bool isReadyToUpdate() const = 0;
        virtual bool alwaysUpdate() const = 0;
    };

    template <typename TProtocol>
    concept ProtocolType = requires {
        typename TProtocol::SettingsType;
        typename TProtocol::TransportCategory;
    };

    template <typename TProtocol>
    concept ProtocolPixelSettingsConstructible = ProtocolType<TProtocol> &&
                                                 (!std::same_as<typename TProtocol::SettingsType, void>) &&
                                                 std::constructible_from<TProtocol,
                                                                         uint16_t,
                                                                         typename TProtocol::SettingsType>;

    template <typename TProtocol>
    concept ProtocolSettingsTransportBindable = ProtocolType<TProtocol> &&
                                                requires(typename TProtocol::SettingsType &settings,
                                                         ResourceHandle<ITransport> bus) {
                                                    settings.bus = std::move(bus);
                                                };

    template <typename TProtocol, typename TTransport>
    concept ProtocolTransportCompatible = ProtocolType<TProtocol> &&
                                          TransportLike<TTransport> &&
                                          TransportCategoryCompatible<typename TProtocol::TransportCategory,
                                                                      typename TTransport::TransportCategory>;

    template <typename TProtocol, typename TTransport>
        requires ProtocolTransportCompatible<TProtocol, TTransport> &&
                 (!std::same_as<typename TProtocol::SettingsType, void>)
    struct ProtocolTransportSettings : public TProtocol::SettingsType
    {
        using SettingsType = typename TProtocol::SettingsType;

        template <typename... TTransportArgs>
            requires std::constructible_from<TTransport, TTransportArgs...> &&
                     std::constructible_from<SettingsType, std::unique_ptr<TTransport>>
        explicit ProtocolTransportSettings(TTransportArgs &&...transportArgs)
            : SettingsType{std::make_unique<TTransport>(std::forward<TTransportArgs>(transportArgs)...)}
        {
        }

        template <typename... TTransportArgs>
            requires std::constructible_from<TTransport, TTransportArgs...> &&
                     requires(SettingsType &settings, ResourceHandle<ITransport> bus) {
                         settings.bus = std::move(bus);
                     }
        explicit ProtocolTransportSettings(SettingsType settings,
                                           TTransportArgs &&...transportArgs)
            : SettingsType{std::move(settings)}
        {
            this->bus = std::make_unique<TTransport>(std::forward<TTransportArgs>(transportArgs)...);
        }
    };

} // namespace npb
