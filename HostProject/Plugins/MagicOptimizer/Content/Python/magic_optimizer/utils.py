"""
Utility functions for MagicOptimizer plugin
"""

import os
import json
from pathlib import Path
from typing import Any, Dict, List, Optional, Union
import logging

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def resolve_enum(enum_cls: Any, name: str, default: Any = None) -> Any:
    """
    Resolve enum value by name with fuzzy matching
    
    Args:
        enum_cls: The enum class to search in
        name: The name to search for
        default: Default value if not found
        
    Returns:
        The resolved enum value or default
    """
    if not name:
        return default
        
    key = str(name).lower().replace("_", "").replace(" ", "")
    
    for attr in dir(enum_cls):
        if not attr.startswith("__"):
            if attr.lower().replace("_", "").replace(" ", "") == key:
                return getattr(enum_cls, attr)
    
    logger.warning(f"Could not resolve enum '{name}' in {enum_cls.__name__}")
    return default

def get_project_root() -> str:
    """Get the current project root directory"""
    return os.getcwd()

def ensure_directory_exists(path: str) -> None:
    """Ensure a directory exists, create if it doesn't"""
    os.makedirs(path, exist_ok=True)

def format_path_for_unreal(path: str) -> str:
    """Format a path for Unreal Engine (forward slashes)"""
    if not path:
        return ""
    formatted = path.replace('\\', '/')
    # Ensure it starts with /Game/ if it's a content path
    if formatted.startswith('Game/') and not formatted.startswith('/Game/'):
        formatted = '/Game/' + formatted
    return formatted

def get_file_size(file_path: str) -> int:
    """Get file size in bytes"""
    try:
        return os.path.getsize(file_path)
    except (OSError, FileNotFoundError):
        return 0

def is_valid_asset_path(path: str) -> bool:
    """Check if a path is a valid Unreal asset path"""
    if not path:
        return False
    return path.startswith('/Game/') and not path.endswith('/')

def load_json_config(file_path: str) -> Dict[str, Any]:
    """Load JSON configuration file"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            return json.load(f)
    except (FileNotFoundError, json.JSONDecodeError) as e:
        logger.error(f"Failed to load config {file_path}: {e}")
        return {}

def save_json_config(file_path: str, data: Dict[str, Any]) -> bool:
    """Save JSON configuration file"""
    try:
        ensure_directory_exists(os.path.dirname(file_path))
        with open(file_path, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        return True
    except Exception as e:
        logger.error(f"Failed to save config {file_path}: {e}")
        return False

def get_asset_name_from_path(asset_path: str) -> str:
    """Extract asset name from full asset path"""
    if not asset_path:
        return ""
    return os.path.basename(asset_path).split('.')[0]

def get_asset_directory(asset_path: str) -> str:
    """Get the directory containing an asset"""
    if not asset_path:
        return ""
    return os.path.dirname(asset_path)

def is_texture_asset(asset_path: str) -> bool:
    """Check if asset is a texture based on path/extension"""
    texture_extensions = {'.uasset', '.ubulk', '.uexp'}
    asset_name = asset_path.lower()
    return any(ext in asset_name for ext in texture_extensions)

def is_mesh_asset(asset_path: str) -> bool:
    """Check if asset is a mesh based on path/extension"""
    mesh_extensions = {'.uasset', '.ubulk', '.uexp'}
    asset_name = asset_path.lower()
    return any(ext in asset_name for ext in mesh_extensions)

def is_material_asset(asset_path: str) -> bool:
    """Check if asset is a material based on path/extension"""
    material_extensions = {'.uasset', '.ubulk', '.uexp'}
    asset_name = asset_path.lower()
    return any(ext in asset_name for ext in material_extensions)

def create_backup_path(original_path: str, backup_suffix: str = "_backup") -> str:
    """Create a backup path for an asset"""
    if not original_path:
        return ""
    base_path = original_path.rsplit('.', 1)[0]
    extension = original_path.rsplit('.', 1)[1] if '.' in original_path else ""
    return f"{base_path}{backup_suffix}.{extension}" if extension else f"{base_path}{backup_suffix}"

def validate_preset_name(preset_name: str) -> bool:
    """Validate preset name format"""
    valid_presets = {
        'PC_Ultra', 'PC_Balanced', 'Console_Optimized', 
        'Mobile_Low', 'Mobile_Ultra_Lite', 'VR', 
        'Cinematic', 'UI_Crisp', 'Archviz_High_Fidelity'
    }
    return preset_name in valid_presets
