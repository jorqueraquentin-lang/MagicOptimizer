#!/usr/bin/env python3
import os, sys, json, hashlib, subprocess, datetime, re
from pathlib import Path

EXCLUDES = {".git", ".cursor", ".idea", ".vs", "Artifacts", "Binaries", "Intermediate", "DerivedDataCache", "Saved", "node_modules"}
HASH_LIMIT_BYTES = 64 * 1024 * 1024   # 64 MB per file hashing cap
LFS_SOFT_LIMIT = 50 * 1024 * 1024
LFS_HARD_LIMIT = 100 * 1024 * 1024

def repo_root_from_git():
    try:
        out = subprocess.check_output(["git", "rev-parse", "--show-toplevel"], stderr=subprocess.DEVNULL, text=True).strip()
        p = Path(out)
        return p if p.exists() else None
    except Exception:
        return None

def resolve_repo_root():
    p = repo_root_from_git()
    if not p:
        # Tools/Audit/audit_repo.py -> repo root is two parents up by default
        p = Path(__file__).resolve().parents[2]
    return p.resolve()

def iter_files(root: Path):
    root = root.resolve()
    for cur_root, dirs, files in os.walk(root, topdown=True):
        # prune by exact dir name only
        dirs[:] = [d for d in dirs if d not in EXCLUDES]
        for name in files:
            yield Path(cur_root) / name

def sha256_for(path: Path, size: int):
    # do not hash huge files or UE binary assets
    if size > HASH_LIMIT_BYTES:
        return None
    suf = path.suffix.lower()
    if suf in {".uasset", ".umap"}:
        return None
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def read_json(path: Path):
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        return None

def scan(root: Path):
    files = []
    top_counts = {}
    empty_dirs = []

    # collect files and basic stats
    for p in iter_files(root):
        try:
            sz = p.stat().st_size
        except OSError:
            continue
        rel = p.relative_to(root)
        top = rel.parts[0] if len(rel.parts) > 0 else ""
        top_counts[top] = top_counts.get(top, 0) + 1
        files.append({"abs": str(p), "rel": str(rel).replace("\\", "/"), "size": sz})

    # empty folder detection
    for cur_root, dirs, file_names in os.walk(root, topdown=True):
        dirs[:] = [d for d in dirs if d not in EXCLUDES]
        if not dirs and not file_names:
            empty_dirs.append(str(Path(cur_root).relative_to(root)).replace("\\", "/"))

    # duplicates by content hash
    by_hash = {}
    for f in files:
        p = Path(f["abs"])
        digest = sha256_for(p, f["size"])
        if digest:
            by_hash.setdefault(digest, []).append(f["rel"])
    duplicates = [{"hash": k, "files": sorted(v)} for k, v in by_hash.items() if len(v) > 1]
    duplicates.sort(key=lambda d: (-len(d["files"]), d["hash"]))

    # large files and LFS candidates
    oversize = [f for f in files if f["size"] >= LFS_SOFT_LIMIT]
    blobs100 = [f for f in files if f["size"] >= LFS_HARD_LIMIT]

    # config drift
    uplugin = None
    for cand in list(Path(root, "Plugins").rglob("*.uplugin")):
        uplugin = cand
        break
    uproject = None
    for cand in list(root.rglob("*.uproject")):
        uproject = cand
        break

    uplugin_json = read_json(uplugin) if uplugin else None
    uproject_json = read_json(uproject) if uproject else None

    inconsistencies = []
    if uplugin_json:
        eng = uplugin_json.get("EngineAssociation")
        if eng != "5.6.1":
            inconsistencies.append(f"Plugin EngineAssociation is {eng}, expected 5.6.1")
        # module types sanity
        mods = {m.get("Name"): m.get("Type") for m in uplugin_json.get("Modules", [])}
        if mods and "MagicOptimizer" in mods and mods.get("MagicOptimizer") != "Runtime":
            inconsistencies.append("MagicOptimizer module should be Runtime")
        if mods and "MagicOptimizerEditor" in mods and mods.get("MagicOptimizerEditor") != "Editor":
            inconsistencies.append("MagicOptimizerEditor module should be Editor")
    if uproject_json and uplugin_json:
        pass  # nothing strict beyond hotfix pin for now

    # Build.cs scans and public header guards
    runtime_build = None
    editor_build = None
    for p in list(root.rglob("*.Build.cs")):
        txt = Path(p).read_text(encoding="utf-8", errors="ignore")
        if p.name.lower().startswith("magicoptimizereditor"):
            editor_build = txt
        if p.name.lower().startswith("magicoptimizer."):
            runtime_build = txt
    runtime_editor_deps = []
    if runtime_build:
        for token in ["UnrealEd", "Slate", "SlateCore", "ContentBrowser", "AssetRegistry"]:
            if re.search(rf'["\']{token}["\']', runtime_build):
                runtime_editor_deps.append(token)

    public_with_editor = []
    for p in list(Path(root, "Plugins").rglob("Source/MagicOptimizer/Public/**/*.h")):
        txt = Path(p).read_text(encoding="utf-8", errors="ignore")
        if "WITH_EDITOR" in txt:
            public_with_editor.append(str(p.relative_to(root)).replace("\\", "/"))

    # name based heuristics
    red_flags = []
    name_regex = re.compile(r"(GetName\s*\(|GetPathName\s*\(|StartsWith\s*\(|EndsWith\s*\()")
    for p in list(Path(root, "Plugins").rglob("Source/**/*.cpp")) + list(Path(root, "Plugins").rglob("Source/**/*.h")):
        txt = Path(p).read_text(encoding="utf-8", errors="ignore")
        for m in name_regex.finditer(txt):
            rel = str(Path(p).relative_to(root)).replace("\\", "/")
            line = txt.count("\n", 0, m.start()) + 1
            red_flags.append({"file": rel, "line": line, "snippet": txt[m.start():m.start()+60]})

    summary = {
        "total_files": len(files),
        "total_size_bytes": int(sum(f["size"] for f in files)),
        "top_counts": dict(sorted(top_counts.items())),
        "largest": sorted(files, key=lambda f: f["size"], reverse=True)[:10]
    }

    report = {
        "summary": summary,
        "duplicates": duplicates,
        "empty_folders": sorted(empty_dirs),
        "dead_symlinks": [],  # optional
        "orphans": {"headers_never_included": [], "uncompiled_sources": []},  # optional in this pass
        "module_issues": {
            "runtime_editor_deps": runtime_editor_deps,
            "public_with_editor": sorted(public_with_editor)
        },
        "config_drift": {
            "uplugin_path": str(uplugin) if uplugin else None,
            "uproject_path": str(uproject) if uproject else None,
            "inconsistencies": inconsistencies
        },
        "git_hygiene": {
            "oversize_files": sorted([{"rel": f["rel"], "size": f["size"]} for f in oversize], key=lambda x: -x["size"]),
            "lfs_candidates_over_100mb": sorted([{"rel": f["rel"], "size": f["size"]} for f in blobs100], key=lambda x: -x["size"])
        },
        "red_flags": {"name_based_asset_heuristics": red_flags[:200]},
        "scripts_and_tests": {},
        "todo_fixme": [],
        "recommendations": []
    }

    return files, report

def write_reports(root: Path, report: dict):
    ts = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    out_dir = root / "Reports" / f"Audit_{ts}"
    out_dir.mkdir(parents=True, exist_ok=True)
    # JSON
    (out_dir / "audit.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    # Markdown summary
    md = []
    s = report["summary"]
    md.append("# Audit summary")
    md.append(f"Total files: {s['total_files']}")
    md.append(f"Total size: {s['total_size_bytes']} bytes")
    md.append("")
    md.append("## High priority")
    for inc in report["config_drift"]["inconsistencies"]:
        md.append(f"- {inc}")
    if report["module_issues"]["runtime_editor_deps"]:
        md.append(f"- Runtime module depends on editor modules: {', '.join(report['module_issues']['runtime_editor_deps'])}")
    md_txt = "\n".join(md)
    (out_dir / "audit.md").write_text(md_txt, encoding="utf-8")
    return out_dir

def main():
    root = resolve_repo_root()
    print("Repo root =", root)
    print("Excluded dirs =", sorted(EXCLUDES))
    print("Top-level entries =", sorted([p.name for p in root.iterdir()]))

    files, report = scan(root)
    if report["summary"]["total_files"] < 20 or report["summary"]["total_size_bytes"] == 0:
        # print diagnostic sample
        sample = [f["rel"] for f in files[:50]]
        print("Guard trip. Sample files:", sample)
        # fallback quick list without excludes to help debug
        nf = []
        for cur_root, _, names in os.walk(root, topdown=True):
            for nm in names:
                nf.append(os.path.relpath(os.path.join(cur_root, nm), root))
                if len(nf) >= 50:
                    break
            if len(nf) >= 50:
                break
        print("Fallback sample no excludes:", nf)
        sys.exit(2)

    out_dir = write_reports(root, report)
    print("Report written to", out_dir)
    print("Executive summary")
    print((out_dir / "audit.md").read_text(encoding="utf-8").splitlines()[:10])

if __name__ == "__main__":
    main()
