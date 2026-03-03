#pragma once

#include <cstdint>
#include <utility>

#include "IProtocol.h"
#include "transports/ITransport.h"

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

        static size_t requiredBufferSize(uint16_t,
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
            if (this->_transport != nullptr)
            {
                this->_transport->begin();
            }
        }

        void bindTransport(ITransport *transport) override
        {
            this->_transport = transport;
        }

        void update(span<const TColor>, span<uint8_t> buffer = span<uint8_t>{}) override
        {
        }

        bool isReadyToUpdate() const override
        {
            if (this->_transport != nullptr)
            {
                return this->_transport->isReadyToUpdate();
            }

            return true;
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

    private:
        SettingsType _settings;
    };

} // namespace lw


