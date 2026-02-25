#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <type_traits>
#include <utility>

#include <Print.h>

#include "IProtocol.h"
#include "../ResourceHandle.h"

namespace npb
{

    template <typename TColor>
    struct DebugProtocolSettingsT
    {
        ResourceHandle<Print> output = nullptr;
        bool invert = false;
        ResourceHandle<IProtocol<TColor>> protocol = nullptr;
    };

    template <typename TColor = Color>
    class DebugProtocol : public IProtocol<TColor>
    {
    public:
        using SettingsType = DebugProtocolSettingsT<TColor>;
        using TransportCategory = AnyTransportTag;

        DebugProtocol(uint16_t pixelCount,
                      SettingsType settings)
            : _settings{std::move(settings)}
            , _pixelCount{pixelCount}
        {
        }

        DebugProtocol(uint16_t pixelCount,
                      Print &output,
                      bool invert = false)
            : DebugProtocol(pixelCount,
                            SettingsType{.output = output, .invert = invert})
        {
        }

        DebugProtocol(uint16_t pixelCount,
                      Print &output,
                      ResourceHandle<IProtocol<TColor>> protocol,
                      bool invert = false)
            : DebugProtocol(pixelCount,
                            SettingsType{.output = output, .invert = invert, .protocol = std::move(protocol)})
        {
        }

        void initialize() override
        {
            if (_settings.output != nullptr)
            {
                _settings.output->print("[PROTOCOL] begin pixelCount=");
                _settings.output->println(_pixelCount);
            }

            if (_settings.protocol != nullptr)
            {
                _settings.protocol->initialize();
            }
        }

        void update(std::span<const TColor> colors) override
        {
            if (_settings.output == nullptr)
            {
                return;
            }

            static constexpr char HexDigits[] = "0123456789ABCDEF";

            _settings.output->print("[PROTOCOL] colors(");
            _settings.output->print(colors.size());
            _settings.output->print("): ");

            for (size_t colorIndex = 0; colorIndex < colors.size(); ++colorIndex)
            {
                if (colorIndex > 0)
                {
                    _settings.output->print(' ');
                }

                const auto &color = colors[colorIndex];
                for (size_t channelIndex = 0; channelIndex < TColor::ChannelCount; ++channelIndex)
                {
                    using ComponentType = typename TColor::ComponentType;
                    using UnsignedComponentType = std::make_unsigned_t<ComponentType>;

                    UnsignedComponentType value = static_cast<UnsignedComponentType>(color[channelIndex]);
                    if (_settings.invert)
                    {
                        value = static_cast<UnsignedComponentType>(~value);
                    }

                    int shift = static_cast<int>(sizeof(UnsignedComponentType) * 8) - 4;
                    while (shift >= 0)
                    {
                        const uint8_t nibble = static_cast<uint8_t>((value >> shift) & 0x0F);
                        _settings.output->print(HexDigits[nibble]);
                        shift -= 4;
                    }
                }
            }

            _settings.output->println();

            if (_settings.protocol != nullptr)
            {
                _settings.protocol->update(colors);
            }
        }

        bool isReadyToUpdate() const override
        {
            if (_settings.protocol != nullptr)
            {
                return _settings.protocol->isReadyToUpdate();
            }

            return true;
        }

        bool alwaysUpdate() const override
        {
            if (_settings.protocol != nullptr)
            {
                return _settings.protocol->alwaysUpdate();
            }

            return false;
        }

    private:
        SettingsType _settings;
        uint16_t _pixelCount{0};
    };

    using DebugProtocolSettings = DebugProtocolSettingsT<Color>;

} // namespace npb
