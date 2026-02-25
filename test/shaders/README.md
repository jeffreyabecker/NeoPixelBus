# Shader Tests

Category folder for native shader-focused tests.

## Spec Coverage Matrix

Source spec: `docs/testing-spec-colors-shaders.md`

| Spec Section | Domain | Test Folder |
|---|---|---|
| 1 | Color Domain | `test/shaders/test_color_domain_section1` |
| 2 | ColorIterator Domain | `test/shaders/test_color_iterator_section2` |
| 3 | CurrentLimiterShader Domain | `test/shaders/test_current_limiter_shader_section3` |
| 4 | AggregateShader Domain | `test/shaders/test_aggregate_shader_section4` |

## Run

- Full native suite: `pio test -e native-test`
- Shader-only subset:
	- `pio test -e native-test --filter shaders/test_color_domain_section1`
	- `pio test -e native-test --filter shaders/test_color_iterator_section2`
	- `pio test -e native-test --filter shaders/test_current_limiter_shader_section3`
	- `pio test -e native-test --filter shaders/test_aggregate_shader_section4`
