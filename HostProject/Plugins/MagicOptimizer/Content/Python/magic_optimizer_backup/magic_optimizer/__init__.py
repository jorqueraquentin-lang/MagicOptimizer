"""
MagicOptimizer - UE5.6 Asset Optimization Plugin
Python backend for asset optimization workflows

Version: 1.0.0
Author: Perseus XR
"""

__version__ = "1.0.0"
__author__ = "Perseus XR"
__description__ = "UE5.6 Asset Optimization Plugin Python Backend"

from . import entry
from . import io_csv
from . import utils
from . import ue_settings

__all__ = [
    'entry',
    'io_csv', 
    'utils',
    'ue_settings'
]
