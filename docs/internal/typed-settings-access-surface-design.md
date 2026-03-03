# Typed Settings Access Surface (Design-Only, No RTTI)

This document defines a non-RTTI, non-type-erased settings access surface for bus consumers that already know concrete protocol/transport/shader types.

## Scope

- Design surface only.
- No runtime behavior changes yet.
- No requirement for RTTI or `dynamic_cast`.
- Primary target: static/composite buses produced by `makeBus(...)`.

## Problem Statement

Current seam contracts intentionally keep `IPixelBus<TColor>` minimal and virtual-first. That is good for frame-flow behavior, but it does not provide a standard way to mutate protocol/shader/transport settings at runtime, especially for composite buses.

Primary motivating use-case:

- adjust TM* protocol settings at runtime without reallocating bus internals.
- adjust selected shader settings on running buses.

## Design Principles

- No type erasure for typed settings operations.
- No RTTI dependency (`dynamic_cast` is not required and may be unavailable on target toolchains).
- Caller-owned type/index correctness contract is explicit.
- Keep `IPixelBus<TColor>` minimal; expose typed settings access as a capability on concrete bus types.
- Composite buses must preserve the same typed access model as single static buses.
- Settings mutations should not force bus reallocation or strand shape changes.

## Seam Contract Changes (Proposed)

This section defines the intended surface changes to each core seam so typed runtime settings mutation works consistently.

### `IPixelBus<TColor>`

Required (phase 1):

- add strand-level enumeration surface:
    - `size_t protocolCount() const`
    - `IProtocol<TColor>* getProtocol(size_t index)` + const overload
    - `ITransport* getTransport(size_t index)` + const overload
    - `IShader<TColor>* getShader(size_t index)` + const overload
- preserve existing frame lifecycle methods unchanged (`begin`, `show`, readiness, `pixelBuffer`, topology).

Optional (follow-up):

- add non-virtual helper wrappers for typed index access (`protocolAs<TProtocol>(index)`, etc.) in header-only helpers.

Rationale:

- keeps virtual seam minimal but sufficient for strand-level coordination and diagnostics.
- enables typed helpers above base seam without RTTI.

### `IProtocol<TColor>`

Required (phase 1):

- keep existing update/encoding responsibilities unchanged (`update`, `requiredBufferSizeBytes`, `alwaysUpdate`).
- add peer-aware begin overload:
    - `void begin(IProtocol<TColor>* protocol, ITransport* transport, IShader<TColor>* shader)`
- define a normalized runtime settings mutation convention for protocols with mutable runtime settings:
    - preferred: `void updateSettings(const SettingsType&)`
    - allowed alternative: `SettingsType& mutableSettings()` with explicit re-encode/apply step.

TM* specificity (phase 1):

- `Tm1814ProtocolT` should expose explicit current-setting mutation API, e.g.:
    - `void updateCurrentSettings(const Tm1814CurrentSettings&)`
    - optional accessor for inspection.

Optional (follow-up):

- add a lightweight, non-RTTI protocol kind/tag API for debug assertion support.

Rationale:

- protocol remains owner of encoding, clamping, and settings-byte materialization.
- bus/shader coordination does not bypass protocol invariants.

### `ITransport`

Required (phase 1):

- add peer-aware begin overload:
    - `void begin(IProtocol<TColor>* protocol, ITransport* transport, IShader<TColor>* shader)`
- keep transport as transfer seam; runtime settings tuning remains protocol/shader-focused for initial target.

Optional (follow-up):

- if transport-level tunables are introduced (clock/rate/mode at runtime), define a transport-specific mutation convention similar to protocol settings.

Rationale:

- avoids widening scope while current use-case is protocol current registers.

### `IShader<TColor>`

Required (phase 1):

- keep `apply(span<TColor>)` unchanged.
- add peer-aware begin overload:
    - `void begin(IProtocol<TColor>* protocol, ITransport* transport, IShader<TColor>* shader)`
- add optional compile-time-detected tuning signal API on participating shaders (not virtual):
    - `bool hasProtocolTuning() const`
    - `ProtocolTuningState protocolTuningState() const`

Optional (follow-up):

- standardize a reusable shader tuning traits helper to avoid per-shader detection duplication.

Rationale:

- avoids virtual seam breakage for all shaders.
- enables behind-the-scenes protocol tuning for selected shaders like `CurrentLimiterShader`.

### Cross-seam behavior contract

- shader emits tuning intent only.
- bus orchestrates typed strand matching and application order.
- protocol applies/clamps settings and encodes output.
- transport transmits encoded bytes.

This split preserves ownership boundaries while enabling runtime protocol tuning.

## Proposed Surface

## 1) Minimal runtime seam additions (optional but useful)

Add protocol/transport/shader enumeration hooks to `IPixelBus<TColor>` for generic flows:

- `protocolCount() const`
- `getProtocol(size_t index)` / const overload
- `getTransport(size_t index)` / const overload
- `getShader(size_t index)` / const overload

These are still pointer-based virtual seams and therefore type-erased at the base layer, but they enable index-based cross-cutting code and diagnostics.

## 2) Typed helpers layered above base access

Add typed helper APIs on concrete bus types (or as constrained free helpers):

- `template<typename TProtocol> TProtocol& protocolAs(size_t index)`
- `template<typename TTransport> TTransport& transportAs(size_t index)`
- `template<typename TShader> TShader& shaderAs(size_t index)`

Implementation model:

- bounds-check index (debug assert).
- cast via `static_cast<TProtocol*>(getProtocol(index))`.
- caller must request the exact dynamic type at that index.

`reinterpret_cast` must not be used.

## 3) Typed bulk access for composite/static buses (preferred ergonomics)

Provide compile-time typed visitors on `StaticBus` (therefore also composite results that resolve to `StaticBus`):

- `forEachProtocolOf<TProtocol>(Fn&& fn)`
- `forEachTransportOf<TTransport>(Fn&& fn)`
- `forEachShaderOf<TShader>(Fn&& fn)`

Behavior:

- iterate all strands.
- invoke callback only for exact matching strand component type.
- no casts at callsite.
- zero RTTI.

This is the recommended path for runtime setting updates on composite buses.

## 4) Protocol settings mutation contract

For protocol-specific runtime updates, protocols should expose one of:

- `SettingsType& mutableSettings()` (+ optional `applySettings()`/`onSettingsChanged()`)
- `void updateSettings(const SettingsType&)`

TM* protocols should converge on a consistent mutation surface so callers can tune settings through typed bus access.

## 5) Shader-driven protocol tuning (behind-the-scenes plan)

This section defines how shader-derived control signals can mutate protocol settings without exposing type-erased runtime plumbing to users.

### Motivating case: `CurrentLimiterShader` + TM* protocol current registers

TM1814 exposes per-channel current settings (`Tm1814CurrentSettings`) encoded into protocol settings bytes, independent from absolute pixel values. This is a good first pattern for shader-driven protocol coordination:

- shader computes power budget / limiting intent from frame content.
- protocol receives an updated current-register target before protocol serialization.
- pixel values may still be scaled, but protocol-level current can also be adjusted.

This lets us use chip-native current management where available instead of relying only on component scaling.

### Architecture direction

Use a typed coordination path at bus level, not a direct shader->protocol pointer dependency.

- shaders remain focused on color transforms and optional control-signal emission.
- bus orchestration layer performs typed per-strand coordination.
- protocols remain owners of encoding rules and bounds for their settings.

Recommended model for phase 1:

1. During `show()`, run shader as today.
2. Collect optional control signal(s) from shader-capable types (starting with current budget signal).
3. Apply typed protocol tuning for matching strand protocol types (`Tm1814ProtocolT<...>` first).
4. Serialize/transmit frame with updated protocol settings.

### Peer-awareness and binding model (protocol/transport/shader)

To make shader-driven protocol tuning practical, each strand component needs awareness of its peers.

Proposed binding rule:

- strand peers are wired once by bus via `begin(...)` before first frame.
- bindings are non-owning references/pointers to objects in the same strand.
- binding is stable for the lifetime of the bus/strand instance.

Unified peer-aware begin shape:

- `void begin(IProtocol<TColor>* protocol, ITransport* transport, IShader<TColor>* shader)`

Usage per component:

- protocol receives `transport` and `shader` peers.
- shader receives `protocol` and `transport` peers.
- transport receives `protocol` and `shader` peers when needed.

This gives two valid orchestration styles:

1. bus-centric coordination only (shader emits signal, bus applies protocol update), or
2. shader-assisted coordination (shader can call protocol mutation through bound peer reference).

Phase-1 recommendation for safety/clarity:

- implement peer binding hooks now,
- keep actual update ordering and final write authority in bus show pipeline.

### Binding lifecycle and ordering

- construction: strand objects created as today.
- begin-binding step: bus calls peer-aware `begin(protocol, transport, shader)` for each strand component.
- show step: shader apply -> optional shader-derived tuning signal -> protocol update/serialize -> transport transmit.

Bindings must occur before first `show()`.

### Ownership and safety rules for peer references

- references are non-owning and strand-local only.
- no cross-strand peer access.
- no replacement/rebinding during normal runtime.
- no dynamic allocation required for binding itself.
- invalid use after bus destruction is undefined; components are not expected to outlive owning bus.

### Why not rely only on direct shader->protocol calls?

Even with bound references, purely direct shader->protocol mutation can hide ordering and precedence behavior. Keeping bus as orchestration authority ensures deterministic sequencing and preserves existing seam ownership boundaries.

### Proposed control-signal contract (non-virtual, opt-in)

Add optional shader-side helper API shape (compile-time detected):

- `bool hasProtocolTuning() const`
- `ProtocolTuningState protocolTuningState() const`

Where `ProtocolTuningState` is a small POD carrying normalized values (for example, desired per-channel current percentage or absolute mA budget). This keeps shader internals private while exposing only what protocol coordination needs.

No base `IShader` virtual changes are required in phase 1; use trait/introspection helpers to detect participation.

### Strand-mapping and type-safety rules

- Coordination occurs per strand index.
- Only apply tuning when strand shader and strand protocol are both in compatible typed sets.
- No cross-strand protocol mutation.
- Use compile-time visitor helpers on `StaticBus` for the preferred path.
- For index-based fallback, keep the explicit caller correctness contract and `static_cast` semantics.

### Precedence and composition rules

To avoid hidden feedback loops, define strict ordering:

1. user-authored explicit protocol settings update (highest priority),
2. shader-driven protocol tuning,
3. protocol defaults.

`CurrentLimiter`-driven tuning should be deterministic and idempotent per frame for the same inputs.

### TM* phase-1 target behavior

Phase 1 target protocol: `Tm1814ProtocolT`.

- Add explicit runtime mutation entrypoint to protocol, for example:
    - `void updateCurrentSettings(const Tm1814CurrentSettings&)`, and/or
    - `Tm1814CurrentSettings& mutableCurrentSettings()`.
- Keep existing buffer-size contract unchanged.
- Ensure update only changes settings bytes, not allocation/layout.

`Tm1914ProtocolT` currently encodes mode/reset behavior rather than per-channel current, so it is a secondary follow-up target (same coordination pattern, different settings payload).

### Future extensibility

The same coordination channel can later support chips with protocol-level gamma/white-balance settings.

- do not hard-code current-limiter assumptions into the generic bus surface.
- keep control signal generic enough to add gamma/white-balance classes later.
- bind actual protocol application by explicit typed adapters.

### Factory constraints and shader specialization requirements

To keep this safe at compile time, factories must enforce protocol/shader compatibility where shader-driven protocol tuning is requested.

Factory requirements:

- add compile-time checks in descriptor/factory paths to prevent incompatible shader/protocol pairings for protocol-tuning features.
- preserve existing category compatibility checks (protocol vs transport) and extend with shader/protocol tuning compatibility checks.
- fail fast with static assertions when a shader tuning specialization is unavailable for the selected protocol.

Shader specialization strategy:

- introduce protocol-aware tuning helpers that are type-specialized by protocol family.
- example pattern:
    - `CurrentLimiterShader<TColor>` remains generic for pixel-domain limiting.
    - add protocol-tuning adapter/specialization path keyed by protocol type (for example TM* family).
- for TM* phase 1:
    - provide explicit restricted implementations for `Tm1814ProtocolT<...>`.
    - leave other protocols on no-op/default tuning path until dedicated specialization exists.

Illustrative shape (design intent, not final API commitment):

- shader-level template parameterization such as `CurrentLimiterShader<TProtocol>` is acceptable if it composes cleanly with color contracts and existing factory descriptors.
- alternatively, keep shader type as-is and place protocol specialization in a dedicated `ProtocolTuningAdapter<TShader, TProtocol>` trait.

Preferred direction for this codebase:

- avoid breaking current public shader aliases abruptly.
- use explicit trait specialization for protocol tuning first, then evaluate whether a protocol-typed shader surface is still needed.

This keeps generic shader behavior stable while enabling strict TM*-only compile-time specialization for protocol current-register tuning.

### Testing plan additions for this scenario

- unit tests: `CurrentLimiter` control-signal generation is stable for fixed inputs.
- protocol tests: TM1814 runtime current-setting update mutates encoded settings bytes as expected.
- bus integration tests: typed static/composite bus applies shader-driven TM1814 updates per strand without reallocating buffers.
- negative tests: non-TM strands are unchanged by TM-specific coordination logic.

## Safety Contract

The non-RTTI typed cast helpers rely on this explicit invariant:

- consumer knows and supplies the exact component type for the selected strand index.

Violation is programmer error. In debug builds we should provide assertions where practical.

Recommended debug checks:

- index bounds.
- optional lightweight type tag check (`ProtocolKind`/`TransportKind`/`ShaderKind`) before `static_cast`.

No runtime overhead checks are required in release builds.

## Composite Bus Behavior

`factory::makeBus(std::move(a), std::move(b), ...)` currently produces `StaticBus<...>`-backed aggregates. The typed visitor and typed-cast capability should therefore work unchanged on both single and composite static buses.

No additional type-erased wrapper type should be introduced for composite settings access.

## DynamicBus Position

`DynamicBus` stores strand components as interface pointers and can be heterogeneous at runtime. Full compile-time typed visitor coverage is not always possible.

Planned behavior:

- keep index-based base hooks available.
- offer opt-in typed cast helpers with explicit contract (`static_cast` + caller responsibility), where useful.
- document that `DynamicBus` has weaker compile-time guarantees than `StaticBus` for typed settings ergonomics.

## Non-Reallocation Requirement

Settings alteration in this design means mutating existing component state only.

Not in scope for this surface:

- changing strand count.
- changing strand lengths.
- replacing protocol/transport/shader instances.
- reallocating root/shader/protocol buffer layout due solely to settings APIs.

If specific settings imply changed protocol buffer requirements, that behavior must be explicitly documented per protocol and treated as a separate follow-up design.

## API Sketch (Illustrative)

```cpp
template<typename TColor>
class IPixelBus
{
public:
    virtual size_t protocolCount() const = 0;
    virtual IProtocol<TColor>* getProtocol(size_t index) = 0;
    virtual const IProtocol<TColor>* getProtocol(size_t index) const = 0;
    // transport/shader analogs...
};

template<typename TProtocol, typename TColor>
TProtocol& protocolAs(IPixelBus<TColor>& bus, size_t index)
{
    // debug: assert(index < bus.protocolCount());
    return *static_cast<TProtocol*>(bus.getProtocol(index));
}
```

## TODO List (Implementation Plan)

1. Add optional strand enumeration hooks to `IPixelBus<TColor>` and implement them in `PixelBus`.
2. Add typed cast helper utilities (`protocolAs`, `transportAs`, `shaderAs`) with debug bounds assertions.
3. Add typed visitor APIs to `StaticBus` based on `OwnedTuple` strand groups.
4. Ensure composite buses returned by `factory::makeBus(...)` expose the same typed visitor APIs.
5. Define/document `DynamicBus` typed access limitations and available helpers.
6. Normalize TM* protocol runtime settings mutation API (`updateSettings` and/or `mutableSettings`).
7. Add shader-to-protocol coordination hook in bus show path with explicit ordering guarantees.
8. Implement phase-1 adapter: `CurrentLimiter` control signal -> `Tm1814ProtocolT` current settings.
9. Add tests for TM* runtime settings updates through typed static/composite bus access.
10. Add integration tests validating no buffer reallocation/layout change during runtime tuning.
11. Add compile-contract tests validating typed helper constraints and usage shape.
12. Add user-facing usage examples for shader and TM* protocol runtime tuning.
13. Update internal backlog item to reference this design and implementation milestones.

## Open Questions

- Do we want base `IPixelBus` hooks now, or start with `StaticBus` typed visitors only and defer virtual seam expansion?
- Should we require a lightweight protocol kind tag for debug assertions, or rely on index mapping documentation only?
- Which TM* protocol settings are explicitly runtime-mutable in phase 1?
- Should shader-derived protocol tuning be enabled by default when both sides are present, or require explicit opt-in config per strand?
