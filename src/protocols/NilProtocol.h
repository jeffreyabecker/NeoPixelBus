#pragma once

#include <cstdint>
#include <utility>

#include "IProtocol.h"

namespace lw
{

    struct NilProtocolSettings : public ProtocolSettings
    {
    };

    template <typename TColor>
    class NilProtocol : public IProtocol<TColor>
    {
    public:
        using SettingsType = NilProtocolSettings;

        static constexpr size_t requiredBufferSize(uint16_t,
                               const SettingsType &)
        {
            return 0;
        }

        explicit NilProtocol(PixelCount pixelCount,
                             SettingsType settings = {})
            : IProtocol<TColor>(pixelCount)
            , _settings{std::move(settings)}
        {
        }

        void begin() override
        {
        }

        void update(span<const TColor>, span<uint8_t> buffer = span<uint8_t>{}) override
        {
        }

        ProtocolSettings &settings() override
        {
            return _settings;
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

    private:
        SettingsType _settings;
    };

} // namespace lw


