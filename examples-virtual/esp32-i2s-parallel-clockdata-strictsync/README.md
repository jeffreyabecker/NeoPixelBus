# ESP32 I2S Parallel ClockData Strict-Sync Example

This example demonstrates composing:

- `Esp32I2sParallelClockDataTransport`
- `EncodedClockDataSelfClockingTransport`
- `Ws2812xProtocol`

for two logical lanes sharing one physical I2S-parallel DMA backend.

## Wiring

Update the lane pins in `main.cpp`:

- `Lane0Pin`
- `Lane1Pin`

Connect each lane pin to a separate WS2812x-style strip data input.
Use common GND between MCU and strips.

## Strict-Sync Requirement

This transport uses a strict sync barrier per frame.

- Every active lane must submit one frame each epoch.
- In this sample, that means both lane buses must call `show()` every loop iteration.
- If one lane does not submit, DMA start is intentionally blocked.

## Notes

- Target: ESP32 with I2S parallel support (not ESP32-S3/C3 in this transport file guard).
- Encoding is 3-step (`110` for 1, `100` for 0) via `EncodedClockDataSelfClockingTransport`.
