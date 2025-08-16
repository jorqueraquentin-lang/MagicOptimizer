"""
Preset manager for MagicOptimizer plugin
"""

import os
import json
from typing import Dict, List, Any, Optional
import logging
from pathlib import Path

from . import preset_configs
from .. import utils

logger = logging.getLogger(__name__)

class PresetManager:
    """Manages optimization presets"""
    
    def __init__(self, presets_dir: str = None):
        self.presets_dir = presets_dir or "presets"  # Relative to plugin Python directory
        self.presets = {}
        self.load_presets()
    
    def load_presets(self) -> None:
        """Load all available presets"""
        try:
            # Load built-in presets
            self.presets = preset_configs.PRESET_CONFIGS.copy()
            
            # Load custom presets from files
            custom_presets = self._load_custom_presets()
            self.presets.update(custom_presets)
            
            logger.info(f"Loaded {len(self.presets)} presets")
        except Exception as e:
            logger.error(f"Error loading presets: {e}")
            # Fallback to built-in presets only
            self.presets = preset_configs.PRESET_CONFIGS.copy()
    
    def _load_custom_presets(self) -> Dict[str, Any]:
        """Load custom presets from JSON files"""
        custom_presets = {}
        
        if not os.path.exists(self.presets_dir):
            return custom_presets
        
        try:
            for file_path in Path(self.presets_dir).glob("*.json"):
                if file_path.name.startswith("custom_"):
                    try:
                        with open(file_path, 'r', encoding='utf-8') as f:
                            preset_data = json.load(f)
                            
                        preset_name = preset_data.get('name', file_path.stem)
                        custom_presets[preset_name] = preset_data
                        logger.info(f"Loaded custom preset: {preset_name}")
                        
                    except Exception as e:
                        logger.warning(f"Failed to load custom preset {file_path}: {e}")
        except Exception as e:
            logger.error(f"Error scanning custom presets: {e}")
        
        return custom_presets
    
    def get_preset(self, preset_name: str) -> Optional[Dict[str, Any]]:
        """
        Get a specific preset by name
        
        Args:
            preset_name: Name of the preset
            
        Returns:
            Preset configuration or None if not found
        """
        return self.presets.get(preset_name)
    
    def get_all_presets(self) -> List[str]:
        """Get list of all available preset names"""
        return list(self.presets.keys())
    
    def get_preset_description(self, preset_name: str) -> str:
        """
        Get description for a specific preset
        
        Args:
            preset_name: Name of the preset
            
        Returns:
            Preset description
        """
        preset = self.get_preset(preset_name)
        if preset:
            return preset.get('description', 'No description available')
        return 'Unknown preset'
    
    def validate_preset(self, preset_name: str) -> bool:
        """
        Validate if a preset name exists
        
        Args:
            preset_name: Name of the preset
            
        Returns:
            True if preset exists
        """
        return preset_name in self.presets
    
    def create_custom_preset(self, name: str, description: str, config: Dict[str, Any]) -> bool:
        """
        Create a custom preset
        
        Args:
            name: Name of the preset
            description: Description of the preset
            config: Preset configuration
            
        Returns:
            True if successful
        """
        try:
            # Validate preset structure
            if not self._validate_preset_structure(config):
                logger.error("Invalid preset structure")
                return False
            
            # Create preset data
            preset_data = {
                "name": name,
                "description": description,
                "textures": config.get('textures', {}),
                "meshes": config.get('meshes', {}),
                "materials": config.get('materials', {}),
                "runtime": config.get('runtime', {}),
                "safety": config.get('safety', {})
            }
            
            # Save to file
            filename = f"custom_{name.lower().replace(' ', '_')}.json"
            file_path = os.path.join(self.presets_dir, filename)
            
            success = utils.save_json_config(file_path, preset_data)
            if success:
                # Add to loaded presets
                self.presets[name] = preset_data
                logger.info(f"Created custom preset: {name}")
                return True
            else:
                logger.error(f"Failed to save custom preset: {name}")
                return False
                
        except Exception as e:
            logger.error(f"Error creating custom preset: {e}")
            return False
    
    def update_preset(self, preset_name: str, config: Dict[str, Any]) -> bool:
        """
        Update an existing preset
        
        Args:
            preset_name: Name of the preset to update
            config: New configuration
            
        Returns:
            True if successful
        """
        try:
            if not self.validate_preset(preset_name):
                logger.error(f"Preset not found: {preset_name}")
                return False
            
            # Validate preset structure
            if not self._validate_preset_structure(config):
                logger.error("Invalid preset structure")
                return False
            
            # Update preset
            self.presets[preset_name].update(config)
            
            # Save to file if it's a custom preset
            if preset_name.startswith('Custom_'):
                filename = f"custom_{preset_name.lower().replace(' ', '_')}.json"
                file_path = os.path.join(self.presets_dir, filename)
                utils.save_json_config(file_path, self.presets[preset_name])
            
            logger.info(f"Updated preset: {preset_name}")
            return True
            
        except Exception as e:
            logger.error(f"Error updating preset: {e}")
            return False
    
    def delete_custom_preset(self, preset_name: str) -> bool:
        """
        Delete a custom preset
        
        Args:
            preset_name: Name of the preset to delete
            
        Returns:
            True if successful
        """
        try:
            if not self.validate_preset(preset_name):
                logger.error(f"Preset not found: {preset_name}")
                return False
            
            # Check if it's a built-in preset
            if preset_name in preset_configs.PRESET_CONFIGS:
                logger.error(f"Cannot delete built-in preset: {preset_name}")
                return False
            
            # Remove from loaded presets
            del self.presets[preset_name]
            
            # Delete file
            filename = f"custom_{preset_name.lower().replace(' ', '_')}.json"
            file_path = os.path.join(self.presets_dir, filename)
            
            if os.path.exists(file_path):
                os.remove(file_path)
                logger.info(f"Deleted custom preset: {preset_name}")
                return True
            else:
                logger.warning(f"Preset file not found: {file_path}")
                return True
                
        except Exception as e:
            logger.error(f"Error deleting custom preset: {e}")
            return False
    
    def get_preset_safety_settings(self, preset_name: str) -> Dict[str, Any]:
        """
        Get safety settings for a specific preset
        
        Args:
            preset_name: Name of the preset
            
        Returns:
            Safety settings dictionary
        """
        preset = self.get_preset(preset_name)
        if preset:
            return preset.get('safety', {}).copy()
        return {}
    
    def get_preset_texture_settings(self, preset_name: str) -> Dict[str, Any]:
        """
        Get texture settings for a specific preset
        
        Args:
            preset_name: Name of the preset
            
        Returns:
            Texture settings dictionary
        """
        preset = self.get_preset(preset_name)
        if preset:
            return preset.get('textures', {}).copy()
        return {}
    
    def get_preset_mesh_settings(self, preset_name: str) -> Dict[str, Any]:
        """
        Get mesh settings for a specific preset
        
        Args:
            preset_name: Name of the preset
            
        Returns:
            Mesh settings dictionary
        """
        preset = self.get_preset(preset_name)
        if preset:
            return preset.get('meshes', {}).copy()
        return {}
    
    def get_preset_material_settings(self, preset_name: str) -> Dict[str, Any]:
        """
        Get material settings for a specific preset
        
        Args:
            preset_name: Name of the preset
            
        Returns:
            Material settings dictionary
        """
        preset = self.get_preset(preset_name)
        if preset:
            return preset.get('materials', {}).copy()
        return {}
    
    def get_preset_runtime_settings(self, preset_name: str) -> Dict[str, Any]:
        """
        Get runtime settings for a specific preset
        
        Args:
            preset_name: Name of the preset
            
        Returns:
            Runtime settings dictionary
        """
        preset = self.get_preset(preset_name)
        if preset:
            return preset.get('runtime', {}).copy()
        return {}
    
    def export_preset(self, preset_name: str, output_path: str) -> bool:
        """
        Export a preset to a file
        
        Args:
            preset_name: Name of the preset to export
            output_path: Output file path
            
        Returns:
            True if successful
        """
        try:
            preset = self.get_preset(preset_name)
            if not preset:
                logger.error(f"Preset not found: {preset_name}")
                return False
            
            success = utils.save_json_config(output_path, preset)
            if success:
                logger.info(f"Exported preset {preset_name} to {output_path}")
                return True
            else:
                logger.error(f"Failed to export preset {preset_name}")
                return False
                
        except Exception as e:
            logger.error(f"Error exporting preset: {e}")
            return False
    
    def import_preset(self, file_path: str) -> bool:
        """
        Import a preset from a file
        
        Args:
            file_path: Path to preset file
            
        Returns:
            True if successful
        """
        try:
            preset_data = utils.load_json_config(file_path)
            if not preset_data:
                logger.error(f"Failed to load preset from {file_path}")
                return False
            
            # Validate preset structure
            if not self._validate_preset_structure(preset_data):
                logger.error("Invalid preset structure")
                return False
            
            preset_name = preset_data.get('name', 'Imported_Preset')
            
            # Check for name conflicts
            if preset_name in self.presets:
                preset_name = f"Imported_{preset_name}_{len(self.presets)}"
                preset_data['name'] = preset_name
            
            # Add to presets
            self.presets[preset_name] = preset_data
            
            # Save as custom preset
            self.create_custom_preset(preset_name, preset_data.get('description', ''), preset_data)
            
            logger.info(f"Imported preset: {preset_name}")
            return True
            
        except Exception as e:
            logger.error(f"Error importing preset: {e}")
            return False
    
    def _validate_preset_structure(self, config: Dict[str, Any]) -> bool:
        """
        Validate preset configuration structure
        
        Args:
            config: Preset configuration
            
        Returns:
            True if valid
        """
        try:
            required_sections = ['textures', 'meshes', 'materials', 'runtime', 'safety']
            
            for section in required_sections:
                if section not in config:
                    logger.error(f"Missing required section: {section}")
                    return False
                
                if not isinstance(config[section], dict):
                    logger.error(f"Section {section} must be a dictionary")
                    return False
            
            return True
            
        except Exception as e:
            logger.error(f"Error validating preset structure: {e}")
            return False
    
    def get_preset_summary(self, preset_name: str) -> Dict[str, Any]:
        """
        Get summary information for a preset
        
        Args:
            preset_name: Name of the preset
            
        Returns:
            Summary dictionary
        """
        preset = self.get_preset(preset_name)
        if not preset:
            return {}
        
        summary = {
            "name": preset_name,
            "description": preset.get('description', ''),
            "texture_settings": {
                "max_size_color": preset.get('textures', {}).get('max_size_color', 'Unknown'),
                "max_size_normal": preset.get('textures', {}).get('max_size_normal', 'Unknown'),
                "compression_quality": preset.get('textures', {}).get('compression_quality', 'Unknown')
            },
            "mesh_settings": {
                "nanite_enabled": preset.get('meshes', {}).get('nanite_enabled', 'Unknown'),
                "lod_generation": preset.get('meshes', {}).get('lod_generation', 'Unknown')
            },
            "safety_settings": {
                "dry_run_default": preset.get('safety', {}).get('dry_run_default', True),
                "max_changes": preset.get('safety', {}).get('max_changes', 'Unknown'),
                "conservative_mode": preset.get('safety', {}).get('conservative_mode', 'Unknown')
            }
        }
        
        return summary
    
    def list_presets_by_category(self) -> Dict[str, List[str]]:
        """
        List presets organized by category
        
        Returns:
            Dictionary of preset categories and names
        """
        categories = {
            "PC": [],
            "Console": [],
            "Mobile": [],
            "VR": [],
            "Cinematic": [],
            "UI": [],
            "Archviz": [],
            "Custom": []
        }
        
        for preset_name in self.presets:
            if preset_name.startswith('PC_'):
                categories["PC"].append(preset_name)
            elif preset_name.startswith('Console'):
                categories["Console"].append(preset_name)
            elif preset_name.startswith('Mobile'):
                categories["Mobile"].append(preset_name)
            elif preset_name.startswith('VR'):
                categories["VR"].append(preset_name)
            elif preset_name.startswith('Cinematic'):
                categories["Cinematic"].append(preset_name)
            elif preset_name.startswith('UI'):
                categories["UI"].append(preset_name)
            elif preset_name.startswith('Archviz'):
                categories["Archviz"].append(preset_name)
            elif preset_name.startswith('Custom'):
                categories["Custom"].append(preset_name)
            else:
                # Default category
                if "PC" not in categories:
                    categories["PC"] = []
                categories["PC"].append(preset_name)
        
        return categories
