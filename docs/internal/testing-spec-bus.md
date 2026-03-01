# Formal Test Specification: Busses and Topologies

Source:
- `docs/internal/testing-spec-hierarchical.md` (migrated bus + topology sections)

This document defines focused formal test cases for bus and topology behavior.
Each test includes: Description, Preconditions, Operations, and Expected Results.

## 1. Busses

### 1.1 PixelBus

#### 1.1.1 Bulk Set/Get Round-Trip (Iterator + Span)
- Description: Verify iterator-based and span-based bulk writes can be read back without data corruption.
- Preconditions:
  - `PixelBus` instance configured with `pixelCount >= 8`.
  - Protocol test double captures update frame.
  - Test vectors include non-uniform channel values.
- Operations:
  - Write frame using iterator API (`setPixelColors(offset, first, last)`).
  - Read frame using iterator API and compare.
  - Repeat with span API.
- Expected Results:
  - Read-back values match write vectors for all in-range indices.
  - No writes occur outside target range.

#### 1.1.2 End-Range Partial Write Clamp
- Description: Verify writes near end of buffer clamp to available pixels.
- Preconditions:
  - `PixelBus` with known pixel count.
  - Source buffer longer than remaining destination range.
- Operations:
  - Call bulk set with offset near last pixel and oversized input.
  - Read back full frame.
- Expected Results:
  - Only available tail pixels are modified.
  - No overflow and no change beyond clamped boundary.

#### 1.1.3 Dirty/Always-Update Show Behavior
- Description: Verify `show()` update policy based on dirty flag and protocol `alwaysUpdate()`.
- Preconditions:
  - Protocol stub tracks `update()` call count.
- Operations:
  - Call `show()` on clean frame with `alwaysUpdate=false`.
  - Modify one pixel and call `show()`.
  - Enable `alwaysUpdate=true` and call `show()` twice without edits.
- Expected Results:
  - No update on clean frame when `alwaysUpdate=false`.
  - Exactly one update after mutation.
  - Updates occur on every `show()` when `alwaysUpdate=true`.

#### 1.1.4 Out-of-Range Single-Pixel Safety
- Description: Verify out-of-range single-pixel access safety.
- Preconditions:
  - `PixelBus` with fixed pixel count.
- Operations:
  - Call `setPixelColor(indexOutOfRange, value)`.
  - Call `getPixelColor(indexOutOfRange)`.
- Expected Results:
  - Set operation is no-op.
  - Get operation returns default-constructed color.

#### 1.1.5 P0: Offset Greater Than Pixel Count
- Description: Validate bulk API safety when `offset > pixelCount`.
- Preconditions:
  - `PixelBus` and `BusDriverPixelBus` instances.
  - Bounds sanitizer or equivalent memory guard in native tests.
- Operations:
  - Invoke iterator and span set/get methods with invalid offset.
- Expected Results:
  - No memory corruption or crash.
  - Operation behaves as no-op.

### 1.2 SegmentBus

#### 1.2.1 Segment Origin Mapping
- Description: Verify segment local index 0 maps to parent offset.
- Preconditions:
  - Parent bus initialized with known baseline colors.
  - Segment created with non-zero parent offset.
- Operations:
  - Write segment index 0 and read parent at mapped position.
- Expected Results:
  - Parent pixel at `parentOffset` matches segment write.

#### 1.2.2 Segment Bulk Range Isolation
- Description: Verify segment bulk writes stay inside segment boundaries.
- Preconditions:
  - Parent bus with sentinel values outside segment.
- Operations:
  - Perform segment bulk write covering full segment.
  - Inspect parent inside and outside mapped range.
- Expected Results:
  - Only mapped parent range is modified.
  - External parent pixels remain unchanged.

#### 1.2.3 Multi-Segment Isolation
- Description: Verify multiple segments on same parent do not cross-modify ranges.
- Preconditions:
  - Two non-overlapping segment views on same parent.
- Operations:
  - Write segment A then inspect segment B range (and vice versa).
- Expected Results:
  - Writes are isolated to each segment's mapped parent range.

#### 1.2.4 Segment Offset Out-of-Range No-Op
- Description: Verify operations with `offset >= segmentLength` are ignored.
- Preconditions:
  - Segment with known length.
- Operations:
  - Call set/get bulk methods with out-of-range offset.
- Expected Results:
  - No mutations and no crashes.

#### 1.2.5 Segment Oversize Clamp
- Description: Verify oversized writes are clamped at segment end.
- Preconditions:
  - Segment length smaller than source length.
- Operations:
  - Write oversized bulk data near segment tail.
- Expected Results:
  - Writes stop exactly at segment end.

### 1.3 ConcatBus

#### 1.3.1 Uneven Child Index Resolution
- Description: Verify global index mapping across uneven child lengths.
- Preconditions:
  - At least two child buses with different lengths.
- Operations:
  - Write/read boundary indices around child transitions.
- Expected Results:
  - Global indices resolve to correct child/local index pairs.

#### 1.3.2 Pixel Count Aggregation
- Description: Verify `pixelCount()` equals sum of children.
- Preconditions:
  - Concat bus created with known child lengths.
- Operations:
  - Query `pixelCount()`.
- Expected Results:
  - Returned value equals exact sum of child counts.

#### 1.3.3 Lifecycle Fan-Out
- Description: Verify `begin()` and `show()` fan out to all children.
- Preconditions:
  - Child bus spies count lifecycle calls.
- Operations:
  - Invoke `begin()` and `show()` on concat bus.
- Expected Results:
  - Each child receives exactly one corresponding call.

#### 1.3.4 Remove Updates Mapping
- Description: Verify `remove()` updates offset map and total count.
- Preconditions:
  - Concat bus with 3+ children.
- Operations:
  - Remove middle child and verify mapping around previous boundaries.
- Expected Results:
  - Pixel count shrinks correctly.
  - Indices remap correctly for remaining children.

#### 1.3.5 Invalid Remove/Add Behavior
- Description: Verify robustness for invalid remove/add operations.
- Preconditions:
  - Concat bus initialized.
- Operations:
  - Remove non-member child.
  - Add null handle.
- Expected Results:
  - Remove returns false and state unchanged.
  - Null add is ignored and state unchanged.

### 1.4 MosaicBus

#### 1.4.1 2D Coordinate Mapping
- Description: Verify `(x,y)` set/get resolves to correct panel and local index.
- Preconditions:
  - Mosaic with known panel size/layout/tile layout.
- Operations:
  - Set known coordinates and inspect corresponding child bus pixels.
- Expected Results:
  - All tested coordinates map to expected child/local positions.

#### 1.4.2 Linear Flattening Consistency
- Description: Verify linear bulk APIs match expected panel flattening.
- Preconditions:
  - Mosaic with multiple panels.
- Operations:
  - Write with linear `setPixelColors`; read back via 2D and linear access.
- Expected Results:
  - Linear and 2D views are consistent.

#### 1.4.3 canShow All-Children Gate
- Description: Verify `canShow()` requires all children ready.
- Preconditions:
  - Child bus readiness controllable by test doubles.
- Operations:
  - Toggle one child not-ready, then all-ready.
- Expected Results:
  - Returns false when any child not-ready; true when all ready.

#### 1.4.4 Out-of-Bounds 2D Safety
- Description: Verify out-of-range `(x,y)` access is safe.
- Preconditions:
  - Mosaic with known dimensions.
- Operations:
  - Call set/get with negative and over-bound coordinates.
- Expected Results:
  - Set is no-op; get returns default color; no crash.

#### 1.4.5 Sparse Tile Safety and Empty Bus Geometry
- Description: Verify sparse/missing tile handling and empty-mosaic dimensions.
- Preconditions:
  - Case A: configuration references more tile indices than provided buses.
  - Case B: zero child buses.
- Operations:
  - Access coordinates in unresolved tiles.
  - Query `width()`/`height()` with empty buses.
- Expected Results:
  - Unresolved tile accesses are safely ignored/defaulted.
  - Empty bus geometry reports zero width/height.

## 2. Topologies

### 2.1 PanelLayout

#### 2.1.1 All-Layout Golden Mapping (4x4)
- Description: Verify `mapLayout` output against golden coordinate/index tables for all 16 layouts.
- Preconditions:
  - Table-driven expected index maps for all layout enums.
- Operations:
  - Evaluate all `(x,y)` positions for each layout.
- Expected Results:
  - Every computed index matches the corresponding golden table.

### 2.2 tilePreferredLayout

#### 2.2.1 Parity-Based Rotation Selection
- Description: Verify parity logic for each base layout group and row/column parity combination.
- Preconditions:
  - Test matrix for `(baseLayoutGroup, oddRow, oddCol)`.
- Operations:
  - Call `tilePreferredLayout` for each matrix row.
- Expected Results:
  - Returned layout matches expected parity-driven mapping.

### 2.3 PanelTopology

#### 2.3.1 In-Bounds Probe Mapping
- Description: Verify `mapProbe` returns expected indices for valid coordinates.
- Preconditions:
  - Fixed panel dimensions and layout.
- Operations:
  - Probe representative in-bounds coordinates.
- Expected Results:
  - Returns `std::optional` with correct index values.

#### 2.3.2 Clamped Map Behavior
- Description: Verify `map` clamps out-of-range coordinates to nearest edge.
- Preconditions:
  - Known dimensions.
- Operations:
  - Call `map` with negative and over-bound coordinates.
- Expected Results:
  - Result equals index of clamped edge coordinate.

#### 2.3.3 Pixel Count Invariant
- Description: Verify topology pixel count invariant.
- Preconditions:
  - Width/height configured.
- Operations:
  - Query `pixelCount()`.
- Expected Results:
  - Returns `width * height`.

#### 2.3.4 Out-of-Bounds Probe Nullopt
- Description: Verify `mapProbe` returns `nullopt` when out-of-bounds.
- Preconditions:
  - Valid topology instance.
- Operations:
  - Probe negative and `>=width/height` coordinates.
- Expected Results:
  - Returns `std::nullopt` for all invalid probes.

### 2.4 TiledTopology

#### 2.4.1 Cross-Tile Probe Correctness
- Description: Verify `mapProbe` across tile boundaries and panel edges.
- Preconditions:
  - Multi-tile topology with non-trivial panel and tile layouts.
- Operations:
  - Probe coordinates straddling tile boundaries.
- Expected Results:
  - Returned indices match expected tile + local mapping.

#### 2.4.2 Global Edge Clamp Behavior
- Description: Verify global `map` clamping at mosaic edges.
- Preconditions:
  - Valid tiled config.
- Operations:
  - Call `map` with negative and over-bound global coordinates.
- Expected Results:
  - Returned index corresponds to clamped coordinate.

#### 2.4.3 Topology Hint Classification
- Description: Verify `topologyHint` values for first, in-panel, last, and out-of-bounds.
- Preconditions:
  - Topology with known per-panel indexing.
- Operations:
  - Query hints at representative coordinates.
- Expected Results:
  - Returns `FirstOnPanel`, `InPanel`, `LastOnPanel`, or `OutOfBounds` correctly.

#### 2.4.4 Out-of-Bounds Probe Safety
- Description: Verify out-of-range probes return nullopt.
- Preconditions:
  - Valid topology instance.
- Operations:
  - Probe invalid coordinates.
- Expected Results:
  - `mapProbe` returns `std::nullopt`.

#### 2.4.5 P0: Zero-Dimension Config Guard
- Description: Verify behavior for zero panel dimensions is explicitly guarded.
- Preconditions:
  - Config with `panelWidth==0` and/or `panelHeight==0`.
- Operations:
  - Construct topology and invoke `map`/`mapProbe`.
- Expected Results:
  - No underflow/crash; behavior follows documented guard policy.

#### 2.4.6 Non-Existent Tile Probe Boundedness
- Description: Verify mismatch assumptions for nominal-but-absent tiles remain bounded.
- Preconditions:
  - Configuration where probing may reference nominal positions beyond physical tiles.
- Operations:
  - Probe affected coordinates.
- Expected Results:
  - API returns bounded/safe result with no invalid access.
