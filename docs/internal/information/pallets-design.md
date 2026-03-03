# Palette Design (WLED-Informed)

Status: proposed  
Scope: internal design for upcoming palette/pallet support in NpbNext  
Reference source: `C:\ode\WLED` (`wled00/FX.h`, `wled00/FX_fcn.cpp`, `wled00/json.cpp`, `wled00/palettes.h`, `default-palettes.json`)

---

## 1) Intent

Design a palette system ("pallets" in user wording) that captures WLED-level usefulness while fitting NpbNext virtual-first architecture and C++17 constraints.

Primary goals:

- Keep palette logic independent from protocol/transport seams.
- Preserve deterministic behavior and low overhead on MCUs.
- Support built-in, generated, and user/custom palettes.
- Keep initial scope focused on runtime palette behavior (no import/export requirement).

---

## 2) WLED behaviors to carry forward

Observed in WLED runtime:

- Palette is selected by `uint8_t` id (`seg.palette` / JSON `"pal"`).
- Mixed palette categories share one id space:
  - Reserved/builtin behaviors (`Default`, `Random`, `Color 1`, `Color 1&2`, etc.).
  - FastLED fixed palettes (`Party`, `Cloud`, `Lava`, ...).
  - PROGMEM gradients (`gGradientPalettes[...]`).
  - Custom palettes (high id range in WLED).
- Sampling behavior includes:
  - Mapping index from pixel position to `[0..255]`.
  - Optional wrap/non-wrap behavior.
  - Blend mode (`LINEARBLEND` or `NOBLEND`, globally controlled in WLED).
- Dynamic/random palettes are first-class and support time-based transitions.
- API exposure includes names list and expanded palette samples (`/json/pal`, `/json/palx`).

Useful lessons:

- Keep a stable, compact palette id.
- Separate **selection** from **sampling**.
- Treat dynamic/random palettes as generators, not static data blobs.

---

## 3) NpbNext architectural fit

### 3.1 Current seam alignment

- `IShader<TColor>` remains the transform seam (`apply(span<TColor>)`).
- `IPixelBus`, `IProtocol`, and `ITransport` stay palette-agnostic.
- Palette support belongs in `colors/` utility code (not protocol/transport).

### 3.2 Proposed new non-virtual core types

Add value types (no per-pixel virtual dispatch):

- `PaletteStop<TColor>`
  - `uint8_t index` (`0..255`)
  - `TColor color`
- `Palette<TColor>`
  - sorted stops (`lw::span<const PaletteStop<TColor>>` view in hot paths)
- `PaletteBlendMode`
  - `Linear`, `Nearest`
- `PaletteWrapMode`
  - `Wrap`, `Clamp`
- `PaletteSampleOptions`
  - blend mode, wrap mode, brightness scale

Core helper API (constexpr/inline where possible):

- `samplePalette(const Palette<TColor>&, uint8_t index, PaletteSampleOptions)`
- `mapPositionToPaletteIndex(pixelIndex, pixelCount, wrapMode)`

### 3.3 Scope clarification: no registry layer

Current direction is utility-first without a palette registry in LumaWave.

- Keep palette data as caller-owned inputs to utility functions.
- Keep identifiers/lookup concerns external to LumaWave core palette utilities.
- Revisit a registry only if a concrete runtime need emerges.

---

## 4) Usage model

### 4.1 Keep usage explicit

Do not hide position/time mapping inside protocol or transport.

Use one of these integration paths:

1. Effect/topology code computes palette index per pixel and writes colors.
2. External callers use palette utility helpers directly to map/sample colors.

### 4.2 Utility-first integration

Treat palettes as standalone utility methods in `colors/`:

- Callers provide index/mapping context.
- Utilities return sampled colors from gradient definitions.
- Blending policy remains caller-controlled via `PaletteSampleOptions`.

This keeps palette logic reusable across effects, buses, and external tooling without forcing shader integration.

### 4.3 Dynamic palette architecture options

For dynamic behavior (random smooth / random cycle), two implementation directions were considered.

#### Option A: virtual accessors on `Palette`

Example direction:

- make `Palette<TColor>` polymorphic,
- add virtual sampling/access methods,
- implement static/dynamic palette subclasses.

Pros:

- Single runtime abstraction for callers.
- Straightforward type-erased ownership patterns.

Cons:

- Introduces vtable and runtime dispatch into a currently value/utility-first path.
- Weakens constexpr friendliness and compile-time evaluation potential.
- Adds ABI/surface complexity to a core type currently intended as a lightweight view over stops.
- Encourages heap/lifetime coupling for polymorphic instances in embedded contexts.

#### Option B: templated helpers on `TPaletteLike`

Example direction:

- keep `Palette<TColor>` as current stop-based value type,
- add helper templates accepting any `TPaletteLike` with a small required API,
- provide adapters for static palette views and dynamic palette state objects.

Pros:

- Preserves zero-overhead path for static palettes.
- Keeps constexpr-friendly design for compile-time-capable call sites.
- Avoids forcing one inheritance hierarchy on callers.
- Works well with utility-first architecture and explicit caller-managed state.

Cons:

- Requires documenting a small concept-like contract for `TPaletteLike`.
- Template errors can be less friendly without careful static assertions.

#### Recommendation

Prefer **Option B** (`TPaletteLike` helper templates) and keep `Palette<TColor>` non-virtual.

Rationale:

- aligns with LumaWave utility-first goals,
- avoids unnecessary virtual overhead in hot paths,
- keeps static and dynamic palette usage composable without introducing a registry or polymorphic base requirement.

#### WLED usage alignment

WLED usage is close to the target usage here:

- Effects sample colors from a current palette using indexed lookup paths.
- Dynamic behavior (random smooth/cycle, transitions) is implemented by updating palette state over time, not by introducing polymorphic palette object families.
- Runtime control remains effect/segment/state driven, while palette sampling stays a tight utility path.

Implication for LumaWave:

- Keep `Palette<TColor>` as a lightweight stop-view data type.
- Model dynamic behavior as caller-managed state/adapters consumed by templated helper functions.
- Avoid virtual accessors on `Palette<TColor>` unless a future proven need requires runtime polymorphism.

---

## 5) Data model and compact serialization

### 5.1 Internal canonical format

Use a compact gradient format:

- Variable stop count (`>=2`).
- Monotonic stop indices (`0..255`).
- Color stored in `TColor`.

### 5.2 Compact binary format (normative)

Define one compact, versioned binary payload for storage and transport of gradient palettes.

Byte layout (little-endian where multi-byte appears):

- `magic[2]`: ASCII `LP` (`0x4C 0x50`)
- `lengthBytes[2]`: total message length in bytes (entire payload, including CRC)
- `version[2]`: format version (`0x0001` for this spec)
- `flags[2]`:
  - bits 0..3: `componentBytes` (`1` = 8-bit channels, `2` = 16-bit channels)
  - bits 4..6: `channelCount` (`3`, `4`, or `5`)
  - bits 7..15: reserved, must be `0`
- `stopCount[1]`: number of stops (`2..255`)
- `stops[]`: repeated `stopCount` entries
- `crc16[2]`: CRC-16/CCITT-FALSE over all prior bytes

Bitfield extraction reference (`flags` is a little-endian `uint16_t`):

- `componentBytes = flags & 0x000F`
- `channelCount = (flags >> 4) & 0x0007`
- `reserved = flags & 0xFF80` (must be `0`)

Each stop entry:

- `index[1]` (`0..255`, strictly increasing)
- channels: `channelCount` components, each `componentBytes` wide (`1` or `2`)

Size formulas:

- Header without checksum: `9` bytes
- Per-stop bytes: `1 + channelCount * componentBytes`
- Total: `9 + stopCount*(1 + channelCount*componentBytes) + 2`

Design notes:

- Identifier/category metadata are intentionally not serialized in this payload.
- Palette name is intentionally out-of-band (caller/application metadata), not in this payload.
- This payload represents only gradient color data.
- Unknown future versions must fail decode with explicit error.

### 5.3 Compact text form (non-normative)

For logs/tests/manual editing, support a concise string form equivalent to the binary payload:

- RGB8 form: `idx:RRGGBB|idx:RRGGBB|...`
- RGBW8 form: `idx:RRGGBBAA|idx:RRGGBBAA|...`
- Example: `0:780000|22:B31600|51:FF6800|255:0000A0`

Parsing rules:

- `idx` is decimal `0..255`.
- Hex is uppercase/lowercase tolerant.
- Indices must be strictly increasing.
- At least two stops required.

### 5.4 Validation and failure behavior

Decoder must reject payload when any of these holds:

- bad `magic`, unsupported `version`, reserved bits set, `componentBytes` not in `{1,2}`, `channelCount` not in `{3,4,5}`, or `stopCount < 2`
- `lengthBytes` smaller than minimum message size, larger than received bytes, or not equal to computed expected total
- truncated payload or checksum mismatch
- invalid stop ordering (equal/decreasing index)

Decoder result should include:

- `ok`/`errorCode`
- decoded `Palette<TColor>`

Implementation note:

- Decode/parse utilities should be written `constexpr`-friendly where practical, so literal inputs such as `parse("lpb1:...")` can produce compile-time values (subject to C++17 constexpr limits and fixed-capacity output containers).

### 5.5 Text-mode transport (URL-safe Base64) — deferred

To support text-only channels (URLs, config fields, CLI args), define an encoded representation of the same binary payload.

Status: deferred for now. The current implementation scope is binary payload only.

Encoding baseline:

- Start from the exact binary payload defined in 5.2.
- Encode using URL-safe Base64 (`A-Z a-z 0-9 - _`).
- Prefer no padding (`=`) on output; decoder should accept padded or unpadded input.

Candidate framing options:

1. Prefix envelope (recommended)
  - Format: `lpb1:<base64url>`
  - Example: `lpb1:TFABAAEAEwJ...`
  - Pros: unambiguous detection, versioned textual envelope, easy routing.
2. MIME-like token
  - Format: `data:application/x-lp-palette;v=1;base64url,<payload>`
  - Pros: self-describing metadata, explicit media type.
  - Cons: longer overhead.
3. Bare Base64URL (not recommended as primary)
  - Format: `<base64url>` only
  - Pros: shortest textual form.
  - Cons: ambiguous vs random user text; requires heuristic decode attempts.

Recommended parser detection order:

1. If input is bytes and starts with `0x4C 0x50` (`LP`), parse as compact binary.
2. Else if input is text and starts with `lpb1:`, parse as prefixed Base64URL:
  - strip prefix,
  - Base64URL-decode,
  - run normal binary validation (magic/length/version/flags/checksum).
3. Else (optional compatibility mode) attempt bare Base64URL decode and accept only if decoded bytes pass full binary validation.
4. Otherwise return `unsupported-format`.

Interoperability note:

- Keep one canonical binary decoder; text mode is only an envelope/transport layer around that payload.
- Do not define a separate checksum or length for text mode; rely on decoded binary `lengthBytes` and `crc16`.

Constexpr guidance:

- Prefer a shared constexpr-capable core (`decodeBinary` + `decodeTextEnvelope`) that avoids dynamic allocation and operates on spans/views over caller-provided buffers.
- Provide a runtime wrapper for dynamic inputs, but keep the core parser usable in constant-evaluation contexts when the input is a string literal.

---

## 6) Compatibility policy with WLED

### 6.1 What to match

- Indexed gradient sampling behavior.
- Reserved semantic palette behaviors (`Default`, random family, primary/secondary/tertiary convenience palettes).

### 6.2 What not to copy directly

- WLED global singleton state patterns.
- WLED custom-id encoding tricks (`255 - i + palettesCount`).
- Hard coupling to FastLED-specific palette container types in public API.

---

## 7) Proposed phase plan

### Phase 1 (MVP)

- Add core value types + sampler.
- Add built-in gradient table (small initial set).
- Add utility entry points for external callers/effects to map + sample palettes.
- Add native tests for:
  - interpolation correctness,
  - wrap/clamp behavior,
  - edge indices (`0`, `255`),
  - stop-boundary exactness.

### Phase 2

- Add lightweight convenience helpers for common external-call patterns.
- Add caller-oriented metadata helper patterns (without introducing a core registry abstraction).

### Phase 3

- Add dynamic palette generators (random smooth / random cycle).
- Add transition blending between palettes with explicit duration.
- Add optional docs/examples showing WLED palette migration.

---

## 8) Test strategy

Add focused suites under `test/colors/` (native first):

- `test_palette_sampling`
- `test_palette_registry`

Contract checks (compile-first) should verify:

- palette types are C++17-compatible,
- no protocol/transport headers are required by palette headers.

---

## 9) Naming decision

User request wording uses "pallets"; implementation/docs should use **palette** consistently.

Suggested public names:

- `PaletteStop`
- `Palette`
- `PaletteId`

Avoid introducing `Pallet*` symbols in API.

---

## 10) Open decisions

- Whether dynamic palette generators should be modeled as strategy objects or enum + settings.
- Whether palette blend mode should be global (WLED-like) or per-shader/per-segment (recommended: per-shader).
