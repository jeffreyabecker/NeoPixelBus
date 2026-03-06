# Examples Index

Examples in this folder use the public include surface from `LumaWave.h`.
When a suitable global alias is not available, examples intentionally use fully qualified `lw::...` names.

## Hello Examples

- `hello/ws2812/ws2812.ino`: Basic one-wire strip animation.
- `hello/apa102/apa102.ino`: Basic SPI/DotStar strip animation.
- `hello/light/light.ino`: Single-light RGBCW16 output with platform-default light driver.

## Strip Composition

- `multi-strip/composite-strip/composite-strip.ino`: Composite strip composition (ownership model).
- `multi-strip/aggregate-strip/aggregate-strip.ino`: Aggregate strip composition (non-owning references).
- `multi-strip/topology-tiled-grid/topology-tiled-grid.ino`: `Topology` mapping for a 4x4 grid of 16x16 tiles.

Choose composite strip composition when you want ownership in one composed object.
Choose aggregate strip composition when child strips are created elsewhere and only referenced.

## Shader Examples

- `shaders/cct-white-balance/cct-white-balance.ino`: RGBCW CCT white-balance shader example over SPI transport.
- `shaders/gamma/gamma.ino`: Deterministic gamma correction example.
- `shaders/chain-awb-gamma/chain-awb-gamma.ino`: Aggregate shader chain (`AutoWhiteBalanceShader` + `GammaShader`).

## Platform Transport Examples

- RP2040:
  - `platform/rp2040/transport-pio/transport-pio.ino`
  - `platform/rp2040/transport-spi/transport-spi.ino`
  - `platform/rp2040/transport-uart/transport-uart.ino`
- ESP32:
  - `platform/esp32/transport-rmt/transport-rmt.ino`
  - `platform/esp32/transport-i2s/transport-i2s.ino`
  - `platform/esp32/transport-dma-spi/transport-dma-spi.ino`
- ESP8266:
  - `platform/esp8266/transport-dma-i2s/transport-dma-i2s.ino`
  - `platform/esp8266/transport-dma-uart/transport-dma-uart.ino`

## Existing Utility Example

- `platform/platformio-smoke/src/main_virtual_smoke.cpp`: Minimal compile/smoke target used by the workspace build filter.
