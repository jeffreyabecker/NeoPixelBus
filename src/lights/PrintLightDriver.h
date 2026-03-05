#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>
#include <vector>

#if __has_include(<Arduino.h>)
#include <Arduino.h>
#endif

#include "core/Writable.h"
#include "lights/ILightDriver.h"

namespace lw
{

    template <typename TWritable,
              typename = std::enable_if_t<Writable<TWritable>>>
    struct PrintLightDriverSettingsT : LightDriverSettingsBase
    {
        TWritable *output = nullptr;
        bool asciiOutput = false;
        bool debugOutput = false;
        const char *identifier = nullptr;

        static PrintLightDriverSettingsT<TWritable> normalize(PrintLightDriverSettingsT<TWritable> settings)
        {
#if defined(ARDUINO)
            if (settings.output == nullptr)
            {
                settings.output = &Serial;
            }
#endif
            return settings;
        }
    };

    template <typename TColor,
              typename TWritable,
              typename = std::enable_if_t<Writable<TWritable>>>
    class PrintLightDriverT : public ILightDriver<TColor>
    {
    public:
        using ColorType = TColor;
        using LightDriverSettingsType = PrintLightDriverSettingsT<TWritable>;

        explicit PrintLightDriverT(LightDriverSettingsType settings)
            : _settings(std::move(settings))
        {
            captureIdentifier();
        }

        explicit PrintLightDriverT(TWritable &output)
            : _settings{.output = &output}
        {
            captureIdentifier();
        }

        void begin() override
        {
            if (_settings.debugOutput)
            {
                writeDebugPrefix();
                writeLine("begin");
            }
        }

        bool isReadyToUpdate() const override
        {
            return true;
        }

        void write(const ColorType &color) override
        {
            if (_settings.output == nullptr)
            {
                return;
            }

            if (_settings.debugOutput)
            {
                writeDebugPrefix();
                writeLine("write");
            }

            if (_settings.asciiOutput)
            {
                writeColorAscii(color);
                return;
            }

            writeColorBinary(color);
        }

    private:
        void writeColorBinary(const ColorType &color)
        {
            using ComponentType = typename ColorType::ComponentType;
            using UnsignedComponentType = std::make_unsigned_t<ComponentType>;

            for (size_t index = 0; index < ColorType::ChannelCount; ++index)
            {
                const UnsignedComponentType component = static_cast<UnsignedComponentType>(color.channelAtIndex(index));
                for (size_t offset = 0; offset < sizeof(ComponentType); ++offset)
                {
                    const size_t shift = (sizeof(ComponentType) - 1U - offset) * 8U;
                    const uint8_t byte = static_cast<uint8_t>((component >> shift) & 0xFFU);
                    writeBytes(&byte, 1);
                }
            }
        }

        void writeColorAscii(const ColorType &color)
        {
            using ComponentType = typename ColorType::ComponentType;
            using UnsignedComponentType = std::make_unsigned_t<ComponentType>;

            static constexpr char Hex[] = "0123456789ABCDEF";
            char componentBuffer[sizeof(ComponentType) * 2U]{};

            for (size_t index = 0; index < ColorType::ChannelCount; ++index)
            {
                UnsignedComponentType component = static_cast<UnsignedComponentType>(color.channelAtIndex(index));

                for (size_t nibble = 0; nibble < (sizeof(ComponentType) * 2U); ++nibble)
                {
                    const size_t shift = ((sizeof(ComponentType) * 2U) - 1U - nibble) * 4U;
                    componentBuffer[nibble] = Hex[(component >> shift) & 0x0FU];
                }

                writeBytes(reinterpret_cast<const uint8_t *>(componentBuffer), sizeof(componentBuffer));
            }
        }

        void writeBytes(const uint8_t *data, size_t length)
        {
            if (_settings.output == nullptr || data == nullptr || length == 0)
            {
                return;
            }

            _settings.output->write(data, length);
        }

        void writeText(const char *text)
        {
            if (text == nullptr)
            {
                return;
            }

            writeBytes(reinterpret_cast<const uint8_t *>(text), std::strlen(text));
        }

        void writeDebugPrefix()
        {
            writeText("[LIGHT");
            if (_settings.identifier != nullptr && _settings.identifier[0] != '\0')
            {
                writeText(":");
                writeText(_settings.identifier);
            }
            writeText("] ");
        }

        void writeLine(const char *text)
        {
            writeText(text);
            writeText("\r\n");
        }

        void captureIdentifier()
        {
            if (_settings.identifier == nullptr || _settings.identifier[0] == '\0')
            {
                return;
            }

            const size_t length = std::strlen(_settings.identifier);
            _identifierStorage.assign(_settings.identifier,
                                      _settings.identifier + length + 1U);
            _settings.identifier = _identifierStorage.data();
        }

        LightDriverSettingsType _settings;
        std::vector<char> _identifierStorage{};
    };

#if __has_include(<Arduino.h>)
    using PrintLightDriverSettings = PrintLightDriverSettingsT<Print>;
    template <typename TColor>
    using PrintLightDriver = PrintLightDriverT<TColor, Print>;
#endif

} // namespace lw
