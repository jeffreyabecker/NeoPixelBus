The following chips were dropped because they do something really weird:
TM1829
MBI6033
TLC59711

consider using https://github.com/MikeLankamp/fpm for instead of floats

Lets consider writing some example animations using ColorIterators

we need to add allocation helpers for concat and matrix bus examples

we need to rewrite the dynamic allocation examples


https://www.superlightingled.com/blog/sk9822-vs-apa102-vs-apa107-vs-hd107-vs-hd107s-vs-hd108/


check if src\transports\esp32\Esp32RmtOneWireTransport.h is could be using the one wire wrapper

platform based Spi transports need to support
invert, clockRateHz, bitOrder, dataMode, clockPin, dataPin as their parameters



Rp platforms are handling DMA incorrectly. the config needs to be redone in each transfer call not once at the start.
RP has two settings for DMA sizing:
channel_config_set_transfer_data_size -- logically the 'word size' of the dma tx: 8/16/32 bits
transfer_count (5th parameter to dma_channel_configure) -- the count of 'words' for the dma transfer.

Examine if the src\transports\esp32\Esp32RmtOneWireTransport.h can be converted to use the OneWireWrapper.


We should update the print bus to take a raw vs encoded parameter


for dotstar, adafruit has a white-only strip with links to the tutorials here:
https://www.adafruit.com/product/2433
https://learn.adafruit.com/adafruit-dotstar-leds
https://cdn-learn.adafruit.com/assets/assets/000/084/592/original/APA102_White_LED.pdf?1574117639

None of that code seems to cover the white-only cases

Thoughts: For dotstar it might be useful to use 16 bit colors and try to normalize-out the brightnes setting. The current implementation had two logic paths wich always produced the same bit pattern


alias matrix:
The following are aliases for Ws2812 but may have different default settings
| Alias      | Compatible Color Types |
|------------|-----------------------|
| Ws2812     | RGB                   |
| Ws2812x    | RGB                   |
| Ws2813     | RGB                   |
| Ws2816     | RGB                   |
| Ws2811     | RGB                   |
| Ws2805     | RGB                   |
| Ws2814     | RGBW                  |
| Lc8812     | RGBW                  |
| Sk6812     | RGBW                  |
| Apa106     | RGB                   |
| Intertek   | RGB                   |



the following are aliases for Dotstar but may have different default settigns
| LED IC Chip | Gray Scale (Bit Depth) | LED Color      |
|-------------|-----------------------|-----------------|
| SK9822      | uint8_t               | RGB             |
| APA102      | uint8_t               | RGB             |
| APA107      | uint8_t               | RGB             |
| HD107       | uint8_t               | RGB             |
| HD107s      | uint8_t               | RGB             |
| HD108       | uint16_t              | RGB, RGBW       |




Lets make a note about working on support for white-only leds


RP2040 PIO transport decision (accepted):
- Drop `RpPioOneWireTransport` and standardize on `RpPioTransport` (clock/data PIO program).
- Use `OneWireWrapper<RpPioTransport>` for one-wire protocols.
- Rationale: RP2040 has a limited PIO program budget (2 PIO blocks total), so keeping one shared program maximizes flexibility.
- Outcome: users can mix one-wire and two-wire buses more freely without competing program variants per PIO/state machine.