# Buffer Access Surface (Design-Only, Not Wired)

This document defines the proposed per-bus buffer accessor surface for centralizing root/shader/protocol memory access.

## Scope

- Design surface only.
- No factory/bus wiring yet.
- No behavior changes in current runtime paths.

## Goals

- Replace direct multi-`BufferHolder` plumbing with one buffer access object passed by reference.
- Expose typed accessors for root/shader/protocol regions.
- Keep per-strand protocol slicing explicit and inspectable.
- Support both single and composite bus surfaces through a shared provider contract.
- Keep layout management implementation-specific and out of the access seam.

## Core Contracts

Location:

- `src/core/BufferAccess.h`
- `src/buses/ProtocolBufferBinder.h`

### Access interface

`IBufferAccess<TColor>` exposes typed accessors:

- `rootPixels()`
- `shaderScratch()`
- `protocolArena()`
- `protocolSlice(size_t strandIndex)`

Each accessor has mutable and const overloads and returns `span<T>` / `span<const T>` for the corresponding region.

`IBufferAccess<TColor>` intentionally does not include layout mutation (slice assignment, arena growth).

### Buffer-spec immutability

- The buffer "spec" (the number of `rootPixels`, the number of `shaderScratch` pixels, and the
	count and sizes of protocol slices) is immutable and established at the surface's construction
	time. Implementations must not change the number of slices or the per-slice sizes after
	construction.
- Implementations MAY grow or shrink the underlying protocol arena capacity (for owned arenas)
	to satisfy runtime capacity needs, but this must not alter the canonical slice sizes or the
	slice count. In other words: capacity changes are allowed, layout/size changes are not.
- Assigning per-strand slice offsets (i.e., where each slice lives inside the arena) is
	acceptable as a runtime layout step, provided offsets stay within the arena and the
	pre-declared slice sizes are respected.
- Factories and bus builders are responsible for determining the canonical protocol slice sizes
	(for example from protocol `requiredBufferSizeBytes()` reports) and passing those sizes into
	surface constructors. Surfaces such as `OwningBuffer` use the provided
	`protocolSizes` as authoritative and MUST treat them as fixed.


### Provider interface

`IBufferAccessProvider<TColor>` provides:

- `bufferAccess()`
- `bufferAccess() const`

This is the seam target for both single-bus and composite-bus implementations.

## Holder-backed adapter

Holder-backed access is currently implemented as local/internal adapters in bus
implementations (for example in `OwningUnifiedPixelBus.h`) rather than a shared
`src/core/BufferAccessSurface.h` type.

These adapters centralize span retrieval and support per-strand protocol slice
metadata. Layout control methods remain surface-specific utilities, not part of
the `IBufferAccess<TColor>` seam contract.

## Reusable binder

`ProtocolBufferBinder<TColor>` is a shared helper for static/dynamic owning buses:

- bind transport per strand
- compute required protocol bytes
- ensure protocol arena capacity
- assign per-strand protocol slices
- call `protocol->setBuffer(...)`

Binder APIs are templated on a layout-capable surface type rather than requiring layout controls on `IBufferAccess<TColor>`.

This is intentionally standalone so existing buses can adopt it in a later migration step.

## Planned migration phases

1. Add adapters and contracts (done)
2. Switch owning bus internals/factories to `IBufferAccess` (next)
3. Expose provider through composite bus surface
4. Remove legacy multi-`BufferHolder` constructor paths
