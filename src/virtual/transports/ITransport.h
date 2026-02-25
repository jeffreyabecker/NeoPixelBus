#pragma once

#include <concepts>
#include <cstdint>
#include <cstddef>
#include <span>

namespace npb
{

    struct AnyTransportTag
    {
    };
    struct TransportTag : public AnyTransportTag
    {
    };

    struct OneWireTransportTag : public AnyTransportTag
    {
    };

    class ITransport
    {
    public:
        virtual ~ITransport() = default;

        virtual void begin() = 0;

        virtual void beginTransaction()
        {
        }

        virtual void transmitBytes(std::span<const uint8_t> data) = 0;

        virtual void endTransaction()
        {
        }

        virtual bool isReadyToUpdate() const
        {
            return true;
        }
    };

    template <typename TTransport>
    concept TransportLike = std::derived_from<TTransport, ITransport> &&
                            requires {
                                typename TTransport::TransportCategory;
                                typename TTransport::TransportSettingsType;
                            };

    template <typename TTransport, typename TTag>
    concept TaggedTransportLike = TransportLike<TTransport> &&
                                  std::same_as<typename TTransport::TransportCategory, TTag>;

    template <typename TTransport>
    concept SettingsConstructibleTransportLike = TransportLike<TTransport> &&
                                               std::constructible_from<TTransport, typename TTransport::TransportSettingsType>;

    template <typename TProtocolTransportCategory, typename TTransportCategory>
    concept TransportCategoryCompatible = std::derived_from<TProtocolTransportCategory, AnyTransportTag> &&
                                          std::derived_from<TTransportCategory, AnyTransportTag> &&
                                          (std::same_as<TProtocolTransportCategory, AnyTransportTag> ||
                                           std::same_as<TTransportCategory, TProtocolTransportCategory>);

} // namespace npb
