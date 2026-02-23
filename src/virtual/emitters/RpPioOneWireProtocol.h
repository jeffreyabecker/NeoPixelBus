#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cstddef>
#include <memory>
#include <utility>

#include "OneWireTiming.h"
#include "Ws2812xProtocol.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../buses/ISelfClockingTransport.h"
#include "../buses/RpPioSelfClockingTransport.h"

namespace npb
{

    /// Construction settings for RpPioOneWireProtocol.
    struct RpPioOneWireProtocolSettings
    {
        uint8_t pin;
        uint8_t pioIndex = 1;              // 0 = PIO0, 1 = PIO1 (2 on RP2350)
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;               // compensate for external inverting hardware
        ColorOrderTransformConfig colorConfig;
    };

    /// One-wire NRZ emitter for RP2040/RP2350 using PIO + DMA.
    ///
    /// Each instance drives a single strip on one pin.  Internally it claims
    /// the next available state machine on the selected PIO block.  Up to 4
    /// strips can share one PIO block (one emitter per SM).
    ///
    /// The user is responsible for choosing the PIO block and not exceeding
    /// the available state machines.
    ///
    /// DMA channels are claimed cooperatively via `dma_claim_unused_channel()`
    /// so multiple PIO-based emitter types can coexist.
    class RpPioOneWireProtocol : public Ws2812xProtocol
    {
    public:
        RpPioOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            RpPioOneWireProtocolSettings settings)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                makeDefaultTransport(settings, pixelCount))
        {
        }

        RpPioOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            RpPioOneWireProtocolSettings settings,
            ResourceHandle<ISelfClockingTransport> transport)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                std::move(transport))
        {
        }

        // Non-copyable, non-movable (owns hardware resources)
        RpPioOneWireProtocol(const RpPioOneWireProtocol &) = delete;
        RpPioOneWireProtocol &operator=(const RpPioOneWireProtocol &) = delete;
        RpPioOneWireProtocol(RpPioOneWireProtocol &&) = delete;
        RpPioOneWireProtocol &operator=(RpPioOneWireProtocol &&) = delete;

    private:
        static ResourceHandle<ISelfClockingTransport> makeDefaultTransport(
            const RpPioOneWireProtocolSettings &settings,
            uint16_t pixelCount)
        {
            ColorOrderTransform transform{settings.colorConfig};

            RpPioSelfClockingTransportConfig cfg{};
            cfg.pin = settings.pin;
            cfg.pioIndex = settings.pioIndex;
            cfg.invert = settings.invert;
            cfg.timing = settings.timing;
            cfg.frameBytes = transform.bytesNeeded(pixelCount);

            return ResourceHandle<ISelfClockingTransport>{
                std::make_unique<RpPioSelfClockingTransport>(cfg)};
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_RP2040
