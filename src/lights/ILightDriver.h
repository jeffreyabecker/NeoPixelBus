#pragma once

#include <type_traits>

#include "core/Compat.h"

namespace lw
{

    template <typename TColor>
    class ILightDriver
    {
    public:
        using ColorType = TColor;

        virtual ~ILightDriver() = default;

        virtual void begin() = 0;
        virtual bool isReadyToUpdate() const = 0;
        virtual void write(const ColorType &color) = 0;
    };

    struct LightDriverSettingsBase
    {
    };

    template <typename TDriver, typename = void>
    struct LightDriverLikeImpl : std::false_type
    {
    };

    template <typename TDriver>
    struct LightDriverLikeImpl<TDriver,
                               std::void_t<typename TDriver::ColorType,
                                           typename TDriver::LightDriverSettingsType>>
        : std::integral_constant<bool,
                                 std::is_convertible<TDriver *, ILightDriver<typename TDriver::ColorType> *>::value>
    {
    };

    template <typename TDriver>
    static constexpr bool LightDriverLike = LightDriverLikeImpl<TDriver>::value;

    template <typename TDriver>
    static constexpr bool SettingsConstructibleLightDriverLike =
        LightDriverLike<TDriver> &&
        std::is_constructible<TDriver, typename TDriver::LightDriverSettingsType>::value;

} // namespace lw
