# Unified PixelBus From-Scratch Architecture

Status: proposed

## Purpose

Define a fresh bus architecture with one primary bus abstraction and one composition model, with explicit strand metadata and optional topology support.

This proposal intentionally does not preserve the current BusDriver architecture and is written as a clean-slate design.

## Primary Goals

1. Collapse bus hierarchy complexity into one core bus type.
2. Remove `IAssignableBufferBus<TColor>` and `I2dPixelBus<TColor>`.
3. Represent physical output wiring with explicit `StrandExtent<TColor>` records.
4. Support both static and dynamic ownership models over the same runtime behavior.
5. Keep topology mapping hot-path methods non-virtual.

## Non-Negotiable Constraints

1. `Topology::width()`, `Topology::height()`, `Topology::map(...)`, and `Topology::isInBounds(...)` stay non-virtual.
2. Methods called inside those functions also stay non-virtual.
3. Do not introduce additional public template knobs such as `TTopology`.
4. Avoid per-pixel virtual dispatch overhead.

## Core Data Model

```cpp
template <typename TColor>
struct StrandExtent
{
    IProtocol<TColor>* protocol = nullptr;
    ITransport* transport = nullptr;
    IShader<TColor>* shader = nullptr;
    size_t offset = 0;
    size_t length = 0;
};
```

Notes:

1. `shader` is typed as `IShader<TColor>*` (not `IShader<TColor*>`).
2. `offset` + `length` define the span into the root buffer sent through this strand.
3. Each strand is independent for readiness and update semantics.

## Unified Interface Surface

### `IPixelBus<TColor>`

Keep one bus interface and add optional topology access directly on it:

```cpp
template <typename TColor>
class IPixelBus
{
public:
    virtual ~IPixelBus() = default;

    virtual void begin() = 0;
    virtual void show() = 0;
    virtual bool isReadyToUpdate() const = 0;

    virtual span<TColor> pixelBuffer() = 0;
    virtual span<const TColor> pixelBuffer() const = 0;

    virtual const Topology* topologyOrNull() const
    {
        return nullptr;
    }
};
```

`topologyOrNull()` is not used in tight loops directly; loop code caches the pointer once.

## Core Runtime Type

### `PixelBus<TColor>`

Constructor shape:

```cpp
PixelBus(BufferHolder<TColor> rootBuffer,
         BufferHolder<TColor> shaderBuffer,
         Topology topology,
         span<StrandExtent<TColor>> strands);
```

`shaderBuffer` contract:

1. `shaderBuffer` is scratch space used when a strand has a shader.
2. It must be sized to at least the longest single strand length.
3. Sizing/validity is caller responsibility.

Responsibilities:

1. Own root pixel buffer (`BufferHolder<TColor>`).
2. Own shader scratch buffer (`BufferHolder<TColor>`).
3. Own topology value (`Topology`) and return pointer in `topologyOrNull()`.
4. Borrow strand metadata array (`span<StrandExtent<TColor>>`).
5. On `begin()`: initialize root and shader buffers and call transport/protocol initialization per strand.
6. On `show()`: for each strand, slice root span by `[offset, offset + length)`.
7. If shader exists, copy slice to shader buffer segment, apply shader there, then update protocol from shader segment.
8. If shader does not exist, update protocol directly from root slice.
9. On `isReadyToUpdate()`: aggregate readiness from each strand protocol/transport pair.

Caller-owned validity rules:

1. Caller supplies valid strand pointers and extents.
2. Overlap is not allowed.
3. Sparse or inconsistent layouts are caller responsibility.
4. Zero-length strands are treated as no-op.

## Specialized Implementations

### `NilBus<TColor>`

- Test and diagnostics bus.
- Keeps no-op `begin/show`, `isReadyToUpdate() == true`.
- Exposes local in-memory buffer.
- `topologyOrNull()` may return nullptr or a trivial topology (policy decision; default nullptr).

### `SegmentBus<TColor>`

- Retained to keep API consistency.
- Non-owning view over parent `IPixelBus<TColor>` span window.
- Delegates lifecycle to parent.
- Does not create new protocol/transport endpoints.

### `StaticOwningBus<TColor, ...>`

- Statically typed/allocated owner for protocols/transports/shaders/strand table.
- Inherits `PixelBus<TColor>`.
- Constructs internal arrays and passes views into base `PixelBus`.
- No heap allocations required by bus object itself in steady state.

### `DynamicOwningBus<TColor>`

- Accepts externally allocated protocol/transport/shader objects and strand records.
- Owns them and deletes on destruction.
- Inherits `PixelBus<TColor>`.
- Ownership mode is explicit to avoid ambiguity with borrowed pointers.

## Strand Execution Semantics

For each strand on `show()`:

1. Acquire segment span from root buffer.
2. If shader exists, copy to shader buffer segment and run `shader->apply(shaderSegment)`.
3. If protocol readiness gate passes, call protocol update using shader segment when present, otherwise root segment.
4. Transport handling follows protocol contract (bound bus pointer, constructor-injected transport, or protocol-managed transport).

Design principle: strand processing is per-strand virtual-cost, not per-pixel virtual-cost.

## Topology and Hot Path

Hot-path rendering guidance:

1. Cache `const Topology* topo = bus.topologyOrNull();` once outside loops.
2. Guard null once.
3. Use `topo->map(...)`, `topo->isInBounds(...)`, `topo->width()`, `topo->height()` inside loops.

This preserves non-virtual mapping behavior and avoids dispatch in per-pixel loops.

## Factory Simplification Model

Target: one composition entry point instead of separate concat/mosaic/segment factories.

Conceptual API:

```cpp
auto bus = makeBus<TColor>(BusBuildSpec<TColor>{ ... });
```

`BusBuildSpec<TColor>` includes:

1. root buffer policy (owned/external),
2. shader buffer policy (owned/external and required scratch size),
3. topology settings,
4. strand definitions,
5. ownership mode (`Static` or `Dynamic`).

Resulting implementations:

1. `makeBus` returns `StaticOwningBus` for fully typed compile-time descriptors.
2. `makeBus` returns `DynamicOwningBus` for runtime-provided pointer collections.

## Factory Return-Node Pattern

To keep call-site ergonomics while enabling aggregate flattening, factory functions should return
an intermediate factory node type that is implicitly convertible to a concrete bus.

Conceptual shape:

```cpp
template <typename... TDescriptorState>
class BusFactoryNode
{
public:
    auto build() const -> /* concrete bus */;

    operator /* concrete bus */() const
    {
        return build();
    }
};
```

Design intent:

1. Preserve `auto bus = makeBus<...>(...)` ergonomics.
2. Allow aggregate factories to consume node metadata instead of materializing child buses first.
3. Let aggregate factories flatten children into one `StaticOwningBus` strand table.

### Composite Flattening Rule

Composite factory methods should prefer node-aware overloads:

1. Accept one or more `BusFactoryNode` values.
2. Read descriptor state (color type, protocol/transport types, strand lengths, topology hints).
3. Produce a single `StaticOwningBus` with merged strands and cumulative offsets.

This avoids constructing temporary child buses solely for metadata extraction.

### Conversion and Safety Guidance

Implicit conversion is useful for compatibility, but factory internals should prefer explicit build paths:

1. Use `node.build()` inside composition logic to avoid accidental early materialization.
2. Keep implicit conversion available for direct user assignment.
3. Add node traits/concepts (for example `is_bus_factory_node`) so overload resolution remains deterministic.

## Factory Node Adoption Plan

### Phase A: Introduce nodes without breaking call sites

1. Add `BusFactoryNode<...>` types for descriptor `makeBus(...)` entry points.
2. Keep implicit conversion to concrete bus enabled.
3. Keep legacy return types temporarily available behind transitional overloads.

### Phase B: Composite-first flattening

1. Add node-aware composite overloads in `makeCompositeBus` paths.
2. Flatten child nodes into a single `StaticOwningBus` strand table.
3. Prefer node metadata extraction over temporary child bus materialization.

### Phase C: Internal hardening

1. Use explicit `node.build()` inside factory internals.
2. Add traits/concepts to disambiguate node vs bus overloads.
3. Add compile-time tests for flattening correctness and resulting strand offsets.

### Phase D: Surface cleanup

1. Deprecate legacy composite wrappers that construct temporary bus objects.
2. Remove transitional overloads once ecosystem call sites are migrated.
3. Keep direct `auto bus = makeBus<...>(...)` ergonomics unchanged for end users.

## Migration Plan

### Phase 0: Introduce new architecture in parallel

1. Add new `StrandExtent`, `PixelBus`, `StaticOwningBus`, `DynamicOwningBus`.
2. Keep existing buses compiling.

### Phase 1: Bridge factories

1. Rebuild `make<Mosaic/Concat/Segment>Bus` wrappers to emit new unified build specs.
2. Mark old wrappers as transitional aliases.

### Phase 2: Remove split interfaces

1. Delete `IAssignableBufferBus` and `I2dPixelBus`.
2. Update all class inheritance to `IPixelBus` or `PixelBus`-derived types.

### Phase 3: Remove legacy composites

1. Remove legacy `MosaicBus`, `ConcatBus`, and static wrapper variants after parity validation.
2. Keep `SegmentBus` as API-consistency utility (new implementation on unified base).

## Open Design Decisions

1. Whether strand shader runs in-place only or supports read/write split.
2. Whether `PixelBus` caches per-strand dirty state to skip redundant protocol updates.
3. How protocol transport binding is represented in new build specs when protocol already owns transport.

## Risks

1. Migration risk: moderate because many factory and test call sites assume current specialized bus types.
2. Contract risk: protocol/transport binding behavior must stay deterministic.
3. Performance risk: low if per-strand processing is linear and map/bounds operations remain non-virtual.

## Acceptance Criteria

1. One primary bus runtime (`PixelBus`) supports old concat/mosaic behavior through strand configuration.
2. `IAssignableBufferBus` and `I2dPixelBus` are deleted.
3. Topology hot-path functions remain non-virtual.
4. Static and dynamic ownership modes are both supported and tested.
5. Factory entry points converge to one build method.
