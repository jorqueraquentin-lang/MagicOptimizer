# MagicOptimizer Developer Guide (UE5.6)

This is the developer-focused guide. For the product-facing overview, see the root `README.md`.

- Audience: engineers and technical artists working on the plugin
- Goals: clarity, consistency, and fast iteration
- Conventions: CSV-first data flow, baby steps, test-after-each-change, append-only dev log

---

## Project Overview (developer view)

A comprehensive UE5.6 plugin for automated asset optimization and editor tools, featuring a Python backend for analysis logic and C++ frontend for seamless Unreal Engine integration.

- Four-phase workflow: Audit → Recommend → Apply → Verify
- Safety-first: Dry Run, change caps, backups, verification
- UE-native UX: Slate UI in-editor; Python+C++ under the hood

### Why Audit First (context for developers)
- Great recommendations start with great visibility
- Audit is non-destructive and fast, so you can run it often
- Everything else in MagicOptimizer builds on it

---

## Architecture Overview

- C++ Frontend (Slate UI, UE integration)
  - `SOptimizerPanel` orchestrates phases, loads CSVs, renders lists
  - Services (future-proofing):
    - CSV IO: `Services/Csv/TextureCsvReader.{h,cpp}`
    - Editor Actions: `Services/Editor/ContentBrowserActions.{h,cpp}`
  - View Models:
    - `Public/ViewModels/TextureModels.h` (texture audit/rec rows)

- Python Backend (analysis + actions)
  - Entry point: `Content/Python/magic_optimizer/entry.py`
  - Phases: Audit → Recommend → Apply → Verify
  - CI harness: `tools/run_ci.ps1`, `Content/Python/magic_optimizer/ci_run.py`, `ci_shot.py`

- Data & Artifacts
  - CSV outputs: `Saved/MagicOptimizer/Audit/*.csv`
  - Knowledge (self-learning): `Saved/MagicOptimizer/Knowledge/*`
  - CI artifacts: `docs/ci/<timestamp>/`

---

## Feature Index (Why, How, Status, Long-Term)

Below, each feature lists: Why it exists, How we implement it (roughly), Current Status, and Long-Term Objective.

### Textures: Audit
- Why: Establish a trustworthy inventory (path, size, format) to drive all subsequent decisions.
- How: AssetRegistry + EditorAssetLibrary, robust dimension extraction; write `textures.csv` (path,width,height,format).
- Status: Implemented. CSV populated; UI table loads; sort/filter; row actions.
- Long-Term: Enrich with mips, sRGB, LODGroup, NeverStream, uses estimate, size-on-disk; handle virtual textures; profile-specific deltas.

### Textures: Recommend
- Why: Turn audit facts into actionable changes per target profile.
- How: Python rules over `textures.csv`; emit `textures_recommend.csv` (issues,recommendations).
- Status: Initial rules live (oversize, normal/mask compression mismatch, missing dims). UI section shows rows; actions Copy/Open.
- Long-Term: Rule packs per profile, confidence scoring, explain-why, quick-fixes, batch apply safety.

### Textures: Apply (safety-first)
- Why: Execute improvements reliably with guardrails.
- How: Python apply functions; dry-run; change caps; backups; per-asset verification; collections.
- Status: Skeleton planned (not implemented).
- Long-Term: One-click apply with diffs, revert snapshots, and verification metrics.

### Textures: Verify
- Why: Prove outcomes match targets (dimensions, formats, budgets).
- How: Re-audit changed assets; assert expectations; budget checks.
- Status: Planned.
- Long-Term: CI-grade verifications, thresholds per profile, failure diagnostics.

### Path Filters & Selection
- Why: Scope audits precisely.
- How: UI include/exclude fields → Python includes/excludes; optional selection scoping.
- Status: Implemented end-to-end.
- Long-Term: Saved presets, folder pickers, filter chips.

### UI: Texture Results Table
- Why: Human-friendly inspection of large datasets.
- How: Virtualized list with header columns (Path|Width|Height|Format), sort, filter bar (text/min W/min H), summary (Filtered N/M), per-row actions.
- Status: Implemented. Deprecation warnings only.
- Long-Term: Column chooser, copy/export, multi-select actions, column filters.

### UI: Python Output + Debug JSON
- Why: Clean UX with optional deep diagnostics.
- How: Friendly message and collapsible raw JSON view.
- Status: Implemented.
- Long-Term: Structured summaries, link to artifacts, error triage.

### In-Editor Console Commands
- Why: Fast manual and scripted testing.
- How: `MagicOptimizer.Open`, `MagicOptimizer.Run`.
- Status: Implemented.
- Long-Term: Phase-specific switches and parameterization.

### CSV-First Pipeline
- Why: Diffable, reproducible, easy to test.
- How: All phases read/write CSVs; UI consumes CSVs; CI captures CSVs.
- Status: Implemented for Textures.
- Long-Term: Shared schema library, strict quoting/escaping, large-data resilience.

### CI Auto-Testing Harness
- Why: Continuous validation without manual steps.
- How: `tools/run_ci.ps1` launches UE headless/with RHI, runs phases, opens tab, takes before/after screenshots, collects logs/CSVs, prunes artifacts.
- Status: Implemented. Artifacts saved under `docs/ci/`; flush script available.
- Long-Term: Assertions (headers/buttons/rows), markdown summaries with thumbnails, multi-profile matrix.

### Self-Learning Knowledge System
- Why: Accumulate cross-project insights (names, formats, recurring issues) to evolve recommendations.
- How: Python emits JSONL events + CSV aggregates into `Saved/MagicOptimizer/Knowledge/` each run.
- Status: Implemented for Textures (`events.jsonl`, `kb_textures.csv`, `kb_texture_recs.csv`).
- Long-Term: Anonymization toggles, summarization scripts, mesh/material coverage, rule auto-tuning.

### Logging & Backlogs
- Why: High observability for debugging and autonomous iteration.
- How: Append-only `docs/DEVELOPMENT_LOG.md`; runtime logs under `Saved/MagicOptimizer/`; CI logs copied to artifacts.
- Status: Implemented (append-only policy).
- Long-Term: Compact daily summaries, error highlight, links to failing artifacts.

### Repo Hygiene & History Safety
- Why: Keep repo lean, avoid 100MB+ blockers.
- How: Tight `.gitignore`, `.gitattributes`, CI artifact pruning, large-file history purge.
- Status: Implemented.
- Long-Term: Pre-commit checks and PR bots.

---

## Quality-of-Life (QoL) Rollup

- Auto-Testing Harness
  - Status: Working for Audit/Recommend; screenshots + artifacts captured; retention + flush in place.
  - Next: self-check assertions, markdown summaries with thumbnails.

- Self-Learning Knowledge Base
  - Status: Texture events and aggregates recorded per run.
  - Next: anonymization/opt-out toggles, summarizer to actionable insights.

- Clean Python Output with Debug Toggle
  - Status: Implemented.
  - Next: richer summaries, error surfacing.

- Console Commands
  - Status: Implemented (`Open`, `Run`).
  - Next: per-phase args, profile/filters.

- CSV-first + Robust Parsing
  - Status: Implemented for textures; UI hardened against partial rows.
  - Next: shared schema utilities; cross-category reuse.

- Architecture Services + ViewModels
  - Status: CSV IO + Editor actions services added; row models extracted.
  - Next: sub-widgets per section; table view-model for sort/filter state; apply/verify services.

- CI Artifacts Pruning
  - Status: Implemented (keep-N/max-age); manual flush script.
  - Next: per-branch retention policies.

- Actionability & External Workflow
  - Status: Planned UX tags (Auto / Semi-auto / External) with badges and Apply gating.
  - Next: implement tagging, disable Apply for External with tooltip and How-To links; export task list (CSV/MD).

---

## Developer Workflow

1) Make one focused change (UI/Python/Services)
2) Build (MSBuild)
3) Run CI for the relevant phase(s)
4) Inspect artifacts (CSV, screenshots, logs)
5) Append entry to `docs/DEVELOPMENT_LOG.md` with timestamp + results

Testing Protocol:
- Pre-Change Checklist (current state)
- Post-Change Assertions (headers/buttons/rows/columns) and log study
- Predictable artifacts for diffing; retention with pruning

Development Philosophy:
- ONE CHANGE AT A TIME
- TEST AFTER EVERY CHANGE
- SMALL INCREMENTS
- ROLLBACK READY
- DOCUMENT EVERYTHING

---

## Platform Presets (reference)

- PC: `PC_Ultra`, `PC_Balanced`, `PC_Low`
- Console: `Console_Optimized`, `Console_Legacy`
- Mobile/VR/AR: `Mobile_Low`, `Mobile_Ultra_Lite`, `VR`, `AR`
- Specialized: `Cinematic`, `UI_Crisp`, `Archviz_High_Fidelity`, `Custom`

---

## Installation & Setup (developer quick ref)

Prerequisites: UE 5.6, VS 2022 (C++), Python (UE), .NET 8 Desktop Runtime

- Clone to `Plugins/`
- Enable plugin, restart editor
- Access via Window → Magic Optimizer

Project Structure:
```
HostProject/Plugins/MagicOptimizer/
├── MagicOptimizer.uplugin
├── Source/MagicOptimizer/
│   ├── Public/
│   └── Private/
├── Content/Python/magic_optimizer/
│   ├── textures/
│   ├── meshes/
│   ├── materials/
│   └── presets/
└── Resources/
```

---

## Usage Guide (flows for dev tests)

- Open plugin, choose profile/categories/scope
- Run Audit → Review CSV/UI
- Run Recommend → Review issues/rec CSV/UI
- Apply (when implemented) → Verify

Safety Features:
- Dry Run, change caps, backups, revert, collections, verification (planned/partial)

Advanced Options:
- Conservative mode, apply-only keys, custom rules, batch processing (planned)

---

## Performance & Benchmarks (targets)

- Textures: 20–40% memory reduction
- Meshes: 30–50% triangle reduction
- Materials: 15–25% shader reduction
- Runtime: 10–20% perf improvement

Processing speed goals:
- Audit: 1000+ assets/min
- Recommend: real-time
- Apply: 500+ assets/min with safety
- Verify: comprehensive

---

## Known Issues & Limitations

- Core features WIP; textures vertical prioritized
- Focus: Windows; UE5.6 PythonScriptPlugin required
- Some Slate APIs deprecated (warnings only)

---

## Contributing (internal)

- Follow baby-steps workflow and testing protocol
- Keep services/viewmodels separated; avoid monolith widgets
- Update `docs/DEVELOPMENT_LOG.md` after changes

License: MIT (see `LICENSE`)

Acknowledgments: Unreal Engine Team, Python Community, Open Source contributors

Support & Contact: Use repo issues/discussions; dev log for internal trace
