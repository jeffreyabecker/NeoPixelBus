# Bit Rate Calculation Guide

Status: draft

This guide explains how to calculate a required transport bit rate (`clockRateHz` / `bitRateHz`) for NeoPixelBus protocols.

## Core Formula

1. Compute total transmitted bytes per frame, including protocol framing.
2. Convert to bits per frame.
3. Multiply by target frame rate.

R_bit = B_frame * fps

Where:

- R_bit: required on-wire bit rate (bits/second)
- B_frame: bits transmitted per full frame
- fps: target frames per second

And:

B_frame = 8 * bytes_frame

## One-Wire Protocols (WS2812x/TM1814/TM1914)

For one-wire timing-based protocols, first determine the NRZ bit rate from timing:

R_nrz = 1e9 / (t0hNs + t0lNs)

Then apply encoded cadence:

- 3-step encoding: R_clk = 3 * R_nrz
- 4-step encoding: R_clk = 4 * R_nrz

`OneWireTiming::bitPattern()` selects 3-step or 4-step. If it selects 4-step, prefer 4-step throughput assumptions.

## Clocked Protocol Frame Sizes

Use these byte counts to compute required bit rate:

- DotStar: `4 + (4 * N) + 4 + ceil(N / 16)`
- HD108 RGB: `16 + (8 * N) + 4`
- HD108 RGBCW: `16 + (12 * N) + 4`
- LPD6803: `4 + (2 * N) + ceil(N / 8)`
- LPD8806: `ceil(N / 32) + (3 * N) + ceil(N / 32)`
- P9813: `4 + (4 * N) + 4`
- WS2801: `3 * N`
- SM16716: `3 * N`
- SM168x: depends on variant framing + payload bytes
- TLC5947 / TLC59711: protocol-defined packed payload size + latch/framing behavior

`N` = pixel count.

## Worked Examples

### 1) DotStar, 150 pixels, 60 FPS

Bytes/frame:

`4 + (4 * 150) + 4 + ceil(150/16) = 4 + 600 + 4 + 10 = 618`

Bits/frame:

`618 * 8 = 4944`

Required bit rate:

`4944 * 60 = 296640 bps`

Choose a practical configured rate above this (for example, 500 kHz or 1 MHz).

### 2) WS2812x-like timing with 4-step selected

If timing yields `R_nrz ~= 800 kHz`:

`R_clk = 4 * 800 kHz = 3.2 MHz`

Set transport clock at or above this value (subject to hardware limits and margin).

## RP2040 PIO SPI Notes

In `RpPioSpiTransport`, `clockRateHz` is the direct transport clock target used for PIO divider setup and DMA pacing.

Practical guidance:

- Start from computed minimum `R_bit`.
- Add margin (typically 10% to 30%).
- Keep within hardware/protocol limits.
- Validate end-to-end with target pixel count and FPS on hardware.

## Rule of Thumb

Set:

`clockRateHz >= 1.2 * (bytes_frame * 8 * fps)`

Then verify stable output on device and adjust upward/downward as needed for timing and EMI constraints.
