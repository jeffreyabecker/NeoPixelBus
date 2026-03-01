# Compile Contracts

Compile-time contract suites enforce protocol/transport concept and compatibility rules via `static_assert`.

## Documentation

- [Protocol and Transport Contracts](../../docs/internal/protocol-transport-contracts.md)

## Suites

- `test_protocol_transport_contract_matrix_compile`

## Run

- Contract suite only:
  - `pio test -e native-test --filter contracts/test_protocol_transport_contract_matrix_compile`
- Full native suite:
  - `pio test -e native-test`
