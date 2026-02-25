#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include "core/Compat.h"

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

        virtual void transmitBytes(span<const uint8_t> data) = 0;

        virtual void endTransaction()
        {
        }

        virtual bool isReadyToUpdate() const
        {
            return true;
        }
    };

    template <typename TTransportSettings, typename = void>
    struct TransportSettingsWithInvertImpl : std::false_type
    {
    };

    template <typename TTransportSettings>
    struct TransportSettingsWithInvertImpl<TTransportSettings,
                                           std::void_t<decltype(std::declval<TTransportSettings &>().invert)>>
        : std::integral_constant<bool,
                                 std::is_same<remove_cvref_t<decltype(std::declval<TTransportSettings &>().invert)>, bool>::value>
    {
    };

    template <typename TTransportSettings>
    static constexpr bool TransportSettingsWithInvert =
        TransportSettingsWithInvertImpl<TTransportSettings>::value;

    template <typename TTransport, typename = void>
    struct TransportLikeImpl : std::false_type
    {
    };

    template <typename TTransport>
    struct TransportLikeImpl<TTransport,
                             std::void_t<typename TTransport::TransportCategory,
                                         typename TTransport::TransportSettingsType>>
        : std::integral_constant<bool,
                                 std::is_convertible<TTransport *, ITransport *>::value &&
                                     TransportSettingsWithInvert<typename TTransport::TransportSettingsType>>
    {
    };

    template <typename TTransport>
    static constexpr bool TransportLike = TransportLikeImpl<TTransport>::value;

    template <typename TTransport, typename TTag>
    static constexpr bool TaggedTransportLike =
        TransportLike<TTransport> &&
        std::is_same<typename TTransport::TransportCategory, TTag>::value;

    template <typename TTransport>
    static constexpr bool SettingsConstructibleTransportLike =
        TransportLike<TTransport> &&
        std::is_constructible<TTransport, typename TTransport::TransportSettingsType>::value;

    template <typename TProtocolTransportCategory, typename TTransportCategory>
    static constexpr bool TransportCategoryCompatible =
        std::is_base_of<AnyTransportTag, TProtocolTransportCategory>::value &&
        std::is_base_of<AnyTransportTag, TTransportCategory>::value &&
        (std::is_same<TProtocolTransportCategory, AnyTransportTag>::value ||
         std::is_same<TTransportCategory, TProtocolTransportCategory>::value);

} // namespace npb

