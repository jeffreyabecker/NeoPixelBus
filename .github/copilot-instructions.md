# Copilot Instructions for NeoPixelBus

## Project Context

- Repository: `NeoPixelBus_by_Makuna`.
- Primary target: PlatformIO + Arduino core, with RP2040/Pico2W as the default workflow.
- Language standard for active code paths: C++17 (`-std=gnu++17`) in primary and native-test environments.
- Architecture is virtual-first and centered on explicit seams: `IPixelBus`, `IShader`, `IProtocol`, and `ITransport`.

## Source of Truth

When generating or modifying code, align with these docs first:

- `docs/consumer-virtual-architecture.md`
- `docs/protocol-transport-contracts.md`
- `docs/factory-function-design-goals.md`
- `docs/cxx17-migration-plan.md`
- `docs/testing-plan-native-unity-arduinofake.md`
- `docs/testing-spec-hierarchical.md`
- `docs/testing-spec-bus.md`
- `docs/testing-spec-colors-shaders.md`
- `docs/testing-spec-transports.md`
- `docs/testing-spec-protocols.md`
- `docs/testing-spec-byte-stream.md`
- `docs/post-truncation-example-authoring-plan.md` (when authoring examples)

## C++ and Style Rules

- Compile target for active environments is C++17 (`-std=gnu++17`); do not introduce changes that require compiling project code as C++20/C++23.
- Follow existing project style: Allman braces, 4-space indentation, `#pragma once` in headers.
- Do not introduce C++20-only surface features in active virtual-first headers.
	- Avoid `concept`, `requires`, and direct dependency on C++20-only APIs at public seam boundaries.
- Use the compatibility layer in `src/core/Compat.h`:
	- Prefer `npb::span` over direct `std::span` in project headers.
	- Prefer `npb::remove_cvref_t` over direct `std::remove_cvref_t` where compatibility matters.
- Avoid exceptions unless explicitly needed; keep hot paths simple and predictable.
- Keep virtual dispatch at seam boundaries and avoid per-pixel virtual overhead.

## Architecture and Ownership Rules

- Keep responsibilities separated:
	- `IPixelBus`: storage/composition + frame lifecycle.
	- `IShader`: pixel transforms.
	- `IProtocol`: chip byte-stream encoding/framing.
	- `ITransport`: hardware/peripheral transfer behavior.
- Preserve protocol/transport category compatibility (`TransportTag`, `OneWireTransportTag`, `AnyTransportTag`).
- For transport settings types, maintain required `public bool invert` contract.
- Use canonical static bus-driver naming already adopted by the codebase:
	- `StaticBusDriverPixelBusT`
	- `makeStaticDriverPixelBus(...)`
	- Do not reintroduce legacy `Owning*` bus-driver names.
- Keep ownership intent explicit and consistent with current interfaces and `ResourceHandle` usage where still present.

## Arduino Dependency Boundary Rules

- Prefer minimizing `Arduino.h` dependency in virtual-first/core contracts.
- Keep direct platform calls (`micros`, `yield`, pin I/O, etc.) at transport/platform edges or narrow adapters.
- Do not break Arduino-first consumer API compatibility while refactoring internals.

## Factory and API Authoring Rules

- Favor the single-expression `makeBus(...)` composition style with clear protocol/transport config roles.
- Use protocol aliases (for example `Ws2812`) where available to reduce boilerplate.
- Require explicit color type only when inference cannot determine it.
- Avoid adding ambiguous overloads solely to hide required explicit template information.

## Testing and Validation Rules

- For behavior or contract changes, run targeted native tests first, then broader suites as needed.
- Minimum high-value gates for contract-sensitive changes:
	- `pio test -e native-test`
	- `pio test -e native-test --filter contracts/test_protocol_transport_contract_matrix_compile`
- For protocol byte-stream changes, validate against relevant protocol and byte-stream specs.
- For shader coverage policy in strict deterministic tests:
	- In scope: `CurrentLimiterShader`, `AggregateShader`
	- Out of scope by default: `GammaShader`, `WhiteBalanceShader`

## Examples Guidance

- Author examples under `examples-virtual/` with explicit layer declarations (`Protocol`, `Transport`, `BusType`).
- Keep construction order visible: color contract -> transport -> protocol -> bus -> optional shader/topology.
- Ensure protocol/transport pairings are category-correct and documented in example comments.

