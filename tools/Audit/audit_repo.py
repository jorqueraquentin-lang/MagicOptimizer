#!/usr/bin/env python3
"""
Repo audit tool for MagicOptimizer (UE 5.6.1).

Scope: Read-only scanning, writes reports under Reports/Audit_<timestamp>/.
Safety: Excludes engine, intermediates, artifacts, and .git. No code changes.
"""
from __future__ import annotations

import os
import re
import sys
import json
import hashlib
import stat
import subprocess
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Tuple, Iterable, Set, Optional

REPO_ROOT = Path(__file__).resolve().parents[2]
REPORTS_ROOT = REPO_ROOT / 'Reports'

EXCLUDE_DIRS = {
    '.git', '.vs', '.idea', 'node_modules',
    'Artifacts', 'Binaries', 'Intermediate', 'DerivedDataCache', 'Saved',
}

HASH_MAX_BYTES = 512 * 1024 * 1024  # 512 MB
HASH_CHUNK = 1024 * 1024

TEXT_EXTS = {'.cs', '.cpp', '.h', '.hpp', '.txt', '.md', '.json', '.py', '.ini', '.uplugin', '.uproject', '.ps1', '.bat', '.sh', '.yaml', '.yml'}

EDITOR_INCLUDE_PAT = re.compile(r'#\s*include\s+[<\"](?=.*(UnrealEd|Editor|SlateCore|Slate|EditorStyle|ContentBrowser|AssetRegistry))[^"]+[>\"]', re.I)
WITH_EDITOR_PAT = re.compile(r'WITH_EDITOR')
NAME_HEUR_PATTERNS = [
    re.compile(r'GetName\s*\(\)\s*\.\s*StartsWith\s*\(', re.I),
    re.compile(r'GetPathName\s*\(', re.I),
    re.compile(r'AssetPath\s*\.\s*Contains\s*\(', re.I),
]


def is_excluded(path: Path) -> bool:
    parts = set(path.parts)
    return any(part in EXCLUDE_DIRS for part in parts)


def walk_files(root: Path) -> Iterable[Path]:
    for dirpath, dirnames, filenames in os.walk(root):
        # prune excluded dirs in-place
        dirnames[:] = [d for d in dirnames if d not in EXCLUDE_DIRS]
        for name in filenames:
            p = Path(dirpath) / name
            yield p


def file_hash_sha256(path: Path) -> Optional[str]:
    try:
        if path.stat().st_size > HASH_MAX_BYTES:
            return None
        h = hashlib.sha256()
        with path.open('rb') as f:
            while True:
                chunk = f.read(HASH_CHUNK)
                if not chunk:
                    break
                h.update(chunk)
        return h.hexdigest()
    except Exception:
        return None


def list_empty_folders(root: Path) -> List[str]:
    empties: List[str] = []
    for dirpath, dirnames, filenames in os.walk(root):
        rel = Path(dirpath).relative_to(root)
        if any(part in EXCLUDE_DIRS for part in rel.parts):
            continue
        if not dirnames and not filenames:
            empties.append(str(rel).replace('\\', '/'))
    return sorted(empties)


def list_dead_symlinks(root: Path) -> List[str]:
    dead: List[str] = []
    for p in walk_files(root):
        try:
            if p.is_symlink():
                target = os.readlink(p)
                if not os.path.exists(os.path.join(p.parent, target)):
                    dead.append(str(p.relative_to(root)).replace('\\', '/'))
        except OSError:
            continue
    return sorted(dead)


def parse_build_cs_dependencies(path: Path) -> Dict[str, List[str]]:
    txt = path.read_text(encoding='utf-8', errors='ignore')
    deps: Dict[str, List[str]] = {}
    for key in ('PublicDependencyModuleNames', 'PrivateDependencyModuleNames'):
        m = re.search(key + r'\.AddRange\(new\s*\[\]\s*\{([^}]*)\}', txt)
        if m:
            entries = [e.strip().strip('"') for e in m.group(1).split(',') if e.strip()]
            deps[key] = entries
    return deps


def scan_orphans_and_includes(root: Path) -> Tuple[List[str], List[str], List[Dict[str, str]]]:
    headers: Set[str] = set()
    included: Set[str] = set()
    public_with_editor: List[Dict[str, str]] = []
    for p in walk_files(root):
        if p.suffix.lower() in {'.h', '.hpp'}:
            headers.add(str(p))
            if 'Public' in p.parts:
                txt = p.read_text(encoding='utf-8', errors='ignore')
                if WITH_EDITOR_PAT.search(txt):
                    public_with_editor.append({"file": str(p.relative_to(root)), "match": "WITH_EDITOR"})
        if p.suffix.lower() in {'.cpp', '.h', '.hpp'}:
            txt = p.read_text(encoding='utf-8', errors='ignore')
            for inc in re.findall(r'#\s*include\s+[<\"]([^>\"]+)[>\"]', txt):
                included.add(inc.split('/')[-1])

    headers_never_included = []
    for h in headers:
        name = Path(h).name
        if name not in included:
            headers_never_included.append(str(Path(h).relative_to(root)))
    return sorted(headers_never_included), [], public_with_editor


def scan_runtime_editor_includes(runtime_src: Path) -> List[Dict[str, str]]:
    hits: List[Dict[str, str]] = []
    for p in walk_files(runtime_src):
        if p.suffix.lower() not in {'.h', '.hpp', '.cpp'}:
            continue
        txt = p.read_text(encoding='utf-8', errors='ignore')
        for m in EDITOR_INCLUDE_PAT.finditer(txt):
            hits.append({"file": str(p.relative_to(REPO_ROOT)), "include": m.group(0)})
    return hits


def grep_name_based_heuristics(root: Path) -> List[Dict[str, object]]:
    hits: List[Dict[str, object]] = []
    for p in walk_files(root):
        if p.suffix.lower() not in {'.h', '.hpp', '.cpp', '.py'}:
            continue
        txt = p.read_text(encoding='utf-8', errors='ignore')
        for pat in NAME_HEUR_PATTERNS:
            for m in pat.finditer(txt):
                # compute line number
                line_no = txt.count('\n', 0, m.start()) + 1
                snippet = txt[max(0, m.start()-40): m.end()+40].replace('\n', ' ')
                hits.append({"file": str(p.relative_to(REPO_ROOT)), "line": line_no, "snippet": snippet})
    hits.sort(key=lambda d: (d['file'], d['line']))
    return hits


def try_dry_build() -> Tuple[int, List[str]]:
    """Run a dry editor build to capture warnings. Return (exitcode, warnings)."""
    warnings: List[str] = []
    uproject = next(REPO_ROOT.glob('HostProject/*.uproject'), None)
    if not uproject:
        return (0, warnings)
    ubt = Path(os.environ.get('UE_UBT', r'C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe'))
    if not ubt.exists():
        return (0, warnings)
    cmd = [str(ubt), '-mode=Build', f'-project={str(uproject)}', '-target=HostProjectEditor Win64 Development', '-progress']
    try:
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding='utf-8')
        out = proc.stdout
        for line in out.splitlines():
            if 'warning:' in line.lower():
                warnings.append(line.strip())
        return (proc.returncode, warnings)
    except Exception:
        return (0, warnings)


def load_json(path: Path) -> Optional[dict]:
    try:
        return json.loads(path.read_text(encoding='utf-8', errors='ignore'))
    except Exception:
        return None


def gather_config_drift() -> Dict[str, object]:
    uplugin = next((REPO_ROOT / 'HostProject' / 'Plugins' / 'MagicOptimizer').glob('*.uplugin'), None)
    uproject = next((REPO_ROOT / 'HostProject').glob('*.uproject'), None)
    data = {"uplugin": {}, "uproject": {}, "inconsistencies": []}
    if uplugin:
        up = load_json(uplugin) or {}
        data["uplugin"] = up
    if uproject:
        pr = load_json(uproject) or {}
        data["uproject"] = pr
    # Simple checks
    ua = (data.get('uplugin') or {}).get('EngineAssociation')
    if ua != '5.6.1':
        data['inconsistencies'].append('Plugin EngineAssociation is not 5.6.1')
    return data


def list_git_tracked_binaries_and_large(repo_root: Path) -> Tuple[List[str], List[str], List[str]]:
    tracked: List[str] = []
    oversize: List[str] = []
    lfs_candidates: List[str] = []
    try:
        files = subprocess.check_output(['git', 'ls-files'], cwd=repo_root, text=True).splitlines()
    except Exception:
        files = []
    for rel in files:
        p = repo_root / rel
        if not p.exists():
            continue
        sz = p.stat().st_size
        if sz > 50 * 1024 * 1024:
            oversize.append(rel)
            if p.suffix.lower() not in TEXT_EXTS:
                lfs_candidates.append(rel)
        if p.suffix.lower() in {'.uasset', '.umap'}:
            tracked.append(rel)
    return sorted(tracked), sorted(oversize), sorted(set(lfs_candidates))


def detect_duplicates(file_info: List[Tuple[str, int, Optional[str]]]) -> List[Dict[str, object]]:
    clusters: Dict[str, List[str]] = {}
    for rel, size, h in file_info:
        if not h:
            continue
        clusters.setdefault(h, []).append(rel)
    dup_list = [{"hash": h, "files": sorted(paths)} for h, paths in clusters.items() if len(paths) > 1]
    dup_list.sort(key=lambda d: (-(len(d['files'])), d['hash']))
    return dup_list


def collect_scripts_and_tests(repo_root: Path) -> Dict[str, List[str]]:
    packaging = []
    for p in [repo_root / 'HostProject' / 'Scripts' / 'PackagePlugin.ps1', repo_root / 'HostProject' / 'Scripts' / 'PackagePlugin.bat']:
        if p.exists(): packaging.append(str(p.relative_to(repo_root)))
    rules = []
    rule_path = repo_root / 'HostProject' / '.cursor' / 'rules' / 'ue561-plugin-builder.mdc'
    if rule_path.exists(): rules.append(str(rule_path.relative_to(repo_root)))
    tests = [str(p.relative_to(repo_root)) for p in (repo_root / 'HostProject' / 'Plugins' / 'MagicOptimizer' / 'Source').glob('**/Tests/*.cpp')]
    return {"packaging_scripts": packaging, "cursor_rules": rules, "automation_tests": sorted(tests)}


def main() -> int:
    ts = datetime.now().strftime('%Y%m%d_%H%M%S')
    out_dir = REPORTS_ROOT / f'Audit_{ts}'
    out_dir.mkdir(parents=True, exist_ok=True)

    # Inventory
    file_info: List[Tuple[str, int, Optional[str]]] = []
    largest: List[Tuple[int, str]] = []
    for p in walk_files(REPO_ROOT):
        rel = str(p.relative_to(REPO_ROOT)).replace('\\', '/')
        try:
            st = p.stat()
        except FileNotFoundError:
            continue
        size = st.st_size
        h = file_hash_sha256(p)
        file_info.append((rel, size, h))
        if size > 0:
            largest.append((size, rel))
    total_size = sum(sz for _, sz, _ in file_info)
    largest.sort(reverse=True)
    largest_top = largest[:20]

    # empties/symlinks
    empties = list_empty_folders(REPO_ROOT)
    deadlinks = list_dead_symlinks(REPO_ROOT)

    # orphans and headers
    headers_never_included, uncompiled_sources, public_with_editor = scan_orphans_and_includes(REPO_ROOT)

    # module issues
    runtime_src = REPO_ROOT / 'HostProject' / 'Plugins' / 'MagicOptimizer' / 'Source' / 'MagicOptimizer'
    runtime_editor_includes = scan_runtime_editor_includes(runtime_src)

    # config drift
    config = gather_config_drift()

    # name-based heuristics evidence
    name_heur = grep_name_based_heuristics(REPO_ROOT)

    # dry build warnings
    exit_code, warnings = try_dry_build()

    # git hygiene
    tracked_artifacts, oversize_files, lfs_candidates = list_git_tracked_binaries_and_large(REPO_ROOT)

    # scripts and tests
    scripts_tests = collect_scripts_and_tests(REPO_ROOT)

    summary = {
        "total_files": len(file_info),
        "total_size": total_size,
        "largest_files": [{"size": s, "path": p} for s, p in largest_top],
        "warnings_count": len(warnings),
        "duplicates_count": 0,
        "empty_folders": len(empties),
        "dead_symlinks": len(deadlinks),
    }

    duplicates = detect_duplicates(file_info)
    summary["duplicates_count"] = len(duplicates)

    audit = {
        "summary": summary,
        "duplicates": duplicates,
        "empty_folders": empties,
        "dead_symlinks": deadlinks,
        "orphans": {"headers_never_included": headers_never_included, "uncompiled_sources": uncompiled_sources},
        "module_issues": {
            "runtime_editor_includes": runtime_editor_includes,
            "runtime_editor_deps": [],
            "public_with_editor": public_with_editor,
        },
        "config_drift": config,
        "git_hygiene": {
            "tracked_artifacts": tracked_artifacts,
            "oversize_files": oversize_files,
            "lfs_candidates": lfs_candidates,
        },
        "red_flags": {"name_based_asset_heuristics": name_heur},
        "scripts_and_tests": scripts_tests,
        "todo_fixme": [],
        "recommendations": [],
    }

    # TODO/FIXME counts
    todo_counts: Dict[str, int] = {}
    for p in walk_files(REPO_ROOT):
        if p.suffix.lower() in TEXT_EXTS:
            txt = p.read_text(encoding='utf-8', errors='ignore')
            cnt = txt.upper().count('TODO') + txt.upper().count('FIXME')
            if cnt:
                todo_counts[str(p.relative_to(REPO_ROOT))] = cnt
    audit['todo_fixme'] = sorted([{ 'file': k, 'count': v } for k, v in todo_counts.items()], key=lambda d: (-d['count'], d['file']))

    # recommendations (high-signal only)
    recs: List[Dict[str, str]] = []
    if config.get('inconsistencies'):
        recs.append({"severity": "High", "item": "EngineAssociation mismatch", "detail": "Plugin EngineAssociation should be 5.6.1."})
    if runtime_editor_includes:
        recs.append({"severity": "High", "item": "Editor includes in runtime", "detail": "Runtime code should not include editor-only headers; move into Editor module."})
    if oversize_files:
        recs.append({"severity": "Med", "item": "Oversize tracked files", "detail": "Consider Git LFS for large binaries over 50MB."})
    if duplicates:
        recs.append({"severity": "Low", "item": "Duplicate files", "detail": "Remove duplicated blobs to reduce repo size."})
    audit['recommendations'] = recs

    # Write JSON
    json_path = out_dir / 'audit.json'
    json_path.write_text(json.dumps(audit, indent=2), encoding='utf-8')

    # Write Markdown
    md = []
    md.append(f"# Workspace Audit ({ts})\n")
    md.append("## Executive Summary\n")
    md.append(f"- Total files: {summary['total_files']}\n")
    md.append(f"- Total size: {summary['total_size']} bytes\n")
    md.append(f"- Duplicates: {summary['duplicates_count']} clusters\n")
    md.append(f"- Empty folders: {summary['empty_folders']}\n")
    md.append(f"- Dead symlinks: {summary['dead_symlinks']}\n")
    md.append(f"- Build warnings: {summary['warnings_count']}\n")
    if recs:
        md.append("\n## High Priority\n")
        for r in recs:
            if r['severity'] == 'High':
                md.append(f"- {r['item']}: {r['detail']}\n")
    md.append("\n## Details\n")
    md.append("### Duplicates\n")
    for d in duplicates[:20]:
        md.append(f"- {d['hash']}: {len(d['files'])} files\n")
    md.append("\n### Config Drift\n")
    for inc in config.get('inconsistencies', []):
        md.append(f"- {inc}\n")
    md.append("\n### Module Issues\n")
    for hit in runtime_editor_includes[:20]:
        md.append(f"- {hit['file']}: {hit['include']}\n")
    md_path = out_dir / 'audit.md'
    md_path.write_text('\n'.join(md), encoding='utf-8')

    # Print short summary
    top = sorted(( (c['severity'], c['item']) for c in recs ), key=lambda x: x[0])[:5]
    print(f"Audit report written to: {md_path}")
    if top:
        print("Top issues:")
        for sev, item in top:
            print(f"- [{sev}] {item}")
    return 0


if __name__ == '__main__':
    sys.exit(main())


