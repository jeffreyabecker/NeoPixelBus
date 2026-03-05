# Global Alias Candidate Plan (Draft)

Goal: improve ergonomics while avoiding symbol collisions in user applications.

Default policy:
- Keep `LumaWave.h` namespaced-only (`lw::...`).
- Do not alias into the C++ global namespace by default.
- If aliases are provided, expose them through an opt-in header only.

## Tier A (recommended opt-in aliases)

These names are distinctive and high-value for common sketches/examples.

- `Rgb8Color`
- `Rgbw8Color`
- `Rgbcw8Color`
- `Rgb16Color`
- `Rgbw16Color`
- `Rgbcw16Color`
- `PixelBus`
- `LightBus`
- `ReferenceBus`
- `AggregateBus`

## Tier B (consider later, after feedback)

Useful, but somewhat broader/less specific.

- `CCTWhiteBalanceShader`
- `AutoWhiteBalanceShader`
- `GammaShader`
- `CurrentLimiterShader`
- `Ws2812xProtocol`
- `Apa102Protocol`

## Tier C (do not alias globally)

High collision risk or too generic/infrastructural.

- `Color`
- `DefaultColorType`
- `span`
- `Topology`
- `TopologySettings`
- `ProtocolSettings`
- `TransportSettingsBase`
- `IProtocol`
- `ITransport`
- `IShader`
- Palette policy/meta/helper types (`BlendOp*`, `Wrap*`, `Nearest*`, traits)
- Alias-layer generic names (`Debug`, `None`) in `lw::protocols`

## Proposed rollout (before implementation)

1. Add `LumaWave/GlobalAliases.h` as an opt-in header.
2. Add Tier A aliases only.
3. Guard with explicit macro (for example `LW_ENABLE_GLOBAL_ALIASES`).
4. Add compile tests verifying default include has no global aliases.
5. Add compile tests verifying opt-in include exports only approved aliases.
6. Gather user feedback before introducing Tier B.
