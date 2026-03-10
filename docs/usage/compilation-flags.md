# Compilation Flags

Factory, dynamic builder, and INI parser flags have been removed from the public surface.

## Removed Legacy Flags

The following flags are no longer supported:

- `LW_FACTORY_SYSTEM_DISABLED`
- `LW_ENABLE_LEGACY_FACTORY`
- `LW_FACTORY_ENABLE_STATIC`
- `LW_FACTORY_ENABLE_DYNAMIC_BUILDER`
- `LW_FACTORY_ENABLE_INI`
- `LW_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS`
- `LW_MAIN_HEADER_ENABLE_GLOBAL_NAMESPACE_IMPORTS`

## Color Compilation Flags

Color-related compile-time controls remain supported.

| Flag | Default | Controls | Allowed Values | Notes |
|------|---------|----------|----------------|-------|
| `LW_DISABLE_TEMPLATE_COMBINATORIAL_TYPES` | `0` | Removes high-risk combinatorial template types from the exported surface | `0`, `1` | When `1`, disables `PixelBus`, `CompositeBus`, and `CompositeShader` exports and their public aliases. Runtime/interface-based alternatives such as `IPixelBus`, `AggregateBus`, `AggregateShader`, and `LightBus` remain available. |
| `LW_COLOR_MINIMUM_COMPONENT_COUNT` | `4` | Minimum internal channel count for color storage (`DefaultColorType`/internal color padding) | `3`, `4`, `5` | Global memory/compatibility trade-off; `4` defaults to RGBW-style internal storage. |
| `LW_COLOR_MINIMUM_COMPONENT_SIZE` | `8` | Minimum internal component bit depth for color storage | `8`, `16` | May widen internal storage component type to `uint16_t` when set to `16`. |
| `LW_COLOR_MATH_BACKEND` | `lw::detail::ScalarColorMathBackend` | Color math backend used by `ColorMath` helpers | Backend template type macro | Must resolve as `Backend<TColor>` with required static math API. |
| `LW_PALETTE_RANDOM_BACKEND` | `lw::detail::palettegen::XorShift32RandomBackend` | Palette random backend used by palette generators (`nextRandom`) | Backend type macro | Must resolve as `Backend` with `static constexpr uint32_t next(uint32_t&)`. |

### Validation Constraints

- `LW_COLOR_MINIMUM_COMPONENT_COUNT` must be in the range `[3, 5]`.
- `LW_COLOR_MINIMUM_COMPONENT_SIZE` must be `8` or `16`.

### Example Build Defines

- Memory-lean RGB internal storage:
  - `-D LW_COLOR_MINIMUM_COMPONENT_COUNT=3`
  - `-D LW_COLOR_MINIMUM_COMPONENT_SIZE=8`

- Force 16-bit internal component storage:
  - `-D LW_COLOR_MINIMUM_COMPONENT_SIZE=16`

- Disable exported combinatorial template types:
  - `-D LW_DISABLE_TEMPLATE_COMBINATORIAL_TYPES=1`

- Override palette random backend:
  - `-D LW_PALETTE_RANDOM_BACKEND=MyPaletteRandomBackend`
