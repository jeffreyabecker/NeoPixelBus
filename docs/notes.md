Nordic NRF -- look into https://github.com/bjornspockeli/nRF52_ppi_timper_gpiote_example for DMA to GPIO

TODO: Add RpPioSpiClass using :https://github.com/earlephilhower/arduino-pico/blob/master/libraries/SoftwareSPI/src/SoftwareSPI.h *NOT DMA*
TODO: add a RpPioUart class using https://github.com/earlephilhower/arduino-pico/blob/master/cores/rp2040/SerialPIO.h



Implement a parallel transport for 

UCS8904 -- https://github.com/Makuna/NeoPixelBus/issues/516


---

## Parallel ClockData pad policy sketch (for two-wire protocols)

Status note:
- Parallel transport umbrella exports are temporarily disabled during rework.
- See `docs/parallel-transports-rework.md`.

Problem:
- One-wire lanes can usually be front-aligned with tail idle fill.
- Two-wire clocked lanes (DotStar/APA102, etc.) may interpret extra shifted bits as valid payload.
- A shared parallel clock means all active lanes must emit the same total bit count per frame.

Proposal:
- Introduce protocol-owned pad policy for parallel clock-data assembly.
- Transport owns lane synchronization + DMA kickoff only.
- Protocol decides lane framing and safe padding bytes.

Interface sketch:

```cpp
struct ParallelLaneFrameInfo
{
	size_t payloadBytes;      // protocol payload size for this lane
	size_t targetFrameBytes;  // total bytes lane must emit this epoch
};

class IParallelPadPolicy
{
public:
	virtual ~IParallelPadPolicy() = default;

	// Called at epoch start with all lane payload sizes.
	virtual size_t computeTargetFrameBytes(std::span<const size_t> lanePayloadBytes) const = 0;

	// Write protocol-specific leading bytes (if any).
	virtual size_t writePrefix(uint8_t* dest,
							   size_t capacity,
							   uint8_t lane,
							   size_t payloadBytes,
							   size_t targetFrameBytes) const = 0;

	// Write protocol-specific trailing bytes/pad to reach target frame size.
	virtual size_t writeSuffixAndPad(uint8_t* dest,
									 size_t capacity,
									 uint8_t lane,
									 size_t payloadBytes,
									 size_t targetFrameBytes) const = 0;
};
```

How this fits strict-sync parallel transport:
1. Each lane submits payload (begin/transmit/end).
2. On first lane in epoch, transport snapshots participating lane mask.
3. When all lanes submitted, transport asks policy for `targetFrameBytes`.
4. Per lane stream = `prefix + payload + suffix/pad`.
5. Transport packs lane streams into bitplanes and starts one DMA transfer.

Default policies:
- `OneWireTailIdlePadPolicy`: no prefix, zero tail fill.
- `DotStarPadPolicy`: start frame/end frame semantics + deterministic tail clocks.
- `ShiftRegisterRightAlignPolicy` (optional): right-align payload at frame end for pure shift-register chips.

Benefits:
- Preserves strict-sync barrier behavior.
- Keeps protocol semantics out of hardware transports.
- Enables mixed-length lanes safely for two-wire protocols.