#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <utility>

#include <Arduino.h>

#include "ITransport.h"
#include "../Writable.h"

namespace npb
{

    template <Writable TWritable = Print>
    struct PrintTransportSettingsT
    {
        TWritable *output = nullptr;
        bool invert = false;
    };

    template <Writable TWritable = Print>
    class PrintTransportT : public ITransport
    {
    public:
        using TransportSettingsType = PrintTransportSettingsT<TWritable>;
        using TransportCategory = AnyTransportTag;
        explicit PrintTransportT(PrintTransportSettingsT<TWritable> config)
            : _config{std::move(config)}
        {
        }

        explicit PrintTransportT(TWritable &output)
            : _config{.output = &output}
        {
        }

        void begin() override
        {
            // Print instance setup is owned by caller.
        }

        void beginTransaction() override
        {
            // no-op for Print sink
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            if (_config.output == nullptr)
            {
                return;
            }

            _config.output->write(data.data(), data.size());
        }

        void endTransaction() override
        {
            // no-op for Print sink
        }

    private:
        PrintTransportSettingsT<TWritable> _config;
    };

    using PrintTransportSettings = PrintTransportSettingsT<Print>;
    using PrintTransport = PrintTransportT<Print>;

} // namespace npb
