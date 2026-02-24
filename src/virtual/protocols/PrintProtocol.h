#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <memory>
#include <type_traits>

#include <Print.h>

#include "IProtocol.h"
#include "../ResourceHandle.h"

namespace npb
{

    struct PrintProtocolSettings
    {
        Print& output;
    };

    template <typename TColor = Color>
    class PrintProtocol : public IProtocol<TColor>
    {
    public:
        using SettingsType = PrintProtocolSettings;
        using TransportCategory = TransportTag;

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

        void update(std::span<const TColor> colors) override
        {
            static constexpr char HexDigits[] = "0123456789ABCDEF";

            for (const auto &color : colors)
            {
                for (size_t channelIndex = 0; channelIndex < TColor::ChannelCount; ++channelIndex)
                {
                    using ComponentType = typename TColor::ComponentType;
                    using UnsignedComponentType = std::make_unsigned_t<ComponentType>;

                    UnsignedComponentType value = static_cast<UnsignedComponentType>(color[channelIndex]);
                    int shift = static_cast<int>(sizeof(UnsignedComponentType) * 8) - 4;
                    while (shift >= 0)
                    {
                        uint8_t nibble = static_cast<uint8_t>((value >> shift) & 0x0F);
                        _settings.output.print(HexDigits[nibble]);
                        shift -= 4;
                    }
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
