# MagicOptimizer for Unreal Engine 5.6

Build once, ship smart. MagicOptimizer audits your project, recommends safe improvements, and helps you apply and verify changes — all inside UE.

— UE 5.6 — Working Plugin Shell — MIT —

## Why MagicOptimizer

- Fast visibility: one-click Audit for textures (meshes/materials next)
- Actionable recommendations per target profile
- Safe-by-default: CSV-first, dry runs, caps, and verification
- UE-native UX: C++ Slate UI + Python analysis under the hood

## Current status

- ✅ Compiles and loads in UE 5.6; panel opens
- ✅ Texture Audit CSV and in-UI table (sort/filter), per-row Copy/Open
- ✅ Include/Exclude path filters wired to Python
- ✅ Headless CI runner with screenshots and artifacts
- ✅ Self-learning knowledge logging (textures)
- ⏳ Recommend → Apply → Verify depth and more categories in progress

## Try it in 60 seconds

1) Copy this repo into your project's `Plugins/` folder
2) Enable MagicOptimizer in Plugins and restart the editor
3) Open: Window → MagicOptimizer (or run `MagicOptimizer.Open`)
4) Click “Run Audit” and inspect the Texture results

## Feature highlights (teaser)

- Four-phase pipeline: Audit → Recommend → Apply → Verify (CSV-first)
- Texture table: Path | Width | Height | Format with sort/filter and row actions
- Filters: include/exclude path CSVs; selection-aware runs
- Clean Python output + optional raw JSON debug view
- CI harness: headless or with RHI, before/after screenshots, artifact pruning/flush
- Self-learning: append-only knowledge events and aggregates (opt-in, textures now)

## Docs

- Developer Guide: `docs/DEV_README.md`
- Development Log (append-only): `docs/DEVELOPMENT_LOG.md`

## Roadmap (teaser)

- Complete Texture vertical (deeper Recommend/Apply/Verify)
- Extend to Meshes and Materials
- Knowledge summaries → evolving rule packs
- CI self-checks and markdown reports

—

Last updated: 2025-08-15
UE5.6 Compatibility: ✅ Verified
