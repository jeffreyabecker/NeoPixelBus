# Compilation Flags

This document defines the canonical naming and dependency rules for factory compile flags.

## Goals

- Keep factory feature slicing explicit and predictable.
- Ensure flags can be reasoned about from name alone.
- Preserve a stable global off-switch for consumers that do not use factory APIs.
- Make dependency behavior deterministic (no hidden transitive enablement).

## Current Baseline

The current public baseline remains:

- `LW_FACTORY_SYSTEM_DISABLED`
  - Meaning: disable inclusion of `factory/Factory.h` from `LumaWave/Factory.h` and `LumaWave/All.h`.
  - Scope: factory module include gate.
  - Default: undefined (factory system enabled by default).

Existing subsystem-specific flag already in use:

- `LW_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS`
  - Meaning: expose SPI descriptor trait path (`NeoSpi` and SPI descriptor traits).
  - Scope: SPI descriptor/traits only.
  - Default: `1` (enabled).

- `LW_FACTORY_ENABLE_STATIC`
  - Meaning: enable static factory API surface (`makeBus` template path, descriptors/traits static path, shader/composite static helpers).
  - Scope: static factory includes and global exports in `factory/Factory.h`.
  - Default: `1` (enabled).

- `LW_FACTORY_ENABLE_DYNAMIC_BUILDER`
  - Meaning: enable runtime/dynamic factory surface (`MakeDynamicBus`, `DynamicBusBuilder`).
  - Scope: dynamic factory includes and global exports in `factory/Factory.h`.
  - Default: `1` (enabled).

- `LW_FACTORY_ENABLE_INI`
  - Meaning: enable INI/spec parser exposure (`BuildDynamicBusBuilderFromIni`, `IniReader`).
  - Scope: INI includes and global exports in `factory/Factory.h`.
  - Default: `1` (enabled).
  - Dependency: forced to `0` when `LW_FACTORY_ENABLE_DYNAMIC_BUILDER=0`.

- `LW_MAIN_HEADER_ENABLE_GLOBAL_NAMESPACE_IMPORTS`
  - Meaning: control whether `factory/Factory.h` re-exports factory symbols into global namespace via `using` declarations.
  - Scope: global namespace convenience imports from factory umbrella.
  - Default: `1` (enabled, backward-compatible).

## Canonical Naming Rules

All new factory subsystem flags must follow:

- `LW_FACTORY_ENABLE_<SUBSYSTEM>`

Rules:

1. `<SUBSYSTEM>` is uppercase snake case.
2. Positive enable flags are preferred over negative/disabling names.
3. `ENABLE` flags are boolean-ish (`0` or `1`) and must be documented with explicit default.
4. Flags must not encode platform in the name unless the subsystem itself is platform-specific.
5. One subsystem = one flag family; avoid multiple synonyms.

## Standard Semantics

- Unspecified flag means default value is used.
- `0` means disabled.
- `1` means enabled.
- `LW_FACTORY_SYSTEM_DISABLED` takes precedence over all `LW_FACTORY_ENABLE_*` flags.

Precedence order:

1. `LW_FACTORY_SYSTEM_DISABLED` (global hard off)
2. Subsystem `LW_FACTORY_ENABLE_*` values
3. Internal per-header/platform guards

## Factory Subsystem Flags

| Flag | Default | Controls | Depends On |
|------|---------|----------|------------|
| `LW_FACTORY_ENABLE_STATIC` | `1` | Static factory path (`makeBus`, static descriptors/traits, `MakeBus.h`, `MakeCompositeBus.h`, `MakeShader.h`) | None |
| `LW_FACTORY_ENABLE_DYNAMIC_BUILDER` | `1` | `DynamicBusBuilder` APIs and dynamic builder runtime composition path | None |
| `LW_FACTORY_ENABLE_INI` | `1` | INI/spec parsing helpers and builder-from-INI path (`IniReader`, `BuildDynamicBusBuilderFromIni`) | `LW_FACTORY_ENABLE_DYNAMIC_BUILDER=1` |
| `LW_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS` | `1` | SPI descriptor trait exposure (`NeoSpi`, SPI descriptor traits) | `LW_FACTORY_ENABLE_STATIC=1` recommended |
| `LW_MAIN_HEADER_ENABLE_GLOBAL_NAMESPACE_IMPORTS` | `1` | Global namespace imports from `factory/Factory.h` | None |

Dependency notes:

- `INI` requires dynamic builder semantics because INI flow materializes builder nodes before build.
- `STATIC` and `DYNAMIC_BUILDER` are intentionally independent to support static-only and runtime-builder-only consumers.
- `SPI_DESCRIPTOR_TRAITS` is a static-descriptor concern and should not force-enable static paths.
- `LW_MAIN_HEADER_ENABLE_GLOBAL_NAMESPACE_IMPORTS=0` keeps APIs namespaced (`lw::factory::...`) and avoids global symbol injection.

## Required Documentation Template for Each New Flag

Every factory flag added to code must be documented with:

- Purpose/scope (what headers/APIs are gated)
- Default value
- Dependencies and incompatibilities
- Precedence behavior with `LW_FACTORY_SYSTEM_DISABLED`
- One short usage example (build flag form)

## Example Build Defines

- PlatformIO:
  - `-D LW_FACTORY_ENABLE_STATIC=1`
  - `-D LW_FACTORY_ENABLE_DYNAMIC_BUILDER=0`
  - `-D LW_FACTORY_ENABLE_INI=0`

- Builder-only without INI/spec parser exports:
  - `-D LW_FACTORY_ENABLE_STATIC=0`
  - `-D LW_FACTORY_ENABLE_DYNAMIC_BUILDER=1`
  - `-D LW_FACTORY_ENABLE_INI=0`

- Keep factory enabled but disable global namespace convenience imports:
  - `-D LW_MAIN_HEADER_ENABLE_GLOBAL_NAMESPACE_IMPORTS=0`

- Global factory off:
  - `-D LW_FACTORY_SYSTEM_DISABLED`

## Rollout Notes

- Treat this file as the source of truth for factory compile-flag naming.
- New flags should be introduced without alias names unless migration is required.
- If migration aliases are needed, document deprecation window and planned removal release in this file.

## Color Compilation Flags

Color-related compile-time controls are not part of the factory subsystem, but they interact with factory/buffer sizing and should be documented alongside factory flag planning.

### Current Color Flags

| Flag | Default | Controls | Allowed Values | Notes |
|------|---------|----------|----------------|-------|
| `LW_COLOR_MINIMUM_COMPONENT_COUNT` | `4` | Minimum internal channel count for color storage (`DefaultColorType`/internal color padding) | `3`, `4`, `5` | Global memory/compatibility trade-off; `4` defaults to RGBW-style internal storage. |
| `LW_COLOR_MINIMUM_COMPONENT_SIZE` | `8` | Minimum internal component bit depth for color storage | `8`, `16` | May widen internal storage component type to `uint16_t` when set to `16`. |
| `LW_COLOR_MATH_BACKEND` | `lw::detail::ScalarColorMathBackend` | Color math backend used by `ColorMath` helpers | Backend template type macro | Must resolve as `Backend<TColor>` with required static math API. |

### Validation Constraints

- `LW_COLOR_MINIMUM_COMPONENT_COUNT` must be in the range `[3, 5]`.
- `LW_COLOR_MINIMUM_COMPONENT_SIZE` must be `8` or `16`.

These constraints are compile-time enforced in `src/colors/Color.h`.

### Interaction with Factory Planning

- Color flags change internal color storage behavior globally, including factory-created buses.
- Lowering `LW_COLOR_MINIMUM_COMPONENT_COUNT` can reduce root/shader memory footprints for RGB strips.
- Raising `LW_COLOR_MINIMUM_COMPONENT_SIZE` can increase memory usage and should be chosen intentionally.
- Factory compile flags (`LW_FACTORY_ENABLE_*`) and color flags should remain orthogonal: subsystem slicing must not implicitly modify color storage policy.

### Example Build Defines (Color)

- Memory-lean RGB internal storage:
  - `-D LW_COLOR_MINIMUM_COMPONENT_COUNT=3`
  - `-D LW_COLOR_MINIMUM_COMPONENT_SIZE=8`

- Force 16-bit internal component storage:
  - `-D LW_COLOR_MINIMUM_COMPONENT_SIZE=16`
