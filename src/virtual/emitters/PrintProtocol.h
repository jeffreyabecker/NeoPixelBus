#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>

#include <Print.h>

#include "IProtocol.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"

namespace npb
{

    struct PrintProtocolSettings
    {
        Print& output;
    };

    class PrintProtocol : public IProtocol
    {
    public:
        PrintProtocol(uint16_t pixelCount,
                     ResourceHandle<IShader> shader,
                     PrintProtocolSettings settings)
            : _settings{std::move(settings)}
            , _shader{std::move(shader)}
            , _scratchColors(pixelCount)
        {
        }

        void initialize() override
        {
            // no-op — no hardware to set up
        }

        void update(std::span<const Color> colors) override
        {
            static constexpr char HexDigits[] = "0123456789ABCDEF";

            // Apply shaders in batch
            std::span<const Color> source = colors;
            if (nullptr != _shader)
            {
                std::copy(colors.begin(), colors.end(), _scratchColors.begin());
                _shader->apply(_scratchColors);
                source = _scratchColors;
            }

            // Emit fixed channel order: R G B CW WW (RGBCW)
            for (const auto &color : source)
            {
                static constexpr size_t ChannelOrder[] =
                {
                    Color::IdxR,
                    Color::IdxG,
                    Color::IdxB,
                    Color::IdxCW,
                    Color::IdxWW
                };

                for (size_t channelIndex : ChannelOrder)
                {
                    const uint8_t value = color[channelIndex];
                    _settings.output.print(HexDigits[value >> 4]);
                    _settings.output.print(HexDigits[value & 0x0F]);
                }

                _settings.output.print(' ');
            }
            _settings.output.println();
        }

        bool isReadyToUpdate() const override
        {
            return true;
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

    private:
        PrintProtocolSettings _settings;
        ResourceHandle<IShader> _shader;
        std::vector<Color> _scratchColors;       // pre-allocated at construction
    };

} // namespace npb
