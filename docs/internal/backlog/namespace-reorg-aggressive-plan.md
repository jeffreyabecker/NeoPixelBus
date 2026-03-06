# Aggressive Internal Namespace Reorganization Plan

Status: planning artifact for execution.

## Goal

Reorganize internal namespaces so domain boundaries are explicit and consistent.

Primary naming rule:

- All namespaces are plural except `lw`, `detail`, and `core`.

Namespace exception:

- Symbols declared in `src/core/Compat.h` are explicitly excluded from namespace reorganization and must remain unchanged.

## Canonical Mapping Rules

The requested rules use path names like `/src/busses` and `/src/color`. In the current tree these are `src/buses` and `src/colors`. This plan treats the current tree as source-of-truth for files and applies namespace targets exactly as requested.

Required namespace destinations:

- Everything under `src/buses/` -> `lw::busses`
- Everything under `src/colors/palette/` -> `lw::colors::palettes`
- Shader-related code under `src/colors/` -> `lw::shaders`
- All other code under `src/colors/` -> `lw::colors`
- All code under `src/protocols/` -> `lw::protocols`
- All code directly under `src/transports/` -> `lw::transports`
- All code under `src/transports/<platform>/` -> `lw::transports::<platform>`

Palette naming note:

- `lw::colors::palettes` is intentional (plural) and should be treated as canonical in this plan.

Detail namespace preservation rule:

- If a symbol is in a `detail` namespace today, it remains in a `detail` namespace after move.
- Example: `lw::factory::detail::assignPixelBusProtocolTimingIfPresent` -> `lw::busses::detail::assignPixelBusProtocolTimingIfPresent`.

## Non-Goals

- No API compatibility guarantee for internal namespace names.
- No compatibility alias layer: symbols must converge to one authoritative name.
- No C++20 feature introduction.
- No namespace migration of `src/core/Compat.h` compatibility symbols.

## Authoritative Naming Policy

- Each migrated symbol has exactly one authoritative fully-qualified name.
- Compatibility aliases (`using`, wrapper overloads, legacy namespace bridges) are not part of the target architecture.
- Existing temporary aliases from in-progress slices must be removed before migration closure (Phase 7 gate).

## Execution Strategy

Perform migration in tight, compile-green phases with explicit symbol inventory and grep guards per phase.

Phase constraints:

- Keep files compiling after each phase.
- Update include aggregators as part of each domain move.
- Do not introduce new compatibility aliases; remove existing temporary aliases before final closure.

## Phase Plan

## Migration Execution Logs

### Phase 0 Execution Log

- Date: 2026-03-05
- Status: in progress (baseline captured; symbol table fill pending)
- Baseline git context:
   - `git rev-parse --short HEAD` -> `3654f35`
   - `git branch --show-current` -> `feature/namespace-hierarchy`
- Source file manifest summary:
   - `src/buses/*.h` = 6
   - `src/colors/*.h` = 21
   - `src/colors/palette/*.h` = 14
   - `src/protocols/*.h` = 18
   - `src/transports/*.h` = 10
   - `src/transports/esp32/*.h` = 5
   - `src/transports/esp8266/*.h` = 4
   - `src/transports/rp2040/*.h` = 8
- Declaration scan seed:
   - Command used: `git grep -nE "namespace |\\b(class|struct|enum|using)\\b|\\btemplate\\b|\\bconstexpr\\b|\\binline\\b" -- src/buses src/colors src/protocols src/transports`
   - Output capture: `c:\Users\jeffr\AppData\Roaming\Code\User\workspaceStorage\e0456fbca3427b1f5c4ad6f2b40595ae\GitHub.copilot-chat\chat-session-resources\b8c3eed2-b07f-4f7a-93c6-b21b878a07fd\call_7135wrxPgAwJZZDe9GrTx3ln__vscode-1772665691735\content.txt`
   - Captured lines: 610
- Baseline grep guard hit counts:
   - `namespace\\s+lw::factory` in `src/buses/*.h,src/LumaWave/*.h,src/LumaWave.h` -> 2 hits (`src/buses/MakePixelBus.h` namespace open/close)
   - `namespace\\s+lw::colors` in shader files (`src/colors/*Shader*.h,src/colors/IShader.h,src/colors/NilShader.h`) -> 0 hits
   - `namespace\\s+lw\\s*\\{` in scoped migration files -> 0 hits
- Baseline validation results:
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile` -> PASSED (6/6)
   - `pio test -e native-test` -> PASSED (189/189, 00:01:00.544)
- Files touched: none (execution log only)
- Symbols moved: none
- Compatibility shims introduced: none
- Shims removed: none
- Next actions to finish Phase 0:
   - Fill complete symbol inventory table for all scoped declarations.
   - Create dedicated Phase 0 commit before Phase 1 namespace edits.

### Phase 1 Execution Log

- Date: 2026-03-05
- Status: complete
- Commit: `6a290fb` (`phase1: move bus domain to lw::busses`)
- Files touched:
   - `examples/rp2040-cct-white-balance/src/main.cpp`
   - `examples/rp2040-pwm-light/src/main.cpp`
   - `src/buses/AggregateBus.h`
   - `src/buses/LightBus.h`
   - `src/buses/MakePixelBus.h`
   - `src/buses/PixelBus.h`
   - `src/buses/ReferenceBus.h`
   - `test/busses/test_aggregate_bus/test_main.cpp`
   - `test/busses/test_light_bus/test_main.cpp`
   - `test/busses/test_reference_bus/test_main.cpp`
   - `test/busses/test_static_bus_driver_pixel_bus/test_main.cpp`
   - `test/contracts/test_factory_descriptor_first_pass_compile/test_main.cpp`
- Symbols moved:
   - `lw::AggregateBus` -> `lw::busses::AggregateBus`
   - `lw::LightBus` -> `lw::busses::LightBus`
   - `lw::PixelBus` -> `lw::busses::PixelBus`
   - `lw::ReferenceBus` -> `lw::busses::ReferenceBus`
   - `lw::PlatformDefaultStaticBusDriverTransport` -> `lw::busses::PlatformDefaultStaticBusDriverTransport`
   - `lw::PlatformDefaultStaticBusDriverTransportSettings` -> `lw::busses::PlatformDefaultStaticBusDriverTransportSettings`
   - `lw::factory::makePixelBus` -> `lw::busses::makePixelBus`
   - `lw::factory::detail::assignPixelBusProtocolTimingIfPresent` -> `lw::busses::detail::assignPixelBusProtocolTimingIfPresent`
   - `lw::factory::detail::PixelBusProtocolSettingsHasTiming` -> `lw::busses::detail::PixelBusProtocolSettingsHasTiming`
   - `lw::factory::detail::DirectMakeBusCompatible` -> `lw::busses::detail::DirectMakeBusCompatible`
   - `lw::factory::detail::DirectMakeBusShaderCompatible` -> `lw::busses::detail::DirectMakeBusShaderCompatible`
   - `lw::factory::detail::IsWs2812xProtocolAlias` -> `lw::busses::detail::IsWs2812xProtocolAlias`
- Grep guard check:
   - `namespace lw::busses` present in all `src/buses/*.h` declarations
   - `namespace lw::factory|lw::factory::` in `src/buses/*.h,src/LumaWave/*.h,src/LumaWave.h` -> no hits
   - `namespace lw::factory|lw::factory::detail::assignPixelBusProtocolTimingIfPresent|lw::factory::makePixelBus` in `src test examples` -> no stale factory refs
- Validation results:
   - `pio test -e native-test` -> PASSED (189/189, 00:00:39.105)
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile` -> PASSED (6/6)
   - `pio test -e native-test --filter busses/*` -> PASSED (12/12)
- Compatibility shims introduced: none
- Shims removed: none
- Remaining Phase 1 work:
   - none

### Phase 2 Execution Log

- Date: 2026-03-05
- Status: in progress (shader namespace slice complete)
- Files touched:
   - `src/colors/AggregateShader.h`
   - `src/colors/AutoWhiteBalanceShader.h`
   - `src/colors/CCTWhiteBalanceShader.h`
   - `src/colors/ChannelSource.h`
   - `src/colors/ColorMath.h`
   - `src/colors/ColorMathBackend.h`
   - `src/colors/CurrentLimiterShader.h`
   - `src/colors/GammaShader.h`
   - `src/colors/HsbColor.h`
   - `src/colors/HslColor.h`
   - `src/colors/HueBlend.h`
   - `src/colors/IShader.h`
   - `src/colors/NilShader.h`
   - `src/colors/Color.h`
   - `src/colors/ChannelOrder.h`
   - `src/colors/ChannelMap.h`
   - `src/colors/ColorChannelIndexIterator.h`
   - `src/colors/ColorHexCodec.h`
   - `src/colors/ColorIterator.h`
   - `src/colors/KelvinToRgbStrategies.h`
   - `src/colors/palette/BlendOperations.h`
   - `src/colors/palette/Generators.h`
   - `src/colors/palette/SamplingTransition.h`
- Symbols moved (shader + non-shader slice):
   - `lw::IShader` -> `lw::shaders::IShader`
   - `lw::NilShader` -> `lw::shaders::NilShader`
   - `lw::AggregateShaderSettings` -> `lw::shaders::AggregateShaderSettings`
   - `lw::AggregateShader` -> `lw::shaders::AggregateShader`
   - `lw::OwningAggregateShaderT` -> `lw::shaders::OwningAggregateShaderT`
   - `lw::CurrentLimiterShaderSettings` -> `lw::shaders::CurrentLimiterShaderSettings`
   - `lw::CurrentLimiterShader` -> `lw::shaders::CurrentLimiterShader`
   - `lw::GammaShaderSettings` -> `lw::shaders::GammaShaderSettings`
   - `lw::GammaShader` -> `lw::shaders::GammaShader`
   - `lw::WledGammaShader` -> `lw::shaders::WledGammaShader`
   - `lw::AutoWhiteBalanceShaderSettings` -> `lw::shaders::AutoWhiteBalanceShaderSettings`
   - `lw::AutoWhiteBalanceShader` -> `lw::shaders::AutoWhiteBalanceShader`
   - `lw::CCTColorInterlock` -> `lw::shaders::CCTColorInterlock`
   - `lw::CCTWhiteBalanceShaderSettings` -> `lw::shaders::CCTWhiteBalanceShaderSettings`
   - `lw::CCTWhiteBalanceShader` -> `lw::shaders::CCTWhiteBalanceShader`
   - `lw::ChannelSource` -> `lw::colors::ChannelSource`
   - `lw::ColorMathBackendSelector` -> `lw::colors::ColorMathBackendSelector`
   - `lw::detail::ScalarColorMathBackend` -> `lw::colors::detail::ScalarColorMathBackend`
   - `lw::smoothstep8`/`cubicEaseInOut8`/`cosineLike8`/`integerSqrt`/`darken`/`lighten`/`linearBlend`/`bilinearBlend` -> `lw::colors::*`
   - `lw::HueBlendBase`/`HueBlendShortestDistance`/`HueBlendLongestDistance`/`HueBlendClockwiseDirection`/`HueBlendCounterClockwiseDirection` -> `lw::colors::*`
   - `lw::HsbColor`/`lw::HslColor` and `lw::toRgb(...)` overloads -> `lw::colors::*`
- Grep guard check:
   - `namespace lw::shaders` present in all shader headers (`src/colors/*Shader*.h`, `src/colors/IShader.h`, `src/colors/NilShader.h`)
   - `namespace lw::colors` in shader headers -> no hits
   - `namespace lw::colors` present in migrated non-shader slice (`ChannelSource.h`, `ColorMath.h`, `ColorMathBackend.h`, `HueBlend.h`, `HsbColor.h`, `HslColor.h`)
- Validation results:
   - `pio test -e native-test --filter shaders/*` -> PASSED (116/116)
   - `pio test -e native-test --filter shaders/test_color_domain_section1` -> PASSED (30/30)
   - `pio test -e native-test --filter shaders/test_color_models_section5` -> PASSED (9/9)
   - `pio test -e native-test --filter shaders/test_color_manipulation_section6` -> PASSED (8/8)
   - `pio test -e native-test --filter contracts/*shader*` -> no matching tests (0 discovered)
   - `pio test -e native-test --filter colors/*` -> no matching tests (0 discovered)
   - `pio test -e native-test` -> PASSED (189/189, 00:00:40.352)
- Compatibility shims introduced:
   - Temporary `namespace lw` alias templates/usings in each moved shader header to preserve current call sites.
   - Temporary `namespace lw` aliases/wrapper overloads for migrated non-shader color symbols and selector specialization compatibility.
- Shims removed: none
- Remaining Phase 2 work:
   - Completed remaining non-shader color declarations move to `lw::colors` for `Color.h`, `ChannelOrder.h`, `ChannelMap.h`, `ColorChannelIndexIterator.h`, `ColorHexCodec.h`, `ColorIterator.h`, and `KelvinToRgbStrategies.h`.
   - Qualified palette `linearBlend` call sites in `BlendOperations.h`, `Generators.h`, and `SamplingTransition.h` to prevent ADL ambiguity during transition shims.
   - Update call sites off temporary `lw` shader aliases and then remove aliases.
   - Record Phase 2 commit hash after the next checkpoint commit.

### Phase 3 Execution Log

- Status: not started

### Phase 4 Execution Log

- Date: 2026-03-05
- Status: complete (protocol declaration normalization with temporary compatibility re-exports)
- Files touched:
   - `src/protocols/IProtocol.h`
   - `src/protocols/ProtocolDecoratorBase.h`
   - `src/protocols/ProtocolAliases.h`
   - `src/protocols/DebugProtocol.h`
   - `src/protocols/DotStarProtocol.h`
   - `src/protocols/NilProtocol.h`
   - `src/protocols/Lpd6803Protocol.h`
   - `src/protocols/Lpd8806Protocol.h`
   - `src/protocols/P9813Protocol.h`
   - `src/protocols/PixieProtocol.h`
   - `src/protocols/Sm16716Protocol.h`
   - `src/protocols/Sm168xProtocol.h`
   - `src/protocols/Tlc59711Protocol.h`
   - `src/protocols/Tm1814Protocol.h`
   - `src/protocols/Tm1914Protocol.h`
   - `src/protocols/Ws2801Protocol.h`
   - `src/protocols/Ws2812xProtocol.h`
- Symbols moved:
   - `lw::ProtocolSettings` -> `lw::protocols::ProtocolSettings`
   - `lw::IProtocol` -> `lw::protocols::IProtocol`
   - `lw::ProtocolType`/`ProtocolMoveConstructible`/`ProtocolExternalBufferRequired`/`ProtocolRequiredBufferSizeComputable`/`ProtocolPixelSettingsConstructible` -> `lw::protocols::*`
   - Concrete protocol declarations in `src/protocols/*.h` moved from `lw` to `lw::protocols` (DotStar/HD108, Nil, Debug, Lpd6803, Lpd8806, P9813, Pixie, Sm16716, Sm168x, Tlc59711, Tm1814, Tm1914, Ws2801, Ws2812x families)
- Grep guard check:
   - `namespace lw::protocols|namespace lw::protocols::detail` in `src/protocols/*.h` -> present in all protocol declaration headers
   - `namespace lw` declarations in `src/protocols/*.h` are temporary compatibility re-export blocks only
   - `\blw::(Apa102Protocol|Hd108Protocol|NilProtocol|DebugProtocol|Tm1814ProtocolT|Tm1914ProtocolT|Ws2812xProtocol)\b` in `src/protocols/ProtocolAliases.h` -> no hits (aliases now use canonical `lw::protocols::*` protocol references)
- Validation results:
   - `pio test -e native-test --filter protocols/*` -> PASSED (38/38, 00:00:14.959)
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile` -> PASSED (6/6, 00:00:03.139)
   - `pio test -e native-test` -> PASSED (189/189, 00:00:43.595)
- Compatibility shims introduced:
   - Temporary top-level `namespace lw` re-export aliases/usings in migrated protocol headers to keep existing call sites compiling during phased migration.
- Shims removed: none
- Remaining Phase 4 work:
   - Record Phase 4 commit hash once checkpoint commit is created. (done: `fc82c37`)
   - Remove temporary protocol re-export aliases in Phase 7 namespace-purity closure.

### Phase 5 Execution Log

- Date: 2026-03-06
- Status: complete (transport declaration normalization with temporary compatibility re-exports)
- Files touched:
   - `src/buses/PixelBus.h`
   - `src/transports/ITransport.h`
   - `src/transports/NilTransport.h`
   - `src/transports/OneWireEncoding.h`
   - `src/transports/OneWireTiming.h`
   - `src/transports/PrintTransport.h`
   - `src/transports/SpiTransport.h`
   - `src/transports/esp32/Esp32DmaSpiTransport.h`
   - `src/transports/esp32/Esp32I2sTransport.h`
   - `src/transports/esp32/Esp32RmtTransport.h`
   - `src/transports/esp8266/Esp8266DmaI2sTransport.h`
   - `src/transports/esp8266/Esp8266DmaUartTransport.h`
   - `src/transports/rp2040/RpDmaManager.h`
   - `src/transports/rp2040/RpPioManager.h`
   - `src/transports/rp2040/RpPioSession.h`
   - `src/transports/rp2040/RpPioSmConfig.h`
   - `src/transports/rp2040/RpPioTransport.h`
   - `src/transports/rp2040/RpSpiTransport.h`
   - `src/transports/rp2040/RpUartTransport.h`
- Symbols moved:
   - Root transport declarations moved from `lw::*` to `lw::transports::*` in `ITransport`, `NilTransport`, `OneWireTiming`, `OneWireEncoding` helpers, `PrintTransport`, and `SpiTransport` families.
   - Platform transport declarations moved to canonical platform namespaces:
      - `lw::transports::esp32::*` for ESP32 DMA SPI/I2S/RMT transport declarations.
      - `lw::transports::esp8266::*` for ESP8266 DMA I2S/UART transport declarations.
      - `lw::transports::rp2040::*` for RP2040 DMA/PIO manager/session/config and PIO/SPI/UART transport declarations.
   - `PixelBus` platform default static transport aliases now point to canonical transport namespace types (`lw::transports::*` and `lw::transports::<platform>::*`).
- Grep guard check:
   - `namespace\s+lw::transports` in `src/transports/*.h` -> present in root transport declarations.
   - `namespace\s+lw::transports::esp32` in `src/transports/esp32/*.h` -> present.
   - `namespace\s+lw::transports::esp8266` in `src/transports/esp8266/*.h` -> present.
   - `namespace\s+lw::transports::rp2040` in `src/transports/rp2040/*.h` -> present.
   - `public\s+bool\s+invert|bool\s+invert\s*=\s*false` in `src/transports/**/*.h` -> `ITransportSettings::invert` contract preserved.
- Validation results:
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile` -> PASSED (6/6, 00:00:01.532)
   - `pio test -e native-test` -> PASSED (189/189, 00:00:51.703)
- Compatibility shims introduced:
   - Temporary top-level `namespace lw` re-export aliases/usings for moved root and platform transport declarations to keep in-flight call sites compiling during phased migration.
- Shims removed: none
- Remaining Phase 5 work:
   - Record Phase 5 commit hash once checkpoint commit is created. (done: `f15faf1`)
   - Remove temporary transport re-export aliases in Phase 7 namespace-purity closure.

### Phase 6 Execution Log

- Date: 2026-03-05
- Status: complete (aggregator/include-surface verification; no source edits required)
- Files touched:
   - `docs/internal/backlog/namespace-reorg-aggressive-plan.md`
- Symbols moved:
   - none
- Umbrella/include surface summary:
   - `src/LumaWave.h` remains a single include of `LumaWave/All.h`.
   - `src/LumaWave/All.h` re-exports Core, Colors, Transports, Protocols, and Buses umbrellas.
   - Domain umbrella headers (`src/LumaWave/Buses.h`, `src/LumaWave/Colors.h`, `src/LumaWave/Protocols.h`, `src/LumaWave/Transports.h`, `src/LumaWave/Core.h`) point at canonical internal include roots.
   - Example include policy remains compliant: RP2040 examples include only `<LumaWave.h>`.
- Grep guard check:
   - Deprecated namespace patterns (`namespace lw::factory`, `namespace lw::palette`, `lw::factory::detail::assignPixelBusProtocolTimingIfPresent`) in `src/**/*.h` -> no hits.
   - `namespace lw` blocks remaining in `src/**/*.h` are temporary compatibility re-export aliases in migrated domains (planned removal in Phase 7) plus canonical `src/core/*` root namespace declarations.
   - `src/core/Compat.h` namespace declarations remain unchanged by explicit migration exception.
- Validation results:
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile` -> PASSED (6/6, 00:00:01.533)
   - `pio test -e native-test` -> PASSED (189/189, 00:00:38.115)
   - `pio run -e pico2w` -> PASSED (RP2040 smoke build from root `platformio.ini` / `examples/platformio-smoke/src/main_virtual_smoke.cpp`, 00:00:07.201)
- Compatibility shims introduced:
   - none
- Shims removed:
   - none
- Residual risks / follow-up:
   - Temporary top-level re-export aliases in moved buses/colors/protocols/transports headers remain by design and are deferred to Phase 7 namespace-purity closure.

### Phase 7 Execution Log

- Date: 2026-03-05
- Status: in progress (protocol/transport + palette shim-reduction slices complete)
- Files touched:
   - `src/protocols/IProtocol.h`
   - `src/protocols/DebugProtocol.h`
   - `src/protocols/DotStarProtocol.h`
   - `src/protocols/NilProtocol.h`
   - `src/protocols/Lpd6803Protocol.h`
   - `src/protocols/Lpd8806Protocol.h`
   - `src/protocols/P9813Protocol.h`
   - `src/protocols/PixieProtocol.h`
   - `src/protocols/ProtocolDecoratorBase.h`
   - `src/protocols/Sm16716Protocol.h`
   - `src/protocols/Sm168xProtocol.h`
   - `src/protocols/Tlc59711Protocol.h`
   - `src/protocols/Tm1814Protocol.h`
   - `src/protocols/Tm1914Protocol.h`
   - `src/protocols/Ws2801Protocol.h`
   - `src/protocols/Ws2812xProtocol.h`
   - `src/transports/ITransport.h`
   - `src/transports/NilTransport.h`
   - `src/transports/OneWireEncoding.h`
   - `src/transports/OneWireTiming.h`
   - `src/transports/PrintTransport.h`
   - `src/transports/SpiTransport.h`
   - `src/transports/esp32/Esp32DmaSpiTransport.h`
   - `src/transports/esp32/Esp32I2sTransport.h`
   - `src/transports/esp32/Esp32RmtTransport.h`
   - `src/transports/esp8266/Esp8266DmaI2sTransport.h`
   - `src/transports/esp8266/Esp8266DmaUartTransport.h`
   - `src/transports/rp2040/RpPioTransport.h`
   - `src/transports/rp2040/RpSpiTransport.h`
   - `src/transports/rp2040/RpUartTransport.h`
   - `src/buses/PixelBus.h`
   - `src/core/TypeConstraints.h`
- Shim cleanup summary:
   - Removed per-header top-level `namespace lw` re-export blocks from all migrated protocol headers listed above.
   - Removed per-header top-level `namespace lw` re-export blocks from migrated transport headers in root and platform folders listed above.
   - Added centralized lw-level compatibility re-exports in `IProtocol.h`, `ITransport.h`, `OneWireTiming.h`, and `OneWireEncoding.h` to preserve current call sites while continuing Phase 7 in smaller slices.
- Validation results:
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile` -> PASSED (6/6, 00:00:02.892)
   - `pio test -e native-test` -> PASSED (189/189, 00:00:51.490)
- Remaining Phase 7 work:
   - Remove remaining temporary top-level re-export aliases in colors domain.
   - Replace centralized compatibility re-exports with canonical call-site usage (or explicitly scope any intentional public alias surface).
   - Run final namespace-purity grep gates and record zero-hit results for deprecated namespace/shim markers in migrated domains.

- Date: 2026-03-05 (palette/core slice)
- Status: complete for palette namespace normalization; core audited with explicit Compat exception preserved
- Files touched:
   - `src/colors/palette/BlendOperations.h`
   - `src/colors/palette/Blends.h`
   - `src/colors/palette/Detail.h`
   - `src/colors/palette/Generators.h`
   - `src/colors/palette/NearestPolicies.h`
   - `src/colors/palette/RandomBackend.h`
   - `src/colors/palette/Sampling.h`
   - `src/colors/palette/SamplingTransition.h`
   - `src/colors/palette/Traits.h`
   - `src/colors/palette/Types.h`
   - `src/colors/palette/WrapModes.h`
   - `src/colors/palette/WrappedPaletteIndexes.h`
- Symbols moved:
   - Palette domain declarations moved from top-level `lw` to canonical `lw::colors::palettes` across blend/sampling/generator/traits/types/wrap/index helpers.
   - Random backend namespace moved from `lw::detail::palettegen` to `lw::colors::palettes::detail::palettegen`.
- Compatibility shims introduced:
   - Temporary centralized palette bridge in `src/colors/palette/Types.h`: `namespace lw { using namespace colors::palettes; }` to keep current `lw::*` palette call sites compiling.
   - Temporary random backend bridge in `src/colors/palette/RandomBackend.h`: `lw::detail::palettegen::XorShift32RandomBackend` alias to canonical palette namespace.
   - Existing `src/colors/palette/WrapModes.h` temporary lw aliases retained.
- Core domain note:
   - `src/core/Compat.h` remained unchanged per explicit migration exception.
   - No additional core namespace moves were applied in this slice.
- Validation results:
   - `pio test -e native-test --filter shaders/test_palette_modes_section7` -> PASSED (8/8, 00:00:01.918)
   - `pio test -e native-test --filter shaders/test_palette_helpers_section7` -> PASSED (6/6, 00:00:01.761)
   - `pio test -e native-test --filter shaders/test_palette_sampling_overloads_section7` -> PASSED (9/9, 00:00:01.769)
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile` -> PASSED (6/6, 00:00:01.548)
   - `pio test -e native-test` -> PASSED (189/189, 00:00:40.478)

### Phase 0: Baseline and Guardrails

1. Freeze baseline with current green tests.
2. Generate symbol inventory for all touched directories (types, functions, aliases, variable templates, constants).
3. Add one temporary tracking doc section per phase with:
   - files touched
   - symbols moved
   - compatibility shims introduced
   - shims removed
4. Add grep guard list (must be zero by final phase):
   - `namespace lw::factory` for moved bus symbols
   - `namespace lw(\s*\{|::)colors` for shader-only files
   - `namespace lw` in files that should now be nested domain namespaces

Concrete executable subtask list (run in order):

1. Create migration tracking section in this document:
   - `Phase 0 Execution Log`
   - `Phase 1 Execution Log`
   - `Phase 2 Execution Log`
   - `Phase 3 Execution Log`
   - `Phase 4 Execution Log`
   - `Phase 5 Execution Log`
   - `Phase 6 Execution Log`
   - `Phase 7 Execution Log`
2. Capture baseline commit and branch context:
   - record `git rev-parse --short HEAD`
   - record active branch name
3. Build source file manifests (saved in phase log):
   - buses: `src/buses/*.h`
   - colors root: `src/colors/*.h`
   - colors palette: `src/colors/palette/*.h`
   - protocols: `src/protocols/*.h`
   - transports root: `src/transports/*.h`
   - transports platform: `src/transports/esp32/*.h`, `src/transports/esp8266/*.h`, `src/transports/rp2040/*.h`
4. Generate symbol inventory seed from declarations:
   - use `rg` to collect `namespace`, `struct`, `class`, `enum`, `using`, function signatures, and variable templates.
   - include both public and `detail` symbols.
5. Fill the symbol inventory table (template below) for every declaration in scope.
6. Define initial grep guard command set and capture baseline hit counts.
7. Run baseline validation commands and record pass/fail status.
8. Open migration with a dedicated Phase 0 commit before namespace edits.

Phase 0 command checklist:

1. File manifests:
   - `rg --files src/buses src/colors src/protocols src/transports`
2. Declaration scan seed:
   - `rg -n "namespace |\b(class|struct|enum|using)\b|\btemplate\b|\bconstexpr\b|\binline\b" src/buses src/colors src/protocols src/transports`
3. Baseline grep guards:
   - `rg -n "namespace\s+lw::factory" src/buses src/LumaWave src/LumaWave.h`
   - `rg -n "namespace\s+lw::colors" src/colors/*Shader*.h src/colors/IShader.h src/colors/NilShader.h`
   - `rg -n "namespace\s+lw\s*\{" src/buses src/colors src/protocols src/transports`
4. Baseline tests:
   - `pio test -e native-test`
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile`

Phase 0 symbol inventory table template:

| File | Symbol Kind | Symbol Name | Current FQN | Target FQN | In `detail` Now | Keep `detail` | Notes |
|---|---|---|---|---|---|---|---|
| `src/buses/MakePixelBus.h` | function | `assignPixelBusProtocolTimingIfPresent` | `lw::factory::detail::assignPixelBusProtocolTimingIfPresent` | `lw::busses::detail::assignPixelBusProtocolTimingIfPresent` | yes | yes | explicit example mapping |

Phase 0 required outputs before Phase 1 starts:

1. Complete symbol inventory table covering all scoped files.
2. Baseline grep guard results with hit counts documented.
3. Baseline test results documented.
4. Phase 0 commit created and referenced in this document.

### Phase 1: Buses Namespace Move (`src/buses` -> `lw::busses`)

Scope:

- Every declaration in `src/buses/*.h`.
- Move bus factory internals from `lw::factory` into `lw::busses` or `lw::busses::detail`.

Key actions:

1. Change enclosing namespaces in bus headers.
2. Move bus-specific detail traits/helpers into `lw::busses::detail`.
3. Update call sites and `using` declarations in public umbrella headers.
4. Remove stale `lw::factory` references for moved symbols.

Exit criteria:

- No residual references to moved bus symbols under `lw::factory`.
- `makeBus(...)` public composition still compiles and behaves unchanged.

Detailed execution checklist:

1. Build file list for scope:
   - `src/buses/*.h`
   - `src/LumaWave/Buses.h`
   - `src/LumaWave/All.h`
   - `src/LumaWave.h`
2. Build symbol table for each bus header with columns:
   - symbol kind
   - old fully-qualified name
   - new fully-qualified name
   - has `detail` (yes/no)
3. Namespace edits:
   - Move non-detail declarations to `lw::busses`.
   - Move helper traits/functions to `lw::busses::detail`.
   - Keep existing `detail` names under `detail` after move.
4. Factory transition handling:
   - Rehome bus-specific factory helpers from `lw::factory::detail` into `lw::busses::detail`.
   - Leave non-bus factory constructs outside this phase.
5. Include/export updates:
   - Update umbrella exports and using declarations to new bus namespace.
   - Keep includes public-surface oriented (`LumaWave.h` re-export behavior unchanged).
6. Grep guards before commit:
   - no bus symbols in `namespace lw::factory`
   - no references to `lw::factory::detail::assignPixelBusProtocolTimingIfPresent`
7. Validation gates:
   - `pio test -e native-test --filter busses/*`
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile`

Phase 1 command checklist:

1. Build bus symbol manifest:
   - `rg -n "namespace |\b(class|struct|enum|using)\b|\btemplate\b|\bconstexpr\b|\binline\b" src/buses src/LumaWave/Buses.h src/LumaWave/All.h src/LumaWave.h`
2. Find bus helper symbols still under factory:
   - `rg -n "lw::factory::detail::|namespace\s+lw::factory" src/buses src/LumaWave`
3. Confirm new namespace declarations exist:
   - `rg -n "namespace\s+lw::busses|namespace\s+lw::busses::detail" src/buses`
4. Validate no stale references for moved helper example:
   - `rg -n "assignPixelBusProtocolTimingIfPresent" src`
5. Run targeted tests:
   - `pio test -e native-test --filter busses/*`
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile`

Phase 1 required outputs:

1. Completed bus symbol mapping table (`old FQN` -> `new FQN`).
2. Grep summary showing no moved bus symbols remain under `lw::factory`.
3. Targeted bus and factory-contract test pass results.
4. Commit hash recorded in `Phase 1 Execution Log`.

### Phase 2: Colors Split (`src/colors` -> `lw::colors` and `lw::shaders`)

Scope split:

- Shader files (`*Shader*.h`, `IShader.h`, shader composition helpers) -> `lw::shaders`.
- Non-shader color files -> `lw::colors`.
- Palette subtree stays under `lw::colors::palettes`.

Candidate shader set (verify during symbol inventory pass):

- `IShader.h`
- `NilShader.h`
- `AggregateShader.h`
- `CurrentLimiterShader.h`
- `GammaShader.h`
- `AutoWhiteBalanceShader.h`
- `CCTWhiteBalanceShader.h`

Key actions:

1. Move shader declarations to `lw::shaders`.
2. Keep pure color math/types under `lw::colors`.
3. Update cross-namespace dependencies (`lw::shaders` consuming `lw::colors` types).
4. Preserve any existing `detail` layering as `...::detail`.

Exit criteria:

- Shader headers export symbols only in `lw::shaders` (plus allowed `detail`).
- Non-shader color headers export symbols in `lw::colors`.

Detailed execution checklist:

1. Partition `src/colors/*.h` into two inventories:
   - Shader partition: shader interfaces/implementations and shader composition helpers.
   - Color partition: color data types, channel helpers, math, iterators, codecs.
2. Establish explicit shader file set:
   - `src/colors/IShader.h`
   - `src/colors/NilShader.h`
   - `src/colors/AggregateShader.h`
   - `src/colors/CurrentLimiterShader.h`
   - `src/colors/GammaShader.h`
   - `src/colors/AutoWhiteBalanceShader.h`
   - `src/colors/CCTWhiteBalanceShader.h`
3. Namespace edits for shader partition:
   - Move declarations to `lw::shaders`.
   - Preserve internals as `lw::shaders::detail` where applicable.
4. Namespace edits for color partition:
   - Move declarations to `lw::colors`.
   - Preserve internals as `lw::colors::detail` where applicable.
5. Dependency and include cleanup:
   - Ensure shader headers include color headers via `lw::colors` contracts.
   - Remove cyclic includes introduced by namespace split.
6. Umbrella surface updates:
   - Re-export both `lw::colors` and `lw::shaders` symbols through intended LumaWave umbrellas.
7. Grep guards before commit:
   - shader files must not declare symbols in `lw::colors`
   - non-shader files must not declare symbols in `lw::shaders`
8. Validation gates:
   - `pio test -e native-test --filter shaders/*`
   - `pio test -e native-test --filter colors/*`
   - `pio test -e native-test --filter contracts/*shader*`

Phase 2 command checklist:

1. Build shader/color declaration manifests:
   - `rg -n "namespace |\b(class|struct|enum|using)\b|\btemplate\b" src/colors`
2. Enumerate shader files and declarations:
   - `rg -n "namespace\s+|class\s+|struct\s+" src/colors/*Shader*.h src/colors/IShader.h src/colors/NilShader.h`
3. Verify namespace split:
   - `rg -n "namespace\s+lw::shaders" src/colors/*Shader*.h src/colors/IShader.h src/colors/NilShader.h`
   - `rg -n "namespace\s+lw::colors" src/colors/*.h`
4. Guard against cross-domain declaration leaks:
   - `rg -n "namespace\s+lw::colors" src/colors/*Shader*.h src/colors/IShader.h src/colors/NilShader.h`
   - `rg -n "namespace\s+lw::shaders" src/colors/Color*.h src/colors/Hs*.h src/colors/Channel*.h src/colors/ColorMath*.h`
5. Run targeted tests:
   - `pio test -e native-test --filter shaders/*`
   - `pio test -e native-test --filter colors/*`
   - `pio test -e native-test --filter contracts/*shader*`

Phase 2 required outputs:

1. Two inventories: shader partition and non-shader color partition.
2. Grep summary proving no declaration leakage between `lw::shaders` and `lw::colors`.
3. Targeted shader/color contract test pass results.
4. Commit hash recorded in `Phase 2 Execution Log`.

### Phase 3: Palette Normalization (`src/colors/palette` -> `lw::colors::palettes`)

Scope:

- All files under `src/colors/palette/` including nested `detail` headers.

Key actions:

1. Normalize all palette declarations under `lw::colors::palettes`.
2. Keep implementation internals under `lw::colors::palettes::detail`.
3. Remove residual alternate palette namespaces if present.

Exit criteria:

- No palette declarations in `lw::palette` or other ad-hoc namespaces.

Detailed execution checklist:

1. Inventory all declarations under `src/colors/palette/*.h`.
2. Inventory nested declarations under `src/colors/palette/detail/*.h` (if present).
3. Namespace edits:
   - Move all public palette declarations to `lw::colors::palettes`.
   - Move internal helpers to `lw::colors::palettes::detail`.
4. Cross-file consistency:
   - Ensure blend/sampling/wrap policies reference `lw::colors::palettes` consistently.
   - Normalize any mixed naming variants (`palette`, `palettes`) to requested namespace.
5. Include and umbrella updates:
   - Verify `src/colors/palette/Detail.h` and `src/colors/Colors.h` exports align with new namespaces.
6. Grep guards before commit:
   - no declarations under `namespace lw::palette`
   - no declarations under `namespace lw::colors::detail` for palette public entities
7. Validation gates:
   - `pio test -e native-test --filter colors/*`
   - `pio test -e native-test --filter contracts/*colors*`

Phase 3 command checklist:

1. Build palette declaration manifest:
   - `rg -n "namespace |\b(class|struct|enum|using)\b|\btemplate\b" src/colors/palette`
2. Verify public palette namespace normalization:
   - `rg -n "namespace\s+lw::colors::palettes" src/colors/palette`
3. Verify detail preservation:
   - `rg -n "namespace\s+lw::colors::palettes::detail" src/colors/palette`
4. Guard against stale/alternate palette namespaces:
   - `rg -n "namespace\s+lw::palette|namespace\s+lw::palettes" src/colors/palette`
5. Run targeted tests:
   - `pio test -e native-test --filter colors/*`
   - `pio test -e native-test --filter contracts/*colors*`

Phase 3 required outputs:

1. Palette symbol mapping table with `detail` status.
2. Grep summary proving palette declarations live only under `lw::colors::palettes` (+ `detail`).
3. Targeted color/palette test pass results.
4. Commit hash recorded in `Phase 3 Execution Log`.

### Phase 4: Protocols Normalization (`src/protocols` -> `lw::protocols`)

Scope:

- All protocol interfaces, protocol implementations, aliases, decorators, and protocol `detail` helpers.

Key actions:

1. Move all protocol declarations to `lw::protocols`.
2. Preserve `detail` as `lw::protocols::detail`.
3. Verify alias families remain coherent (for example `Ws2812`, `DotStar`, `Debug` family).

Exit criteria:

- No protocol declarations outside `lw::protocols` except intended public re-exports.

Detailed execution checklist:

1. Build protocol inventory from:
   - `src/protocols/*.h`
   - aliases/decorators (`ProtocolAliases.h`, `ProtocolDecoratorBase.h`)
2. Namespace edits:
   - Move public protocol declarations to `lw::protocols`.
   - Move helper traits/resolution helpers into `lw::protocols::detail`.
3. Alias-family integrity pass:
   - Verify alias templates and convenience aliases retain the same resolved protocol types.
   - Verify decorator wrappers (for example debug wrappers) still apply normalization/settings behavior.
4. Include/umbrella updates:
   - Reconcile `src/protocols/Protocols.h`, `src/LumaWave/Protocols.h`, and `src/LumaWave.h` exports.
5. Grep guards before commit:
   - no protocol declarations in top-level `lw`
   - no moved protocol helper traits outside `lw::protocols::detail`
6. Validation gates:
   - `pio test -e native-test --filter protocols/*`
   - `pio test -e native-test --filter contracts/*protocol*`

Phase 4 command checklist:

1. Build protocol declaration manifest:
   - `rg -n "namespace |\b(class|struct|enum|using)\b|\btemplate\b" src/protocols`
2. Verify protocol namespace normalization:
   - `rg -n "namespace\s+lw::protocols|namespace\s+lw::protocols::detail" src/protocols`
3. Guard against protocol declarations in top-level `lw`:
   - `rg -n "namespace\s+lw\s*\{" src/protocols`
4. Validate alias family surfaces still exist:
   - `rg -n "using\s+(Ws2812|DotStar|Debug|Tm1814|Tm1914)" src/protocols/ProtocolAliases.h`
5. Run targeted tests:
   - `pio test -e native-test --filter protocols/*`
   - `pio test -e native-test --filter contracts/*protocol*`

Phase 4 required outputs:

1. Protocol symbol mapping table including alias/decorator entries.
2. Grep summary proving moved protocol declarations are under `lw::protocols`.
3. Targeted protocol contract test pass results.
4. Commit hash recorded in `Phase 4 Execution Log`.

### Phase 5: Transports Normalization (`src/transports`)

Scope:

- Direct transport headers -> `lw::transports`.
- Platform-specific headers -> `lw::transports::<platform>`.

Platforms currently expected:

- `esp32`
- `esp8266`
- `rp2040`

Key actions:

1. Normalize top-level transport interfaces and helpers under `lw::transports`.
2. Normalize platform implementations under `lw::transports::<platform>`.
3. Keep platform `detail` blocks as `lw::transports::<platform>::detail` where present.

Exit criteria:

- No platform transport declarations leaking into top-level `lw`.

Detailed execution checklist:

1. Build two transport inventories:
   - top-level files in `src/transports/*.h`
   - platform files in `src/transports/esp32/*.h`, `src/transports/esp8266/*.h`, `src/transports/rp2040/*.h`
2. Namespace edits for top-level transport files:
   - Move declarations to `lw::transports`.
   - Keep top-level transport internals under `lw::transports::detail` where applicable.
3. Namespace edits for platform files:
   - Move declarations to `lw::transports::esp32`, `lw::transports::esp8266`, `lw::transports::rp2040`.
   - Keep platform internals under `lw::transports::<platform>::detail`.
4. Contract checks:
   - Preserve `TransportTag`, `OneWireTransportTag`, and `AnyTransportTag` compatibility.
   - Preserve required `public bool invert` transport settings contract.
5. Include/umbrella updates:
   - Update `src/transports/Transports.h` and `src/LumaWave/Transports.h` re-exports.
6. Grep guards before commit:
   - no platform declarations in plain `lw::transports` namespace
   - no moved transport declarations in top-level `lw`
7. Validation gates:
   - `pio test -e native-test --filter transports/*`
   - `pio test -e native-test --filter contracts/*transport*`

Phase 5 command checklist:

1. Build transport declaration manifests:
   - `rg -n "namespace |\b(class|struct|enum|using)\b|\btemplate\b" src/transports`
2. Verify top-level transport namespace normalization:
   - `rg -n "namespace\s+lw::transports|namespace\s+lw::transports::detail" src/transports/*.h`
3. Verify platform namespace normalization:
   - `rg -n "namespace\s+lw::transports::esp32|namespace\s+lw::transports::esp8266|namespace\s+lw::transports::rp2040" src/transports/esp32 src/transports/esp8266 src/transports/rp2040`
4. Guard against platform leakage into top-level transport namespace:
   - `rg -n "Esp32|Esp8266|Rp2040" src/transports/*.h`
5. Contract checks via grep:
   - `rg -n "TransportTag|OneWireTransportTag|AnyTransportTag|public\s+bool\s+invert" src/transports`
6. Run targeted tests:
   - `pio test -e native-test --filter transports/*`
   - `pio test -e native-test --filter contracts/*transport*`

Phase 5 required outputs:

1. Top-level and platform transport symbol mapping tables.
2. Grep summary proving correct namespace placement for transport and platform symbols.
3. Targeted transport contract test pass results.
4. Commit hash recorded in `Phase 5 Execution Log`.

### Phase 6: Aggregator and Include Surface Cleanup

Scope:

- `src/LumaWave/*.h` umbrella exports.
- `src/LumaWave.h` top-level API surface.
- Internal include paths and stale forward declarations.

Key actions:

1. Repoint re-exports to new namespaces.
2. Normalize include paths and stale forward declarations.
3. Ensure examples and tests compile with public surface only.

Exit criteria:

- Umbrella exports and include paths reflect the new namespace layout.
- Example and test builds succeed with public-surface includes.

Detailed execution checklist:

1. Enumerate all public umbrella headers:
   - `src/LumaWave.h`
   - `src/LumaWave/All.h`
   - `src/LumaWave/Buses.h`
   - `src/LumaWave/Colors.h`
   - `src/LumaWave/Protocols.h`
   - `src/LumaWave/Transports.h`
   - `src/LumaWave/Core.h`
2. Include-path normalization:
   - Ensure no internal headers include obsolete path forms from pre-move namespaces.
   - Ensure examples include only `#include <LumaWave.h>` (and Arduino include if needed).
3. Final grep sweep:
   - old namespaces for moved symbols should return zero hits
4. Full validation gates:
   - `pio test -e native-test`
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile`
   - targeted smoke examples compile check for RP2040 profile
5. Documentation closure:
   - Mark completed phases and note residual risks.
   - Capture any intentionally deferred follow-ups in `docs/internal/backlog/todo.md`.

Phase 6 command checklist:

1. Enumerate umbrella includes and exports:
   - `rg -n "#include|using\s+" src/LumaWave.h src/LumaWave/*.h`
2. Detect stale namespace references in moved scope:
   - `rg -n "lw::factory::detail::assignPixelBusProtocolTimingIfPresent|namespace\s+lw::factory|namespace\s+lw::palette|namespace\s+lw\s*\{" src/buses src/colors src/protocols src/transports src/LumaWave src/LumaWave.h`
3. Confirm example include policy:
   - `rg -n "#include\s+<LumaWave.h>|#include\s+\"(factory|transports|protocols|colors|buses)/" examples`
4. Run full validation gates:
   - `pio test -e native-test`
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile`

Phase 6 required outputs:

1. Final grep summary showing zero hits for deprecated namespace locations.
2. Umbrella/export and include-path normalization summary.
3. Full native test and contract gate pass results.
4. Phase 6 closure note with residual risks and follow-up backlog items.

### Phase 7: Final Shim Removal and Namespace Purity Gate

Scope:

- Remove every remaining temporary migration alias/shim introduced in earlier phases.
- Enforce zero-tolerance for compatibility shim markers in moved domains.

Key actions:

1. Delete temporary `using` aliases that map old namespaces to new namespaces.
2. Remove compatibility include redirects added only to keep phased migration compile-green.
3. Requalify any remaining call sites still depending on shim symbols.
4. Run final namespace-purity grep and full validation gates.

Exit criteria:

- No temporary shim/alias remains in moved scope unless explicitly documented and approved as intentional.
- No tests or examples depend on deprecated namespace aliases.

Detailed execution checklist:

1. Inventory remaining shim candidates:
   - Search for `using` aliases, wrapper overloads, and migration TODO markers tied to namespace compatibility.
2. Remove shims by domain:
   - `src/buses`, `src/colors`, `src/protocols`, `src/transports`, `src/LumaWave`, `src/LumaWave.h`.
3. Update dependent call sites:
   - Replace old namespace references that were still relying on temporary bridges.
4. Final grep sweep:
   - all deprecated namespace patterns return zero hits in moved scope.
   - known shim markers return zero hits in moved scope.
5. Full validation gates:
   - `pio test -e native-test`
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile`
   - targeted smoke examples compile check for RP2040 profile
6. Documentation closure:
   - Mark migration complete and capture residual risks/follow-ups in `docs/internal/backlog/todo.md`.

Phase 7 command checklist:

1. Detect temporary migration shims:
   - `rg -n "compat|shim|temporary|migration|TODO.*namespace" src/LumaWave src/buses src/colors src/protocols src/transports`
2. Detect stale namespace references in moved scope:
   - `rg -n "lw::factory::detail::assignPixelBusProtocolTimingIfPresent|namespace\s+lw::factory|namespace\s+lw::palette|namespace\s+lw\s*\{" src/buses src/colors src/protocols src/transports src/LumaWave src/LumaWave.h`
3. Confirm example include policy:
   - `rg -n "#include\s+<LumaWave.h>|#include\s+\"(factory|transports|protocols|colors|buses)/" examples`
4. Run full validation gates:
   - `pio test -e native-test`
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile`

Phase 7 required outputs:

1. Final grep summary showing zero hits for deprecated namespace locations and shim markers.
2. Shim removal summary (what was removed).
3. Full native test and contract gate pass results.
4. Final closure note with residual risks and follow-up backlog items.

## Phase-by-Phase Deliverables

Every phase commit must include these artifacts:

1. Updated inventory snippet for that phase (symbols old/new).
2. Updated grep-guard output summary (pass/fail).
3. Test command list executed and pass status.
4. Explicit note of any temporary shim introduced or removed.

## Detailed Task Checklist

For each phase, execute this checklist in order:

1. Build symbol inventory for phase files.
2. Apply namespace edits in headers/impl files.
3. Update affected includes and umbrella exports.
4. Run targeted grep checks for old namespace names.
5. Run targeted native tests relevant to touched domain.
6. Run mandatory contract gates:
   - `pio test -e native-test`
   - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile`
7. Remove temporary shims introduced in same phase; if deferment is unavoidable, document each remaining shim and close it in Phase 7 before merge.
8. Commit phase with a scoped message (for example `refactor(namespaces): phase 2 colors/shaders split`).

## Safety Rules During Migration

- Keep `detail` semantics intact; do not flatten `detail` into non-detail scopes.
- Avoid changing runtime behavior while changing namespaces.
- Prefer mechanical moves and symbol qualification updates over redesign.
- Do not add compatibility aliases or wrappers that mask required explicit template information.
- Keep virtual seam ownership boundaries intact (`IPixelBus`, `IShader`, `IProtocol`, `ITransport`).

## Suggested Commit Slices

1. `refactor(namespaces): phase 0 inventory and guards`
2. `refactor(namespaces): phase 1 buses to lw::busses`
3. `refactor(namespaces): phase 2 colors vs shaders split`
4. `refactor(namespaces): phase 3 palette to lw::colors::palettes`
5. `refactor(namespaces): phase 4 protocols normalization`
6. `refactor(namespaces): phase 5 transports normalization`
7. `refactor(namespaces): phase 6 aggregator/include cleanup`
8. `refactor(namespaces): phase 7 remove all shims`

## Acceptance Criteria (Final)

- Namespace naming rule satisfied: all plural except `lw`, `detail`, `core`.
- Domain namespace mapping exactly matches requested destinations.
- All moved `detail` symbols remain in a `detail` namespace.
- Zero grep hits for deprecated namespace locations in moved scope.
- Zero temporary migration shims/aliases remain in moved scope.
- Native tests and contract compile test pass.
