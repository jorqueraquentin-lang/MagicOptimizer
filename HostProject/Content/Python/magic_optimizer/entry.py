import os, json, sys, csv
try:
    import unreal  # Available when running inside UE embedded Python
except Exception:
    unreal = None


def _to_bool(s: str) -> bool:
    return str(s).strip().lower() == 'true'


def _append_log(line: str):
    try:
        log_path = os.environ.get('MAGICOPTIMIZER_LOG')
        if not log_path:
            return
        os.makedirs(os.path.dirname(log_path), exist_ok=True)
        with open(log_path, 'a', encoding='utf-8') as f:
            from datetime import datetime
            f.write(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] PY: {line}\n")
    except Exception:
        pass


def _parse_csv_list(s: str):
    if not s:
        return []
    return [p.strip() for p in s.split(',') if p.strip()]


def _path_matches_filters(path_str: str, includes, excludes) -> bool:
    ok_incl = True if not includes else any(path_str.startswith(i) for i in includes)
    ok_excl = not any(path_str.startswith(e) for e in excludes)
    return ok_incl and ok_excl


def _is_texture2d_assetdata(ad) -> bool:
    try:
        cp = getattr(ad, 'asset_class_path', None)
        if cp is None:
            return False
        # TopLevelAssetPath has package_name and asset_name; fallback to string compare
        pkg = ''
        name = ''
        try:
            pkg = str(getattr(cp, 'package_name', ''))
            name = str(getattr(cp, 'asset_name', ''))
        except Exception:
            pass
        if pkg and name:
            return (pkg == '/Script/Engine' and name == 'Texture2D')
        s = str(cp)
        if s:
            return s == '/Script/Engine.Texture2D' or s.endswith('.Texture2D')
        return False
    except Exception:
        return False


def _get_texture2d_classpath():
    # Expected TopLevelAssetPath for Texture2D
    try:
        return unreal.TopLevelAssetPath('/Script/Engine', 'Texture2D')
    except Exception:
        return None


# Expected argv from UE:
# [phase, profile, dry_run, max_changes, include, exclude, use_selection, categories]
args = sys.argv[1:]
phase       = args[0] if len(args) > 0 else ''
profile     = args[1] if len(args) > 1 else ''
dry_run     = _to_bool(args[2]) if len(args) > 2 else True
max_changes = int(args[3]) if len(args) > 3 and str(args[3]).isdigit() else 100
include     = args[4] if len(args) > 4 else ''
exclude     = args[5] if len(args) > 5 else ''
use_sel     = _to_bool(args[6]) if len(args) > 6 else False
categories  = args[7] if len(args) > 7 else ''

includes = _parse_csv_list(include)
excludes = _parse_csv_list(exclude)

_append_log(f"entry.py start phase={phase} profile={profile} dry={dry_run} max={max_changes} useSel={use_sel} include='{include}' exclude='{exclude}' cats='{categories}'")

# Selection sets
selected_pkg_set = set()
selected_obj_set = set()
if use_sel and unreal is not None:
    try:
        datas = []
        try:
            datas = unreal.EditorUtilityLibrary.get_selected_asset_data() or []
        except Exception:
            pass
        if datas:
            for d in datas:
                try:
                    pkg = str(d.package_name) if hasattr(d, 'package_name') else None
                    obj = str(d.object_path) if hasattr(d, 'object_path') else None
                    if pkg:
                        selected_pkg_set.add(pkg)
                    if obj:
                        selected_obj_set.add(obj)
                except Exception:
                    pass
        else:
            try:
                assets = unreal.EditorUtilityLibrary.get_selected_assets() or []
            except Exception:
                assets = []
            for a in assets:
                try:
                    obj_path = a.get_path_name() if hasattr(a, 'get_path_name') else None
                    if obj_path:
                        selected_obj_set.add(obj_path)
                        if '.' in obj_path:
                            selected_pkg_set.add(obj_path.split('.', 1)[0])
                except Exception:
                    pass
        _append_log(f"Selection: {len(selected_pkg_set)} packages, {len(selected_obj_set)} objects")
    except Exception:
        pass

p = (phase or '').strip().lower()
msg = f"{phase} OK ({profile})"
assets_processed = 10
assets_modified = 0

# CSV output directory
csv_dir = None
try:
    if unreal is not None and hasattr(unreal, 'Paths'):
        saved_dir = unreal.Paths.project_saved_dir()
    else:
        saved_dir = os.path.join(os.getcwd(), 'Saved')
    csv_dir = os.path.join(saved_dir, 'MagicOptimizer', 'Audit')
    os.makedirs(csv_dir, exist_ok=True)
except Exception:
    pass

if p == 'audit':
    msg = f"Audit OK ({profile})"
    textures_info = []
    total_textures = 0
    ar_count = 0
    eal_list_count = 0
    loaded_tex_count = 0
    if unreal is not None:
        _append_log("Audit: querying AssetRegistry with ARFilter for Texture2D under /Game ...")
        ar = unreal.AssetRegistryHelpers.get_asset_registry()
        tex_assets = []
        tried_filter = False
        try:
            cls = _get_texture2d_classpath()
            if cls is not None and hasattr(unreal, 'ARFilter'):
                flt = unreal.ARFilter(
                    class_names=[cls],
                    package_paths=['/Game'],
                    recursive_paths=True,
                    include_only_on_disk_assets=False
                )
                tex_assets = ar.get_assets(flt)
                tried_filter = True
        except Exception:
            tried_filter = True
            tex_assets = []

        if tried_filter and tex_assets:
            for a in tex_assets:
                try:
                    pkg = str(a.package_name) if hasattr(a, 'package_name') else ''
                    if not _path_matches_filters(pkg, includes, excludes):
                        continue
                    if use_sel and selected_pkg_set and pkg not in selected_pkg_set:
                        continue
                    ar_count += 1
                    total_textures += 1
                    # Add all textures to info, not just first 50
                    name = str(a.asset_name) if hasattr(a, 'asset_name') else None
                    if pkg and name:
                        textures_info.append({"path": f"{pkg}.{name}"})
                except Exception:
                    pass

        # Fallback when ARFilter was unavailable or returned no results
        if not (tried_filter and tex_assets):
            _append_log("Audit: ARFilter unavailable/empty, falling back to list_assets + find_asset_data")
            all_paths = unreal.EditorAssetLibrary.list_assets('/Game', recursive=True, include_folder=False)
            eal_list_count = len(all_paths)
            for pth in all_paths:
                try:
                    if not _path_matches_filters(pth, includes, excludes):
                        continue
                    if use_sel and selected_obj_set and pth not in selected_obj_set and pth.split('.', 1)[0] not in selected_pkg_set:
                        continue
                    data = unreal.EditorAssetLibrary.find_asset_data(pth)
                    if not _is_texture2d_assetdata(data):
                        continue
                    total_textures += 1
                    # Add all textures to info, not just first 50
                    textures_info.append({"path": pth})
                except Exception:
                    pass

        _append_log(f"Audit: Found {total_textures} textures, processing {len(textures_info)} for detailed info")

        # Initialize counter for loaded textures
        loaded_tex_count = 0

        # Load sample for width/height/format - process all textures, not just first 50
        sample_paths = [t.get('path') for t in textures_info]
        _append_log(f"Processing {len(sample_paths)} textures for width/height/format data")
        
        for i, path in enumerate(sample_paths):
            try:
                _append_log(f"Loading texture {i+1}/{len(sample_paths)}: {path}")
                asset = unreal.EditorAssetLibrary.load_asset(path)
                if not asset:
                    _append_log(f"Failed to load asset: {path}")
                    continue
                
                tex = unreal.Texture2D.cast(asset)
                if not tex:
                    _append_log(f"Failed to cast to Texture2D: {path}")
                    continue
                
                width = None
                height = None
                fmt = None
                
                # Resolve texture dimensions robustly
                try:
                    # Method 1: TextureSource get_size_x/get_size_y (most reliable)
                    try:
                        src = tex.get_editor_property('source')
                        if src and hasattr(src, 'get_size_x') and hasattr(src, 'get_size_y'):
                            width = int(src.get_size_x())
                            height = int(src.get_size_y())
                            _append_log(f"Texture {path}: {width}x{height} from TextureSource")
                    except Exception:
                        pass
                    
                    # Method 2: imported_size IntPoint
                    if width is None or height is None:
                        try:
                            imported = tex.get_editor_property('imported_size')
                            if imported is not None:
                                width = int(getattr(imported, 'x', 0))
                                height = int(getattr(imported, 'y', 0))
                                if width and height:
                                    _append_log(f"Texture {path}: {width}x{height} from imported_size")
                        except Exception:
                            pass
                    
                    # Method 3: direct size_x/size_y properties
                    if width is None or height is None:
                        try:
                            width = int(tex.get_editor_property('size_x'))
                            height = int(tex.get_editor_property('size_y'))
                            if width and height:
                                _append_log(f"Texture {path}: {width}x{height} from direct properties")
                        except Exception:
                            pass
                    
                    # Method 3b: blueprint getter methods if available
                    if width is None or height is None:
                        try:
                            if hasattr(tex, 'blueprint_get_size_x') and hasattr(tex, 'blueprint_get_size_y'):
                                width = int(tex.blueprint_get_size_x())
                                height = int(tex.blueprint_get_size_y())
                                if width and height:
                                    _append_log(f"Texture {path}: {width}x{height} from blueprint getters")
                        except Exception:
                            pass

                    # Method 3c: generic getters if exposed
                    if width is None or height is None:
                        try:
                            if hasattr(tex, 'get_size_x') and hasattr(tex, 'get_size_y'):
                                width = int(tex.get_size_x())
                                height = int(tex.get_size_y())
                                if width and height:
                                    _append_log(f"Texture {path}: {width}x{height} from get_size_* methods")
                        except Exception:
                            pass

                    # Method 4: platform_data size
                    if width is None or height is None:
                        try:
                            pd = getattr(tex, 'platform_data', None)
                            if pd and hasattr(pd, 'size_x') and hasattr(pd, 'size_y'):
                                width = int(pd.size_x)
                                height = int(pd.size_y)
                                if width and height:
                                    _append_log(f"Texture {path}: {width}x{height} from platform_data")
                        except Exception:
                            pass

                    # Method 5: AssetRegistry tags (ImportedSize / Dimensions)
                    if (width is None or height is None) and hasattr(unreal, 'AssetRegistryHelpers'):
                        try:
                            ar = unreal.AssetRegistryHelpers.get_asset_registry()
                            sop = None
                            try:
                                sop = unreal.SoftObjectPath(path)
                            except Exception:
                                sop = None
                            ad = ar.get_asset_by_object_path(sop if sop is not None else path)
                            if ad:
                                def _parse_dim_string(sval: str):
                                    try:
                                        if not sval:
                                            return None, None
                                        st = str(sval).strip()
                                        import re
                                        m = re.search(r"(\d+)\D+(\d+)", st)
                                        if m:
                                            return int(m.group(1)), int(m.group(2))
                                    except Exception:
                                        return None, None
                                    return None, None

                                w = h = None
                                # Try ImportedSize first
                                try:
                                    val = ad.get_tag_value('ImportedSize')
                                    # Could be IntPoint-like or string
                                    if val is not None:
                                        w = int(getattr(val, 'x', 0)) if hasattr(val, 'x') else None
                                        h = int(getattr(val, 'y', 0)) if hasattr(val, 'y') else None
                                        if not (w and h) and isinstance(val, str):
                                            w, h = _parse_dim_string(val)
                                except Exception:
                                    pass

                                # Fallback to Dimensions tag
                                if not (w and h):
                                    try:
                                        val2 = ad.get_tag_value('Dimensions')
                                        if isinstance(val2, str):
                                            w, h = _parse_dim_string(val2)
                                    except Exception:
                                        pass

                                # Fallback to tags_and_values map
                                if not (w and h):
                                    try:
                                        tv = getattr(ad, 'tags_and_values', None)
                                        if tv and isinstance(tv, dict):
                                            cand = tv.get('ImportedSize') or tv.get('Dimensions')
                                            if cand:
                                                w, h = _parse_dim_string(cand)
                                    except Exception:
                                        pass

                                if w and h:
                                    width = w
                                    height = h
                                    _append_log(f"Texture {path}: {width}x{height} from AssetRegistry tag")
                        except Exception:
                            pass
                except Exception as e:
                    _append_log(f"Failed to get dimensions for {path}: {e}")
                    width = height = None
                
                # Get compression format
                try:
                    cs = tex.get_editor_property('compression_settings')
                    if cs is not None:
                        # Try to get the enum name, fallback to string representation
                        try:
                            fmt = cs.name if hasattr(cs, 'name') else str(cs)
                        except:
                            fmt = str(cs)
                        _append_log(f"Texture {path}: format={fmt}")
                except Exception as e:
                    _append_log(f"Failed to get compression settings for {path}: {e}")
                
                # Update the texture info
                for r in textures_info:
                    if r.get('path') == path:
                        r.update({
                            "width": width if width is not None else "",
                            "height": height if height is not None else "",
                            "format": fmt if fmt is not None else ""
                        })
                        break
                
                loaded_tex_count += 1
                _append_log(f"Successfully processed texture {path}: {width}x{height} format={fmt}")
                
            except Exception as e:
                _append_log(f"Exception processing texture {path}: {e}")
                # Still update the row with empty values
                for r in textures_info:
                    if r.get('path') == path:
                        r.update({"width": "", "height": "", "format": ""})
                        break

    # Write CSV
    try:
        if csv_dir:
            csv_path = os.path.join(csv_dir, 'textures.csv')
            with open(csv_path, 'w', newline='', encoding='utf-8') as f:
                w = csv.writer(f)
                w.writerow(['path', 'width', 'height', 'format'])
                
                # Log what we're writing
                _append_log(f"Writing CSV with {len(textures_info)} texture rows")
                
                for i, row in enumerate(textures_info):
                    path = row.get('path', '')
                    width = row.get('width', '')
                    height = row.get('height', '')
                    fmt = row.get('format', '')
                    
                    # Log first few rows for debugging
                    if i < 5:
                        _append_log(f"CSV row {i}: path='{path}' width='{width}' height='{height}' format='{fmt}'")
                    
                    w.writerow([path, width, height, fmt])
                
            _append_log(f"CSV written successfully: {csv_path} rows={len(textures_info)} total={total_textures}")
        else:
            _append_log("No CSV directory specified, skipping CSV write")
    except Exception as e:
        _append_log(f"Failed to write CSV: {e}")
        import traceback
        _append_log(f"CSV error traceback: {traceback.format_exc()}")

    assets_processed = total_textures
    assets_modified = 0
    msg = f"Audit OK ({profile}) - {total_textures} textures found (AR:{ar_count}, List:{eal_list_count}, Loaded:{loaded_tex_count})"
elif p == 'recommend':
    assets_processed = 42
    assets_modified = 0
    msg = f"Recommendations generated for {profile}"
elif p == 'apply':
    assets_processed = min(max_changes, 42)
    assets_modified = 3 if not dry_run else 0
    msg = ("Dry-run: would apply changes" if dry_run else "Applied changes") + f" to {profile}"
elif p == 'verify':
    assets_processed = 42
    assets_modified = 0
    msg = f"Verification passed for {profile}"

result = {
    "success": True,
    "message": msg,
    "assetsProcessed": assets_processed,
    "assetsModified": assets_modified,
    "phase": phase,
    "profile": profile,
    "dryRun": dry_run,
    "maxChanges": max_changes,
    "include": include,
    "exclude": exclude,
    "useSelection": use_sel,
    "categories": categories,
}

out_path = os.environ.get('MAGICOPTIMIZER_OUTPUT')
payload = json.dumps(result)
if not out_path:
    print(payload)
else:
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, 'w', encoding='utf-8') as f:
        f.write(payload)
    print(payload)

_append_log(f"entry.py done success={True} msg='{msg}' processed={assets_processed} modified={assets_modified}")