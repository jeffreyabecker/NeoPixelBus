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
- `src/core/BufferAccessSurface.h`
- `src/buses/ProtocolBufferBinder.h`

### Access interface

`IBufferAccess<TColor>` exposes typed accessors:

- `rootPixels()`
- `shaderScratch()`
- `protocolArena()`
- `protocolSlice(size_t strandIndex)`

Each accessor has mutable and const overloads and returns `span<T>` / `span<const T>` for the corresponding region.

`IBufferAccess<TColor>` intentionally does not include layout mutation (slice assignment, arena growth).

### Provider interface

`IBufferAccessProvider<TColor>` provides:

- `bufferAccess()`
- `bufferAccess() const`

This is the seam target for both single-bus and composite-bus implementations.

## Holder-backed adapter

`BufferAccessSurface<TColor>` adapts existing:

- `BufferHolder<TColor>` root
- `BufferHolder<TColor>` shader
- `BufferHolder<uint8_t>` protocol arena
- optional `ProtocolSliceRange[]`

It centralizes span retrieval and supports setting per-strand protocol slice metadata.
These layout methods are surface-specific utilities, not part of the seam contract.
Current naming for these surface-only controls:

- `assignProtocolSliceLayout(...)`
- `reserveProtocolArenaBytes(...)`

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
