# Namespace Move Full Symbol Inventory

Status: planning artifact for refactor execution.

## Scope

This inventory covers all declarations in the current move scope:

- `src/buses/MakePixelBus.h`
- `src/protocols/ProtocolAliases.h`
- `src/lights/ILightDriver.h`
- `src/lights/NilLightDriver.h`
- `src/lights/PrintLightDriver.h`
- `src/lights/AnalogPwmLightDriver.h`
- `src/lights/esp32/Esp32LedcLightDriver.h`
- `src/lights/esp32/Esp32SigmaDeltaLightDriver.h`
- `src/lights/rp2040/RpPwmLightDriver.h`
- `src/lights/Lights.h`
- `src/LumaWave/Lights.h`

## Namespace/File Move Rules

- Helper-only factory/protocol traits move into `...::detail`.
- Protocol alias family stays in `lw::protocols` for this phase.
- Light-driver abstractions move from `lw` in `src/lights/*` into transport namespaces/files.
- No `lw::transports::light` sub-namespace in this phase.

## File-by-File Inventory

### `src/buses/MakePixelBus.h`
Current enclosing namespace: `lw::factory`
Target file: unchanged (`src/buses/MakePixelBus.h`)

Types (move to `lw::factory::detail`):
- `PixelBusProtocolSettingsHasTiming<TProtocolSettings, ...>`
- `DirectMakeBusCompatible<TProtocol, TTransport, TProtocolConfig, TTransportConfig, ...>`
- `DirectMakeBusShaderCompatible<TShader, TColor, ...>`
- `IsWs2812xProtocolAlias<TProtocolAlias, ...>`

Functions (move to `lw::factory::detail`):
- `assignPixelBusProtocolTimingIfPresent(TProtocolSettings&, OneWireTiming)`

Functions (remain in `lw::factory`):
- `makePixelBus(PixelCount, TProtocolConfig&&, TTransportConfig&&)`
- `makePixelBus(PixelCount, TTransportConfig&&)`
- `makePixelBus(PixelCount, TProtocolConfig&&, OneWireTiming, TTransportConfig&&)`
- `makePixelBus(PixelCount, OneWireTiming, TTransportConfig&&)`
- `makePixelBus(PixelCount, TProtocolConfig&&, TTransportConfig&&, TShader&&)`
- `makePixelBus(PixelCount, TTransportConfig&&)` for `TWsAlias`
- `makePixelBus(PixelCount, TProtocolSettings&&, TTransportConfig&&)` for `TWsAlias`
- `makePixelBus(PixelCount, OneWireTiming, TTransportConfig&&)` for `TWsAlias`
- `makePixelBus(PixelCount, TProtocolSettings&&, OneWireTiming, TTransportConfig&&)` for `TWsAlias`

Enums: none
Using aliases: none

### `src/protocols/ProtocolAliases.h`
Current enclosing namespace: `lw::protocols`
Target file: unchanged (`src/protocols/ProtocolAliases.h`)

Types (move to `lw::protocols::detail`):
- `ResolveProtocolType<TProtocolCandidate, ...>`
- `Debug<TWrappedProtocolSpec>::WrappedSpecHasNormalizeSettings<TWrappedSpec, ...>` (currently nested private)

Types (remain in `lw::protocols`):
- `DotStar<TInterfaceColor, TDefaultChannelOrder, TStripColor>`
- `Hd108<TInterfaceColor, TDefaultChannelOrder, TStripColor>`
- `None<TColor>`
- `Debug<TWrappedProtocolSpec>`
- `Tm1814<TInterfaceColor>`
- `Tm1914<TInterfaceColor>`
- `Ws2812x<TInterfaceColor, TDefaultChannelOrder, TDefaultTiming, TStripColor, TIdleHigh>`

Using aliases (remain in `lw::protocols`):
- `DotStarType<...>`
- `APA102`
- `APA102Type`
- `Hd108Type<...>`
- `HD108`
- `HD108Type`
- `NoneType<TColor>`
- `DebugType<TWrappedProtocolSpec>`
- `Tm1814Type<TInterfaceColor>`
- `Tm1914Type<TInterfaceColor>`
- `Ws2812xType<...>`
- `Ws2812<TInterfaceColor>`
- `Ws2812Type<TInterfaceColor>`
- `Ws2811<TInterfaceColor>`
- `Ws2811Type<TInterfaceColor>`
- `Ws2805<TInterfaceColor>`
- `Ws2805Type<TInterfaceColor>`
- `Sk6812<TInterfaceColor>`
- `Sk6812Type<TInterfaceColor>`
- `Tm1829<TInterfaceColor>`
- `Tm1829Type<TInterfaceColor>`
- `Ws2814<TInterfaceColor>`
- `Ws2814Type<TInterfaceColor>`

Member functions (remain with owning types unless helper extraction is applied):
- `DotStar::defaultSettings()`
- `DotStar::normalizeSettings(SettingsType)`
- `Hd108::defaultSettings()`
- `Hd108::normalizeSettings(SettingsType)`
- `None::defaultSettings()`
- `None::normalizeSettings(SettingsType)`
- `Debug::defaultSettings()`
- `Debug::normalizeSettings(SettingsType)`
- `Debug::normalizeWrappedSettings(WrappedSettingsType)`
- `Tm1814::defaultSettings()`
- `Tm1814::normalizeSettings(SettingsType)`
- `Tm1914::defaultSettings()`
- `Tm1914::normalizeSettings(SettingsType)`
- `Ws2812x::defaultTiming()`
- `Ws2812x::defaultSettings()`
- `Ws2812x::normalizeSettings(SettingsType)`

Enums: none

### `src/lights/ILightDriver.h`
Current enclosing namespace: `lw`
Target file: `src/transports/ILightDriver.h`

Types (target `lw::transports`):
- `ILightDriver<TColor>`
- `LightDriverSettingsBase`
- `LightDriverLikeImpl<TDriver, ...>`

Variable templates (target `lw::transports`):
- `LightDriverLike<TDriver>`
- `SettingsConstructibleLightDriverLike<TDriver>`

Member functions (target `lw::transports`):
- `ILightDriver::~ILightDriver()`
- `ILightDriver::begin()`
- `ILightDriver::isReadyToUpdate() const`
- `ILightDriver::write(const ColorType&)`

Enums: none
Using aliases: none

### `src/lights/NilLightDriver.h`
Current enclosing namespace: `lw`
Target file: `src/transports/NilLightDriver.h`

Types (target `lw::transports`):
- `NilLightDriverSettings`
- `NilLightDriver<TColor>`

Member functions (target `lw::transports`):
- `NilLightDriverSettings::normalize(NilLightDriverSettings)`
- `NilLightDriver::NilLightDriver(LightDriverSettingsType = {})`
- `NilLightDriver::begin()`
- `NilLightDriver::isReadyToUpdate() const`
- `NilLightDriver::write(const ColorType&)`

Enums: none
Using aliases: none

### `src/lights/PrintLightDriver.h`
Current enclosing namespace: `lw`
Target file: `src/transports/PrintLightDriver.h`

Types (target `lw::transports`):
- `PrintLightDriverSettingsT<TWritable, ...>`
- `PrintLightDriverT<TColor, TWritable, ...>`

Using aliases (target `lw::transports`):
- `PrintLightDriverSettings` (Arduino-only)
- `PrintLightDriver<TColor>` (Arduino-only)

Member functions (target `lw::transports`):
- `PrintLightDriverSettingsT::normalize(PrintLightDriverSettingsT<TWritable>)`
- `PrintLightDriverT::PrintLightDriverT(LightDriverSettingsType)`
- `PrintLightDriverT::PrintLightDriverT(TWritable&)`
- `PrintLightDriverT::begin()`
- `PrintLightDriverT::isReadyToUpdate() const`
- `PrintLightDriverT::write(const ColorType&)`
- `PrintLightDriverT::writeColorBinary(const ColorType&)`
- `PrintLightDriverT::writeColorAscii(const ColorType&)`
- `PrintLightDriverT::writeBytes(const uint8_t*, size_t)`
- `PrintLightDriverT::writeText(const char*)`
- `PrintLightDriverT::writeDebugPrefix()`
- `PrintLightDriverT::writeLine(const char*)`
- `PrintLightDriverT::captureIdentifier()`

Enums: none

### `src/lights/AnalogPwmLightDriver.h`
Current enclosing namespace: `lw` (file gated by `ARDUINO_ARCH_ESP8266`)
Target file: `src/transports/esp8266/AnalogPwmLightDriver.h`

Types (target `lw::transports::esp8266`):
- `AnalogPwmLightDriverSettings`
- `AnalogPwmLightDriver<TColor>`

Type aliases and constants inside settings (target `lw::transports::esp8266`):
- `AnalogPwmLightDriverSettings::PinsMap`
- `AnalogPwmLightDriverSettings::MaxChannels`
- `AnalogPwmLightDriverSettings::DefaultPwmRange`
- `AnalogPwmLightDriverSettings::DefaultPwmFrequencyHz`

Member functions (target `lw::transports::esp8266`):
- `AnalogPwmLightDriverSettings::normalize(AnalogPwmLightDriverSettings)`
- `AnalogPwmLightDriver::AnalogPwmLightDriver(LightDriverSettingsType)`
- `AnalogPwmLightDriver::~AnalogPwmLightDriver()`
- `AnalogPwmLightDriver::begin()`
- `AnalogPwmLightDriver::isReadyToUpdate() const`
- `AnalogPwmLightDriver::write(const ColorType&)`

Enums: none

### `src/lights/esp32/Esp32LedcLightDriver.h`
Current enclosing namespace: `lw` (file gated by ESP32/ESP8266)
Target file(s):
- `src/transports/esp32/Esp32LedcLightDriver.h`
- `src/transports/esp8266/Esp8266LedcLightDriver.h` (alias split target)

Types (target `lw::transports::esp32`):
- `Esp32LedcLightDriverSettings`
- `Esp32LedcLightDriver<TColor>`

Using aliases (target `lw::transports::esp8266`):
- `Esp8266LedcLightDriverSettings`
- `Esp8266LedcLightDriver<TColor>`

Type aliases and constants inside settings (target `lw::transports::esp32`):
- `Esp32LedcLightDriverSettings::PinsMap`
- `Esp32LedcLightDriverSettings::MaxChannels`
- `Esp32LedcLightDriverSettings::DefaultFrequencyHz`
- `Esp32LedcLightDriverSettings::DefaultResolutionBits`

Member functions (target `lw::transports::esp32`):
- `Esp32LedcLightDriverSettings::normalize(Esp32LedcLightDriverSettings)`
- `Esp32LedcLightDriver::Esp32LedcLightDriver(LightDriverSettingsType)`
- `Esp32LedcLightDriver::~Esp32LedcLightDriver()`
- `Esp32LedcLightDriver::begin()`
- `Esp32LedcLightDriver::isReadyToUpdate() const`
- `Esp32LedcLightDriver::write(const ColorType&)`
- `Esp32LedcLightDriver::computeMaxDuty(uint8_t)`
- `Esp32LedcLightDriver::activeChannelCount() const`

Enums: none

### `src/lights/esp32/Esp32SigmaDeltaLightDriver.h`
Current enclosing namespace: `lw` (file gated by `ARDUINO_ARCH_ESP32`)
Target file: `src/transports/esp32/Esp32SigmaDeltaLightDriver.h`

Types (target `lw::transports::esp32`):
- `Esp32SigmaDeltaLightDriverSettings`
- `Esp32SigmaDeltaLightDriver<TColor>`

Type aliases and constants inside settings/class (target `lw::transports::esp32`):
- `Esp32SigmaDeltaLightDriverSettings::PinsMap`
- `Esp32SigmaDeltaLightDriverSettings::MaxChannels`
- `Esp32SigmaDeltaLightDriver::HardwareChannelCount`

Member functions (target `lw::transports::esp32`):
- `Esp32SigmaDeltaLightDriverSettings::normalize(Esp32SigmaDeltaLightDriverSettings)`
- `Esp32SigmaDeltaLightDriver::Esp32SigmaDeltaLightDriver(LightDriverSettingsType)`
- `Esp32SigmaDeltaLightDriver::~Esp32SigmaDeltaLightDriver()`
- `Esp32SigmaDeltaLightDriver::begin()`
- `Esp32SigmaDeltaLightDriver::isReadyToUpdate() const`
- `Esp32SigmaDeltaLightDriver::write(const ColorType&)`
- `Esp32SigmaDeltaLightDriver::activeChannelCount() const`
- `Esp32SigmaDeltaLightDriver::mapComponentToDensity(TComponent) const`

Enums: none

### `src/lights/rp2040/RpPwmLightDriver.h`
Current enclosing namespace: `lw` (file gated by `ARDUINO_ARCH_RP2040`)
Target file: `src/transports/rp2040/RpPwmLightDriver.h`

Types (target `lw::transports::rp2040`):
- `RpPwmLightDriverSettings`
- `RpPwmLightDriver<TColor>`

Type aliases and constants inside settings (target `lw::transports::rp2040`):
- `RpPwmLightDriverSettings::PinsMap`
- `RpPwmLightDriverSettings::MaxChannels`

Member functions (target `lw::transports::rp2040`):
- `RpPwmLightDriverSettings::normalize(RpPwmLightDriverSettings)`
- `RpPwmLightDriver::RpPwmLightDriver(LightDriverSettingsType)`
- `RpPwmLightDriver::~RpPwmLightDriver()`
- `RpPwmLightDriver::begin()`
- `RpPwmLightDriver::isReadyToUpdate() const`
- `RpPwmLightDriver::write(const ColorType&)`

Enums: none

### `src/lights/Lights.h`
Current content: umbrella includes only
Target action: delete file

Declarations in this file: none

### `src/LumaWave/Lights.h`
Current content: umbrella include only
Target action: remove file or convert to temporary compatibility include per migration policy

Declarations in this file: none

## Non-Symbol Move Actions

- Update include paths from `lights/...` to `transports/...` (notably `src/buses/LightBus.h` and any aggregators).
- Remove `#include "LumaWave/Lights.h"` from `src/LumaWave/All.h`.
- Ensure transport umbrella exports the moved light-driver headers.

## Validation Checklist For This Inventory

- Verify every declaration above is either moved or explicitly kept in place.
- Verify no residual include path references to `lights/` remain.
- Verify no residual references to `LumaWave/Lights.h` remain unless a temporary compatibility shim is intentionally used.
