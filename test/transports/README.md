# Transport Tests

Category folder for native transport-focused tests.

## Spec Coverage Matrix

Source spec: `docs/internal/testing-spec-transports.md`

| Spec Section | Domain | Test Folder | Status |
|---|---|---|---|
| 1.1.1-1.1.10 | OneWireWrapper | `test/transports/test_onewirewrapper_spec` | Implemented, Passing |

## Run

- Full native suite: `pio test -e native-test`
- Transport suite: `pio test -e native-test --filter transports/test_onewirewrapper_spec`
