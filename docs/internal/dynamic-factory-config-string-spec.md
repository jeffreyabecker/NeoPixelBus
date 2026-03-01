# Dynamic Factory Config String Spec (MVP)

## Goal

Define a small runtime config string format that can be parsed without using `std::string` or `std::string_view`, then used to build a dynamically allocated bus instance.

This system is intentionally parallel to descriptor/template factories:
- Descriptor factory: compile-time types (`makeBus<ProtocolDesc, TransportDesc>(...)`)
- Dynamic factory: runtime text config (`makeDynamicBus("...")`)

Named selection overload (for mixed specs):
- `makeDynamicBus(spec, name)`
- `tryMakeDynamicBus(spec, name)`

These overloads read only `bus.<name>.*` entries from a larger config blob so callers do not need to pre-filter strings.

## Parser Constraints

- Input is C-style text (`const char*`) or character span.
- Parsing uses pointer/length slices only.
- No dynamic string objects (`std::string`, `std::string_view`) in parser or factory code.

## Grammar

Config is a semicolon-delimited list of key/value pairs:

`key=value;key=value;...`

Whitespace around keys/values is ignored.

### Required Keys

- `pixels`
- `protocol`

### Optional Keys

- `transport` (default: `platformdefault`)

## Supported Keys and Values (MVP)

### `pixels`

- Unsigned decimal integer
- Range: `1..65535`

Examples:
- `pixels=60`
- `pixels = 300`

### `protocol`

Case-insensitive values:
- `dotstar` or `apa102`
- `ws2812`
- `ws2811`
- `sk6812`

### `transport`

Case-insensitive values:
- `platformdefault`
- `default` (alias of `platformdefault`)
- `nil`

## Behavior Rules

- Unknown keys fail parse.
- Unknown values fail parse.
- Missing `=` in a segment fails parse.
- Empty segments are ignored (e.g. trailing `;`).
- Missing required keys fail parse.

## Factory Behavior (MVP)

- Creates a single-strand dynamic owning bus using:
  - root buffer owned by bus (`pixelCount`)
  - no shader working buffer (`0`)
  - protocol buffer carved from unified arena
  - `NilShader`
- Returns `std::unique_ptr<IPixelBus<Rgb8Color>>`.
- On parse/build failure, returns `nullptr` from convenience API.

## Aggregate/Composite Bus Path (Phase 2)

Aggregate buses are handled as an explicit extension after MVP single-bus support.

Instead of embedding full child configs inline, buses are declared with explicit names and aggregate definitions reference those names.

### Named Bus Model

- Add top-level named declarations:
   - `bus.<name>.pixels=<n>`
   - `bus.<name>.protocol=<protocol>`
   - `bus.<name>.transport=<transport>` (optional, defaults to `platformdefault`)
- Add one aggregate declaration:
   - `aggregate.children=<name>|<name>|...`

### Name-Scoped Single-Bus Build

When using `makeDynamicBus(spec, name)`:

- Parser selects only keys prefixed with `bus.<name>.`.
- Prefix is stripped before normal single-bus key handling.
   - `bus.front.pixels=30` -> `pixels=30`
   - `bus.front.protocol=ws2812` -> `protocol=ws2812`
- Non-matching keys are ignored.
- If the named bus is missing required fields, parse fails.

Example shape (illustrative):

`bus.front.pixels=30;bus.front.protocol=ws2812;bus.front.transport=nil;bus.rear.pixels=30;bus.rear.protocol=dotstar;aggregate.children=front|rear`

### Validation Rules for Named Model

- Bus names are case-insensitive for lookup and must be unique.
- Every name in `aggregate.children` must be declared as `bus.<name>.*`.
- Every declared bus must provide required keys: `pixels`, `protocol`.
- Unknown bus-scoped keys fail parse.
- Duplicate assignment of the same key for the same bus fails parse.
- Missing `aggregate.children` when aggregate mode is requested fails parse.

### Build Rules for Named Model

- Build all named buses first using the same single-bus builder path.
- Resolve aggregate children by referenced names in declared order.
- Compose resolved children with existing composite bus facilities.
- Build failures report the bus name that failed, not only index.

Why this works with Arduino constraints:

- No dynamic string class requirements.
- Parser still uses pointer/range scanning and fixed delimiters.
- Name tokens are compared as slices (`const char*` + length), not heap strings.

## Implementation Plan

1. Add parser model types and parser
   - Add a parser header with:
     - config enums (`protocol`, `transport`)
     - parse error enum
     - parse result struct with `ok()/failed()` helpers
     - parse function overloads for `const char*` and `span<const char>`

2. Add dynamic factory runtime builder
   - Add a factory header with:
     - result/error type for detailed failure
     - `tryMakeDynamicBus(...)` and convenience `makeDynamicBus(...)`
   - Use descriptor traits and existing protocol/transport compatibility rules.
   - Build one-strand `UnifiedDynamicOwningBus<Rgb8Color>` with heap-allocated protocol/transport/shader objects.

3. Expose through public factory surface
   - Include new header from `factory/Factory.h`.
   - Re-export entry points (`makeDynamicBus`, `tryMakeDynamicBus`).

4. Validate
   - Add native tests for parser correctness and dynamic factory build success/failure paths.
   - Run targeted new tests then full `native-test`.

5. Extend to aggregate buses (Phase 2)
   - Add named bus grammar support (`bus.<name>.*`) and aggregate references (`aggregate.children=name|name|...`).
   - Reuse single-bus parse/build per named child with name-based error reporting.
   - Compose children into a composite/aggregate bus and return a single `std::unique_ptr<IPixelBus<...>>`.
   - Add aggregate parser and factory tests (valid, malformed, unknown name, duplicate name/key, mixed child failures).
