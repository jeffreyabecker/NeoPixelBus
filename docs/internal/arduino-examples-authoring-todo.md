# Arduino Examples TODO

Tracks implementation tasks for `arduino-examples-authoring-plan.md`.

## 1) Planning and Inventory

- [ ] Build a one-to-one inventory from `docs/usage/make-bus-static-equivalents.md` sections to concrete static example sketch names.
- [ ] Build parity inventory from static examples to builder equivalents.
- [ ] Build parity inventory from static examples to config equivalents where feasible.

## 2) Folder and Naming Conventions

- [ ] Confirm/create target folders:
  - [ ] `examples/static/`
  - [ ] `examples/builder/`
  - [ ] `examples/config/`
  - [ ] `platform-tests/`
- [ ] Define consistent sketch naming convention (`ex_<domain>_<variant>`).

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

## 6) Shader, Power, and Runtime Patterns

- [ ] Add deterministic shader example with `CurrentLimiterShader`.
- [ ] Add aggregate shader composition example.
- [ ] Add power-aware brightness/current budgeting example.
- [ ] Add non-blocking loop example with frame cadence + serial + conditional rebuild.

## 7) Platform Integration Tests

- [ ] Add `platform-tests` coverage for representative static examples.
- [ ] Add `platform-tests` coverage for representative builder examples.
- [ ] Add `platform-tests` coverage for representative config examples.
- [ ] Add at least one topology and one shader-oriented integration test.

## 8) Documentation and Cross-Links

- [ ] Cross-link new example sketches from usage docs where applicable.
- [ ] Add/update README indexes for `examples/` and `platform-tests/`.
- [ ] Document expected board/environment targets per example.

## 9) Validation Gates

- [ ] Verify compile for default RP2040 workflow.
- [ ] Run targeted native tests related to changed contracts where relevant.
- [ ] Run/verify platform smoke tests for at least one static, one builder, one config example.
