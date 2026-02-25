# Test Index

Top-level index for native test categories and spec-driven suites.

## Spec Source Documents

- [Bus + Topologies Spec](../docs/testing-spec-bus.md)
- [Color + Shader Spec](../docs/testing-spec-colors-shaders.md)
- [Transport Spec](../docs/testing-spec-transports.md)
- [Protocol Spec](../docs/testing-spec-protocols.md)

## Category READMEs

- [Bus Tests](busses/README.md)
- [Shader Tests](shaders/README.md)
- [Transport Tests](transports/README.md)
- [Protocol Tests](protocols/README.md)
- [Topology Tests](topologies/README.md)

## Spec-Driven Suites

- [Bus Spec Section 1](busses/test_bus_spec_section1)
- [Topology Spec Section 2](topologies/test_topology_spec_section2)
- [Color/Shader Spec Section 1](shaders/test_color_domain_section1)
- [Color/Shader Spec Section 2](shaders/test_color_iterator_section2)
- [Color/Shader Spec Section 3](shaders/test_current_limiter_shader_section3)
- [Color/Shader Spec Section 4](shaders/test_aggregate_shader_section4)
- [Transport Spec OneWireWrapper](transports/test_onewirewrapper_spec)
- [Protocol Spec Sections 1.1-1.4 + 1.14](protocols/test_protocol_spec_sections_1_1_to_1_4_and_1_14)
- [Protocol Spec Sections 1.5-1.13](protocols/test_protocol_spec_sections_1_5_to_1_13)

## Quick Run

- Full native suite: `pio test -e native-test`
- Bus + topology suites:
  - `pio test -e native-test --filter busses/test_bus_spec_section1`
  - `pio test -e native-test --filter topologies/test_topology_spec_section2`
- Shader suites:
  - `pio test -e native-test --filter shaders/test_color_domain_section1`
  - `pio test -e native-test --filter shaders/test_color_iterator_section2`
  - `pio test -e native-test --filter shaders/test_current_limiter_shader_section3`
  - `pio test -e native-test --filter shaders/test_aggregate_shader_section4`
- Transport suite:
  - `pio test -e native-test --filter transports/test_onewirewrapper_spec`
- Protocol suites:
  - `pio test -e native-test --filter protocols/test_protocol_spec_sections_1_1_to_1_4_and_1_14`
  - `pio test -e native-test --filter protocols/test_protocol_spec_sections_1_5_to_1_13`
