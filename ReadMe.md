# NeoPixelBus

[![Donate](https://img.shields.io/badge/paypal-donate-yellow.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=6AA97KE54UJR4)

PlatformIO RP2040 NeoPixel library

A library for RP2040 boards (Raspberry Pi Pico family) to control NeoPixel-style LED strips using the Arduino framework in PlatformIO.

For quick questions and support:  
* [Try the new Github Discussions](https://github.com/Makuna/NeoPixelBus/discussions)  
* [Discord NeoPixelBus Invitation](https://discord.gg/c6FrysvZyV) or if you are already a member of [Discord Server NeoPixelBus](https://discord.com/channels/789177382221119519/789177382221119521)  
* Or jump on Gitter   
[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/Makuna/NeoPixelBus?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)  

For bugs, make sure there isn't an active issue and then create one.

## Documentation
[See Wiki](https://github.com/Makuna/NeoPixelBus/wiki)

## Installing This Library (PlatformIO)
Add this library to your `platformio.ini` for an RP2040 environment.

Example:

```ini
[env:pico2w]
platform = raspberrypi
board = pico2_w
framework = arduino
lib_deps =
	Makuna/NeoPixelBus@^2.8.4
```

## Installing This Library From GitHub (advanced, you want to contribute)
Create a directory in your Arduino\Library folder named "NeoPixelBus"
Clone (Git) this project into that folder.  
It should now show up in the import list when you restart Arduino IDE.





