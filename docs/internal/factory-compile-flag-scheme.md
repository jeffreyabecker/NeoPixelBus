# Factory Compile-Flag Naming Scheme

This document defines the canonical naming and dependency rules for factory compile flags.

## Goals

- Keep factory feature slicing explicit and predictable.
- Ensure flags can be reasoned about from name alone.
- Preserve a stable global off-switch for consumers that do not use factory APIs.
- Make dependency behavior deterministic (no hidden transitive enablement).

## Current Baseline

The current public baseline remains:

- `LW_FACTORY_SYSTEM_DISABLED`
  - Meaning: disable inclusion of `factory/Factory.h` from `LumaWave.h`.
  - Scope: umbrella include gate.
  - Default: undefined (factory system enabled by default).

Existing subsystem-specific flag already in use:

- `LW_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS`
  - Meaning: expose SPI descriptor trait path (`NeoSpi` and SPI descriptor traits).
  - Scope: SPI descriptor/traits only.
  - Default: `1` (enabled).

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

## Planned Subsystem Flags

These names are reserved for upcoming slicing work and should be used as-is when implemented.

| Flag | Default | Controls | Depends On |
|------|---------|----------|------------|
| `LW_FACTORY_ENABLE_STATIC` | `1` | Static factory path (`makeBus`, static descriptors/traits, `MakeBus.h`, `MakeCompositeBus.h`, `MakeShader.h`) | None |
| `LW_FACTORY_ENABLE_DYNAMIC_BUILDER` | `1` | `DynamicBusBuilder` APIs and dynamic builder runtime composition path | None |
| `LW_FACTORY_ENABLE_INI` | `1` | INI/spec parsing helpers and builder-from-INI path (`IniReader`, `BuildDynamicBusBuilderFromIni`) | `LW_FACTORY_ENABLE_DYNAMIC_BUILDER=1` |
| `LW_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS` | `1` | SPI descriptor trait exposure (`NeoSpi`, SPI descriptor traits) | `LW_FACTORY_ENABLE_STATIC=1` recommended |

Dependency notes:

- `INI` requires dynamic builder semantics because INI flow materializes builder nodes before build.
- `STATIC` and `DYNAMIC_BUILDER` are intentionally independent to support static-only and runtime-builder-only consumers.
- `SPI_DESCRIPTOR_TRAITS` is a static-descriptor concern and should not force-enable static paths.

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

- Global factory off:
  - `-D LW_FACTORY_SYSTEM_DISABLED`

## Rollout Notes

- Treat this file as the source of truth for factory compile-flag naming.
- New flags should be introduced without alias names unless migration is required.
- If migration aliases are needed, document deprecation window and planned removal release in this file.
