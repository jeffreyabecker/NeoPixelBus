#pragma once

#include <cstdint>
#include <utility>

#include "IProtocol.h"

namespace lw
{

    struct NilProtocolSettings
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

        explicit NilProtocol(uint16_t pixelCount,
                             SettingsType settings = {})
            : IProtocol<TColor>(pixelCount)
            , _settings{std::move(settings)}
        {
        }

        void initialize() override
        {
        }

        void update(span<const TColor>, span<uint8_t> buffer = span<uint8_t>{}) override
        {
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

    private:
        SettingsType _settings;
    };

} // namespace lw


