# Internal Research Notes

This file is for reference material and durable technical observations only.
Actionable work items are tracked in [docs/internal/todo.md](todo.md).

## Reference Links

- DotStar family comparison matrix:
	- https://www.superlightingled.com/blog/sk9822-vs-apa102-vs-apa107-vs-hd107-vs-hd107s-vs-hd108/
- DotStar white-only product/tutorial/datasheet references:
	- https://www.adafruit.com/product/2433
	- https://learn.adafruit.com/adafruit-dotstar-leds
	- https://cdn-learn.adafruit.com/assets/assets/000/084/592/original/APA102_White_LED.pdf?1574117639
- LED datasheet hub:
	- https://www.ledyilighting.com/addressable-pixel-ic-datasheet-hub/
- Candidate fixed-point library for evaluation:
	- https://github.com/MikeLankamp/fpm

## Protocol/Encoding Notes

- 3-step vs 4-step one-wire encoding cadence is a protocol-level trade-off.
- 3-step expands payload by about 1:6 and targets ~2.4 MHz encoded bit rate.
- 4-step expands payload by about 1:8, simplifies arithmetic/no carry-over path, and targets ~3.2 MHz encoded bit rate.

## Scope Notes

- Unsupported-chip triage and decisions are maintained in [docs/internal/neopixelbus-unsupported-chips.md](neopixelbus-unsupported-chips.md) and tracked as tasks in [docs/internal/todo.md](todo.md).