
prompt:
lets update the legacy virtual feature comparision document. I've deleted a bunch of stuff that we're not going to port leaving only the example matrix. Lets convert this to a plan for what examples we'll need to write after truncating the old codebase. We'll want to make a more-clear distinction between protocols and transports in our new example list. 


## 15  Legacy Examples — Porting Assessment


### DotStar (3 examples)

| Example | Key Dependencies | Port Feasibility | Priority |
|---------|-----------------|:-:|:-:|
| DotStarTest | `DotStarBgrFeature`, `DotStarMethod` | **Portable** — protocol exists | Medium |
| DotStarTest_Esp32Advanced | ESP32 HSPI/VSPI, runtime pin config | Needs ESP32 SPI transport | Low |
| DotStarTest_Esp32DmaSpi | `DotStarEsp32DmaSpiMethod` | **Portable** — transport exists | Medium |

### ESP32 Parallel (2 examples)

| Example | Key Dependencies | Port Feasibility | Priority |
|---------|-----------------|:-:|:-:|
| NeoPixel_ESP32_I2sParallel | `NeoEsp32I2s1X8*Method` | Blocked — parallel transport not ported | High |


### Gamma Correction (2 examples)

| Example | Key Dependencies | Port Feasibility | Priority |
|---------|-----------------|:-:|:-:|
| NeoPixelGamma | `NeoGamma`, `HslColor`, `NeoHueBlend` | Partially — `GammaShader` exists but HSL missing | Low |


### Specialty LEDs (2 examples)

| Example | Key Dependencies | Port Feasibility | Priority |
|---------|-----------------|:-:|:-:|
| Hd108Test | `Hd108RgbFeature`, `Hd108Method` | **Portable** — protocol + transport exist | Medium |
| PixieSerial | `PixieStreamMethod` | Needs serial/stream transport | Low |

### Core / Bus Variants (3 examples)

| Example | Key Dependencies | Port Feasibility | Priority |
|---------|-----------------|:-:|:-:|
| NeoPixelTest | `NeoPixelBus`, `RgbColor`, `HslColor` | **Portable** (minus HSL demo lines) | **High** |
| NeoPixelBusLg | `NeoPixelBusLg`, `SetLuminance`, gamma methods | **Portable** via shader pipeline | Medium |


### RP2040 (1 example)

| Example | Key Dependencies | Port Feasibility | Priority |
|---------|-----------------|:-:|:-:|
| NeoPixel_RP2040_PioX4 | RP2040 PIO x4 method | **Portable** — transport exists | Medium |


### Topology (8 examples)

| Example | Key Dependencies | Port Feasibility | Priority |
|---------|-----------------|:-:|:-:|
| NeoPixelTopologyTest | `NeoTopology<>` | **Portable** | Medium |
| NeoPixelTilesTest | `NeoTiles<>` | **Portable** | Medium |

| NeoPixelMosaicTest | `NeoMosaic<>` | **Portable** | Medium |
| NeoPixelRingTopologyTest | `NeoRingTopology<>` | Blocked — ring topology not ported | Low |
| NeoPixelRingDynamicTopologyTest | `NeoRingTopology<>` | Blocked — ring topology not ported | Low |



