# Formal Test Specification (Index)

The formal test specification has been reorganized into focused documents.

## Documents

- `docs/testing-spec-bus.md`
  - Busses and topologies (migrated from sections 1 and 2).

- `docs/testing-spec-colors-shaders.md`
  - Color, ColorIterator, CurrentLimiterShader, AggregateShader.
  - Scope policy: `GammaShader` and `WhiteBalanceShader` are intentionally out-of-scope for deterministic native unit tests.

- `docs/testing-spec-transports.md`
  - Transport-layer tests (migrated from section 5).

- `docs/testing-spec-protocols.md`
  - Protocol tests (migrated from section 6).

## Notes

- Numbering is normalized within each focused document.
- This index replaces the prior monolithic hierarchical document to avoid duplication.

rockesvida
