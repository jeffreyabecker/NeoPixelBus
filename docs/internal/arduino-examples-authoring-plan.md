# Arduino Examples Authoring Plan

Status: Active (supersedes any prior informal example-authoring notes)

## Goal

Establish a complete, maintainable Arduino examples set that demonstrates static factory usage, dynamic builder usage, and runtime configuration workflows, while being easy to validate with platform integration tests.

## Scope Baseline (From Current Session)

1. Include platform integration tests under `platform-tests/`.
2. Include all examples represented in `docs/usage/make-bus-static-equivalents.md` under `examples/`.
3. Include equivalent examples using the builder API under `examples/builder/`.
4. Include equivalent examples using the config API under `examples/config/`.
5. Add two root-level beginner examples under `examples/`:
  - `hello-ws2812`
  - `hello-apa102`
  Both must:
  - use single-strand buses with `PlatformDefault` transport,
  - use no shaders,
  - declare root-level compile-time defines for `dataPin` and `pixelCount`,
  - for `hello-apa102` only, also declare root-level define for `clockPin`,
  - render a rotating rainbow on the strip.
6. In `examples/config/`, include at least one end-to-end sketch that:
   - uses LittleFS for config persistence,
   - sets up a bus at the beginning of `loop()`,
   - exposes serial commands for:
     - reading bus pixel colors,
     - writing bus pixel colors,
     - reading bus configuration,
     - writing configuration and rebuilding the bus.

## Additional Recommended Coverage

### Topology and Composition

- Topology variants:
  - strip,
  - serpentine panel,
  - tiled panel/mosaic.
- Aggregate composition examples:
  - linear aggregate,
  - tiled aggregate with `TopologySettings`.

### Protocol/Transport Pairing Matrix

- Ensure examples cover at least one representative one-wire path and one representative clocked path per major transport family.
- Keep category correctness explicit (`OneWireTransportTag`, `TransportTag`, `AnyTransportTag`) in each relevant example.

### Shader and Power Path Coverage

- Add deterministic shader pipeline examples centered on:
  - `CurrentLimiterShader`,
  - `AggregateShader` composition.
- Include a power-aware example showing brightness/current limit constraints for real-world strip lengths.

### Config API Robustness

- Corrupt/invalid config handling with safe fallback defaults.
- Config schema versioning + migration sample.
- Rebuild behavior guidance:
  - rebuild only on change,
  - explicit policy for preserving vs clearing pixel state on rebuild,
  - serial-visible error/status reporting.

### Runtime/Loop Behavior

- Non-blocking loop pattern combining:
  - frame cadence,
  - serial command handling,
  - conditional bus rebuild.

### Factory Ergonomics and Parity

- Include examples that show when color type inference works and when explicit color type is required.
- Include “same output, different API” parity examples across:
  - static (`makeBus`),
  - builder (`DynamicBusBuilder`),
  - config-based setup.

## Structure Proposal

- `examples/` (root) — beginner hello sketches (`hello-ws2812`, `hello-apa102`).
- `examples/static/` — static `makeBus(...)` examples mirrored from usage docs.
- `examples/builder/` — dynamic builder equivalents.
- `examples/config/` — persisted config + serial mutation workflows.
- `platform-tests/` — integration validation sketches/tests for example classes.

## Mapping Source Docs to Examples

- `docs/usage/make-bus-static-equivalents.md` → `examples/static/*`
- `docs/usage/dynamic-bus-builder.md` → `examples/builder/*`
- `docs/usage/dynamic-bus-builder-ini-spec.md` + config usage docs → `examples/config/*`

## Acceptance Criteria

- Root-level hello examples exist for WS2812 and APA102 and implement define-driven pin/pixel configuration plus rotating rainbow output.
- Every major usage pattern in static and dynamic usage docs has a runnable Arduino example.
- Builder/config examples include at least one direct parity mapping to a static equivalent.
- Config example supports read/write of pixels and config through serial and persists via LittleFS.
- Platform integration tests include representative coverage of static, builder, and config flows.
- All examples compile for at least the project’s default RP2040 workflow.
