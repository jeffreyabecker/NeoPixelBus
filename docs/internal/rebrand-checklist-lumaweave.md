# LumaWeave Rebrand Checklist (Docs-First)

Status: draft
Scope: documentation and package metadata prep before source/API renames

## 1) Branding Baseline

- [ ] Confirm canonical product name: **LumaWeave**
- [ ] Confirm subtitle/tagline (optional)
- [ ] Confirm transition phrase to preserve discoverability (recommended: "formerly NeoPixelBus")
- [ ] Decide deprecation window for old naming in docs

## 2) Public Metadata Files

- [ ] Update library name/description in `library.properties`
- [ ] Update package name/description in `library.json`
- [ ] Update top-level project title and intro text in `ReadMe.md`
- [ ] Keep one explicit migration line: "LumaWeave (formerly NeoPixelBus)"

## 3) Docs Content Sweep

- [ ] Update product naming across `docs/internal/**` where user-facing
- [ ] Update doc titles that include NeoPixelBus naming
- [ ] Update architecture overview references (`docs/internal/consumer-virtual-architecture.md`)
- [ ] Update protocol/transport contracts references (`docs/internal/protocol-transport-contracts.md`)
- [ ] Update factory design/architecture docs to use LumaWeave naming
- [ ] Update testing plans/specs that mention old library name

## 4) Examples + README References

- [ ] Update `examples/platform-debug/README.md` naming
- [ ] Update any example markdown headers mentioning NeoPixelBus
- [ ] Keep include guidance explicit if header names remain temporarily unchanged

## 5) Include/Import Guidance (Docs Only for This Pass)

- [ ] Document current include compatibility strategy in docs:
  - Preferred include target (future): LumaWeave header naming
  - Temporary compatibility include (current): existing NeoPixelBus headers
- [ ] Add a short migration snippet for users

## 6) Links, Badges, and Registry Mentions

- [ ] Update badges and shields text that embed old name
- [ ] Update any hardcoded URLs/paths that include old repo/library naming
- [ ] Verify README links still resolve after wording updates

## 7) Transition/Migration Notes

- [ ] Add a short "Branding Transition" section to README
- [ ] Add migration notes in docs index or release notes doc
- [ ] Define wording policy for old term usage (only compatibility context)

## 8) Verification Pass

Run these searches and review every hit:

```powershell
git grep -n -I "NeoPixelBus\|VirtualNeoPixelBus\|NeoPixel" -- ReadMe.md docs examples
```

```powershell
git grep -n -I "LumaWeave" -- ReadMe.md docs examples library.properties library.json
```

- [ ] Confirm old-name hits are intentional (compatibility/discoverability only)
- [ ] Confirm new-name coverage exists in README + metadata + key docs
- [ ] Prepare docs-only commit message (example: `docs: introduce LumaWeave branding and transition notes`)

## 9) Out of Scope for This Checklist

- Source/header file renames
- Namespace/API symbol renames (including `npb` -> `lw`)
- Binary/package publication changes
- CI/CD or release automation updates

These are tracked in the next phase after docs are aligned.

## 10) Next-Phase Code Rebrand (Tracked, Not Started)

- [ ] Rename namespace declarations from `npb` to `lw`
- [ ] Update fully qualified references from `npb::` to `lw::`
- [ ] Decide compatibility policy (`namespace npb = lw;` alias bridge vs hard break) -- hard break
- [ ] Update examples/tests for `lw` namespace usage
- [ ] Run native contract + bus/spec test gates after namespace migration
