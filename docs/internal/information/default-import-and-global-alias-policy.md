# Default Import and Global Alias Policy

Status: proposed baseline for implementation
Date: 2026-03-05
Scope: public API ergonomics for `#include <LumaWave.h>`

## 1. Problem Statement

The project needs a consistent rule for which symbols should be available by default and which symbols should require explicit qualification.

This is specifically about:

- default imports exposed by `LumaWave.h`
- optional global aliases for sketch ergonomics
- reducing collision risk in user applications

## 2. Current State Summary

Observed in repository as of 2026-03-05:

- `src/LumaWave.h` is include-only and does not export global aliases.
- Aggregation headers under `src/LumaWave/*.h` are include-only facades.
- Public symbols are primarily available in `lw` and nested domains (`lw::busses`, `lw::protocols`, `lw::transports`, `lw::colors`, `lw::shaders`).
- Examples commonly use `using namespace lw;`, but tests usually use explicit `lw::...` qualification.
- There are no existing header-level global `using lw::...` exports in `src/**/*.h`.

Related planning artifacts:

- `docs/internal/information/global-alias-candidate-plan.md` (draft candidate tiers)
- `docs/internal/information/lumawave-public-surface-dump.md` (surface inventory)

## 3. Policy Decision

Default behavior:

- Optionally export a small default symbol set for common usage.
- Control default symbol export with a compile-time flag that defaults to `true`.
- Keep canonical names under `lw` and domain namespaces.

Advanced usage behavior:

- Keep advanced symbols namespaced and require explicit qualification.
- Provide a way to disable default symbol exports by setting the compiler flag to `false`.
- Export only an approved subset of common symbols when enabled.

Rationale:

- Avoid collisions with user code and other libraries.
- Preserve explicit architecture boundaries (buses/protocols/transports/shaders).
- Keep fast paths and contracts predictable while still allowing user convenience when explicitly requested.

## 4. Symbol Tiering

## Tier A: approved for default global aliases first

High-frequency and distinctive symbols with low ambiguity.

- `Rgb8Color`
- `Rgbw8Color`
- `Rgbcw8Color`
- `Rgb16Color`
- `Rgbw16Color`
- `Rgbcw16Color`
- `Topology`
- `TopologySettings`
- `PixelBus` (maps to `lw::busses::PixelBus`)
- `Light` (maps to `lw::busses::LightBus`)
- `Color` -- this is a global default color symbol 

## Tier B: evaluate after Tier A feedback

Useful but with broader semantic footprint.

- `makePixelBus` (maps to `lw::busses::makePixelBus`)
- `Ws2812` / `APA102` protocol aliases
- `CCTWhiteBalanceShader`
- `AutoWhiteBalanceShader`
- `GammaShader`
- `CurrentLimiterShader`

## Tier C: never globally aliased

Collision-prone, infrastructural, or overly generic symbols.


- `DefaultColorType`
- `span`
- `ReferenceBus` (maps to `lw::busses::ReferenceBus`)
- `AggregateBus` (maps to `lw::busses::AggregateBus`)
- `ProtocolSettings`
- `TransportSettingsBase`
- `IProtocol`
- `ITransport`
- `IShader`
- palette policy/meta/helper names (`Blend*`, `Wrap*`, `Nearest*`, traits)
- alias-layer generic names (`Debug`, `None`)

## 5. Implementation Shape

Introduce a new public header for default symbol export behavior:

- `src/LumaWave/GlobalAliases.h`

Expected behavior:

- Enables default symbol export unless `LW_ENABLE_GLOBAL_ALIASES` is explicitly set to `0`.
- Exports only approved aliases (Tier A initially).
- Uses explicit `using` declarations in global scope for selected names.

Example pattern:

```cpp
#pragma once
#include "LumaWave/All.h"

#ifndef LW_ENABLE_GLOBAL_ALIASES
#define LW_ENABLE_GLOBAL_ALIASES 1
#endif

#if LW_ENABLE_GLOBAL_ALIASES
using lw::Rgb8Color;
using lw::Rgbw8Color;
using lw::Rgbcw8Color;
using lw::Rgb16Color;
using lw::Rgbw16Color;
using lw::Rgbcw16Color;
using lw::busses::PixelBus;
using lw::busses::LightBus;
using lw::busses::ReferenceBus;
using lw::busses::AggregateBus;
#endif
```

## 6. Example Authoring Guidance

For sketches and examples:

- Prefer explicit imports over `using namespace lw;`.
- Use narrow declarations in anonymous namespace or local scope.

Preferred style:

```cpp
using lw::Rgb8Color;
using lw::busses::PixelBus;
using lw::protocols::Ws2812;
using lw::transports::SpiTransport;
```

Avoid:

```cpp
using namespace lw;
```

Reason:

- keeps intent explicit
- reduces accidental name shadowing
- mirrors architecture boundaries in example code

## 7. Validation and Tests

Minimum acceptance gates for rollout:

1. Compile test: default include path exports only approved Tier A aliases.
2. Compile test: disabling flag (`LW_ENABLE_GLOBAL_ALIASES=0`) produces namespace-only behavior.
3. Run native contract checks:
   - `pio test -e native-test`
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile`

Optional stricter checks:

- static assertions that Tier C names are not globally visible when default aliases are enabled
- example compile checks confirming explicit-import style remains valid

## 8. Rollout Plan

Phase 1:

- add `LumaWave/GlobalAliases.h` with Tier A aliases enabled by default
- add compile tests for default-on and explicit-disable behavior

Phase 2:

- gather user feedback from examples and issue reports
- evaluate Tier B additions one symbol group at a time

Phase 3:

- keep Tier C permanently excluded
- document final supported aliases in public docs if stable

## 9. Non-Goals

- No broad/global export of advanced or infrastructural symbols.
- No broad `using namespace ...` directives in library headers.
- No compatibility aliases that blur domain ownership boundaries.

## 10. Decision Record

This policy intentionally enables a small default symbol export set for common usage through a compiler flag that defaults to enabled, while keeping advanced symbols explicitly namespaced.
