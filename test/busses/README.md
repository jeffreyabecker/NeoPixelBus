# Bus Tests

Category folder for native bus-focused tests.

## Spec Coverage Matrix

Source spec: `docs/testing-spec-bus.md`

| Spec Section | Domain | Test Folder | Status |
|---|---|---|---|
| 1.1.1-1.1.5 | PixelBus | `test/busses/test_bus_spec_section1` | Implemented, Passing |
| 1.2.1-1.2.5 | SegmentBus | `test/busses/test_bus_spec_section1` | Implemented, Passing |
| 1.3.1-1.3.5 | ConcatBus | `test/busses/test_bus_spec_section1` | Implemented, Passing |
| 1.4.1-1.4.5 | MosaicBus | `test/busses/test_bus_spec_section1` | Implemented, Passing |

## Run

- Full native suite: `pio test -e native-test`
- Bus suites:
	- `pio test -e native-test --filter busses/test_bus_pixelbus_smoke`
	- `pio test -e native-test --filter busses/test_bus_spec_section1`
