# Legacy Factory Removal Plan

## Goal

Remove the legacy factory surface and all runtime builder/INI paths:

- `src/factory/*`
- `src/LumaWave/Factory.h`
- Legacy factory references in docs/tests/examples/package metadata

Target architecture after removal:

- Virtual-first explicit composition via `PixelBus<Protocol, Transport, Shader>` and `makePixelBus(...)`
- No INI parsing or runtime bus graph builder in core library

## Phase 0: Preflight Guardrails (This Change Set)

1. Introduce one explicit kill switch for legacy umbrella include:
	- `LW_ENABLE_LEGACY_FACTORY` (default `1` for migration window)
2. Keep `LW_FACTORY_SYSTEM_DISABLED` as global hard-off override.
3. Emit compile-time deprecation warning when legacy umbrella is enabled.

## Phase 1: Consumer Migration Window

1. Build/test with:
	- `-D LW_ENABLE_LEGACY_FACTORY=0`
	- `-D LW_FACTORY_ENABLE_DYNAMIC_BUILDER=0`
	- `-D LW_FACTORY_ENABLE_INI=0`
2. Remove direct includes of:
	- `LumaWave/Factory.h`
	- `factory/*`
3. Convert runtime/builder composition call sites to explicit static construction:
	- Protocol settings
	- Transport settings
	- `makePixelBus(...)` or `PixelBus<...>`

## Phase 2: Test and Docs Realignment

1. Delete/replace contracts that only validate legacy systems:
	- `test/contracts/test_dynamic_factory_config_parser_and_build/*`
	- `test/contracts/test_ini_reader_first_pass_compile/*`
	- `test/contracts/test_dynamic_bus_builder_ini_reader_compile/*`
2. Remove/replace docs that describe removed APIs:
	- `docs/usage/dynamic-bus-builder.md`
	- `docs/usage/dynamic-bus-builder-ini-spec.md`
	- `docs/usage/helpers/ini-reader.md`
3. Ensure docs point to static equivalents and factory-lite APIs.

## Phase 3: Code Deletion

1. Remove `src/factory/*` and subfolders.
2. Remove `src/LumaWave/Factory.h`.
3. Remove `#include "LumaWave/Factory.h"` from `src/LumaWave/All.h`.
4. Remove `LumaWave/Factory.h` from `library.properties` includes list.
5. Remove legacy factory compile flags from docs and code paths:
	- `LW_FACTORY_ENABLE_STATIC`
	- `LW_FACTORY_ENABLE_DYNAMIC_BUILDER`
	- `LW_FACTORY_ENABLE_INI`
	- `LW_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS`
	- `LW_MAIN_HEADER_ENABLE_GLOBAL_NAMESPACE_IMPORTS`

## Phase 4: Post-Removal Validation

1. Run native contract suites that remain in scope.
2. Run smoke examples for RP2040 default workflow.
3. Confirm no references to removed headers remain:
	- search for `factory/`
	- search for `LumaWave/Factory.h`
	- search for `DynamicBusBuilder`, `IniReader`, `MakeDynamicBus`

## Exit Criteria

1. No source/test/doc includes for removed factory/builder/INI components.
2. `LumaWave/All.h` and `LumaWave.h` expose only non-legacy public surface.
3. CI and local native tests pass on remaining supported APIs.
