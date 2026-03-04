# INI Reader (`IniReader`) Guide

`IniReader` is a lightweight INI parser exposed by `LumaWave/Factory.h` through the factory surface. It is intended for runtime config parsing without `std::string` usage in parser state.

## Quick Start

```cpp
#include <LumaWave/Factory.h>

char config[] =
    "[bus:front]\n"
    "pixels=120\n"
    "enabled=true\n"
    "\n"
    "[bus:rear]\n"
    "pixels=64\n";

auto reader = IniReader::parse(span<char>{config, strlen(config)});

auto front = reader.get("bus:front");
int pixels = front.get<int>("pixels");
bool enabled = front.get<bool>("enabled");
```

## Supported INI Shape

- Sections: `[section]`
- Inheritance: `[child&parent]`
- Hierarchy: `[root:subsection]`
- Key/value lines: `key=value`
- Full-line comments: lines starting with `;` or `#`

Notes:

- Whitespace around section names, keys, and values is trimmed.
- Matching for section names and keys is case-insensitive.
- Inline comments are **not** recognized. Example: `key=value ; note` stores the value as `value ; note`.

## API Overview

### Reader-level

- `IniReader::parse(span<char>)`
- `exists(section)` / `exists(section, key)`
- `get(section)` returns `IniSection`
- `getRaw(section, key)`
- `get<T>(section, key)`
- `test<T>(section, key, expected)` and `test<T>(section, key)`
- `getReader(prefix)` for prefixed section views

### Section-level (`IniSection`)

- `exists(key)`
- `getRaw(key)`
- `get<T>(key)`
- `test<T>(key, expected)` and `test<T>(key)`

`getRaw(...)` returns a `span<char>` view into the original config buffer.

## Prefix Readers (`getReader`)

`getReader("bus")` lets you access sections like `front` and `rear` from `[bus:front]` and `[bus:rear]`.

```cpp
auto busReader = reader.getReader("bus");
auto front = busReader.get("front");
int pixels = front.get<int>("pixels");
```

Prefixes compose with `:`. For example, a nested prefix resolves section lookups as `outer:inner:section`.

## Inheritance Rules

Use `[child&parent]` to inherit keys from another section.

- Lookup order is: child first, then parent chain.
- Child keys override parent keys.
- If an inheritance cycle exists, lookup stops and behaves as not found.

Example:

```ini
[base]
host=controller.local
port=80

[prod&base]
port=443
```

`prod.port` resolves to `443`, while `prod.host` resolves to `controller.local`.

## Typed Conversion Rules

- `bool` true tokens: `true`, `yes`, `on`, `1` (case-insensitive)
- `bool` false tokens: `false`, `no`, `off`, `0` (case-insensitive)
- Invalid/missing bool values convert to `false`
- Integral parsing accepts optional `+`/`-` and decimal digits only
- Unsigned integral with a negative value converts to `0`
- Integral overflow saturates to min/max for the destination type
- Floating point parsing supports decimal and `e`/`E` exponent notation
- Invalid numeric values convert to `0`

## Important Behavior Details

- Lines outside a section are ignored.
- Lines without `=` are ignored.
- Empty keys are ignored.
- Duplicate section headers merge into the same section.
- Duplicate keys in the same section keep first-match lookup behavior.
- `IniReader` stores extents into the original input buffer; keep that buffer alive while using the reader/sections/spans.

## Recommended Usage Pattern

1. Parse once at startup.
2. Create prefix readers (`getReader(...)`) for config domains (for example, `bus`, `network`, `effects`).
3. Use `test<T>(...)` and `exists(...)` for optional settings.
4. Treat missing/invalid numbers and booleans as defaults in your app logic.
