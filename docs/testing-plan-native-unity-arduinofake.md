# Native Testing Plan (PlatformIO + Unity + ArduinoFake)

## Goal

Establish a host-side automated test workflow using:

- PlatformIO `native` test environment
- Unity as the test framework
- ArduinoFake for Arduino API mocking

The test suite is organized into three categories:

- `busses`
- `shaders`
- `protocols`

This enables fast feedback for logic-level validation without requiring RP2040 hardware.

---

## Scope

### In scope

- Add a dedicated PlatformIO native test environment
- Add Unity + ArduinoFake as test dependencies
- Create a category-based `test/` layout
- Provide shared test support/fake bootstrap utilities
- Add initial smoke tests in each category
- Define local and CI test commands

### Out of scope (initial phase)

- Hardware timing/peripheral validation (PIO/DMA/ISR behavior)
- Full transport integration with physical IO
- Performance benchmarking in host tests

---

## Proposed Directory Layout

```text
test/
  support/
    FakeArduinoSetup.h
    FakeArduinoSetup.cpp
    TestHelpers.h

  busses/
    test_bus_smoke/
      test_main.cpp

  shaders/
    test_shader_smoke/
      test_main.cpp

  protocols/
    test_protocol_smoke/
      test_main.cpp
```

Conventions:

- One focused test package per behavior area.
- Shared helpers stay in `test/support`.
- Keep test names descriptive and category-prefixed.

---

## PlatformIO Configuration Plan

Add a new environment to `platformio.ini` for native tests.

### Environment objectives

- Use `platform = native`
- Compile with C++23 (`-std=gnu++23`)
- Reuse core include paths where possible
- Keep embedded-only flags isolated from native tests

### Dependency objectives

- Unity enabled for test runner
- ArduinoFake added as test dependency

### Command objectives

- Run all native tests:
  - `pio test -e native-test`
- Run single category by directory filter:
  - `pio test -e native-test -f busses`
  - `pio test -e native-test -f shaders`
  - `pio test -e native-test -f protocols`

---

## Fake Strategy (ArduinoFake)

Use a centralized bootstrap/reset pattern for mocks:

- `FakeArduinoSetup` initializes default fake behavior used by most tests.
- Each test `setUp()` resets fakes to avoid cross-test contamination.
- Avoid ad-hoc fake setup in each test file unless scenario-specific.

Target APIs to fake first:

- Timing (`millis`, `micros`, `delay`)
- GPIO interactions (`pinMode`, `digitalWrite`, `digitalRead`)
- Optional serial paths used by debug/helper flows

---

## Category Test Plan

## 1) Busses

Primary focus:

- Construction and begin/show lifecycle behavior
- Pixel index and buffer bounds behavior
- Segment mapping and count invariants

Initial smoke coverage:

- Create bus with minimal valid config
- Verify no-op / empty behavior is stable
- Verify expected state transitions after begin/show

## 2) Shaders

Primary focus:

- Deterministic color transforms
- Edge values and clamping behavior
- Ordering/idempotence expectations (where applicable)

Initial smoke coverage:

- Apply shader to small fixed input set
- Assert output values exactly match expected set

## 3) Protocols

Primary focus:

- Encode/frame formatting correctness
- Channel ordering and byte layout rules
- Invalid input guard behavior

Initial smoke coverage:

- Encode simple pixel input and verify byte stream shape
- Validate expected header/footer/padding semantics where defined

---

## Phased Implementation

### Phase 1 — Environment bootstrap

1. Add `native-test` env in `platformio.ini`
2. Add Unity + ArduinoFake dependencies
3. Validate with an empty smoke test

### Phase 2 — Test structure

1. Create `test/support` utilities
2. Create `busses`, `shaders`, `protocols` test roots
3. Add one smoke test package per category

### Phase 3 — Baseline coverage

1. Add core assertions for each category
2. Ensure deterministic execution on host
3. Document category-specific patterns in test files

### Phase 4 — CI integration

1. Add `pio test -e native-test` to CI
2. Keep firmware build as separate CI step
3. Gate PRs on native test pass

---

## Definition of Done

- Native test environment runs locally with a single command
- Category folders exist and are discoverable by PlatformIO
- Shared fake setup utilities are used by tests
- At least one passing smoke test exists for each category
- CI runs native tests on each PR

---

## Risks and Mitigations

- Risk: host/native differences from embedded runtime behavior.
  - Mitigation: treat native tests as logic verification only; keep on-device validation for timing-critical paths.

- Risk: brittle tests due to over-mocking internals.
  - Mitigation: mock only Arduino boundaries and assert public behavior.

- Risk: dependency/API drift in ArduinoFake.
  - Mitigation: centralize wrappers in `test/support` and avoid direct deep coupling in every test.
