#pragma once

#include <cstdint>
#include <utility>

#include "IProtocol.h"
#include "../ResourceHandle.h"
#include "../transports/ITransport.h"

namespace npb
{

    struct NilProtocolSettings
    {
        ResourceHandle<ITransport> bus = nullptr;
    };

    template <typename TColor = Color>
    class NilProtocol : public IProtocol<TColor>
    {
    public:
        using SettingsType = NilProtocolSettings;
        using TransportCategory = AnyTransportTag;

        explicit NilProtocol(uint16_t pixelCount,
                             SettingsType settings = {})
            : _pixelCount{pixelCount}
            , _settings{std::move(settings)}
        {
        }

        void initialize() override
        {
            if (_settings.bus != nullptr)
            {
                _settings.bus->begin();
            }
        }

        void update(std::span<const TColor>) override
        {
        }

        bool isReadyToUpdate() const override
        {
            if (_settings.bus != nullptr)
            {
                return _settings.bus->isReadyToUpdate();
            }

            return true;
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

    private:
        uint16_t _pixelCount{0};
        SettingsType _settings;
    };

} // namespace npb
