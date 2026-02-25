# Virtual-Only Migration Notes

Status: active transition  
Date: 2026-02-25

## Canonical Include

Use the single umbrella header:

- `#include <NeoPixelBus.h>`

`VirtualNeoPixelBus.h` remains available temporarily for compatibility, but new code should include `NeoPixelBus.h`.

## Include Path Replacements

- `#include <VirtualNeoPixelBus.h>` -> `#include <NeoPixelBus.h>`
- `#include "VirtualNeoPixelBus.h"` -> `#include "NeoPixelBus.h"`

## Legacy Removal Status

Completed:
- `src/original/**` removed.
- Build filters no longer reference `src/original/**`.
- Internal tests/examples migrated to `NeoPixelBus.h`.

Pending:
- Optional final retirement of temporary forwarders under `src/virtual/**` after one transition window.

## Validation Gates

Run at minimum after include migrations:

- `pio test -e native-test --filter contracts/test_protocol_transport_contract_matrix_compile`
- `pio test -e native-test --filter shaders/test_color_domain_section1`

## Latest Validation Snapshot

Validated on 2026-02-25:

- `pio test -e native-test` -> PASS (`167/167`)
- `pio run -e pico2w-virtual` -> PASS
- `pio run -e esp8266-smoke` -> PASS
- `pio run -e esp32-smoke` -> PASS

Notes:

- ESP smoke source now validates core color-domain compatibility via `examples/platformio-smoke/src/main_virtual_smoke.cpp` including `colors/Color.h`.
- Full umbrella (`NeoPixelBus.h`) C++17 conversion remains in progress under the C++17 migration workstream.
