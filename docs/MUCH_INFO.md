Optimizor 1.0 — Action Catalogue (UE 5.6) 13/08/25
A chaptered, concrete checklist of everything the plugin can do to a project, split by domain. Each action is labeled by Mode:
•	AUTO — performed automatically by the plugin (with Dry Run and caps)
•	SEMI — offered with a one click wizard/confirm (bulk but user approved)
•	ADVISE — best practice surfaced in UI/report (not changed by code)
All AUTO actions respect: Dry Run, Max Changes, Selection vs Paths, Exclusions, Skip Collection, close editors, source control checkout, verify after write, and history snapshot.
________________________________________
1) Textures — Concrete Actions
1.1 Classification & Inference
•	AUTO Detect texture role via name/path tags (normal/base/masks/grayscale/UI/LUT/packed/UDIM).
•	AUTO Infer usage from referencing materials (up to N referencers) and map to sampler buckets (normal/masks/grayscale/linearcolor/color).
•	ADVISE Flag ambiguous/contradictory usage (same texture sampled as color & linear in different materials).
1.2 Format & Color Pipeline
•	AUTO Set CompressionSettings per class: TC_Normalmap for normals, TC_Masks for packed masks, TC_Grayscale for scalar maps, TC_Default (or TC_BC7 when preset allows) for color.
•	AUTO Set sRGB on/off appropriately (off for normals/masks/linear, on for color/UI).
•	SEMI Project level remap table (e.g., force BC7 for hero albedos on PC Ultra).
•	ADVISE Report alpha channel waste (normals with unused alpha, color with solid alpha).
1.3 Mipmaps & LOD
•	AUTO Toggle GenerateMips and set MipGenSettings (FromGroup / SharpenX / BlurX / NoMipmaps).
•	AUTO Enforce minimum LODBias (e.g., ≥ 1) except UI/LUTs/small detail maps.
•	SEMI Clamp MaxTextureSize per role (color/normal/mask caps); offer resample pass for oversize assets.
•	ADVISE Highlight textures lacking mips that are sampled minified (aliasing risk).
1.4 Streaming & VT
•	AUTO Set NeverStream=true for UI/LUTs; ensure world content streams.
•	AUTO Respect landscapes/UDIMs: don’t disable VT there; otherwise allow force disable VT when preset requires.
•	SEMI Adjust Texture Groups to preset (UI, World, LUT); optional per group max size overrides.
•	ADVISE Recommend Virtual Texturing for giant tileables on high end targets.
1.5 Safety, Exceptions & Scope
•	AUTO Skip assets by path/name hints and Skip Collection.
•	AUTO Close editors before mutation; attempt source control checkout; verify after write.
•	SEMI One click Revert Last Run using snapshot JSON.
1.6 Reporting & Budgeting
•	AUTO Report before/after settings, top change reasons, CSV of changes, memory bytes estimate, and create Collections: Optimizor_Changed_*, Optimizor_Skipped_*.
•	ADVISE Texel density and atlas suggestions (when we can’t change source art).
________________________________________
2) Static Meshes — Concrete Actions
2.1 Nanite & Build Settings
•	AUTO Enable/disable Nanite per preset; set fallback screen size.
•	SEMI Batch Build Nanite for eligible meshes; skip skeletal/unsupported.
•	ADVISE Suggest Nanite for CAD/high poly imports.
2.2 LOD Strategy
•	AUTO Generate LODs with IMeshReduction where missing; set LOD count from tri budget.
•	AUTO Apply screensize curve per preset; optionally enforce triangle caps for L1/L2/L3.
•	SEMI Respect user LODGroups if specified; otherwise apply preset caps.
•	ADVISE Warn on excessive materials per mesh (draw call multiplier).
2.3 Lightmap UVs & Tangents
•	AUTO Generate/repair Lightmap UVs (min/max res, channel selection).
•	AUTO Recompute normals/tangents per project policy (angle threshold).
•	ADVISE Flag overlapping lightmaps, extreme texel densities.
2.4 Collision & Complexity
•	AUTO Set simple collision type (box/sphere/convex) for props missing collision.
•	SEMI Replace complex as simple with simple for non interactive assets.
•	ADVISE List high poly colliders; promote UCX authoring tips.
2.5 Stats & Collections
•	AUTO CSV per mesh: base tris, per LOD tris, materials, Nanite on/off, lightmap res.
•	SEMI Create Changed/Needs Attention Collections (e.g., “No LODs”, “>X materials”).
________________________________________
3) Skeletal Meshes & Animation — Concrete Actions
•	AUTO Apply Animation Compression settings per preset (bitwise/BVH/ACL if available).
•	AUTO Enable Update Rate Optimization on components for distant actors.
•	SEMI Strip/add LODs on skeletal meshes (bone reduction where rules allow).
•	ADVISE Cap bones/influences for crowds; share anim instances; reduce tick rate off screen.
________________________________________
4) Materials — Concrete Actions
•	AUTO Detect & remove dead static switches (compile time only) in dry run preview → confirm → apply.
•	AUTO Identify duplicate texture samplers with identical params; suggest consolidation (SEMI to auto replace).
•	SEMI Normalize normal map space naming (_ogl/_dx) → add note or switch sampler type where safe.
•	ADVISE Flag high instruction count, heavy translucency, per pixel world position ops.
________________________________________
5) Runtime (Scalability, Post, Lighting) — Concrete Actions
5.1 Scalability & Device Profiles
•	AUTO Apply per preset Scalability buckets (View/Shadow/Effects/Post/Foliage) via .ini.
•	SEMI Switch Device Profiles (Desktop/Console/Mobile/VR) and copy tuned overrides.
5.2 Global Illumination & Reflections
•	SEMI Toggle Lumen GI/Reflections on/off; choose HWRT vs Software; set quality levels.
•	ADVISE Fall back to baked/static for mobile/archviz presets when appropriate.
5.3 Shadows
•	AUTO Enable/disable Virtual Shadow Maps; adjust distances/cascades; set per quality defaults.
•	ADVISE Reduce shadowed lights count; trim attenuation radii; prefer static for scenery.
5.4 Post Processing & AA
•	AUTO Configure AA (TSR/TemporalAA/MSAA for VR) per preset; dial bloom/AO/DOF/motion blur.
•	SEMI Enable Dynamic Resolution with bounds for consoles/PC Balanced.
5.5 Streaming & Async Loading
•	AUTO Set r.Streaming.PoolSize hints; configure time slice for async loading.
•	SEMI Enable Fast Geometry Streaming plugin and defaults for large worlds.
•	ADVISE Organize streaming levels; avoid heavy blocking loads.
________________________________________
6) World Partition, HLOD & Culling — Concrete Actions
•	SEMI Enable World Partition on new projects; set cell size templates (open world vs interior).
•	AUTO Generate HLOD clusters with preset reduction; build proxies.
•	AUTO Inject default Cull Distance Volume rules for common small props.
•	ADVISE Place streaming volumes/triggers; cap on screen actor counts.
________________________________________
7) Audio — Concrete Actions
•	AUTO Apply platform appropriate compression settings to SoundWaves (where unset).
•	SEMI Set concurrency/priority limits per preset (mobile lower caps).
•	ADVISE Stream long music; reduce simultaneous ambient loops.
________________________________________
8) Packaging, Cooking & Size — Concrete Actions
•	SEMI Toggle PAK compression on/off (boot time vs disk size trade).
•	AUTO Exclude Editor only content from cook; strip debug content by pattern.
•	SEMI Set platform target texture formats (ASTC/ETC/BCx) and shader pipeline caches.
•	ADVISE Preload hot levels; warm shaders; keep startup maps minimal.
________________________________________
9) Safety & Governance — Concrete Actions
•	AUTO Dry Run default; enforce Max Changes; per change gate via Apply Only keys.
•	AUTO Source control checkout; close editors; verify after write; save dirty packages.
•	SEMI One click Revert using last snapshot; export/import action plan JSON.
•	ADVISE Educate on content guidelines (naming, foldering, tags for rules).
________________________________________
10) Reporting & Telemetry — Concrete Actions
•	AUTO Write TXT/CSV/JSON reports with counts, paths, reasons, memory deltas; emit compact summary to log.
•	AUTO Create Collections for Changed/Skipped/Hot Referencers; open report folder.
•	SEMI Baseline capture: run stat unit/gpu & MemReport and store next to audit.
•	ADVISE Budget dashboards per map (future module).
________________________________________
11) Presets — Concrete Actions
•	AUTO Load preset .json; merge project overrides; display deltas in UI.
•	AUTO Apply preset to Texture/Mesh/Material/Runtime/World rules in one pass.
•	SEMI Duplicate + edit preset into project config; lock presets for CI.
•	ADVISE Per preset best practice tips inline (what engine can’t change).
________________________________________
12) Extensibility Hooks
•	SEMI Per team rule packs (regex/tag → action) for custom pipelines.
•	SEMI CLI/headless mode for CI (cook guards, budget checks).
•	ADVISE Plugin API for third party reducers (Simplygon) and analytics.
________________________________________
13) Coverage Map (AUTO vs SEMI vs ADVISE)
Domain	AUTO	SEMI	ADVISE
Textures	Compression, sRGB, mips, LODBias, VT off/on (safe cases), groups, reporting	Max size resample, group remap	Texel density, atlas, authoring
Static Mesh	Nanite toggle, LOD gen, screen sizes, lightmap UVs, simple collision, stats	Triangle caps, respect LODGroup, complex→simple collision	Materials per mesh, CAD prep
Skeletal	Anim compression, Update Rate Opt	LOD/bone reduction	Crowd budgets, shared BP
Materials	Dead static switches removal, sampler duplicates report	Auto replace duplicate samplers	Shader complexity guidance
Runtime	Scalability ini, VSM, AA/post defaults, streaming pool/time slice	Lumen mode, device profile, Dynamic Res, Fast Geo Streaming	Lighting strategy per platform
World	HLOD gen, cull distance defaults	Enable WP, cell sizes	Streaming layout, actor budgets
Audio	Compression defaults	Concurrency caps	Mix/atten rules
Packaging	Cook strip editor data	Pak compression, formats	Startup flow, shader warmup
Safety & Reports	Dry run, caps, verify, snapshots, collections	Revert, baseline capture	Governance tips
________________________________________
14) Next Step — Implementation Drill Down Index
For each chapter above, we will create two sub documents:
1.	User/Plugin Controls — exact UI toggles, preset bindings, safe defaults.
2.	Code Path — module, APIs, property writes, .ini keys, Python/C++ flows, failure modes.
This Action Catalogue is the canonical scope; we’ll implement and verify chapter by chapter.

Optimizor 1.0 — Concrete Action Catalog (UE 5.6) 13/08/25
A chaptered, implementation-ready checklist of exact actions the plugin performs. Each action lists What, How we decide, User controls, and Safety/Reporting. Scope covers Textures, Meshes, Materials, plus Cross-cutting systems and Preset Advice Bundles.
________________________________________
0) Cross Cutting Systems (applies to all actions)
•	Scan Scope
What: Enumerate assets by selection, include list, or project root; respect exclude lists and skip collections.
How: EditorAssetLibrary.list_assets (fallback AssetRegistry), dedupe by path.
Controls: Dry run, Only selected, Include/Exclude CSV, Change cap.
Safety: Cancellable slow task; per-change cap; skip locked/read-only.
•	Change Application
What: Write properties via safe setters with pre/post edit and package dirtying.
How: Central safe_set(); optional auto-checkout; close editors before writes.
Controls: Save assets, Auto flush dirty, Auto checkout, Close editors, Verify after write.
Safety: Verify after save; retry post_edit_change; collect failures.
•	Collections & Reports
What: Create Collections for Changed/Skipped; write TXT/CSV/JSON; stash revert snapshot.
How: ReportSvc; Saved/Optimizor/History/*.json.
Controls: Report dir/basenames; include memory bytes; timestamped filenames.
Safety: Always dry-run first from UI; history list in Reports tab.
•	Preset Gatekeeping
What: Each preset defines apply_only keys and hard caps per action family.
How: JobRunner filters attempted edits; gate high risk mutations in Conservative mode.
Controls: apply_only, max_changes per run; per-target toggles.
Safety: Exceptions list (path/name tags), skip collection.
________________________________________
1) Textures — Exact Actions
1.	Compression Class Assignment
What: Set CompressionSettings to TC_Normalmap, TC_Masks, TC_Grayscale, TC_Default, TC_HDR for LUTs.
How: Filename heuristics + optional material sampler inference (looks at referencing MaterialExpressionTextureSample.sampler_type).
Controls: Per type override (Color/Normal), inference on/off, referencer cap.
Safety: Gate via apply_only: ["compression"]; verify after write.
2.	sRGB Flag Correction
What: Toggle sRGB=true for color, false for normal/masks/linear.
How: Same heuristics+inference as above.
Controls: srgb_color, srgb_normal.
Safety: Report mismatches; never flip on LUTs/HDR.
3.	Mip Generation Policy
What: Set MipGenSettings (TMGS_FromTextureGroup, sharpen/blur variants, NoMipmaps for UI/LUT).
How: Group policy; UI and LUT force no mips; presets can override.
Controls: generate_mips, explicit mip_gen enum.
Safety: Gate via apply_only: ["mip_gen"].
4.	LOD Group Enforcement
What: Assign LODGroup (World, WorldNormalMap, UI, ColorLookupTable).
How: Classification (UI/Normal/LUT) and project rules (e.g., Mobile* LOD groups if present).
Controls: Optional group force per preset.
Safety: Off in Conservative mode unless preset explicitly allows.
5.	Never Stream for UI/LUT
What: NeverStream=true for UI atlases, fonts, LUTs.
How: Path and tag hints (/UI/, sprite, icon, font, lut).
Controls: apply_only: ["never_stream"] via preset.
Safety: Skip large world-space decals unless preset says so.
6.	Virtual Texture Discipline
What: Disable VirtualTextureStreaming for most regular textures; respect Landscapes/UDIM.
How: Path hints + RESPECT_VT_ON_LANDSCAPES; per preset override.
Controls: virtual_texture=true/false (semantic: allow vs force off).
Safety: Never disable VT on known landscape/UDIM assets.
7.	LOD Bias Lower Bound
What: Enforce LOD Bias >= 1 on non UI, non small/detail textures when preset wants memory savings.
How: Name hints (detail, small dims), preset flag.
Controls: ENFORCE_LOD_BIAS_1.
Safety: Gate with apply_only: ["lod_bias"].
8.	Size Caps (Advisory + Optional Reimport)
What: Detect over budget resolution per channel type; optionally scale on import rule (advice only by default).
How: MaxSizeColor/Normal/Mask per preset.
Controls: Advice vs enforce; per project toggle.
Safety: Default to advice only; emit hotlist.
9.	Memory Impact Estimation
What: Pre/Post byte estimate (format × res × mips).
How: Table lookup; report items and CSV.
________________________________________
2) Static Meshes — Exact Actions
1.	Nanite Enable/Disable
What: Toggle bNaniteEnabled and set fallback ScreenSize.
How: Preset target, triangle count thresholds, blacklist (foliage/physics heavy).
Controls: nanite.enable, nanite.fallback_screensize.
Safety: Skip if per asset tag _keep/__nofix present.
2.	LOD Generation (Triangles/Screen Curve)
What: Generate or re tune LODs via IMeshReduction; set % triangles and ScreenSize per LOD.
How: Strategy: GroupFirst (enforce LODGroup percents) → cap by triangle ceilings → apply screens curve.
Controls: lods.strategy, caps per LOD, screens array, group name.
Safety: Dry run diff table; preserve existing LODs if better.
3.	LOD Group Enforcement
What: Apply project LODGroups (e.g., Mobile*) when present.
How: Lookup UStaticMesh::LODGroup.
Controls: lods.group with enforce=true/false.
Safety: Clamp instead of overwrite when authoring intent is stricter.
4.	Lightmap UVs (Auto Generate)
What: Generate missing lightmap UVs; set Min/Max Lightmap Resolution.
How: Run built in charting; overlap detection.
Controls: lightmap_uvs.generate, min_res, max_res.
Safety: Skip meshes with manual channel tags; report overlaps.
5.	Collision Simplification
What: Create simple convex if missing; remove redundant complex-as-simple when Nanite.
How: Analyze BodySetup primitives count.
Controls: collisions.auto_simple=true.
Safety: Respect physics interaction tags (VR props keep detailed colliders).
6.	Optional Merge Actors (Editor Only)
What: Batch merge by cluster to reduce draw calls for baked sets.
How: Editor utility call; keep source assets if Conservative.
Controls: merge_actors.enabled.
Safety: Only in dry run unless explicitly enabled.
Reporting: per mesh tri counts per LOD, Nanite stats, UV/channel summary; Collections for Changed, Skipped.
________________________________________
3) Skeletal Meshes — Exact Actions (Phase 2.5)
1.	LOD Generation / Reduction
What: Generate LODs via IMeshReduction for Skeletal; set ScreenSize curve.
How: Triangle thresholds per preset; preserve imported LODs.
Controls: skel.lods.strategy, caps, screens.
Safety: Never touch cloth-painted sections by default.
2.	Animation Compression Policy
What: Set compression schemes (ACL/Default) per preset.
How: Apply to AnimSequences in scope; skip cinematic takes.
Controls: anim.compression=preset.
Safety: Dry run delta size + max error snapshot.
________________________________________
4) Materials & Instances — Exact Actions
1.	DX/GL Normal Convention Audit
What: Flag projects mixing OGL vs DX normal maps; identify suspect materials.
How: Scan texture names and sampler types; compare green channel assumptions.
Controls: Report only (advice); optional auto flip mask outputs in MI (off by default).
Safety: No auto rewiring by default.
2.	Static Switch Cleanup
What: Remove unused static branches; collapse constants.
How: Graph analysis in C++; optional Python material traversal.
Controls: static_switch_cleanup=true/false.
Safety: Save backup and diff summary.
3.	Sampler Consolidation
What: Detect duplicate samplers (same texture/settings) and suggest reuse.
How: Hash sampler states.
Controls: Report first; auto fix optional.
Safety: Only within MI where safe.
4.	Packed Map Hints
What: Detect _orm/_rma usage; advise BC5/BC4 opportunities and channel wiring guidance.
How: Cross check texture names + material pins.
Controls: Advice bundle per preset.
________________________________________
5) Levels/World — Advisory Checks (Report Only v1)
•	Foliage Cull & LOD Budget
What: Report foliage types with missing cull distances or LOD ranges off budget.
How: Inspect FoliageType.
Controls: Thresholds per preset.
•	HLOD / World Partition Tiles
What: Report maps without HLOD cluster assets; budget draw calls.
How: Query HLOD setup; emit steps to generate.
•	Lighting Pipeline Mismatch
What: Detect incompatible shadow quality vs platform preset.
How: Project Settings snapshot + preset advice.
(All world checks are non mutating in v1; they attach advice text to the report.)
________________________________________
6) Preset Advice Bundles (auto attached to reports)
Each preset ships an Advice.md snippet injected into the report:
•	PC High: prefers BC7 for color, keep mips sharpen ≤ 0.25; use Nanite for static props; maintain LODBias=0 unless VRAM bound.
•	Console Balanced: cap color at 2k, normals/masks at 1k; enable Nanite broadly; prefer BC5 for normals (no alpha); avoid VT for small props.
•	Mobile Low: downscale aggressively; enforce Mobile* LODGroups; avoid VT; atlas UI into non PoT allowed; disable per pixel normal where costly.
•	VR: favor stability (low shimmer); mipgen Sharpen ≤ 0.25; collisions simplified only for non interactive; ensure 90/120 Hz budgets.
•	Cinematic: BC7 for hero color; BC5 for normals; VT allowed on UDIM/heroes; keep LODBias=0; no auto merge.
•	UI Crisp: No mips; NeverStream; clamp addressing; linear color for grayscale masks.
________________________________________
7) Action→Code Map (where each action lives)
•	Textures: Python module texture_doctor.py (classification, inference, safe writes, reporting). Entry texture_doctor_entry.py bridges C++ JSON to Python.
•	Meshes: C++ MeshDoctor subsystem (new) using IMeshReduction, NaniteTools, MeshUtilities. Report via ReportSvc.
•	Materials: Mixed C++/Python traversal; C++ graph utilities; Python for CSV/JSON enrichment.
•	World Checks: C++ Editor utility functions (read only) producing advice blocks.
________________________________________
8) UI/UX Hooks (SOptimizorTab)
•	Tabs: Textures | Meshes | Materials | Reports.
•	For each action group: checkboxes map to apply_only keys; advanced drawers per family.
•	Footer: Scan (forces dry run) and Auto Fix (honors safety gates).
•	One click Create Collections and Open Report Folder.
________________________________________
9) Milestones (Macro → Micro)
1.	M1 — Texture Foundation: finalize schema; fix current TD bugs; robust reporting; Collections; revert snapshot; preset advice injection.
2.	M2 — Static Meshes: Nanite toggles; LOD generation; lightmap UVs; collision simplifier; CSV stats.
3.	M2.5 — Skeletal/Anim: Skeletal LODs; animation compression policy.
4.	M3 — Materials: DX/GL audit; static switch cleanup; sampler consolidation.
5.	M4 — World Checks & Polish: non mutating map audits; history UI; headless CLI.
________________________________________
10) Known Issues in Current Prototype (quick fixes)
•	Duplicate functions in texture_doctor.py: two gather_textures() defs — keep the second (selection aware), delete the first.
•	STextureDoctorPanel.cpp typos: duplicated Json var; WriteTextToSavedFile() uses undefined Abs (should be Path).
•	Enum plumbing: ensure C++→Python enums are passed as ints and resolved via _enum_from_int(); keep BOM tolerant JSON I/O.
•	ACTIVE_PRESET label: unify name across C++ and Python (use preset).
________________________________________
This catalog is the source of truth for implementation and QA. Next, we can take each chapter and design the exact UI controls and the code pathways (C++/Python) that perform the actions.

Optimizor 1.0 — Build Plan & Order of Operations (UE5.6) 13/08/25
A pragmatic, macro→micro plan to deliver a stable, generalist optimization plugin (C++ + Python) with fast iteration and strong safety.
________________________________________
0) Guiding Principles
•	Macro→Micro: lock the user-facing flow and config schema first; implement subsystems behind it.
•	Single Canonical Config: JSON schema shared across UI ⇄ C++ ⇄ Python. Version it.
•	Safe By Default: dry-run, change caps, revert snapshots, collections for review.
•	Incremental & Testable: each milestone shippable, with automated sanity checks.
•	No dead UI: any surfaced control must be wired into a no-op-safe path on day 1.
________________________________________
1) Foundations (Week 1)
Goal: Solid plugin skeleton, one tab, presets plumbed end-to-end, Texture pipeline callable.
1.1 Module & Tab
•	Create Optimizor editor module (keep TextureDoctor code but mount under Optimizor).
•	Nomad tab: SOptimizorTab with panes: Textures | Meshes | Materials | Reports.
•	Hook LevelEditor Tools menu ➜ opens tab.
1.2 Canonical Config Schema v1
•	Freeze initial JSON keys (dry_run, max_changes, include/exclude, target, preset, texture/mesh/material subtrees).
•	Add schema_version (int) and run_id (timestamp string) added by UI.
•	C++: FOptimizorConfig (UStruct) ⇄ JSON via FJsonObjectConverter.
•	Python entry points accept same JSON per tool: textures.run(cfg), meshes.run(cfg), materials.run(cfg).
1.3 Python Bridge
•	FOptimizorPythonBridge::Run(tool, JsonPath, bAutoFix) ➜ loads module, importlib.reload, executes.
•	Standard stdout markers: [[REPORT_JSON]] { ... } for compact summaries.
•	Error channeling: capture exceptions and surface to UI toast + Report pane.
1.4 Reports & History
•	Saved/Optimizor/History/<run_id>/{report.txt, report.json, changes.csv, revert.json}.
•	Collections: Optimizor_Changed_<run_id> and Optimizor_Skipped_<run_id>.
•	Report pane reads history list and opens folders.
Definition of Done (DoD):
•	Open tab ➜ select preset ➜ Scan Textures ➜ see modal summary + history entry.
•	Auto-Fix writes revert snapshot and respects Max Changes.
________________________________________
2) Texture System Hardening (Week 2)
Goal: Ship-quality Texture optimizer (existing TextureDoctor refined to schema + UI).
2.1 Consolidation & Fixes
•	Remove duplicate gather_textures(); keep fast list-assets path with AR fallback.
•	Align ACTIVE_PRESET_LABEL naming UI⇄py; ensure label appears in reports.
•	Fix file save helper names in Slate panel and BOM-less UTF-8 writes.
•	Ensure ExecPython fallback works when module handle isn’t present.
2.2 Policy & Gates
•	Enforce APPLY_ONLY + Conservative mode.
•	Exceptions: path/name tags, skip collection.
•	Inference from materials (cap referencers N; make optional per preset).
•	Memory impact estimator (before/after) in report.
2.3 UX Polish
•	“Explain change” tooltip per row (hook reasons map).
•	Preset tooltips: best-practice guidance (non-engine changes).
DoD:
•	Runs on sample projects (PC/Console/Mobile/VR/UI-heavy) with <1% error rate, responsive SlowTask.
________________________________________
3) Mesh System (Weeks 3–4)
Goal: LOD/Nanite/lightmap/collision passes with dry-run and reversible changes.
3.1 C++ Subsystem
•	FMeshDoctor (C++) using IMeshReduction/NaniteSettings.
•	Read LODGroup, apply policy order: Group → Caps → Screens.
•	Auto-count LODs from triangle counts + screensize curve.
•	Nanite on/off per preset; fallback screen size control.
3.2 Lightmap & Collision
•	Generate Lightmap UVs within min/max res; detect overlaps, warn if baking risk.
•	Collision: generate simple convex if none (skip if tagged).
3.3 Reporting
•	Per-mesh: base triangles, per-LOD triangles %, Nanite enabled, lightmap res, collision type.
DoD:
•	Dry-run surfaces accurate deltas; Auto-Fix creates revert snapshot for mesh settings.
________________________________________
4) Material System (Week 5)
Goal: Static-cleanup & diagnostics without altering authored look unless requested.
•	Scan graphs for DX/GL normal mixing; list offenders.
•	Detect redundant samplers; suggest consolidation.
•	Find dead static switches; optional auto-prune.
•	Packed map hints (_orm, _rma): verify channel swizzles.
DoD:
•	CSV of material issues with safe auto-fix options gated by APPLY_ONLY.
________________________________________
5) Presets & Best-Practice Notes (Week 6)
Goal: Generalist coverage + embedded guidance.
5.1 Ship Presets
•	PC High, Console Balanced, Mobile Low, Switch, VR, AR, Cinematic, UI Crisp, Archviz, Stylized Mobile, Low-Mem QA.
•	Each preset defines: texture caps, LOD strategy, Nanite policy, lightmap budget, apply-only gates, change cap.
5.2 Preset Notes (non-engine guidance)
•	Color space, authoring DPI, atlas/packing, trim sheets, shader feature toggles, post-process hints.
•	Shown inline in the UI and exported to report header.
DoD:
•	Switching presets updates UI, JSON, and report label consistently.
________________________________________
6) QA, CI, and Samples (Week 7)
Goal: Prevent regressions and validate on real content.
•	Commandlet/Headless: OptimizorCMD -Config=<json> -Target=Textures|Meshes|Materials.
•	Test maps (open world, VR template, mobile-forward, UI heavy).
•	CI checks: fail build if asset budgets exceed preset.
•	Performance: cache scans; incremental re-run with include paths.
________________________________________
7) UX/DevEx Polish (Week 8)
Goal: Delight and productivity.
•	History pane: rerun with last config; “Revert last run”.
•	Perforce changelist helper (optional).
•	Hot-list: most-referenced textures/materials.
•	Tooltips for every control; hover docs.
________________________________________
8) Engineering Practices & Repo Structure
/Plugins/Optimizor/
  Source/
    OptimizorEditor/          # module, tab, UI
    OptimizorCore/            # shared structs, config, report svc
    TextureDoctor/            # texture C++ shims (if any)
    MeshDoctor/
    MaterialDoctor/
  Python/
    textures/texture_doctor.py
    textures/texture_doctor_entry.py
    meshes/mesh_doctor.py
    materials/material_doctor.py
  Config/Presets/*.json
  Resources/
•	Branching: main (stable), dev/* per subsystem, PRs with report artifacts attached.
•	Coding Standards: no dead controls, every write wrapped with revert capture, verify-after-write optional, unit tests for JSON IO.
________________________________________
9) Risk Register & Mitigations
•	Python enum drift: tolerant resolver (already present) + unit tests.
•	Asset saves blocked by source control: Auto-checkout flag + error surfaced per asset.
•	Long scans: path filters + incremental cache + HEARTBEAT progress.
•	Preset creep: schema_version; keep backward-compat in entry points.
•	Material rewrite risk: default to scan-only; opt-in auto-fix.
________________________________________
10) Immediate Tasks (Next 2–3 Coding Sessions)
1.	Create OptimizorEditor module + Nomad tab scaffolding (reuse current module patterns).
2.	Implement FOptimizorConfig + JSON read/write + schema_version.
3.	Build Python bridge wrapper + compact report parsing.
4.	Port TextureDoctor UI into Textures pane; wire to canonical JSON; fix duplicates & save helper.
5.	Report pane listing history, open-folder, create collections.
Exit Criteria: Scan/Auto-Fix for textures works through Optimizor tab and writes history.

Perfect—let’s turn everything we’ve planned into a bullet proof, execution ready plan formatted for the “Plan” feature in Continue (VS Code). It’s broken into small, reviewable tasks with clear dependencies, file targets, prompts for the AI, and acceptance checks. Paste this into a PLAN.md at your repo root (or into Continue’s Plan input) and work top to bottom.
________________________________________
MagicOptimizer — Continue Plan (UE 5.6, C++ + Python)
Goal: Deliverable first, CSV driven Audit → Recommend → Apply → Verify across all pipelines (PC/Console/Mobile/VR/VP).
Style: Macro → micro; ship in thin, testable slices; everything traceable to CSV + JSON in Saved/Optimizor/History/<RunId>/.
Project root for plugin code:
HostProject/Plugins/MagicOptimizer/
Pre flight (once)
•	Ensure UE 5.6 install is verified; .NET 8 Desktop Runtime installed.
•	VS2022 workloads: Game Dev with C++, Desktop C++, v143 toolset, Win 10/11 SDK.
•	UE PythonScriptPlugin enabled in the HostProject.
•	If Marketplace plugins spam warnings (e.g., GraphNUnrealPlugin), disable for now.
________________________________________
Milestone 0 — Repo Hygiene & Scaffolding
T0.1 Create PLAN + folders (no code changes)
Files/Dirs
HostProject/Plugins/MagicOptimizer/
  Content/Python/magic_optimizer/{__init__.py, entry.py, io_csv.py, utils.py}
  Content/Python/magic_optimizer/textures/{audit.py,recommend.py,apply.py,verify.py}
  Content/Python/magic_optimizer/meshes/{audit.py,recommend.py,apply.py,verify.py}
  Content/Python/magic_optimizer/materials/{audit.py,recommend.py,apply.py,verify.py}
  Content/Python/magic_optimizer/runtime/{audit.py,recommend.py,apply.py,verify.py}
  Content/Python/magic_optimizer/world/{audit.py,recommend.py,apply.py,verify.py}
  Content/Python/magic_optimizer/presets/{Mobile_AR.json,PC_High.json,Console.json,VR.json,VirtualProduction.json,Authoring_Safe.json}
Acceptance
•	Folders exist; repo builds unchanged.
Continue prompt (paste into Plan step)
Create empty Python package skeleton under Content/Python/magic_optimizer with the file tree above. Use empty modules with docstrings only.
________________________________________
Milestone 1 — Foundations (C++)
T1.1 Settings & Config structs
Why: Single source of truth for runs; feeds Python.
C++ Targets
•	Source/MagicOptimizer/Public/OptimizerSettings.h/.cpp
•	Source/MagicOptimizer/Public/OptimizerRun.h/.cpp
•	Source/MagicOptimizer/Public/PythonBridge.h/.cpp
•	Source/MagicOptimizer/Public/SOptimizerPanel.h/.cpp
•	Source/MagicOptimizer/Public/OptimizerJson.h/.cpp (optional helper)
What to implement
•	UOptimizerSettings (EditorPerProjectUserSettings): deliverable preset, category mask, scope (selection/include/exclude), safety (dry_run, max_changes…), gates (apply_only…).
•	FOptimizerConfig (USTRUCT) → JSON (snake_case).
•	NewRunId(), EnsureRunDir(), SaveConfigJson().
Acceptance
•	Editor compiles; NewRunId() creates Saved/Optimizor/History/<RunId>/config.json.
Continue prompt
Implement UOptimizerSettings and FOptimizerConfig for MagicOptimizer. Add JSON save/load (snake_case) using FJsonObjectConverter. Create NewRunId() and EnsureRunDir() helpers that write to Saved/Optimizor/History/<RunId>/.
________________________________________
T1.2 Nomad Tab & Header UI
Why: Macro UX first; buttons will call orchestrator.
C++ Targets
•	SOptimizerPanel.h/.cpp
•	Hook in MagicOptimizerModule.cpp (register tab + Tools menu)
UI
•	Header: Deliverable (preset dropdown), Categories (Textures, Meshes, Materials, Runtime, World), Scope (Selection / Include / Exclude), Safety (Dry Run, Max Changes).
•	Buttons: Full Audit, Recommend, Apply, Verify, History…
Acceptance
•	Tab appears under Tools → MagicOptimizer; controls render; buttons are wired to no ops.
Continue prompt
Create SOptimizerPanel with a header row (preset dropdown, category checkboxes, scope, safety) and buttons (Full Audit, Recommend, Apply, Verify, History). Wire buttons to stub orchestrator calls.
________________________________________
T1.3 Python bridge (C++ → Python)
Why: One consistent callsite.
C++ Targets
•	PythonBridge.h/.cpp
Behavior
•	bool RunPython(const FString& Phase, const FString& Category, const FString& ConfigJsonPath, const FString& RunDir, FString& OutStdErr)
•	Uses IPythonScriptPlugin to execute:
•	import importlib, magic_optimizer.entry as e; importlib.reload(e); e.run(r"<json>", phase="<phase>", category="<Category>", run_dir=r"<dir>")
Acceptance
•	Call to audit/all returns without error (Python stubs).
Continue prompt
Implement PythonBridge::RunPython using IPythonScriptPlugin. Build the command string to call magic_optimizer.entry.run(json, phase, category, run_dir) and propagate errors to logs.
________________________________________
T1.4 Run orchestrator
Why: Encapsulate the 4 phase loop and history.
C++ Targets
•	OptimizerRun.h/.cpp
Behavior
•	Read UOptimizerSettings → compose FOptimizerConfig → SaveConfigJson.
•	Execute audit for selected categories → recommend → (optional) apply → verify.
•	Show modal summary: scanned/changed/errors per category; buttons to Open Folder or Open CSV.
Acceptance
•	Clicking Full Audit creates <RunId> with audit CSVs (empty from stubs) + meta.json.
Continue prompt
Implement OptimizerRun::Execute(Mode) that writes config JSON, calls Python phases in order, and collects a JSON summary (counts + artifact paths) for a modal dialog.
________________________________________
Milestone 2 — Textures (end to end)
T2.1 Python: textures/audit.py
Why: Ground truth → CSV.
Behavior
•	Scope: selection or include paths (fallback /Game).
•	Fields (stable):
Path,Name,Class,LODGroup,Width,Height,NumMips,LODBias,CompressionSettings,SRGB,MipGenSettings,NeverStream,VirtualTextureStreaming,AddressX,AddressY,Filter,SourceFormat,ImportedWidth,ImportedHeight,HasAlpha,IsGreyscale,ReferencersTotal,ReferencersMaterials,ResourceSizeBytes
Acceptance
•	Selecting ≥1 textures and clicking Full Audit writes textures_audit.csv with ≥1 row.
Continue prompt
Implement textures/audit.py to enumerate Texture assets by selection or include paths and write textures_audit.csv with the above columns. Guard referencer queries and missing fields.
________________________________________
T2.2 Python: textures/recommend.py
Why: Deterministic suggestions; reasons + flags.
Behavior
•	Use deliverable preset caps & rules:
o	sRGB: true for color/emissive/UI; false for data maps.
o	Compression: Default/Normalmap/Masks/UI2D.
o	MipGen: NoMipmaps for UI/decals else FromTextureGroup.
o	Max size cap (Mobile*, VR*, PC*).
o	Streaming: NeverStream for UI or ≤ threshold.
•	Output textures_recommendations.csv with:
Path,TypeGuess,Suggest_SRGB,Suggest_Compression,Suggest_MipGen,Suggest_MaxTextureSize,Suggest_NeverStream,Suggest_VirtualTextureStreaming,Suggest_AddressX,Suggest_AddressY,Suggest_Filter,Change_SRGB,Change_Compression,Change_MaxSize,Reason
Acceptance
•	Running Recommend creates the CSV; Change_* reflects diffs vs audit.
Continue prompt
Implement textures/recommend.py that reads textures_audit.csv, applies preset rules to propose new values, computes Change_* flags, and writes textures_recommendations.csv.
________________________________________
T2.3 Python: textures/apply.py
Why: Touch only diffs; produce revert + apply CSV.
Behavior
•	Load by object path; resolve enum names → UE enums.
•	Apply only where different; save; collect diffs per asset.
•	Append previous values to revert.json.
•	Respect dry_run, max_changes, apply_only, only_selected.
Output
•	textures_apply.csv: Path,Applied_Keys,Diff,Result,Error
•	Update revert.json.
Acceptance
•	Known wrong normal map becomes TC_Normalmap & sRGB=false; apply CSV shows one row; revert exists.
Continue prompt
Implement textures/apply.py to read recommendations, resolve enums by name, set diffs, save assets, and write textures_apply.csv. Support dry_run, max_changes, and collect a revert.json.
________________________________________
T2.4 Python: textures/verify.py
Why: Confidence; pass/fail + outliers.
Behavior
•	Re audit; compare to recommendations; tolerate within caps.
•	Output textures_verify.csv; roll up verify_report.json (pass/fail, counts).
Acceptance
•	Verify shows pass for corrected assets; lists stragglers.
Continue prompt
Implement textures/verify.py to re audit, compare against recommended targets, write textures_verify.csv, and return a summary JSON.
________________________________________
T2.5 C++: Wire buttons → artifacts
Why: UX loop complete.
Behavior
•	Buttons call orchestrator with selected categories; open latest run folder; show counts with links to CSVs.
Acceptance
•	You can Audit → Recommend → Apply → Verify textures in one click sequence; CSVs appear with correct headers.
Continue prompt
Connect the panel buttons to OptimizerRun::Execute with phase sequences. Add a modal summary with “Open Folder” and “Open CSV” actions for each artifact.
________________________________________
Milestone 3 — Meshes (percent LODs)
T3.1 meshes/audit.py
CSV
Path,Name,LODGroup,Nanite,NumLODs,LOD0_Tris,LOD1_Tris,LOD2_Tris,LOD3_Tris
T3.2 meshes/recommend.py
•	Name → category buckets; deliverable budgets for LOD1–3 triangles.
•	Convert to percent of LOD0; clamp [0.0001, 1.0].
T3.3 meshes/apply.py
•	Build StaticMeshReductionOptions with percents for LOD1–3; leave LOD0=1.0.
•	Respect respect_lod_group; nanite_default toggle is advisory at first.
T3.4 meshes/verify.py
•	Re audit; ensure LOD1–3 triangles ≤ target.
Acceptance (combined)
•	Known mesh reduces per preset; verify passes; no crash if Nanite meshes encountered.
Continue prompt
Implement meshes audit/recommend/apply/verify per UE 5.6 percent based reduction. Guard missing reduction interface and invalid LOD0.
________________________________________
Milestone 4 — Materials (safe cleanups)
T4.1 materials/audit.py
•	Fields: Path,Name,InstructionCount,SamplerCount,UsesTranslucency,HasDeadStaticSwitches,MixedNormalSpaces,DuplicateSamplers
T4.2 materials/recommend.py
•	Suggest StaticSwitch cleanup, sampler consolidation. Reasons only; default to scan only.
T4.3 materials/apply.py (gated)
•	Only if apply_only includes material_cleanup; create diffs and save; slow path: preview first.
T4.4 verify
•	Re audit counts down; record “changed” and “advised” separately.
________________________________________
Milestone 5 — Runtime/World/Packaging (advice + toggles)
•	runtime/ audit: scalability buckets, Lumen, VSM, AA, streaming pool/time slice, device profile. Recommendations per preset; applies through .ini and project settings where safe; otherwise advice rows only.
•	world/ audit: WP enabled, HLOD presence, Cull Distance Volumes; recommend HLOD generation flags (apply is deferred to Phase 2+).
•	packaging/ audit: pak compression, editor only excludes; recommendations per target.
Acceptance
•	CSVs and verify JSONs exist; Apply touches only safe toggles (behind gates).
________________________________________
Milestone 6 — History & Headless
•	History panel lists past runs; open artifacts.
•	CLI/Commandlet or simple shell task: run audit/apply headless with config JSON.
________________________________________
Milestone 7 — Polish
•	Per asset overrides import (CSV/JSON).
•	Tooltips/hover docs; quick fix buckets.
•	Revert last run (consume revert.json).
________________________________________
Preset JSONs (starter templates)
Create in Content/Python/magic_optimizer/presets/:
•	Mobile_AR.json — cap_color=1024, cap_normal=512, cap_mask=512; never_stream_small=256; lod_target_percent={1:0.33,2:0.18,3:0.10}; nanite=false; lumen=Off
•	PC_High.json — cap_color=4096, normal=2048, masks=2048; TSR; Lumen=HWRT; VSM=On; streaming_pool>=2048MB
•	Console.json — balanced caps; Lumen=Software; dynamic res allowed.
•	VR.json — aggressive caps; MSAA; no motion blur; bloom low; TAA off.
•	VirtualProduction.json — high quality textures, Nanite on, Lumen HWRT, VSM; runtime tweaks are mostly advice.
•	Authoring_Safe.json — audit only defaults; zero apply except harmless texture flags.
________________________________________
Cross cutting acceptance (global)
•	Every phase produces CSVs with fixed headers (even if 0 rows).
•	meta.json stores CSV schema versions and preset label for diffability.
•	Apply never writes without a matching recommendations CSV for the same run_id.
•	Verify always re audits and returns a pass/fail with outlier lists.
________________________________________
Common Continue prompts (copy/paste)
1.	Generate C++ settings + JSON I/O
In Source/MagicOptimizer/, add OptimizerSettings.h/.cpp and OptimizerJson.h/.cpp. Implement UOptimizerSettings (EditorPerProjectUserSettings) and FOptimizerConfig with FJsonObjectConverter save/load to Saved/Optimizor/History/<RunId>/config.json. Include fields for preset, categories, scope, safety, gates, and per category configs.
2.	Create Python entry + dispatch
In Content/Python/magic_optimizer/entry.py, implement run(config_path:str, phase:str, category:str, run_dir:str) that loads JSON, ensures run_dir, and dispatches to category modules’ audit/recommend/apply/verify. Return a dict with counts and artifact paths.
3.	Textures audit (full columns)
Implement textures/audit.py that scans selection or include paths and writes textures_audit.csv with the full schema (Path, Name, Class, LODGroup, Width, Height, NumMips, LODBias, CompressionSettings, SRGB, MipGenSettings, NeverStream, VirtualTextureStreaming, AddressX, AddressY, Filter, SourceFormat, ImportedWidth, ImportedHeight, HasAlpha, IsGreyscale, ReferencersTotal, ReferencersMaterials, ResourceSizeBytes).
4.	Textures recommend/apply/verify
Implement textures/recommend.py, textures/apply.py, and textures/verify.py using preset rules and enum name resolution. Write textures_recommendations.csv, textures_apply.csv, and textures_verify.csv respectively; support dry_run, max_changes, apply_only.
5.	Meshes percent LODs
Implement meshes/audit.py (LOD counts and tris), meshes/recommend.py (convert target tris to percent of LOD0; clamp), meshes/apply.py (SetLODs with percents), and meshes/verify.py.
________________________________________
Notes / Risks to watch
•	UE 5.6 percent only mesh reduction (no max_triangles)—always convert from triangle budgets.
•	Object path vs name—store and resolve by object path in CSVs.
•	Source control—if enabled, guard with auto checkout toggle; still save gracefully when off.
•	Editor state—close editors before writes; re load assets post save if Details panel should reflect changes.
________________________________________
You can now paste tasks into Continue’s Plan and run them sequentially. If you want, I can generate starter code files for T1.1–T1.4 and T2.1 immediately so you can compile and run the first Full Audit.

Detailed Implementation Plan
Project Structure & Core Architecture
File Structure

Apply
HostProject/
├── Plugins/
│   └── MagicOptimizer/
│       ├── MagicOptimizer.uplugin
│       ├── Resources/
│       │   └── Icon128.png
│       ├── Source/
│       │   └── MagicOptimizer/
│       │       ├── Public/
│       │       │   ├── MagicOptimizer.h
│       │       │   ├── SOptimizerPanel.h
│       │       │   ├── OptimizerSettings.h
│       │       │   ├── OptimizerRun.h
│       │       │   └── PythonBridge.h
│       │       └── Private/
│       │           ├── MagicOptimizerModule.cpp
│       │           ├── SOptimizerPanel.cpp
│       │           ├── OptimizerSettings.cpp
│       │           ├── OptimizerRun.cpp
│       │           └── PythonBridge.cpp
│       └── Content/
│           └── Python/
│               └── magic_optimizer/
│                   ├── __init__.py
│                   ├── entry.py
│                   ├── io_csv.py
│                   ├── utils.py
│                   ├── textures/
│                   │   ├── audit.py
│                   │   ├── recommend.py
│                   │   ├── apply.py
│                   │   └── verify.py
│                   ├── meshes/
│                   │   ├── audit.py
│                   │   ├── recommend.py
│                   │   ├── apply.py
│                   │   └── verify.py
│                   └── presets/
│                       ├── PC_High.json
│                       ├── Console.json
│                       ├── Mobile_AR.json
│                       ├── VR.json
│                       ├── VirtualProduction.json
│                       ├── Authoring_Safe.json
│                       └── Cinematic.json
Component Diagrams & Class Hierarchies
Component Diagram:

Core Systems: TextureDoctor, MeshDoctor, MaterialDoctor, RuntimeOptimizer, WorldPartitionManager, AudioOptimizer, PackagingManager
Integration Points: IPythonScriptPlugin, UEditorPerProjectUserSettings, IMeshReduction, NaniteTools, AssetRegistry, EditorAssetLibrary, Material Expression Graph Traversal
Class Hierarchies:

MagicOptimizerModule: Main module class.
SOptimizerPanel: UI panel for the optimizer.
OptimizerSettings: Configuration settings for optimization.
PythonBridge: Bridge to run Python scripts.
OptimizerRun: Orchestration of the optimization phases.
Data Flow Diagrams:

User interacts with SOptimizerPanel.
Settings are read from OptimizerSettings.
OptimizerRun orchestrates audit, recommend, apply, and verify phases.
PythonBridge runs Python scripts for each phase.
Results are reported back to the UI.
Core System Implementation
OptimizerSettings
OptimizerSettings.h

Apply
#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OptimizerSettings.generated.h"

UENUM()
enum class EOptimizerCategory : uint8 {
    Textures	UMETA(DisplayName = "Textures"),
    Meshes		UMETA(DisplayName = "Meshes"),
    Materials	UMETA(DisplayName = "Materials"),
    Levels		UMETA(DisplayName = "Levels")
};

UCLASS(config=EditorPerProjectUserSettings)
class MAGICOPTIMIZER_API UOptimizerSettings : public UObject
{
    GENERATED_BODY()

public:
    UOptimizerSettings();
    static UOptimizerSettings* Get();

    // Scope settings
    UPROPERTY(Config, EditAnywhere, Category = "Scope")
    bool bUseSelection;

    UPROPERTY(Config, EditAnywhere, Category = "Scope")
    FString IncludePathsCsv;

    UPROPERTY(Config, EditAnywhere, Category = "Scope")
    FString ExcludePathsCsv;

    UPROPERTY(Config, EditAnywhere, Category = "Global Run")
    bool bDryRun;

    UPROPERTY(Config, EditAnywhere, Category = "Global Run")
    bool bSaveAssets;

    UPROPERTY(Config, EditAnywhere, Category = "Global Run")
    bool bAutoCheckout;

    UPROPERTY(Config, EditAnywhere, Category = "Global Run")
    bool bCloseOpenEditors;

    UPROPERTY(Config, EditAnywhere, Category = "Global Run")
    bool bVerifyAfterWrite;

    UPROPERTY(Config, EditAnywhere, Category = "Global Run")
    int32 MaxChangesPerRun;

    // Target profile
    UPROPERTY(Config, EditAnywhere, Category = "Target Profile")
    FString TargetProfile;

    // Category mask
    UPROPERTY(Config, EditAnywhere, Category = "Target Profile")
    uint8 CategoryMask;

    // Conservative gates
    UPROPERTY(Config, EditAnywhere, Category = "Conservative Gates")
    bool bConservativeMode;

    UPROPERTY(Config, EditAnywhere, Category = "Conservative Gates")
    TArray<FString> ApplyOnlyKeys;

    // Reporting
    UPROPERTY(Config, EditAnywhere, Category = "Reporting")
    FString ReportDirName;

    // Schema version for migrations
    UPROPERTY(Config)
    int32 SchemaVersion;
};
OptimizerSettings.cpp

Apply
#include "OptimizerSettings.h"
#include "UObject/SaveGame.h"
#include "Engine/Engine.h"

UOptimizerSettings::UOptimizerSettings()
{
    bUseSelection = false;
    bDryRun = true;
    bSaveAssets = true;
    bAutoCheckout = false;
    bCloseOpenEditors = true;
    bVerifyAfterWrite = true;
    MaxChangesPerRun = 1000;

    TargetProfile = TEXT("PC_High");

    CategoryMask = EOptimizerCategory::Textures | EOptimizerCategory::Meshes | EOptimizerCategory::Materials | EOptimizerCategory::Levels;

    bConservativeMode = true;
    ReportDirName = TEXT("OptimizorAudit");
    SchemaVersion = 1;
}

UOptimizerSettings* UOptimizerSettings::Get()
{
    static UOptimizerSettings* Settings = nullptr;
    if (!Settings)
    {
        Settings = Cast<UOptimizerSettings>(UGameInstance::GetSubsystem<UObject>(GetTransientPackage(), UOptimizerSettings::StaticClass()));
        if (!Settings)
        {
            Settings = NewObject<UOptimizerSettings>(GetTransientPackage(), UOptimizerSettings::StaticClass());
        }
    }
    return Settings;
}
PythonBridge
PythonBridge.h

Create file
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PythonBridge.generated.h"

UCLASS()
class MAGICOPTIMIZER_API UPythonBridge : public UObject
{
    GENERATED_BODY()

public:
    static bool RunPythonTool(const FString& JsonPath, const FString& Phase, const FString Category, FString& OutStdErr);
};
PythonBridge.cpp

Create file
#include "PythonBridge.h"
#include "PythonScriptPlugin/Public/PythonScriptPlugin.h"

bool UPythonBridge::RunPythonTool(const FString& JsonPath, const FString& Phase, const FString Category, FString& OutStdErr)
{
    FString Command = FString::Printf(TEXT("import importlib, magic_optimizer.entry as e; importlib.reload(e); e.run(r\"%s\", phase=\"%s\", category=\"%s\")"), *JsonPath, *Phase, *Category);
    bool bSuccess = FIPythonScriptPlugin::Get()->ExecPythonCommand(*Command, OutStdErr);
    return bSuccess;
}
OptimizerRun
OptimizerRun.h

Create file
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OptimizerRun.generated.h"

UCLASS()
class MAGICOPTIMIZER_API UOptimizerRun : public UObject
{
    GENERATED_BODY()

public:
    void Run(const FString& Phase, const TArray<FString>& Categories);

private:
    FString NewRunId();
    void WriteConfigJson(const FString& ConfigPath);
    void ExecutePhase(const FString& JsonPath, const FString& Phase, const FString Category);
};
OptimizerRun.cpp

Create file
#include "OptimizerRun.h"
#include "OptimizerSettings.h"
#include "PythonBridge.h"

void UOptimizerRun::Run(const FString& Phase, const TArray<FString>& Categories)
{
    UOptimizerSettings* Settings = UOptimizerSettings::Get();
    FString ConfigPath = FPaths::ProjectSavedDir() + TEXT("Optimizor/config.json");
    
    // Create new run ID and output directory
    FString RunId = NewRunId();
    FString OutputDir = FPaths::ProjectSavedDir() + TEXT("Optimizor/History/") + RunId;
    IFileManager::Get().MakeDirectory(*OutputDir, true);

    // Flatten settings to JSON
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("run_id"), RunId);
    JsonObject->SetStringField(TEXT("phase"), Phase);
    JsonObject->SetStringField(TEXT("target_profile"), Settings->TargetProfile);
    
    // Categories
    TArray<TSharedPtr<FJsonValue>> CategoryArray;
    for (const FString& Category : Categories)
    {
        CategoryArray.Add(MakeShareable(new FJsonValueString(Category)));
    }
    JsonObject->SetArrayField(TEXT("categories"), CategoryArray);

    // Scope
    TSharedPtr<FJsonObject> ScopeObject = MakeShareable(new FJsonObject);
    ScopeObject->SetBoolField(TEXT("use_selection"), Settings->bUseSelection);
    ScopeObject->SetStringField(TEXT("include"), Settings->IncludePathsCsv);
    ScopeObject->SetStringField(TEXT("exclude"), Settings->ExcludePathsCsv);
    JsonObject->SetObjectField(TEXT("scope"), ScopeObject);

    // Safety
    TSharedPtr<FJsonObject> SafetyObject = MakeShareable(new FJsonObject);
    SafetyObject->SetBoolField(TEXT("dry_run"), Settings->bDryRun);
    SafetyObject->SetIntegerField(TEXT("max_changes"), Settings->MaxChangesPerRun);
    SafetyObject->SetBoolField(TEXT("auto_checkout"), Settings->bAutoCheckout);
    SafetyObject->SetBoolField(TEXT("close_open_editors"), Settings->bCloseOpenEditors);
    SafetyObject->SetBoolField(TEXT("verify_after_write"), Settings->bVerifyAfterWrite);
    SafetyObject->SetBoolField(TEXT("save_assets"), Settings->bSaveAssets);
    JsonObject->SetObjectField(TEXT("safety"), SafetyObject);

    // Gates
    TArray<TSharedPtr<FJsonValue>> ApplyOnlyArray;
    for (const FString& Key : Settings->ApplyOnlyKeys)
    {
        ApplyOnlyArray.Add(MakeShareable(new FJsonValueString(Key)));
    }
    TSharedPtr<FJsonObject> GatesObject = MakeShareable(new FJsonObject);
    GatesObject->SetBoolField(TEXT("conservative_mode"), Settings->bConservativeMode);
    GatesObject->SetArrayField(TEXT("apply_only"), ApplyOnlyArray);
    JsonObject->SetObjectField(TEXT("gates"), GatesObject);

    // Paths
    TSharedPtr<FJsonObject> PathsObject = MakeShareable(new FJsonObject);
    PathsObject->SetStringField(TEXT("project_dir"), FPaths::ProjectDir());
    PathsObject->SetStringField(TEXT("output_dir"), OutputDir);
    JsonObject->SetObjectField(TEXT("paths"), PathsObject);

    // Write JSON to config path
    FString JsonStr;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    FString Error;
    if (!FFileHelper::SaveStringToFile(*JsonStr, *ConfigPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to write config JSON"));
        return;
    }

    // Execute each phase for selected categories
    for (const FString& Category : Categories)
    {
        ExecutePhase(ConfigPath, Phase, Category);
    }
}

FString UOptimizerRun::NewRunId()
{
    FDateTime Now = FDateTime::UtcNow();
    return Now.ToString(TEXT("%Y%m%d_%H%M%S"));
}

void UOptimizerRun::WriteConfigJson(const FString& ConfigPath)
{
    // This is handled in the Run function already.
}

void UOptimizerRun::ExecutePhase(const FString& JsonPath, const FString& Phase, const FString Category)
{
    FString StdErr;
    bool bSuccess = UPythonBridge::RunPythonTool(JsonPath, Phase, Category, StdErr);
    
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to execute phase %s for category %s: %s"), *Phase, *Category, *StdErr);
    }
}
Python Integration
Entry Point
entry.py

Apply
import os
import json
from pathlib import Path

# Import category modules
from . import io_csv
from .utils import resolve_enum
from .textures import audit as textures_audit, recommend as textures_recommend, apply as textures_apply, verify as textures_verify
from .meshes import audit as meshes_audit, recommend as meshes_recommend, apply as meshes_apply, verify as meshes_verify

def run(config_path, phase, category):
    with open(config_path, 'r') as f:
        config = json.load(f)
    
    output_dir = config['paths']['output_dir']
    os.makedirs(output_dir, exist_ok=True)

    if category == "Textures":
        if phase == "audit":
            return textures_audit.run(config, output_dir)
        elif phase == "recommend":
            return textures_recommend.run(config, output_dir)
        elif phase == "apply":
            return textures_apply.run(config, output_dir)
        elif phase == "verify":
            return textures_verify.run(config, output_dir)
    elif category == "Meshes":
        if phase == "audit":
            return meshes_audit.run(config, output_dir)
        elif phase == "recommend":
            return meshes_recommend.run(config, output_dir)
        elif phase == "apply":
            return meshes_apply.run(config, output_dir)
        elif phase == "verify":
            return meshes_verify.run(config, output_dir)

    return {
        "phase": phase,
        "category": category,
        "scanned": 0,
        "changed": 0,
        "skipped": 0,
        "errors": 0,
        "artifacts": {},
        "summary": {}
    }
CSV Utilities
io_csv.py

Apply
import csv
import os
from pathlib import Path

def read_csv(file_path, encoding='utf-8'):
    if not os.path.exists(file_path):
        return []
    
    data = []
    try:
        with open(file_path, 'r', newline='', encoding=encoding) as csvfile:
            reader = csv.DictReader(csvfile)
            for row in reader:
                data.append(row)
    except Exception as e:
        print(f"Error reading CSV {file_path}: {e}")
        return []
    
    return data

def write_csv(file_path, fieldnames, rows, encoding='utf-8'):
    os.makedirs(os.path.dirname(file_path), exist_ok=True)
    
    try:
        with open(file_path, 'w', newline='', encoding=encoding) as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(rows)
    except Exception as e:
        print(f"Error writing CSV {file_path}: {e}")

def validate_csv_header(file_path, expected_fields):
    if not os.path.exists(file_path):
        return False
    
    try:
        with open(file_path, 'r', newline='', encoding='utf-8') as csvfile:
            reader = csv.reader(csvfile)
            header = next(reader, [])
            return set(header) >= set(expected_fields)
    except Exception:
        return False

def get_csv_field_value(row, field_name, default=None):
    return row.get(field_name, default)
Utils
utils.py

Apply
import os
from pathlib import Path

def resolve_enum(enum_cls, name, default=None):
    key = str(name).lower().replace("_","")
    for attr in dir(enum_cls):
        if not attr.startswith("__"):
            if attr.lower().replace("_","")==key:
                return getattr(enum_cls, attr)
    return default

def get_project_root():
    return os.getcwd()

def ensure_directory_exists(path):
    os.makedirs(path, exist_ok=True)

def format_path_for_unreal(path):
    formatted = path.replace('\\', '/')
    return formatted

def get_file_size(file_path):
    try:
        return os.path.getsize(file_path)
    except:
        return 0

def is_valid_asset_path(path):
    return path and path.startswith('/Game/') and not path.endswith('/')
UI Components
SOptimizerPanel
SOptimizerPanel.h

Create file
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SOptimizerPanel : public SCompoundWidget
{
    SLATE_BEGIN_ARGS(SOptimizerPanel) {}
    SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs);

private:
    FReply OnAuditClicked();
    FReply OnApplyClicked();

    ECheckBoxState IsTexturesChecked() const;
    void OnTexturesChanged(ECheckBoxState NewState);

    ECheckBoxState IsMeshesChecked() const;
    void OnMeshesChanged(ECheckBoxState NewState);

    ECheckBoxState IsMaterialsChecked() const;
    void OnMaterialsChanged(ECheckBoxState NewState);

    ECheckBoxState IsLevelsChecked() const;
    void OnLevelsChanged(ECheckBoxState NewState);

    ECheckBoxState IsUseSelectionChecked() const;
    void OnUseSelectionChanged(ECheckBoxState NewState);

    FString GetIncludePaths() const;
    void OnIncludePathsChanged(const FText& NewText);

    FString GetExcludePaths() const;
    void OnExcludePathsChanged(const FText& NewText);

    TSharedRef<SWidget> OnGenerateProfileComboItem(FString Item) const;
    void OnProfileSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
    FText GetCurrentProfileText() const;

    TSharedRef<SWidget> OnGenerateRunModeComboItem(FString Item) const;
    void OnRunModeSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
    FText GetCurrentRunModeText() const;

private:
    UOptimizerSettings* OptimizerSettings;
    TArray<FString> TargetProfiles;
    TArray<FString> RunModes;
};
SOptimizerPanel.cpp

Apply
#include "SOptimizerPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SVerticalBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Framework/Notifications/SNotificationList.h"
#include "EditorStyleSet.h"
#include "OptimizerSettings.h"

void SOptimizerPanel::Construct(const F






keep going


Agent

Qwen 2.5 Coder 32b
Alt⏎ Active file
⏎ Enter
Certainly! Let's continue implementing the SOptimizerPanel class and ensure it integrates well with the rest of the plugin.

SOptimizerPanel Implementation
Header File (SOptimizerPanel.h)
We'll keep the header file mostly as is, ensuring all necessary functions are declared.

SOptimizerPanel.h

Create file
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SOptimizerPanel : public SCompoundWidget
{
    SLATE_BEGIN_ARGS(SOptimizerPanel) {}
    SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs);

private:
    FReply OnAuditClicked();
    FReply OnApplyClicked();

    ECheckBoxState IsTexturesChecked() const;
    void OnTexturesChanged(ECheckBoxState NewState);

    ECheckBoxState IsMeshesChecked() const;
    void OnMeshesChanged(ECheckBoxState NewState);

    ECheckBoxState IsMaterialsChecked() const;
    void OnMaterialsChanged(ECheckBoxState NewState);

    ECheckBoxState IsLevelsChecked() const;
    void OnLevelsChanged(ECheckBoxState NewState);

    ECheckBoxState IsUseSelectionChecked() const;
    void OnUseSelectionChanged(ECheckBoxState NewState);

    FString GetIncludePaths() const;
    void OnIncludePathsChanged(const FText& NewText);

    FString GetExcludePaths() const;
    void OnExcludePathsChanged(const FText& NewText);

    TSharedRef<SWidget> OnGenerateProfileComboItem(FString Item) const;
    void OnProfileSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
    FText GetCurrentProfileText() const;

    TSharedRef<SWidget> OnGenerateRunModeComboItem(FString Item) const;
    void OnRunModeSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
    FText GetCurrentRunModeText() const;

private:
    UOptimizerSettings* OptimizerSettings;
    TArray<FString> TargetProfiles;
    TArray<FString> RunModes;
};
Source File (SOptimizerPanel.cpp)
Let's complete the implementation of SOptimizerPanel.

SOptimizerPanel.cpp

Apply
#include "SOptimizerPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SVerticalBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Framework/Notifications/SNotificationList.h"
#include "EditorStyleSet.h"
#include "OptimizerSettings.h"

void SOptimizerPanel::Construct(const FArguments& InArgs)
{
    OptimizerSettings = UOptimizerSettings::Get();

    // Initialize profiles
    TargetProfiles.Add(TEXT("PC_High"));
    TargetProfiles.Add(TEXT("Console"));
    TargetProfiles.Add(TEXT("Mobile_AR"));
    TargetProfiles.Add(TEXT("VR"));
    TargetProfiles.Add(TEXT("VirtualProduction"));
    TargetProfiles.Add(TEXT("Cinematic"));
    TargetProfiles.Add(TEXT("Authoring_Safe"));

    RunModes.Add(TEXT("Audit"));
    RunModes.Add(TEXT("Apply"));

    ChildSlot
    [
        SNew(SVerticalBox)

        // Header row
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.0f)
        [
            SNew(SGridPanel)
            .FillColumn(1, 1.0f)

            + SGridPanel::Slot(0, 0)
            .Padding(2.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Target Profile")))
            ]
            
            + SGridPanel::Slot(1, 0)
            .Padding(2.0f)
            [
                SNew(SComboBox<FString>)
                .OptionsSource(&TargetProfiles)
                .OnGenerateWidget(this, &SOptimizerPanel::OnGenerateProfileComboItem)
                .OnSelectionChanged(this, &SOptimizerPanel::OnProfileSelected)
                .Content()
                [
                    SNew(STextBlock)
                    .Text(this, &SOptimizerPanel::GetCurrentProfileText)
                ]
            ]

            + SGridPanel::Slot(0, 1)
            .Padding(2.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Categories")))
            ]
            
            + SGridPanel::Slot(1, 1)
            .Padding(2.0f)
            [
                SNew(SUniformGridPanel)
                .SlotPadding(4.0f)

                + SUniformGridPanel::Slot(0, 0)
                [
                    SNew(SCheckBox)
                    ..IsChecked(this, &SOptimizerPanel::IsTexturesChecked)
                    .OnCheckStateChanged(this, &SOptimizerPanel::OnTexturesChanged)
                    .Content()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Textures")))
                    ]
                ]

                + SUniformGridPanel::Slot(1, 0)
                [
                    SNew(SCheckBox)
                    ..IsChecked(this, &SOptimizerPanel::IsMeshesChecked)
                    .OnCheckStateChanged(this, &SOptimizerPanel::OnMeshesChanged)
                    .Content()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Meshes")))
                    ]
                ]

                + SUniformGridPanel::Slot(0, 1)
                [
                    SNew(SCheckBox)
                    ..IsChecked(this, &SOptimizerPanel::IsMaterialsChecked)
                    .OnCheckStateChanged(this, &SOptimizerPanel::OnMaterialsChanged)
                    .Content()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Materials")))
                    ]
                ]

                + SUniformGridPanel::Slot(1, 1)
                [
                    SNew(SCheckBox)
                    ..IsChecked(this, &SOptimizerPanel::IsLevelsChecked)
                    .OnCheckStateChanged(this, &SOptimizerPanel::OnLevelsChanged)
                    .Content()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Levels")))
                    ]
                ]
            ]

            + SGridPanel::Slot(0, 2)
            .Padding(2.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Scope")))
            ]
            
            + SGridPanel::Slot(1, 2)
            .Padding(2.0f)
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(SCheckBox)
                    ..IsChecked(this, &SOptimizerPanel::IsUseSelectionChecked)
                    .OnCheckStateChanged(this, &SOptimizerPanel::OnUseSelectionChanged)
                    .Content()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Use Selection")))
                    ]
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(SEditableTextBox)
                    .Text(this, &SOptimizerPanel::GetIncludePaths)
                    .OnTextChanged(this, &SOptimizerPanel::OnIncludePathsChanged)
                    .HintText(FText::FromString(TEXT("Include paths (comma separated)")))
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(SEditableTextBox)
                    .Text(this, &SOptimizerPanel::GetExcludePaths)
                    .OnTextChanged(this, &SOptimizerPanel::OnExcludePathsChanged)
                    .HintText(FText::FromString(TEXT("Exclude paths (comma separated)")))
                ]
            ]

            + SGridPanel::Slot(0, 3)
            .Padding(2.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Run Mode")))
            ]
            
            + SGridPanel::Slot(1, 3)
            .Padding(2.0f)
            [
                SNew(SComboBox<FString>)
                .OptionsSource(&RunModes)
                .OnGenerateWidget(this, &SOptimizerPanel::OnGenerateRunModeComboItem)
                .OnSelectionChanged(this, &SOptimizerPanel::OnRunModeSelected)
                .Content()
                [
                    SNew(STextBlock)
                    .Text(this, &SOptimizerPanel::GetCurrentRunModeText)
                ]
            ]
        ]

        // Main content area
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(4.0f)
        [
            SNew(SScrollBox)

            + SScrollBox::Slot()
            .Padding(4.0f)
            [
                SNew(SVerticalBox)

                // Textures accordion
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Textures")))
                ]

                // Meshes accordion
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Meshes")))
                ]

                // Materials accordion
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Materials")))
                ]

                // Levels accordion
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Levels")))
                ]
            ]
        ]

        // Footer buttons
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.0f)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            .HAlign(HAlign_Left)
            .Padding(4.0f)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Run Audit")))
                .OnClicked(this, &SOptimizerPanel::OnAuditClicked)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .HAlign(HAlign_Left)
            .Padding(4.0f)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Apply")))
                .OnClicked(this, &SOptimizerPanel::OnApplyClicked)
            ]

            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .HAlign(HAlign_Right)
            .Padding(4.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Last run: 0 assets changed")))
            ]
        ]
    ];
}

FReply SOptimizerPanel::OnAuditClicked()
{
    // This would call the Python audit function
    FString Phase = TEXT("audit");
    TArray<FString> Categories;
    if (IsTexturesChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Textures"));
    }
    if (IsMeshesChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Meshes"));
    }
    if (IsMaterialsChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Materials"));
    }
    if (IsLevelsChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Levels"));
    }

    UOptimizerRun* OptimizerRun = NewObject<UOptimizerRun>();
    OptimizerRun->Run(Phase, Categories);

    FNotificationInfo Info(FText::FromString(TEXT("Audit started...")));
    Info.bUseSuccessFailIcons = true;
    Info.ExpireDuration = 3.0f;
    FSlateNotificationManager::Get().AddNotification(Info);
    
    return FReply::Handled();
}

FReply SOptimizerPanel::OnApplyClicked()
{
    // This would call the Python apply function
    FString Phase = TEXT("apply");
    TArray<FString> Categories;
    if (IsTexturesChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Textures"));
    }
    if (IsMeshesChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Meshes"));
    }
    if (IsMaterialsChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Materials"));
    }
    if (IsLevelsChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Levels"));
    }

    UOptimizerRun* OptimizerRun = NewObject<UOptimizerRun>();
    OptimizerRun->Run(Phase, Categories);

    FNotificationInfo Info(FText::FromString(TEXT("Apply started...")));
    Info.bUseSuccessFailIcons = true;
    Info.ExpireDuration = 3.0f;
    FSlateNotificationManager::Get().AddNotification(Info);
    
    return FReply::Handled();
}

ECheckBoxState SOptimizerPanel::IsTexturesChecked() const
{
    return (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Textures) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnTexturesChanged(ECheckBoxState NewState)
{
    if (NewState == ECheckBoxState::Checked)
    {
        OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Textures;
    }
    else
    {
        OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Textures);
    }
}

ECheckBoxState SOptimizerPanel::IsMeshesChecked() const
{
    return (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Meshes) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnMeshesChanged(ECheckBoxState NewState)
{
    if (NewState == ECheckBoxState::Checked)
    {
        OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Meshes;
    }
    else
    {
        OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Meshes);
    }
}

ECheckBoxState SOptimizerPanel::IsMaterialsChecked() const
{
    return (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Materials) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnMaterialsChanged(ECheckBoxState NewState)
{
    if (NewState == ECheckBoxState::Checked)
    {
        OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Materials;
    }
    else
    {
        OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Materials);
    }
}

ECheckBoxState SOptimizerPanel::IsLevelsChecked() const
{
    return (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Levels) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnLevelsChanged(ECheckBoxState NewState)
{
    if (NewState == ECheckBoxState::Checked)
    {
        OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Levels;
    }
    else
    {
        OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Levels);
    }
}

ECheckBoxState SOptimizerPanel::IsUseSelectionChecked() const
{
    return OptimizerSettings->bUseSelection ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnUseSelectionChanged(ECheckBoxState NewState)
{
    OptimizerSettings->bUseSelection = (NewState == ECheckBoxState::Checked);
}

FString SOptimizerPanel::GetIncludePaths() const
{
    return OptimizerSettings->IncludePathsCsv;
}

void SOptimizerPanel::OnIncludePathsChanged(const FText& NewText)
{
    OptimizerSettings->IncludePathsCsv = NewText.ToString();
}

FString SOptimizerPanel::GetExcludePaths() const
{
    return OptimizerSettings->ExcludePathsCsv;
}

void SOptimizerPanel::OnExcludePathsChanged(const FText& NewText)
{
    OptimizerSettings->ExcludePathsCsv = NewText.ToString();
}

TSharedRef<SWidget> SOptimizerPanel::OnGenerateProfileComboItem(FString Item) const
{
    return SNew(STextBlock)
    .Text(FText::FromString(Item));
}

void SOptimizerPanel::OnProfileSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
    if (SelectedItem.IsValid())
    {
        OptimizerSettings->TargetProfile = *SelectedItem;
    }
}

FText SOptimizerPanel::GetCurrentProfileText() const
{
    return FText::FromString(OptimizerSettings->TargetProfile);
}

TSharedRef<SWidget> SOptimizerPanel::OnGenerateRunModeComboItem(FString Item) const
{
    return SNew(STextBlock)
    .Text(FText::FromString(Item));
}

void SOptimizerPanel::OnRunModeSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
    if (SelectedItem.IsValid())
    {
        // Currently not used but can be added for future enhancements
    }
}

FText SOptimizerPanel::GetCurrentRunModeText() const
{
    return FText::FromString(TEXT("Audit")); // Default to Audit mode
}
Testing Strategy
Unit Tests
Texture Analysis: Test texture compression, LOD settings, and virtual texture handling.
Mesh Analysis: Test LOD generation, Nanite toggling, and collision simplification.
Material Analysis: Test normal map conversions, static switch cleanup, and sampler consolidation.
Runtime Settings: Test scalability buckets, shadow maps, post-processing quality adjustments.
Integration Tests
End-to-end Optimization: Run full optimization cycles for all categories.
Python Bridge Stability: Ensure the bridge handles different Python scripts and data.
UI Responsiveness: Validate that UI elements update correctly during operations.
Safety Mechanisms: Test dry-run modes, max-changes caps, and revert snapshots.
Performance Benchmarks
Optimization Speed: Measure time taken for audit, recommend, apply, and verify phases.
Memory Usage: Monitor memory consumption during optimization processes.
Regression Testing: Ensure that optimizations do not introduce regressions in asset quality or performance.
Performance Optimization
Incremental Scans: Implement incremental scans to reduce processing time for unchanged assets.
Batch Processing: Use batch operations for large datasets to improve performance.
Threading and Asynchronous Loading: Leverage Unreal Engine's threading capabilities for parallel processing.
Safety Implementation
Error Handling: Robust error handling with proper logging for all critical operations.
Verification Systems: Implement verification systems to ensure correctness after optimizations.
Revert Mechanisms: Provide full revert snapshots and history tracking to undo changes if necessary.
Conclusion
This detailed implementation plan provides a comprehensive roadmap for developing the "MagicOptimizer" plugin. Each phase focuses on specific tasks, ensuring that all critical components are covered. By following this plan, you can build a robust and efficient asset optimization tool for Unreal Engine

Optimizor 1.0 — Action Catalogue (UE 5.6) – 13/08/25
A chaptered, concrete checklist of everything the plugin can do to a project, split by domain. Each action is labeled by Mode:
•	AUTO — performed automatically by the plugin (with Dry Run and caps)
•	SEMI — offered with a one click wizard/confirm (bulk but user approved)
•	ADVISE — best practice surfaced in UI/report (not changed by code)
All AUTO actions respect: Dry Run, Max Changes, Selection vs Paths, Exclusions, Skip Collection, close editors, source control checkout, verify after write, and history snapshot.
________________________________________
1) Textures — Concrete Actions
1.1 Classification & Inference
•	AUTO Detect texture role via name/path tags (normal/base/masks/grayscale/UI/LUT/packed/UDIM).
•	AUTO Infer usage from referencing materials (up to N referencers) and map to sampler buckets (normal/masks/grayscale/linearcolor/color).
•	ADVISE Flag ambiguous/contradictory usage (same texture sampled as color & linear in different materials).
1.2 Format & Color Pipeline
•	AUTO Set CompressionSettings per class: TC_Normalmap for normals, TC_Masks for packed masks, TC_Grayscale for scalar maps, TC_Default (or TC_BC7 when preset allows) for color.
•	AUTO Set sRGB on/off appropriately (off for normals/masks/linear, on for color/UI).
•	SEMI Project level remap table (e.g., force BC7 for hero albedos on PC Ultra).
•	ADVISE Report alpha channel waste (normals with unused alpha, color with solid alpha).
1.3 Mipmaps & LOD
•	AUTO Toggle GenerateMips and set MipGenSettings (FromGroup / SharpenX / BlurX / NoMipmaps).
•	AUTO Enforce minimum LODBias (e.g., ≥ 1) except UI/LUTs/small detail maps.
•	SEMI Clamp MaxTextureSize per role (color/normal/mask caps); offer resample pass for oversize assets.
•	ADVISE Highlight textures lacking mips that are sampled minified (aliasing risk).
1.4 Streaming & VT
•	AUTO Set NeverStream=true for UI/LUTs; ensure world content streams.
•	AUTO Respect landscapes/UDIMs: don’t disable VT there; otherwise allow force disable VT when preset requires.
•	SEMI Adjust Texture Groups to preset (UI, World, LUT); optional per group max size overrides.
•	ADVISE Recommend Virtual Texturing for giant tileables on high end targets.
1.5 Safety, Exceptions & Scope
•	AUTO Skip assets by path/name hints and Skip Collection.
•	AUTO Close editors before mutation; attempt source control checkout; verify after write.
•	SEMI One click Revert Last Run using snapshot JSON.
1.6 Reporting & Budgeting
•	AUTO Report before/after settings, top change reasons, CSV of changes, memory bytes estimate, and create Collections: Optimizor_Changed_*, Optimizor_Skipped_*.
•	ADVISE Texel density and atlas suggestions (when we can’t change source art).
________________________________________
2) Static Meshes — Concrete Actions
2.1 Nanite & Build Settings
•	AUTO Enable/disable Nanite per preset; set fallback screen size.
•	SEMI Batch Build Nanite for eligible meshes; skip skeletal/unsupported.
•	ADVISE Suggest Nanite for CAD/high poly imports.
2.2 LOD Strategy
•	AUTO Generate LODs with IMeshReduction where missing; set LOD count from tri budget.
•	AUTO Apply screensize curve per preset; optionally enforce triangle caps for L1/L2/L3.
•	SEMI Respect user LODGroups if specified; otherwise apply preset caps.
•	ADVISE Warn on excessive materials per mesh (draw call multiplier).
2.3 Lightmap UVs & Tangents
•	AUTO Generate/repair Lightmap UVs (min/max res, channel selection).
•	AUTO Recompute normals/tangents per project policy (angle threshold).
•	ADVISE Flag overlapping lightmaps, extreme texel densities.
2.4 Collision & Complexity
•	AUTO Set simple collision type (box/sphere/convex) for props missing collision.
•	SEMI Replace complex as simple with simple for non interactive assets.
•	ADVISE List high poly colliders; promote UCX authoring tips.
2.5 Stats & Collections
•	AUTO CSV per mesh: base tris, per LOD tris, materials, Nanite on/off, lightmap res.
•	SEMI Create Changed/Needs Attention Collections (e.g., “No LODs”, “>X materials”).
________________________________________
3) Skeletal Meshes & Animation — Concrete Actions
•	AUTO Apply Animation Compression settings per preset (bitwise/BVH/ACL if available).
•	AUTO Enable Update Rate Optimization on components for distant actors.
•	SEMI Strip/add LODs on skeletal meshes (bone reduction where rules allow).
•	ADVISE Cap bones/influences for crowds; share anim instances; reduce tick rate off screen.
________________________________________
4) Materials — Concrete Actions
•	AUTO Detect & remove dead static switches (compile time only) in dry run preview → confirm → apply.
•	AUTO Identify duplicate texture samplers with identical params; suggest consolidation (SEMI to auto replace).
•	SEMI Normalize normal map space naming (_ogl/_dx) → add note or switch sampler type where safe.
•	ADVISE Flag high instruction count, heavy translucency, per pixel world position ops.
________________________________________
5) Runtime (Scalability, Post, Lighting) — Concrete Actions
5.1 Scalability & Device Profiles
•	AUTO Apply per preset Scalability buckets (View/Shadow/Effects/Post/Foliage) via .ini.
•	SEMI Switch Device Profiles (Desktop/Console/Mobile/VR) and copy tuned overrides.
5.2 Global Illumination & Reflections
•	SEMI Toggle Lumen GI/Reflections on/off; choose HWRT vs Software; set quality levels.
•	ADVISE Fall back to baked/static for mobile/archviz presets when appropriate.
5.3 Shadows
•	AUTO Enable/disable Virtual Shadow Maps; adjust distances/cascades; set per quality defaults.
•	ADVISE Reduce shadowed lights count; trim attenuation radii; prefer static for scenery.
5.4 Post Processing & AA
•	AUTO Configure AA (TSR/TemporalAA/MSAA for VR) per preset; dial bloom/AO/DOF/motion blur.
•	SEMI Enable Dynamic Resolution with bounds for consoles/PC Balanced.
5.5 Streaming & Async Loading
•	AUTO Set r.Streaming.PoolSize hints; configure time slice for async loading.
•	SEMI Enable Fast Geometry Streaming plugin and defaults for large worlds.
•	ADVISE Organize streaming levels; avoid heavy blocking loads.
________________________________________
6) World Partition, HLOD & Culling — Concrete Actions
•	SEMI Enable World Partition on new projects; set cell size templates (open world vs interior).
•	AUTO Generate HLOD clusters with preset reduction; build proxies.
•	AUTO Inject default Cull Distance Volume rules for common small props.
•	ADVISE Place streaming volumes/triggers; cap on screen actor counts.
________________________________________
7) Audio — Concrete Actions
•	AUTO Apply platform appropriate compression settings to SoundWaves (where unset).
•	SEMI Set concurrency/priority limits per preset (mobile lower caps).
•	ADVISE Stream long music; reduce simultaneous ambient loops.
________________________________________
8) Packaging, Cooking & Size — Concrete Actions
•	SEMI Toggle PAK compression on/off (boot time vs disk size trade).
•	AUTO Exclude Editor only content from cook; strip debug content by pattern.
•	SEMI Set platform target texture formats (ASTC/ETC/BCx) and shader pipeline caches.
•	ADVISE Preload hot levels; warm shaders; keep startup maps minimal.
________________________________________
9) Safety & Governance — Concrete Actions
•	AUTO Dry Run default; enforce Max Changes; per change gate via Apply Only keys.
•	AUTO Source control checkout; close editors; verify after write; save dirty packages.
•	SEMI One click Revert using last snapshot; export/import action plan JSON.
•	ADVISE Educate on content guidelines (naming, foldering, tags for rules).
________________________________________
10) Reporting & Telemetry — Concrete Actions
•	AUTO Write TXT/CSV/JSON reports with counts, paths, reasons, memory deltas; emit compact summary to log.
•	AUTO Create Collections for Changed/Skipped/Hot Referencers; open report folder.
•	SEMI Baseline capture: run stat unit/gpu & MemReport and store next to audit.
•	ADVISE Budget dashboards per map (future module).
________________________________________
11) Presets — Concrete Actions
•	AUTO Load preset .json; merge project overrides; display deltas in UI.
•	AUTO Apply preset to Texture/Mesh/Material/Runtime/World rules in one pass.
•	SEMI Duplicate + edit preset into project config; lock presets for CI.
•	ADVISE Per preset best practice tips inline (what engine can’t change).
________________________________________
12) Extensibility Hooks
•	SEMI Per team rule packs (regex/tag → action) for custom pipelines.
•	SEMI CLI/headless mode for CI (cook guards, budget checks).
•	ADVISE Plugin API for third party reducers (Simplygon) and analytics.
________________________________________
13) Coverage Map (AUTO vs SEMI vs ADVISE)
Domain	AUTO	SEMI	ADVISE
Textures	Compression, sRGB, mips, LODBias, VT off/on (safe cases), groups, reporting	Max size resample, group remap	Texel density, atlas, authoring
Static Mesh	Nanite toggle, LOD gen, screen sizes, lightmap UVs, simple collision, stats	Triangle caps, respect LODGroup, complex→simple collision	Materials per mesh, CAD prep
Skeletal	Anim compression, Update Rate Opt	LOD/bone reduction	Crowd budgets, shared BP
Materials	Dead static switches removal, sampler duplicates report	Auto replace duplicate samplers	Shader complexity guidance
Runtime	Scalability ini, VSM, AA/post defaults, streaming pool/time slice	Lumen mode, device profile, Dynamic Res, Fast Geo Streaming	Lighting strategy per platform
World	HLOD gen, cull distance defaults	Enable WP, cell sizes	Streaming layout, actor budgets
Audio	Compression defaults	Concurrency caps	Mix/atten rules
Packaging	Cook strip editor data	Pak compression, formats	Startup flow, shader warmup
Safety & Reports	Dry run, caps, verify, snapshots, collections	Revert, baseline capture	Governance tips
________________________________________
14) Next Step — Implementation Drill Down Index
For each chapter above, we will create two sub documents:
1.	User/Plugin Controls — exact UI toggles, preset bindings, safe defaults.
2.	Code Path — module, APIs, property writes, .ini keys, Python/C++ flows, failure modes.
This Action Catalogue is the canonical scope; we’ll implement and verify chapter by chapter.


