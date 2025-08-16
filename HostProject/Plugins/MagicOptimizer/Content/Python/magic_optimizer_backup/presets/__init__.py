"""
Preset configurations for MagicOptimizer plugin
"""

from .preset_manager import PresetManager
from .preset_configs import *

__all__ = [
    'PresetManager',
    'get_preset_config',
    'get_all_presets',
    'validate_preset'
]
