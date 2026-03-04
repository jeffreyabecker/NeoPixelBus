# LumaWave

[![Donate](https://img.shields.io/badge/paypal-donate-yellow.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=6AA97KE54UJR4)

PlatformIO RP2040 addressable LED library

A library for RP2040 boards (Raspberry Pi Pico family) to control NeoPixel-style LED strips using the Arduino framework in PlatformIO.

For quick questions and support:  
* [GitHub Discussions](https://github.com/Makuna/NpbNext/discussions)  
* [Discord Invitation](https://discord.gg/c6FrysvZyV) or if you are already a member of [Discord Server](https://discord.com/channels/789177382221119519/789177382221119521)  

For bugs, make sure there isn't an active issue and then create one.

## Documentation
[See Wiki](https://github.com/Makuna/NpbNext/wiki)

Local references in this repo:
* [Documentation Index](docs/README.md)

## Installing This Library (PlatformIO)
Add this library to your `platformio.ini` for an RP2040 environment.

Example:

```ini
[env:pico2w]
platform = raspberrypi
board = pico2_w
framework = arduino
lib_deps =
	Makuna/NpbNext@^2.8.4
```

## Installing This Library From GitHub (advanced, you want to contribute)
Create a directory in your Arduino\Library folder named "LumaWave"
Clone (Git) this project into that folder.  
It should now show up in the import list when you restart Arduino IDE.

## Nil Transport (dry-run / host-safe)

`NilTransport` is available from explicit module includes (for example `#include <LumaWave/Transports.h>`) for no-op transport flows (for example dry-run rendering or protocol-only verification).

```cpp
#include <LumaWave/Factory.h>
#include <LumaWave/Protocols.h>
#include <LumaWave/Transports.h>

auto bus = makeBus<APA102, Nil>(32);
bus.begin();
bus.show();
```





