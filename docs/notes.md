

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

## Factory API sketch for static allocation + owned convenience

Goal:
- Keep factory ergonomics for beginners.
- Make static allocation first-class (no hidden heap requirement).
- Preserve explicit lifetime ownership semantics already modeled by `ResourceHandle`.

### Factory scope decision

- Factory API is static-first only.
- Dynamic and hybrid ownership wiring is intentionally left to consumers.
- Avoid duplicated helper surfaces for mixed allocation models.

### MVP surface (WS2812x, one-wire)

```cpp
template <typename TTransport, typename TProtocol>
class OwningPixelBusT
    : private ProtocolStateT<TTransport, TProtocol>
    , public PixelBusT<typename TProtocol::ColorType>
{
    // ...
};

template <typename TTransport, typename TProtocol, typename... TProtocolArgs>
OwningPixelBusT<TTransport, TProtocol> makeOwningPixelBus(
    uint16_t pixelCount,
    typename TTransport::TransportConfigType transportConfig,
    TProtocolArgs&&... protocolArgs);

template <typename TTransport, typename TColor = Rgb8Color>
using Ws2812xOwningPixelBusT = OwningPixelBusT<TTransport, Ws2812xProtocol<TColor>>;

template <typename TTransport, typename TColor = Rgb8Color>
Ws2812xOwningPixelBusT<TTransport, TColor> makeWs2812xBus(
    uint16_t pixelCount,
    const char* channelOrder,
    typename TTransport::TransportConfigType transportConfig);
```

### Owning PixelBus pattern for easy static wiring

```cpp
template <typename TTransport, typename TProtocol>
class OwningPixelBusT
    : private ProtocolStateT<TTransport, TProtocol>
    , public PixelBusT<typename TProtocol::ColorType>
{
	using ColorType = typename TProtocol::ColorType;
	using PixelBusType = PixelBusT<ColorType>;
	using ProtocolStateType = ProtocolStateT<TTransport, TProtocol>;

	template <typename... TProtocolArgs>
	OwningPixelBusT(uint16_t pixelCount,
				typename TTransport::TransportConfigType transportConfig,
				TProtocolArgs&&... protocolArgs)
		: ProtocolStateType(transportConfig, std::forward<TProtocolArgs>(protocolArgs)...)
		, PixelBusType(pixelCount, static_cast<ProtocolStateType&>(*this).protocol())
	{
	}
};
```

This gives one-line static initialization with direct bus API:

```cpp
static auto leds = npb::factory::makeWs2812xBus<npb::RpPioOneWireTransport>(
	PixelCount,
	npb::ChannelOrder::GRB,
	transportConfig);

leds.begin();
leds.setPixelColor(0, npb::Rgb8Color{255, 0, 0});
leds.show();
```

Notes:
- Keep factory API allocation-model focused (static-first).
- If a consumer needs dynamic or mixed ownership in one program, they should wire transport/protocol/bus directly.

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