The following chips were dropped because they do something really weird:
TM1829
MBI6033
TLC59711

consider using https://github.com/MikeLankamp/fpm for instead of floats

Lets consider writing some example animations using ColorIterators



https://www.superlightingled.com/blog/sk9822-vs-apa102-vs-apa107-vs-hd107-vs-hd107s-vs-hd108/



Examine if the src\transports\esp32\Esp32RmtTransport.h can be tuned further for direct one-wire protocol encoding.


for dotstar, adafruit has a white-only strip with links to the tutorials here:
https://www.adafruit.com/product/2433
https://learn.adafruit.com/adafruit-dotstar-leds
https://cdn-learn.adafruit.com/assets/assets/000/084/592/original/APA102_White_LED.pdf?1574117639

None of that code seems to cover the white-only cases

Thoughts: For dotstar it might be useful to use 16 bit colors and try to normalize-out the brightnes setting. The current implementation had two logic paths wich always produced the same bit pattern


Lets make sure all the protocol aliases are in place. First, find the default color order for the following tables.
Reference source: C:\ode\NpbNext\src





Lets make a note about working on support for white-only leds


good matrix of data sheets: https://www.ledyilighting.com/addressable-pixel-ic-datasheet-hub/




3-step vs 4 step cadence is entirely a decision at the protocol level -- theres a space / time / capability trade off. 
3step is a 1:6 expansion and requires a 2.4mhz bit rate
4step is 1:8 but simpler math without carry over and requires a 3.2mhz bit rate.

TODO: refactor the Esp32RmtTransport to externalize the onewire encoding.
TODO: add support plan/implementation backlog for items in docs\internal\neopixelbus-unsupported-chips.md (SM168x one-wire variants, TM1829 descriptor alias, and platform gaps).