# Copilot Instructions for PicoLED

## Project Context

- Firmware targets Raspberry Pi Pico 2 W with PlatformIO + Arduino core (Earlephilhower).
- Language standard is locked to C++23 (`-std=gnu++23`).
- Runtime architecture is modular around kernel services and modules.
- Web UI uses Vite + vanilla JavaScript + standard HTML components (no Preact/React/TypeScript by default).

## Source of Truth

When generating or changing code, prefer consistency with these docs:

- `docs/vision-plan/00-overview.md`
- `docs/vision-plan/01-project-structure.md`
- `docs/vision-plan/02-coding-guidelines.md`
- `docs/vision-plan/06-web-and-http.md`
- `docs/vision-plan/07-build-system.md`

## C++ Rules

- Use C++23 features as supported by the project toolchain.
- Follow Allman braces and 4-space indentation.
- Use `#pragma once` in headers.
- Prefer `std::optional`, `std::variant`, `std::array`, smart pointers, and `constexpr`.
- Avoid exceptions and RTTI-heavy patterns unless explicitly requested.
- Do not use global mutable state when avoidable.
- Use `Palette` spelling consistently for rendering color-ramp types and managers; never use `Pallet`.

## Compiler Constants and Macros

- Prefer `static constexpr` constants in C++ code over direct macro usage whenever possible.
- Use macros primarily as compile-time override hooks for PlatformIO/build flags.
- Define every overrideable compiler constant using this pattern:

```cpp
#ifndef NEOPIXELBUS_[COMPILER_CONSTANT]
#define NEOPIXELBUS_[COMPILER_CONSTANT] (default value)
#endif
static constexpr <type> [CompilerConstant] = NEOPIXELBUS_[COMPILER_CONSTANT];
```

- Code should read/use the `static constexpr` variable (`[CompilerConstant]`) rather than the raw `NEOPIXELBUS_*` macro, except where preprocessor logic is explicitly required (`#if`, `#ifdef`).
- Keep macro names `UPPER_SNAKE_CASE` with `NEOPIXELBUS_` prefix; keep constexpr names `PascalCase` or existing project style for constants.

