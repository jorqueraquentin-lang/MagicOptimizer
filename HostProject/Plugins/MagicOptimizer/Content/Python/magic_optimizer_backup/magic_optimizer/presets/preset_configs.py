"""
Preset configurations for different optimization targets
"""

from typing import Dict, Any, List
import logging

logger = logging.getLogger(__name__)

# Preset definitions
PRESET_CONFIGS = {
    "PC_Ultra": {
        "description": "High-end PC optimizations for maximum quality",
        "textures": {
            "max_size_color": 8192,
            "max_size_normal": 4096,
            "max_size_mask": 2048,
            "compression_quality": "high",
            "mipmap_generation": "enabled",
            "virtual_texture": "enabled",
            "streaming": "enabled"
        },
        "meshes": {
            "nanite_enabled": True,
            "lod_generation": "aggressive",
            "lightmap_uvs": "high_quality",
            "collision_simplification": "minimal",
            "merge_actors": False
        },
        "materials": {
            "normal_convention": "dx",
            "static_switch_cleanup": False,
            "sampler_consolidation": False,
            "packed_map_hints": False
        },
        "runtime": {
            "scalability_bucket": "epic",
            "shadow_quality": "high",
            "post_process_quality": "high",
            "streaming_pool_size": "large"
        },
        "safety": {
            "dry_run_default": True,
            "max_changes": 1000,
            "conservative_mode": False
        }
    },
    
    "PC_Balanced": {
        "description": "Balanced performance vs quality for PC",
        "textures": {
            "max_size_color": 4096,
            "max_size_normal": 2048,
            "max_size_mask": 1024,
            "compression_quality": "medium",
            "mipmap_generation": "enabled",
            "virtual_texture": "enabled",
            "streaming": "enabled"
        },
        "meshes": {
            "nanite_enabled": True,
            "lod_generation": "balanced",
            "lightmap_uvs": "medium_quality",
            "collision_simplification": "moderate",
            "merge_actors": False
        },
        "materials": {
            "normal_convention": "dx",
            "static_switch_cleanup": True,
            "sampler_consolidation": True,
            "packed_map_hints": True
        },
        "runtime": {
            "scalability_bucket": "high",
            "shadow_quality": "medium",
            "post_process_quality": "medium",
            "streaming_pool_size": "medium"
        },
        "safety": {
            "dry_run_default": True,
            "max_changes": 500,
            "conservative_mode": True
        }
    },
    
    "Console_Optimized": {
        "description": "Console-specific optimizations",
        "textures": {
            "max_size_color": 2048,
            "max_size_normal": 1024,
            "max_size_mask": 512,
            "compression_quality": "high",
            "mipmap_generation": "enabled",
            "virtual_texture": "enabled",
            "streaming": "enabled"
        },
        "meshes": {
            "nanite_enabled": False,
            "lod_generation": "aggressive",
            "lightmap_uvs": "medium_quality",
            "collision_simplification": "aggressive",
            "merge_actors": True
        },
        "materials": {
            "normal_convention": "dx",
            "static_switch_cleanup": True,
            "sampler_consolidation": True,
            "packed_map_hints": True
        },
        "runtime": {
            "scalability_bucket": "medium",
            "shadow_quality": "medium",
            "post_process_quality": "low",
            "streaming_pool_size": "small"
        },
        "safety": {
            "dry_run_default": True,
            "max_changes": 300,
            "conservative_mode": True
        }
    },
    
    "Mobile_Low": {
        "description": "Mobile device constraints",
        "textures": {
            "max_size_color": 1024,
            "max_size_normal": 512,
            "max_size_mask": 256,
            "compression_quality": "high",
            "mipmap_generation": "enabled",
            "virtual_texture": "disabled",
            "streaming": "disabled"
        },
        "meshes": {
            "nanite_enabled": False,
            "lod_generation": "very_aggressive",
            "lightmap_uvs": "low_quality",
            "collision_simplification": "very_aggressive",
            "merge_actors": True
        },
        "materials": {
            "normal_convention": "dx",
            "static_switch_cleanup": True,
            "sampler_consolidation": True,
            "packed_map_hints": True
        },
        "runtime": {
            "scalability_bucket": "low",
            "shadow_quality": "low",
            "post_process_quality": "disabled",
            "streaming_pool_size": "minimal"
        },
        "safety": {
            "dry_run_default": True,
            "max_changes": 200,
            "conservative_mode": True
        }
    },
    
    "Mobile_Ultra_Lite": {
        "description": "Maximum mobile optimization",
        "textures": {
            "max_size_color": 512,
            "max_size_normal": 256,
            "max_size_mask": 128,
            "compression_quality": "maximum",
            "mipmap_generation": "enabled",
            "virtual_texture": "disabled",
            "streaming": "disabled"
        },
        "meshes": {
            "nanite_enabled": False,
            "lod_generation": "maximum",
            "lightmap_uvs": "minimal",
            "collision_simplification": "maximum",
            "merge_actors": True
        },
        "materials": {
            "normal_convention": "dx",
            "static_switch_cleanup": True,
            "sampler_consolidation": True,
            "packed_map_hints": True
        },
        "runtime": {
            "scalability_bucket": "minimum",
            "shadow_quality": "disabled",
            "post_process_quality": "disabled",
            "streaming_pool_size": "minimal"
        },
        "safety": {
            "dry_run_default": True,
            "max_changes": 100,
            "conservative_mode": True
        }
    },
    
    "VR": {
        "description": "Virtual reality specific optimizations",
        "textures": {
            "max_size_color": 2048,
            "max_size_normal": 1024,
            "max_size_mask": 512,
            "compression_quality": "high",
            "mipmap_generation": "enabled",
            "virtual_texture": "enabled",
            "streaming": "enabled"
        },
        "meshes": {
            "nanite_enabled": True,
            "lod_generation": "balanced",
            "lightmap_uvs": "medium_quality",
            "collision_simplification": "moderate",
            "merge_actors": False
        },
        "materials": {
            "normal_convention": "dx",
            "static_switch_cleanup": True,
            "sampler_consolidation": True,
            "packed_map_hints": True
        },
        "runtime": {
            "scalability_bucket": "high",
            "shadow_quality": "medium",
            "post_process_quality": "low",
            "streaming_pool_size": "medium"
        },
        "safety": {
            "dry_run_default": True,
            "max_changes": 400,
            "conservative_mode": True
        }
    },
    
    "Cinematic": {
        "description": "High-quality cinematic workflows",
        "textures": {
            "max_size_color": 16384,
            "max_size_normal": 8192,
            "max_size_mask": 4096,
            "compression_quality": "maximum",
            "mipmap_generation": "enabled",
            "virtual_texture": "enabled",
            "streaming": "enabled"
        },
        "meshes": {
            "nanite_enabled": True,
            "lod_generation": "minimal",
            "lightmap_uvs": "maximum_quality",
            "collision_simplification": "minimal",
            "merge_actors": False
        },
        "materials": {
            "normal_convention": "dx",
            "static_switch_cleanup": False,
            "sampler_consolidation": False,
            "packed_map_hints": False
        },
        "runtime": {
            "scalability_bucket": "cinematic",
            "shadow_quality": "maximum",
            "post_process_quality": "maximum",
            "streaming_pool_size": "maximum"
        },
        "safety": {
            "dry_run_default": True,
            "max_changes": 2000,
            "conservative_mode": False
        }
    },
    
    "UI_Crisp": {
        "description": "UI asset optimization focus",
        "textures": {
            "max_size_color": 2048,
            "max_size_normal": 1024,
            "max_size_mask": 512,
            "compression_quality": "high",
            "mipmap_generation": "disabled",
            "virtual_texture": "disabled",
            "streaming": "disabled"
        },
        "meshes": {
            "nanite_enabled": False,
            "lod_generation": "minimal",
            "lightmap_uvs": "minimal",
            "collision_simplification": "minimal",
            "merge_actors": False
        },
        "materials": {
            "normal_convention": "dx",
            "static_switch_cleanup": True,
            "sampler_consolidation": True,
            "packed_map_hints": True
        },
        "runtime": {
            "scalability_bucket": "high",
            "shadow_quality": "medium",
            "post_process_quality": "medium",
            "streaming_pool_size": "small"
        },
        "safety": {
            "dry_run_default": True,
            "max_changes": 300,
            "conservative_mode": True
        }
    },
    
    "Archviz_High_Fidelity": {
        "description": "Architecture visualization optimizations",
        "textures": {
            "max_size_color": 8192,
            "max_size_normal": 4096,
            "max_size_mask": 2048,
            "compression_quality": "high",
            "mipmap_generation": "enabled",
            "virtual_texture": "enabled",
            "streaming": "enabled"
        },
        "meshes": {
            "nanite_enabled": True,
            "lod_generation": "minimal",
            "lightmap_uvs": "high_quality",
            "collision_simplification": "minimal",
            "merge_actors": False
        },
        "materials": {
            "normal_convention": "dx",
            "static_switch_cleanup": False,
            "sampler_consolidation": False,
            "packed_map_hints": False
        },
        "runtime": {
            "scalability_bucket": "high",
            "shadow_quality": "high",
            "post_process_quality": "high",
            "streaming_pool_size": "large"
        },
        "safety": {
            "dry_run_default": True,
            "max_changes": 1500,
            "conservative_mode": False
        }
    }
}

def get_preset_config(preset_name: str) -> Dict[str, Any]:
    """
    Get configuration for a specific preset
    
    Args:
        preset_name: Name of the preset
        
    Returns:
        Preset configuration dictionary
    """
    preset_name = preset_name.replace(" ", "_")
    if preset_name in PRESET_CONFIGS:
        return PRESET_CONFIGS[preset_name].copy()
    else:
        logger.warning(f"Unknown preset: {preset_name}, using PC_Balanced")
        return PRESET_CONFIGS["PC_Balanced"].copy()

def get_all_presets() -> List[str]:
    """Get list of all available preset names"""
    return list(PRESET_CONFIGS.keys())

def validate_preset(preset_name: str) -> bool:
    """Validate if a preset name exists"""
    return preset_name in PRESET_CONFIGS

def get_preset_description(preset_name: str) -> str:
    """Get description for a specific preset"""
    preset_name = preset_name.replace(" ", "_")
    if preset_name in PRESET_CONFIGS:
        return PRESET_CONFIGS[preset_name].get("description", "No description available")
    return "Unknown preset"

def get_preset_safety_settings(preset_name: str) -> Dict[str, Any]:
    """Get safety settings for a specific preset"""
    preset_name = preset_name.replace(" ", "_")
    if preset_name in PRESET_CONFIGS:
        return PRESET_CONFIGS[preset_name].get("safety", {}).copy()
    return {}
