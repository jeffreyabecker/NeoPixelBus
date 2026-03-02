# Compile Contracts

Compile-time contract suites enforce protocol/transport concept and compatibility rules via `static_assert`.

## Documentation

- [Object Model Contracts](../../docs/internal/object-model-contracts.md)

## Suites

- `test_dynamic_bus_builder_first_pass_compile`
- `test_dynamic_bus_builder_ini_reader_compile`
- `test_dynamic_factory_config_parser_and_build`
- `test_factory_descriptor_first_pass_compile`
- `test_ini_reader_first_pass_compile`
- `test_protocol_aliases_first_pass_compile`

## Run

- Example targeted suite:
  - `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile`
- Full native suite:
  - `pio test -e native-test`
