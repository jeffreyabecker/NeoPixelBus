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



Rp platforms are handling DMA incorrectly. the config needs to be redone in each transfer call not once at the start.
RP has two settings for DMA sizing:
channel_config_set_transfer_data_size -- logically the 'word size' of the dma tx: 8/16/32 bits
transfer_count (5th parameter to dma_channel_configure) -- the count of 'words' for the dma transfer.
