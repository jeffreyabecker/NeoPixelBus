# Arduino Examples TODO

Tracks implementation tasks for `arduino-examples-authoring-plan.md`.

## 0) Dependency Gating (Do Not Assume Implemented)

Before implementing any example, explicitly confirm required behavior exists in code (not just docs). If behavior is missing, keep the example as blocked and note fallback scope.

- [ ] For each example sketch, add an implementation status note: `Implemented`, `Partially Implemented`, or `Blocked by Dependency`.
- [ ] For each blocked example, record:
  - [ ] missing capability,
  - [ ] linked internal backlog item,
  - [ ] temporary fallback behavior (if any),
  - [ ] unblock criteria.
- [ ] Do not mark any example task complete until dependency status is verified against actual code paths.

### Known Dependency Risks (from internal backlog)

- [ ] `fullRefreshOnly` / coordinated refresh behavior (internal backlog):
  - [ ] If missing, avoid examples that claim synchronized multi-transport refresh semantics.
- [ ] Non-reallocating settings alteration + common composite-bus interfaces (internal backlog):
  - [ ] If missing, config/shader examples must use rebuild-on-change flows instead of in-place mutation claims.
- [ ] Compile-time allocatable bus path (internal backlog):
  - [ ] If missing, avoid examples that claim fully static/no-heap bus allocation.

## 1) Planning and Inventory

- [x] Build a one-to-one inventory from `docs/usage/make-bus-static-equivalents.md` sections to concrete static example sketch names (first pass).
- [x] Build parity inventory from static examples to builder equivalents (first pass).
- [x] Build parity inventory from static examples to config equivalents where feasible (first pass).
- [ ] Add root-level beginner inventory entries for `hello-ws2812` and `hello-apa102`.

### Root-Level Beginner Examples (Required)

- [ ] Add `examples/hello-ws2812/hello-ws2812.ino`:
  - [ ] Single-strand bus using `PlatformDefault` transport.
  - [ ] No shaders.
  - [ ] Root-level defines for `dataPin` and `pixelCount`.
  - [ ] Rotating rainbow output.
- [ ] Add `examples/hello-apa102/hello-apa102.ino`:
  - [ ] Single-strand bus using `PlatformDefault` transport.
  - [ ] No shaders.
  - [ ] Root-level defines for `dataPin`, `clockPin`, and `pixelCount`.
  - [ ] Rotating rainbow output.

### First-Pass Example Inventory Matrix

| Source Section | Static Sketch (`examples/static/`) | Builder Sketch (`examples/builder/`) | Config Sketch (`examples/config/`) | Notes |
|---|---|---|---|---|
| Minimal pattern | `core-minimal` | `core-minimal` | `core-minimal` | Baseline parity triplet. |
| Pre-build buffer planning (`getFactory`) | `factory-buffer-planning` | `factory-buffer-planning` | `factory-buffer-planning` | Config variant reports required size + allocation status. |
| One-wire manual timing + 4-step | `onewire-manual-timing` | `onewire-manual-timing` | `onewire-manual-timing` | Include explicit `clockRateHz` behavior. |
| Platform-exclusive interface | `platform-exclusive` | `platform-exclusive` | `platform-exclusive` | Use per-platform compile guards. |
| Print transport config | `transport-neoprint-debug` | `transport-neoprint-debug` | `transport-neoprint-debug` | Keep category-compatibility note in comments. |
| Nil transport bus | `transport-nil` | `transport-nil` | `transport-nil` | Useful for dry-run/test style demos. |
| Single shader on bus | `shader-single` | `shader-single` | `shader-single` | Shader settings mutable in config path. |
| Hierarchical shader stack | `shader-hierarchical` | `shader-hierarchical` | `shader-hierarchical` | Builder path may compose post-build; document boundary. |
| Aggregate linear topology | `topology-aggregate-linear` | `topology-aggregate-linear` | `topology-aggregate-linear` | Direct parity target. |
| Aggregate tiled topology | `topology-aggregate-tiled` | `topology-aggregate-tiled` | `topology-aggregate-tiled` | Must include `TopologySettings` mapping. |
| Protocol recipes (bundle) | `protocol-recipes-bundle` | `protocol-recipes-bundle` | `protocol-recipes-bundle` | Single sketch with selectable recipe id. |
| Larger interface color than strip color | `color-interface-wide` | `color-interface-wide` | `color-interface-wide` | Validate explicit interface/build color usage. |
| One-wire non-default channel order | `onewire-channel-order` | `onewire-channel-order` | `onewire-channel-order` | Include serial visibility of active channel order. |

### Protocol Recipe Decomposition (Optional Split if Bundle Gets Too Large)

- [ ] Option A: keep `protocol-recipes-bundle` as single recipe-selector sketch.
- [ ] Option B: split into per-protocol examples if size/readability degrades:
  - [ ] `protocol-apa102`
  - [ ] `protocol-hd108`
  - [ ] `protocol-ws2812`
  - [ ] `protocol-ws2813`
  - [ ] `protocol-tm1829`
  - [ ] `protocol-pixie`
  - [ ] `protocol-ucs8903`
  - [ ] `protocol-ucs8904`

### Config-First E2E Inventory Anchor

- [ ] Add `config-runtime-serial-littlefs` as the canonical end-to-end config example:
  - [ ] LittleFS-backed config persistence.
  - [ ] Bus create/recreate at start of `loop()` based on pending config revision.
  - [ ] Serial commands for pixel read/write.
  - [ ] Serial commands for config read/write + rebuild.
  - [ ] Corrupt-config fallback + schema version handling.

## 2) Folder and Naming Conventions

- [ ] Confirm/create target folders:
  - [ ] `examples/hello-ws2812/`
  - [ ] `examples/hello-apa102/`
  - [ ] `examples/static/`
  - [ ] `examples/builder/`
  - [ ] `examples/config/`
  - [ ] `platform-tests/` (Arduino `.ino` integration tests; intentionally outside `test/` native-test system)
- [x] Define consistent sketch naming convention (`<domain>-<variant>`).

### Sketch Naming Convention

Primary format:

- `<domain>-<variant>`

Optional qualifiers (append only when needed for clarity):

- `<domain>-<variant>-<protocol>`
- `<domain>-<variant>-<platform>`

Rules:

- Use lowercase kebab-case only.
- Keep names concise and stable; avoid board pin values and transient details in names.
- Use the same base name across `examples/static`, `examples/builder`, and `examples/config` for parity.
- Reserve platform suffixes for platform-exclusive sketches only (for example `-rp2040`, `-esp32`).
- For platform integration tests under `platform-tests/`, use `pt-<transport-family>-<scenario>`.
- Example sketch names must not start with `ex_`.
- Root-level beginner sketches are explicitly allowed and required: `hello-ws2812`, `hello-apa102`.

Domain vocabulary (preferred):

- `core`, `factory`, `onewire`, `transport`, `shader`, `topology`, `protocol`, `color`, `config`.

Variant vocabulary (preferred):

- `minimal`, `buffer-planning`, `manual-timing`, `platform-exclusive`, `neoprint-debug`, `nil`, `single`, `hierarchical`, `aggregate-linear`, `aggregate-tiled`, `recipes-bundle`, `interface-wide`, `channel-order`, `runtime-serial-littlefs`.

Examples:

- `examples/static/core-minimal/core-minimal.ino`
- `examples/builder/topology-aggregate-tiled/topology-aggregate-tiled.ino`
- `examples/config/config-runtime-serial-littlefs/config-runtime-serial-littlefs.ino`
- `platform-tests/pt-onewire-rp2040-smoke/pt-onewire-rp2040-smoke.ino`

## 3) Static Example Set

- [ ] Add all static examples represented in `make-bus-static-equivalents.md`.
- [ ] Add topology-focused static examples (strip, serpentine, tiled).
- [ ] Add protocol/transport category representative static examples.

## 4) Builder Example Set

- [ ] Add builder equivalents for each static baseline example.
- [ ] Add explicit parity notes in comments or companion docs where behavior maps 1:1.
- [ ] Add builder examples for aggregate and tiled composition.

## 5) Config Example Set

- [ ] Add core config example using LittleFS.
- [ ] In config example, construct bus at start of `loop()` according to current architecture intent.
- [ ] Add serial command: read pixel colors.
- [ ] Add serial command: write pixel colors.
- [ ] Add serial command: read config.
- [ ] Add serial command: write config and rebuild bus.
- [ ] Add corrupt/invalid config fallback flow.
- [ ] Add config schema versioning + migration flow.
- [ ] Add explicit policy for pixel state across rebuild.
- [ ] Call out any runtime config behavior that is rebuild-based due to missing in-place mutation support.

## 6) Shader, Power, and Runtime Patterns

- [ ] Add deterministic shader example with `CurrentLimiterShader`.
- [ ] Add aggregate shader composition example.
- [ ] Add power-aware brightness/current budgeting example.
- [ ] Add non-blocking loop example with frame cadence + serial + conditional rebuild.

## 7) Platform Integration Tests

Platform integration tests in this plan are Arduino sketch-based (`.ino`) and must live outside the native unit test system (`test/`, `native-test`).
These tests are scoped to platform-specific transport validation only (hardware/peripheral integration), not full subsystem validation.

- [ ] Add `platform-tests` coverage for platform-specific transports (for example RP2040 PIO, ESP32 RMT, SPI-class hardware transports).
- [ ] Add transport-focused smoke sketches that verify init, write/show path, and expected transport-level behavior.
- [ ] Ensure one-wire and clocked transport families each have at least one platform-test sketch.
- [ ] Keep topology/shader/protocol behavioral coverage in native tests and example-level validation, not platform-tests.

## 8) Documentation and Cross-Links

- [ ] Cross-link new example sketches from usage docs where applicable.
- [ ] Add/update README indexes for `examples/` and `platform-tests/`.
- [ ] Include root-level hello examples (`hello-ws2812`, `hello-apa102`) in examples index/docs.
- [ ] Document expected board/environment targets per example.
- [ ] For each example, document dependency assumptions and clearly label any blocked/pending behavior.

## 9) Validation Gates

- [ ] Verify compile for default RP2040 workflow.
- [ ] Run targeted native tests related to changed contracts where relevant.
- [ ] Run/verify platform smoke tests for representative platform-specific transports (not full static/builder/config subsystem parity).
