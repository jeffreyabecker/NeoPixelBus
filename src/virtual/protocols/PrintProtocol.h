#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <memory>

#include <Print.h>

#include "IProtocol.h"
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
                     PrintProtocolSettings settings)
            : _settings{std::move(settings)}
        {
            (void)pixelCount;
        }

        void initialize() override
        {
            // no-op — no hardware to set up
        }

        void update(std::span<const Color> colors) override
        {
            static constexpr char HexDigits[] = "0123456789ABCDEF";

            // Emit fixed channel order: R G B CW WW (RGBCW)
            for (const auto &color : colors)
            {
                static constexpr char ChannelOrder[] = "RGBCW";

                for (char channel : ChannelOrder)
                {
                    const uint8_t value = color[channel];
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
    };

} // namespace npb
