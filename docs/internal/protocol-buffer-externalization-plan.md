# Protocol Buffer Externalization Plan

Status: proposed

## Purpose

Define a staged plan to externalize protocol byte buffers so bus/factory layers own protocol frame memory, while protocols remain movable and receive explicit `ITransport` at construction.

This document is planning-only and intentionally does not include implementation code changes.

## Goals

1. Compute total protocol-byte capacity needed across all strands before protocol construction.
2. Pass per-strand protocol byte-buffer slices using an API shape that is implicitly bounds-checked.
3. Pass `ITransport` to protocol constructors as part of construction flow (not late mutable wiring where avoidable).
4. Add an explicit `IProtocol` buffer-binding seam (`setBuffer(lw::span<uint8_t>)`) for externalized byte buffers.
5. Preserve C++17 compatibility and virtual-first seam boundaries (`IProtocol`, `ITransport`, `IPixelBus`).

## Non-Goals (This Sub-Branch)

1. No protocol encoding algorithm changes.
2. No transport behavior/timing changes.
3. No redesign of topology mapping.
4. No broad API cleanup outside buffer + protocol-construction seams.

## Current Constraint Summary

1. Protocols currently size/own internal byte buffers (notably one-wire protocols).
2. Factory composition is moving toward `StaticOwningBus`/`DynamicOwningBus` and movable protocol values.
3. Some settings currently carry mutable `settings.bus` pointers; this is workable but fragile when objects are moved.

## Proposed Contract Additions

### 0) `IProtocol::setBuffer(...)` Seam Addition

`IProtocol<TColor>` will gain a buffer-binding member so protocol frame bytes are supplied by owner/factory layers.

Planned shape:

1. Add virtual `setBuffer(lw::span<uint8_t> buffer)` to `IProtocol<TColor>`.
2. Protocol implementations store a non-owning span/view to this buffer.
3. `setBuffer(...)` is called during construction/initialization flow before first `update(...)`.

Design intent:

1. Keep protocol update logic independent from allocation source.
2. Preserve movable protocol value semantics (buffer is borrowed, not owned).
3. Centralize capacity control in bus/factory arena planning.

Acceptance criteria:

1. Protocol implementations in migrated path do not allocate frame buffers internally.
2. Calling `update(...)` without a valid bound buffer is guarded (assert/fail-fast in debug/test policy).
3. `setBuffer(...)` can be invoked safely after moves as long as owner lifetime is valid.

### 1) Protocol Byte-Size Contract

Each protocol type must expose a deterministic way to compute required frame-byte size from construction inputs.

Planned shape (concept-level):

1. Protocol trait/resolver computes `requiredFrameBytes(pixelCount, settings)`.
2. Factory aggregates per-strand byte lengths.
3. Owning bus allocates one contiguous protocol-byte arena.
4. Factory slices arena per strand and passes slices during protocol construction.

Acceptance criteria:

1. Total required bytes for N strands are known before constructing protocol instances.
2. No protocol performs internal frame-buffer heap allocation in the externalized path.

### 2) Implicit Bounds-Checked Buffer Passing

Per-strand byte-buffer handoff must prevent accidental overrun by construction, without requiring call-site manual checks.

Planned shape:

1. Use `lw::span<uint8_t>` as the protocol-facing buffer view.
2. Introduce a factory-owned slicer utility that:
   - tracks running offset,
   - validates `offset + requested <= arena.size()`,
   - returns typed span slice.
3. On invalid size math, fail deterministically:
   - debug/test builds: assert/fail-fast,
   - compile-time static paths: static assertions where sizes are constant.

Implicit safety rule:

1. Protocol constructors never receive raw pointer + length pairs directly from user code.
2. Only factory slicer emits protocol buffer spans.

Acceptance criteria:

1. Protocol update paths write only inside assigned span range.
2. Contract tests include at least one negative case for over-allocation request.

### 3) Transport Injection in Protocol Construction

Protocol construction should receive the target `ITransport` as an explicit constructor input in factory paths.

Planned direction:

1. Preferred constructor contract:
   - `(uint16_t pixelCount, SettingsType settings, ITransport& transport, lw::span<uint8_t> frameBuffer)`
   - or equivalent typed transport ref where category-safe and non-ambiguous.
2. Transitional compatibility:
   - existing `(pixelCount, settings)` path may remain temporarily,
   - factory adapters normalize into explicit transport-injected construction.
3. Keep `settings.bus` as transitional compatibility only; de-emphasize as primary binding mechanism.

Acceptance criteria:

1. Protocol instance has valid transport binding immediately after construction.
2. No post-construction pointer patching is required in steady-state path.

### 4) Constructor vs `setBuffer(...)` Ordering Rule

To keep protocol construction deterministic while preserving explicit seams:

1. Constructor establishes protocol identity (`pixelCount`, settings, transport binding).
2. `setBuffer(...)` binds writable frame storage immediately afterward.
3. Protocol is considered operational only after both constructor and `setBuffer(...)` complete.

This permits a strict two-step bring-up while still keeping transport injection part of construction.

## Byte-Length Planning Details

### Strand-Level Required Bytes

For each strand `i`:

1. Resolve protocol type + settings + strand pixel length.
2. Compute `bytes_i` using protocol-specific trait.
3. Record metadata `{offset_i, length_i_pixels, length_i_bytes}`.

### Total Arena Bytes

`totalProtocolBytes = Σ bytes_i` across all strands.

Arena planning rules:

1. Sum in `size_t`.
2. Check overflow before allocation.
3. Zero strands => zero-byte arena allowed.

### Storage Ownership

1. `StaticOwningBus`: owns byte arena value directly.
2. `DynamicOwningBus`: owns byte arena via dynamic storage container.
3. Protocols borrow non-owning spans into arena.

## Factory Flow Plan

1. Resolve protocol/transport descriptors to concrete types.
2. Resolve protocol settings + transport settings.
3. Compute per-strand required bytes and total bytes.
4. Allocate protocol-byte arena once.
5. Construct transport object.
6. Construct protocol with explicit transport ref.
7. Slice per-strand span using checked slicer.
8. Call `protocol.setBuffer(slice)`.
9. Materialize strand records and owning bus.

## Migration Phases

### Phase A: Contracts and Traits

1. Add protocol-byte-size trait API (compile-time concept checks).
2. Add constructor-detection traits for transport injection paths.
3. Add `IProtocol::setBuffer(...)` seam contract and compile checks.
4. Update internal contract docs and compile assertions.

### Phase B: One Protocol Pilot

1. Implement externalized buffer path for `Ws2812xProtocol` first.
2. Keep transitional constructor overloads where needed.
3. Validate movable protocol behavior with factory ownership paths.

### Phase C: Factory Integration

1. Integrate byte-arena planning into descriptor-based `makeBus` flow.
2. Use checked slicer for all protocol buffer handoff.
3. Ensure one-wire wrapped and direct transport paths both compile.

### Phase D: Expand + Cleanup

1. Migrate remaining protocols to externalized buffers.
2. Remove obsolete internal protocol buffer ownership where no longer needed.
3. Reduce reliance on mutable `settings.bus` binding.

## Testing Plan

Minimum gates after each phase:

1. `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile`
2. `pio test -e native-test --filter contracts/test_protocol_aliases_first_pass_compile`

Additions:

1. Contract compile checks for:
   - `IProtocol`/protocol conformance to `setBuffer(lw::span<uint8_t>)`,
   - protocol byte-size trait availability,
   - constructor path with `ITransport&`,
   - move-constructible protocol requirement remains satisfied.
2. Runtime tests:
   - correct total arena sizing for multi-strand composition,
   - per-strand buffer boundaries not crossed,
   - zero-length and mixed-length strand edge cases.

## Risks and Mitigations

1. Risk: transport reference lifetime mismatch.
   - Mitigation: construct transport before protocol, store both in same owning object lifetime domain.
2. Risk: trait complexity causes overload ambiguity.
   - Mitigation: prioritize constructor-detection order explicitly and keep fallback paths transitional.
3. Risk: hidden buffer assumptions in existing protocols.
   - Mitigation: pilot protocol first, then migrate incrementally with targeted tests.

## Definition of Done

1. Descriptor-based factory paths externalize protocol buffers.
2. Protocols in migrated path are move-constructible and do not internally allocate frame buffers.
3. Protocol constructors in migrated path receive explicit transport and checked buffer span.
4. Contract compile tests and targeted runtime tests pass in `native-test`.
