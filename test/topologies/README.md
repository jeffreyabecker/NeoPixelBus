# Topology Tests

Category folder for native topology-focused tests.

## Spec Coverage Matrix

Source spec: `docs/testing-spec-bus.md`

| Spec Section | Domain | Test Folder | Status |
|---|---|---|---|
| 2.1.1 | PanelLayout | `test/topologies/test_topology_spec_section2` | Implemented, Passing |
| 2.2.1 | tilePreferredLayout | `test/topologies/test_topology_spec_section2` | Implemented, Passing |
| 2.3.1-2.3.4 | Topology | `test/topologies/test_topology_spec_section2` | Implemented, Passing |

## Run

- Full native suite: `pio test -e native-test`
- Topology suite: `pio test -e native-test --filter topologies/test_topology_spec_section2`
