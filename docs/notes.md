

UCS8904 -- https://github.com/Makuna/NeoPixelBus/issues/516


---

## Scope note: Narrowing protocol

- `NarrowingProtocol` was removed from the virtual layer.
- It is not relevant to parity with the original codebase (`src/original`).
- It is explicitly out of scope for the 1.0 feature set.

---

## Parallel Transport pad policy sketch (for two-wire protocols)

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

---

## Factory API status note

- The previous factory-function implementation was intentionally removed to avoid rewrite confusion.
- BusDriver classes and BusDriver ownership helpers remain available and are not part of this reset.
- Current factory direction and constraints are documented in `docs/factory-function-design-goals.md`.

---

## Shader settings tagging pattern

Shaders follow the same tagged settings convention used by protocols/transports:

- Define a dedicated settings struct per shader (for example `GammaShaderSettings<TColor>`).
- Inside the shader class, expose `using SettingsType = <ShaderName>Settings<TColor>;`.
- Prefer constructors that take `SettingsType` (for example `explicit GammaShader(SettingsType settings = {})`).
- Keep `IShader` as behavior-only (`apply(...)`) and do not require shared settings members at the interface level.

Example shape:

```cpp
template <typename TColor>
struct ExampleShaderSettings
{
	bool enabled = true;
};

template <typename TColor>
class ExampleShader : public IShader<TColor>
{
public:
	using SettingsType = ExampleShaderSettings<TColor>;

	explicit ExampleShader(SettingsType settings)
		: _enabled(settings.enabled)
	{
	}

	void apply(std::span<TColor> colors) override;

private:
	bool _enabled;
};
```

---

## Allocation behavior: protocol/transport settings and `ResourceHandle`

Question:
- Does storing `ResourceHandle<IProtocol>` / `ResourceHandle<ITransport>` in settings block compile-time/static allocation?

Answer:
- No. `ResourceHandle` is an ownership/lifetime wrapper and can be either:
	- borrowed (non-owning, no heap allocation), or
	- owning (constructed from `std::unique_ptr`, heap allocation).
- Static-friendly factory wiring is preserved when using borrowed handles and value-owned bus/protocol/transport objects.

Current practical behavior in virtual factories:
- `BusDriver` path (`src/virtual/buses/BusDriver.h`) stores transport/protocol by value in owning bus-driver types.
- Transport binding into protocol settings is typically borrowed (`settings.bus = transport`) and does not allocate.
- Heap allocation is only introduced by explicit owning helpers/paths (for example `std::make_unique`, `ProtocolTransportSettings`, and `makeOwned*Shader` helpers).

Guideline for no-heap factory usage:
- Prefer `factory::make*Bus(...)` APIs that pass concrete transport config + protocol settings by value.
- Prefer borrowed shader handles (or `NilShader`) when shader behavior is optional.
- Avoid ownership-convenience helpers that return owned handles if strict no-heap is required.

Implication:
- `ResourceHandle` in settings is not itself the obstacle; the obstacle is choosing owning construction paths.