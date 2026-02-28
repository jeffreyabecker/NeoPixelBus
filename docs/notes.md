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





Lets make a note about working on support for white-only leds


good matrix of data sheets: https://www.ledyilighting.com/addressable-pixel-ic-datasheet-hub/



