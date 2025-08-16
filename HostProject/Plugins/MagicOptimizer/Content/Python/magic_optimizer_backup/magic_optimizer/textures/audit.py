"""
Texture audit system for MagicOptimizer plugin
"""

import os
import json
from typing import Dict, List, Any, Optional
import logging
from pathlib import Path

from .. import io_csv
from .. import utils
from ..presets import get_preset_config

logger = logging.getLogger(__name__)

class TextureAuditor:
    """Audits textures for optimization opportunities"""
    
    def __init__(self, config: Dict[str, Any]):
        self.config = config
        self.preset_config = get_preset_config(config.get('target_profile', 'PC_Balanced'))
        self.output_dir = config.get('paths', {}).get('output_dir', 'Saved/Optimizor/History')
        
    def run(self, config: Dict[str, Any], output_dir: str) -> Dict[str, Any]:
        """
        Run texture audit
        
        Args:
            config: Configuration dictionary
            output_dir: Output directory for reports
            
        Returns:
            Audit results dictionary
        """
        logger.info("Starting texture audit...")
        
        # Get texture assets to audit
        texture_assets = self._get_texture_assets(config)
        
        if not texture_assets:
            logger.warning("No texture assets found to audit")
            return self._create_empty_result("audit", "Textures")
        
        # Audit each texture
        audit_results = []
        for asset_path in texture_assets:
            audit_result = self._audit_texture(asset_path)
            if audit_result:
                audit_results.append(audit_result)
        
        # Generate audit report
        report_path = os.path.join(output_dir, "textures_audit.csv")
        self._generate_audit_report(audit_results, report_path)
        
        # Generate summary
        summary = self._generate_summary(audit_results)
        
        logger.info(f"Texture audit completed. Scanned: {len(texture_assets)}, Issues: {len(audit_results)}")
        
        return {
            "phase": "audit",
            "category": "Textures",
            "scanned": len(texture_assets),
            "changed": 0,  # Audit doesn't change anything
            "skipped": len(texture_assets) - len(audit_results),
            "errors": 0,
            "artifacts": {
                "audit_report": report_path
            },
            "summary": summary
        }
    
    def _get_texture_assets(self, config: Dict[str, Any]) -> List[str]:
        """Get list of texture assets to audit"""
        texture_assets = []
        
        # Check if using selection
        if config.get('use_selection', False):
            # This would be populated by the C++ side
            texture_assets = config.get('selected_textures', [])
        else:
            # Scan include/exclude paths
            include_paths = config.get('include_paths', [])
            exclude_paths = config.get('exclude_paths', [])
            
            # For now, return empty list - this would be populated by UE asset scanning
            # In a real implementation, this would use the Asset Registry
            pass
        
        return texture_assets
    
    def _audit_texture(self, asset_path: str) -> Optional[Dict[str, str]]:
        """
        Audit a single texture asset
        
        Args:
            asset_path: Path to texture asset
            
        Returns:
            Audit result dictionary or None if no issues
        """
        try:
            # Extract texture information
            texture_info = self._extract_texture_info(asset_path)
            
            # Check for optimization opportunities
            issues = self._check_texture_issues(texture_info)
            
            if not issues:
                return None
            
            # Create audit result
            audit_result = {
                "asset_path": asset_path,
                "asset_name": utils.get_asset_name_from_path(asset_path),
                "current_size": texture_info.get('size', 'Unknown'),
                "current_compression": texture_info.get('compression', 'Unknown'),
                "current_srgb": texture_info.get('srgb', 'Unknown'),
                "current_mipmaps": texture_info.get('mipmaps', 'Unknown'),
                "current_lod_group": texture_info.get('lod_group', 'Unknown'),
                "current_virtual_texture": texture_info.get('virtual_texture', 'Unknown'),
                "current_streaming": texture_info.get('streaming', 'Unknown'),
                "issues": '; '.join(issues),
                "priority": self._calculate_priority(issues, texture_info),
                "estimated_memory_savings": self._estimate_memory_savings(texture_info, issues)
            }
            
            return audit_result
            
        except Exception as e:
            logger.error(f"Error auditing texture {asset_path}: {e}")
            return None
    
    def _extract_texture_info(self, asset_path: str) -> Dict[str, Any]:
        """
        Extract texture information from asset
        
        Args:
            asset_path: Path to texture asset
            
        Returns:
            Dictionary with texture information
        """
        # This would use UE's texture asset API in a real implementation
        # For now, return mock data structure
        
        texture_info = {
            'size': '2048x2048',  # Would get from UTexture2D
            'compression': 'BC3',  # Would get from texture settings
            'srgb': True,  # Would get from texture settings
            'mipmaps': True,  # Would get from texture settings
            'lod_group': 'UI',  # Would get from texture settings
            'virtual_texture': False,  # Would get from texture settings
            'streaming': True,  # Would get from texture settings
            'format': 'RGBA8',  # Would get from texture settings
            'source_size': '4096x4096',  # Would get from source data
            'memory_usage': '16MB'  # Would calculate from size and format
        }
        
        return texture_info
    
    def _check_texture_issues(self, texture_info: Dict[str, Any]) -> List[str]:
        """
        Check texture for optimization issues
        
        Args:
            texture_info: Texture information dictionary
            
        Returns:
            List of issue descriptions
        """
        issues = []
        preset = self.preset_config.get('textures', {})
        
        # Check compression settings
        current_compression = texture_info.get('compression', '')
        if current_compression not in ['BC1', 'BC3', 'BC4', 'BC5', 'BC7']:
            issues.append("Non-optimal compression format")
        
        # Check size limits
        current_size = texture_info.get('size', '')
        if current_size:
            try:
                width, height = map(int, current_size.split('x'))
                max_size = preset.get('max_size_color', 4096)
                if width > max_size or height > max_size:
                    issues.append(f"Size exceeds preset limit ({max_size})")
            except (ValueError, AttributeError):
                pass
        
        # Check SRGB settings
        current_srgb = texture_info.get('srgb', True)
        if current_srgb and texture_info.get('lod_group', '') in ['Normal', 'LUT']:
            issues.append("SRGB enabled on non-color texture")
        
        # Check mipmap settings
        current_mipmaps = texture_info.get('mipmaps', True)
        if not current_mipmaps and texture_info.get('lod_group', '') not in ['UI', 'LUT']:
            issues.append("Mipmaps disabled on non-UI texture")
        
        # Check LOD group settings
        current_lod_group = texture_info.get('lod_group', '')
        if current_lod_group not in ['UI', 'Normal', 'LUT', 'World']:
            issues.append("Non-standard LOD group")
        
        # Check virtual texture settings
        current_vt = texture_info.get('virtual_texture', False)
        if current_vt and texture_info.get('lod_group', '') in ['UI', 'LUT']:
            issues.append("Virtual texture enabled on UI/LUT texture")
        
        # Check streaming settings
        current_streaming = texture_info.get('streaming', True)
        if current_streaming and texture_info.get('lod_group', '') in ['UI', 'LUT']:
            issues.append("Streaming enabled on UI/LUT texture")
        
        return issues
    
    def _calculate_priority(self, issues: List[str], texture_info: Dict[str, Any]) -> str:
        """
        Calculate priority level for texture optimization
        
        Args:
            issues: List of issues found
            texture_info: Texture information
            
        Returns:
            Priority level string
        """
        if not issues:
            return "None"
        
        # Count critical issues
        critical_count = sum(1 for issue in issues if any(critical in issue.lower() 
                                                        for critical in ['size', 'memory', 'format']))
        
        if critical_count > 0:
            return "High"
        elif len(issues) > 2:
            return "Medium"
        else:
            return "Low"
    
    def _estimate_memory_savings(self, texture_info: Dict[str, Any], issues: List[str]) -> str:
        """
        Estimate memory savings from optimization
        
        Args:
            texture_info: Texture information
            issues: List of issues found
            
        Returns:
            Estimated memory savings string
        """
        if not issues:
            return "0MB"
        
        # This would calculate actual savings based on current vs optimized settings
        # For now, return a rough estimate
        base_savings = len(issues) * 2  # 2MB per issue as rough estimate
        return f"{base_savings}MB"
    
    def _generate_audit_report(self, audit_results: List[Dict[str, str]], report_path: str) -> None:
        """
        Generate CSV audit report
        
        Args:
            audit_results: List of audit results
            report_path: Path to output report
        """
        if not audit_results:
            logger.warning("No audit results to report")
            return
        
        # Define CSV fields
        fieldnames = [
            "asset_path", "asset_name", "current_size", "current_compression",
            "current_srgb", "current_mipmaps", "current_lod_group",
            "current_virtual_texture", "current_streaming", "issues",
            "priority", "estimated_memory_savings"
        ]
        
        # Write CSV report
        success = io_csv.write_csv(report_path, fieldnames, audit_results)
        if success:
            logger.info(f"Audit report written to {report_path}")
        else:
            logger.error(f"Failed to write audit report to {report_path}")
    
    def _generate_summary(self, audit_results: List[Dict[str, str]]) -> Dict[str, Any]:
        """
        Generate audit summary
        
        Args:
            audit_results: List of audit results
            
        Returns:
            Summary dictionary
        """
        if not audit_results:
            return {"total_issues": 0, "priority_breakdown": {}, "issue_types": {}}
        
        # Count by priority
        priority_counts = {}
        for result in audit_results:
            priority = result.get('priority', 'Unknown')
            priority_counts[priority] = priority_counts.get(priority, 0) + 1
        
        # Count by issue type
        issue_counts = {}
        for result in audit_results:
            issues = result.get('issues', '').split('; ')
            for issue in issues:
                if issue:
                    issue_counts[issue] = issue_counts.get(issue, 0) + 1
        
        # Calculate total estimated savings
        total_savings = 0
        for result in audit_results:
            savings_str = result.get('estimated_memory_savings', '0MB')
            try:
                savings = int(savings_str.replace('MB', ''))
                total_savings += savings
            except (ValueError, AttributeError):
                pass
        
        return {
            "total_issues": len(audit_results),
            "priority_breakdown": priority_counts,
            "issue_types": issue_counts,
            "total_estimated_savings": f"{total_savings}MB"
        }
    
    def _create_empty_result(self, phase: str, category: str) -> Dict[str, Any]:
        """Create empty result structure"""
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

def run(config: Dict[str, Any], output_dir: str) -> Dict[str, Any]:
    """
    Run texture audit (entry point)
    
    Args:
        config: Configuration dictionary
        output_dir: Output directory for reports
        
    Returns:
        Audit results dictionary
    """
    auditor = TextureAuditor(config)
    return auditor.run(config, output_dir)
