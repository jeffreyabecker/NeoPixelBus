#pragma once

#include <cstdint>

#include "ITransport.h"

namespace npb
{

    struct NilTransportSettings
        : TransportSettingsBase
    {
    };

    class NilTransport : public ITransport
    {
    public:
        using TransportSettingsType = NilTransportSettings;
        using TransportCategory = TransportTag;
        explicit NilTransport(NilTransportSettings = {})
        {
        }

        void begin() override
        {
        }

        void beginTransaction() override
        {
        }

        void endTransaction() override
        {
        }

        void transmitBytes(span<uint8_t>) override
        {
        }
    };

} // namespace npb
