#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <span>
#include <algorithm>
#include <concepts>
#include <type_traits>
#include <utility>

#include "IProtocol.h"
#include "colors/IShader.h"
#include "core/ResourceHandle.h"

namespace npb
{

    template <typename TColor, typename TSettings>
    struct WithShaderProtocolSettings : public TSettings
    {
        ResourceHandle<IShader<TColor>> shader;
    };

    template <typename TShader, typename TSettings>
    struct WithEmbeddedShaderProtocolSettings : public TSettings
    {
        TShader shader;
    };

    template <typename TColor, typename TProtocol = IProtocol<TColor>>
        requires std::derived_from<TProtocol, IProtocol<TColor>>
    class WithShader : public TProtocol
    {
    public:
        using SettingsType = WithShaderProtocolSettings<TColor, typename TProtocol::SettingsType>;
        using TransportCategory = typename TProtocol::TransportCategory;

        WithShader(uint16_t pixelCount,
                   SettingsType settings)
            : TProtocol(pixelCount,
                        static_cast<typename TProtocol::SettingsType &&>(settings)),
              _shader{std::move(settings.shader)},
              _scratchColors(pixelCount)
        {
        }

        template <typename... TArgs>
            requires(sizeof...(TArgs) > 0)
        WithShader(uint16_t pixelCount,
                   SettingsType settings,
                   TArgs &&...args)
            : TProtocol(pixelCount, std::forward<TArgs>(args)...),
              _shader{std::move(settings.shader)},
              _scratchColors(pixelCount)
        {
        }

        void update(std::span<const TColor> colors) override
        {
            std::span<const TColor> source = colors;
            if (nullptr != _shader)
            {
                std::copy(colors.begin(), colors.end(), _scratchColors.begin());
                _shader->apply(_scratchColors);
                source = _scratchColors;
            }

            TProtocol::update(source);
        }

    private:
        ResourceHandle<IShader<TColor>> _shader;
        std::vector<TColor> _scratchColors;
    };

    template <typename TColor,
              typename TShader = IShader<TColor>,
              typename TProtocol = IProtocol<TColor>>
        requires(std::derived_from<TProtocol, IProtocol<TColor>> &&
                 std::derived_from<TShader, IShader<TColor>>)
    class WithEmbeddedShader : public TProtocol
    {
    public:
        using SettingsType = WithEmbeddedShaderProtocolSettings<TShader, typename TProtocol::SettingsType>;
        using TransportCategory = typename TProtocol::TransportCategory;

        WithEmbeddedShader(uint16_t pixelCount,
                           SettingsType settings)
            : TProtocol(pixelCount,
                        static_cast<typename TProtocol::SettingsType &&>(settings)),
              _shader{std::move(settings.shader)},
              _scratchColors(pixelCount)
        {
        }

        template <typename... TArgs>
            requires(sizeof...(TArgs) > 0)
        WithEmbeddedShader(uint16_t pixelCount,
                           SettingsType settings,
                           TArgs &&...args)
            : TProtocol(pixelCount, std::forward<TArgs>(args)...),
              _shader{std::move(settings.shader)},
              _scratchColors(pixelCount)
        {
        }

        void update(std::span<const TColor> colors) override
        {
            const size_t colorCount = std::min(colors.size(), _scratchColors.size());
            std::copy_n(colors.begin(), colorCount, _scratchColors.begin());

            std::span<TColor> shadedColors{_scratchColors.data(), colorCount};
            _shader.apply(shadedColors);

            TProtocol::update(std::span<const TColor>{_scratchColors.data(), colorCount});
        }

    private:
        TShader _shader;
        std::vector<TColor> _scratchColors;
    };

    template <typename TColor,
              typename TShader = IShader<TColor>,
              typename TProtocol = IProtocol<TColor>>
    using WithOwnedShader = WithEmbeddedShader<TColor, TShader, TProtocol>;

} // namespace npb


