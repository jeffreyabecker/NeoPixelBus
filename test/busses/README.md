# Bus Tests

Category folder for native bus-focused tests.

## Spec Coverage Matrix

Source spec: `docs/internal/testing-spec-bus.md`

| Spec Section | Domain | Test Folder | Status |
|---|---|---|---|
| 1.1.x / 1.2.x / 1.3.x / 1.4.x | Legacy bus suites | Removed | Removed during unified bus migration |

## Run

- Full native suite: `pio test -e native-test`
- Bus suites:
	- `pio test -e native-test --filter busses/test_static_bus_driver_pixel_bus`
	- `pio test -e native-test --filter busses/test_reference_bus`
