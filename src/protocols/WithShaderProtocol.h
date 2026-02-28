#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <utility>

#include "IProtocol.h"
#include "colors/IShader.h"

namespace npb
{

    template <typename TColor, typename TSettings>
    struct WithShaderProtocolSettings : public TSettings
    {
        IShader<TColor> *shader = nullptr;
        bool allowDirtyShaders = false;
    };

    template <typename TShader, typename TSettings>
    struct WithEmbeddedShaderProtocolSettings : public TSettings
    {
        TShader shader;
        bool allowDirtyShaders = false;
    };

    template <typename TColor,
              typename TProtocol = IProtocol<TColor>,
              typename = std::enable_if_t<std::is_base_of<IProtocol<TColor>, TProtocol>::value>>
    class WithShader : public TProtocol
    {
    public:
        using SettingsType = WithShaderProtocolSettings<TColor, typename TProtocol::SettingsType>;
        using TransportCategory = typename TProtocol::TransportCategory;

        WithShader(uint16_t pixelCount,
                   SettingsType settings)
                        : TProtocol(pixelCount,
                        static_cast<typename TProtocol::SettingsType &&>(settings)),
              _shader{settings.shader},
              _allowDirtyShaders{settings.allowDirtyShaders},
              _scratchColors(_allowDirtyShaders ? 0u : pixelCount)
        {
        }

        template <typename... TArgs,
                  typename = std::enable_if_t<(sizeof...(TArgs) > 0)>>
        WithShader(uint16_t pixelCount,
                   SettingsType settings,
                   TArgs &&...args)
                        : TProtocol(pixelCount, std::forward<TArgs>(args)...),
              _shader{settings.shader},
              _allowDirtyShaders{settings.allowDirtyShaders},
              _scratchColors(_allowDirtyShaders ? 0u : pixelCount)
        {
        }

        void update(span<const TColor> colors) override
        {
            span<const TColor> source = colors;
            if (nullptr != _shader)
            {
                if (_allowDirtyShaders)
                {
                    span<TColor> shadedColors{const_cast<TColor *>(colors.data()), colors.size()};
                    _shader->apply(shadedColors);
                }
                else
                {
                    const size_t colorCount = std::min(colors.size(), _scratchColors.size());
                    std::copy_n(colors.begin(), colorCount, _scratchColors.begin());

                    span<TColor> shadedColors{_scratchColors.data(), colorCount};
                    _shader->apply(shadedColors);
                    source = span<const TColor>{_scratchColors.data(), colorCount};
                }
            }

            TProtocol::update(source);
        }

    private:
        IShader<TColor> *_shader{nullptr};
        bool _allowDirtyShaders{false};
        std::vector<TColor> _scratchColors;
    };

    template <typename TColor,
              typename TShader = IShader<TColor>,
              typename TProtocol = IProtocol<TColor>,
              typename = std::enable_if_t<std::is_base_of<IProtocol<TColor>, TProtocol>::value &&
                                std::is_base_of<IShader<TColor>, TShader>::value>>
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
              _allowDirtyShaders{settings.allowDirtyShaders},
              _scratchColors(_allowDirtyShaders ? 0u : pixelCount)
        {
        }

        template <typename... TArgs,
                  typename = std::enable_if_t<(sizeof...(TArgs) > 0)>>
        WithEmbeddedShader(uint16_t pixelCount,
                           SettingsType settings,
                           TArgs &&...args)
                        : TProtocol(pixelCount, std::forward<TArgs>(args)...),
              _shader{std::move(settings.shader)},
              _allowDirtyShaders{settings.allowDirtyShaders},
              _scratchColors(_allowDirtyShaders ? 0u : pixelCount)
        {
        }

        void update(span<const TColor> colors) override
        {
            if (_allowDirtyShaders)
            {
                span<TColor> shadedColors{const_cast<TColor *>(colors.data()), colors.size()};
                _shader.apply(shadedColors);
                TProtocol::update(colors);
                return;
            }

            const size_t colorCount = std::min(colors.size(), _scratchColors.size());
            std::copy_n(colors.begin(), colorCount, _scratchColors.begin());

            span<TColor> shadedColors{_scratchColors.data(), colorCount};
            _shader.apply(shadedColors);

            TProtocol::update(span<const TColor>{_scratchColors.data(), colorCount});
        }

    private:
        TShader _shader;
        bool _allowDirtyShaders{false};
        std::vector<TColor> _scratchColors;
    };

    template <typename TColor,
              typename TShader = IShader<TColor>,
              typename TProtocol = IProtocol<TColor>>
    using WithOwnedShader = WithEmbeddedShader<TColor, TShader, TProtocol>;

} // namespace npb


