

TODO: the esp32 and esp8266 transports are is incorrectly implemented
    src\original\internal\methods\platform\esp32\NeoEsp32I2sMethod.h 
    src\original\internal\methods\platform\esp8266\NeoEsp8266I2sMethodCore.h
    src\original\internal\methods\platform\esp32\NeoEsp32RmtMethod.h
    for the entry point into the correct methods


TODO: Add a white-balance shader cribed from WLED

TODO: Add RpPioSpiClass using :https://github.com/earlephilhower/arduino-pico/blob/master/libraries/SoftwareSPI/src/SoftwareSPI.h
TODO: add a RpPioUart class using https://github.com/earlephilhower/arduino-pico/blob/master/cores/rp2040/SerialUART.h


TODO: Add a generic ConfigureSpi Method for which abstracts out pin configuration across platforms because they all do it differently




UCS8904 -- https://github.com/Makuna/NeoPixelBus/issues/516