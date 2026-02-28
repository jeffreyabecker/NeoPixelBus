The following chips were dropped because they do something really weird:
TM1829
MBI6033
TLC59711

consider using https://github.com/MikeLankamp/fpm for instead of floats

Lets consider writing some example animations using ColorIterators



https://www.superlightingled.com/blog/sk9822-vs-apa102-vs-apa107-vs-hd107-vs-hd107s-vs-hd108/



Examine if the src\transports\esp32\Esp32RmtOneWireTransport.h can be converted to use the OneWireWrapper.





for dotstar, adafruit has a white-only strip with links to the tutorials here:
https://www.adafruit.com/product/2433
https://learn.adafruit.com/adafruit-dotstar-leds
https://cdn-learn.adafruit.com/assets/assets/000/084/592/original/APA102_White_LED.pdf?1574117639

None of that code seems to cover the white-only cases

Thoughts: For dotstar it might be useful to use 16 bit colors and try to normalize-out the brightnes setting. The current implementation had two logic paths wich always produced the same bit pattern


Lets make sure all the protocol aliases are in place. First, find the default color order for the following tables.
Reference source: C:\Users\jeffr\Documents\Arduino\libraries\NeoPixelBus_by_Makuna\src

alias matrix:
The following are aliases for Ws2812 but may have different default settings
| Alias      | Compatible Color Types | Default Color Order | Basis |
|------------|------------------------|---------------------|-------|
| Ws2812     | RGB                    | GRB                 | project policy aligned to Ws2812x defaults |
| Ws2812x    | RGB                    | GRB                 | source-backed (`Ws2812xOptions.channelOrder = GRB`) |
| Ws2813     | RGB                    | GRB                 | source-backed timing alias to Ws2812x family |
| Ws2816     | RGB                    | GRB                 | source-backed timing alias to Ws2812x family |
| Ws2811     | RGB                    | GRB                 | source-backed timing alias to Ws2812x family |
| Ws2805     | RGB                    | GRB                 | source-backed timing family; policy for color order |
| Ws2814     | RGBW                   | GRBW                | source-backed timing alias to Ws2805 family; RGBW policy |
| Lc8812     | RGBW                   | GRBW                | source-backed timing alias to Sk6812 family; RGBW policy |
| Sk6812     | RGBW                   | GRBW                | source-backed timing family; RGBW policy |
| Apa106     | RGB                    | GRB                 | source-backed timing family; project default policy |
| Intertek   | RGB                    | GRB                 | source-backed method speed exists; color order policy |



the following are aliases for Dotstar but may have different default settings
| LED IC Chip | Gray Scale (Bit Depth) | LED Color  | Default Color Order | Basis |
|-------------|--------------------------|------------|---------------------|-------|
| SK9822      | uint8_t                  | RGB        | BGR                 | project DotStar policy + legacy examples using `DotStarBgrFeature` |
| APA102      | uint8_t                  | RGB        | BGR                 | source-backed via descriptor default (`ChannelOrderBGR`) |
| APA107      | uint8_t                  | RGB        | BGR                 | policy (not explicitly present in reference source) |
| HD107       | uint8_t                  | RGB        | BGR                 | policy (not explicitly present in reference source) |
| HD107s      | uint8_t                  | RGB        | BGR                 | policy (not explicitly present in reference source) |
| HD108       | uint16_t                 | RGB, RGBW  | RGB / RGBW          | source-backed examples use `Hd108RgbFeature`; RGBW extension policy |


UCS references from NeoPixelBus source:

| Alias              | Color Depth | LED Color | Default Color Order | Source Reference |
|--------------------|-------------|-----------|---------------------|------------------|
| Ucs8903            | uint16_t x3 | RGB       | RGB                 | `internal/NeoColorFeatures.h`: `typedef NeoRgb48Feature NeoRgbUcs8903Feature;` |
| Ucs8904            | uint16_t x4 | RGBW      | RGBW                | `internal/NeoColorFeatures.h`: `typedef NeoRgbw64Feature NeoRgbwUcs8904Feature;` |




Lets make a note about working on support for white-only leds


good matrix of data sheets: https://www.ledyilighting.com/addressable-pixel-ic-datasheet-hub/
