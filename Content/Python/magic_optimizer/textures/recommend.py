"""
Texture recommendation system for MagicOptimizer plugin
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

class TextureRecommender:
    """Generates optimization recommendations for textures"""
    
    def __init__(self, config: Dict[str, Any]):
        self.config = config
        self.preset_config = get_preset_config(config.get('target_profile', 'PC_Balanced'))
        self.output_dir = config.get('paths', {}).get('output_dir', 'Saved/Optimizor/History')
        
    def run(self, config: Dict[str, Any], output_dir: str) -> Dict[str, Any]:
        """
        Run texture recommendation
        
        Args:
            config: Configuration dictionary
            output_dir: Output directory for reports
            
        Returns:
            Recommendation results dictionary
        """
        logger.info("Starting texture recommendations...")
        
        # Read audit results
        audit_report_path = os.path.join(output_dir, "textures_audit.csv")
        if not os.path.exists(audit_report_path):
            logger.warning("No audit report found, cannot generate recommendations")
            return self._create_empty_result("recommend", "Textures")
        
        audit_results = io_csv.read_csv(audit_report_path)
        if not audit_results:
            logger.warning("No audit results found")
            return self._create_empty_result("recommend", "Textures")
        
        # Generate recommendations for each texture
        recommendations = []
        for audit_result in audit_results:
            recommendation = self._generate_recommendation(audit_result)
            if recommendation:
                recommendations.append(recommendation)
        
        # Generate recommendation report
        report_path = os.path.join(output_dir, "textures_recommendations.csv")
        self._generate_recommendation_report(recommendations, report_path)
        
        # Generate summary
        summary = self._generate_summary(recommendations)
        
        logger.info(f"Texture recommendations completed. Processed: {len(audit_results)}, Recommendations: {len(recommendations)}")
        
        return {
            "phase": "recommend",
            "category": "Textures",
            "scanned": len(audit_results),
            "changed": 0,  # Recommend doesn't change anything
            "skipped": len(audit_results) - len(recommendations),
            "errors": 0,
            "artifacts": {
                "recommendation_report": report_path
            },
            "summary": summary
        }
    
    def _generate_recommendation(self, audit_result: Dict[str, str]) -> Optional[Dict[str, str]]:
        """
        Generate recommendation for a single texture
        
        Args:
            audit_result: Audit result dictionary
            
        Returns:
            Recommendation dictionary or None if no recommendations
        """
        try:
            asset_path = audit_result.get('asset_path', '')
            issues = audit_result.get('issues', '').split('; ')
            
            if not issues or not issues[0]:
                return None
            
            # Generate specific recommendations based on issues
            recommendations = []
            changes = []
            estimated_savings = "0MB"
            
            for issue in issues:
                if not issue:
                    continue
                    
                issue_lower = issue.lower()
                
                if "size exceeds" in issue_lower:
                    rec, change, savings = self._recommend_size_optimization(audit_result)
                    recommendations.extend(rec)
                    changes.extend(change)
                    estimated_savings = savings
                
                elif "non-optimal compression" in issue_lower:
                    rec, change, savings = self._recommend_compression_optimization(audit_result)
                    recommendations.extend(rec)
                    changes.extend(change)
                    estimated_savings = savings
                
                elif "srgb enabled on non-color" in issue_lower:
                    rec, change, savings = self._recommend_srgb_optimization(audit_result)
                    recommendations.extend(rec)
                    changes.extend(change)
                    estimated_savings = savings
                
                elif "mipmaps disabled" in issue_lower:
                    rec, change, savings = self._recommend_mipmap_optimization(audit_result)
                    recommendations.extend(rec)
                    changes.extend(change)
                    estimated_savings = savings
                
                elif "non-standard lod group" in issue_lower:
                    rec, change, savings = self._recommend_lod_group_optimization(audit_result)
                    recommendations.extend(rec)
                    changes.extend(change)
                    estimated_savings = savings
                
                elif "virtual texture enabled" in issue_lower:
                    rec, change, savings = self._recommend_virtual_texture_optimization(audit_result)
                    recommendations.extend(rec)
                    changes.extend(change)
                    estimated_savings = savings
                
                elif "streaming enabled" in issue_lower:
                    rec, change, savings = self._recommend_streaming_optimization(audit_result)
                    recommendations.extend(rec)
                    changes.extend(change)
                    estimated_savings = savings
            
            if not recommendations:
                return None
            
            # Create recommendation result
            recommendation_result = {
                "asset_path": asset_path,
                "asset_name": audit_result.get('asset_name', ''),
                "current_size": audit_result.get('current_size', ''),
                "current_compression": audit_result.get('current_compression', ''),
                "current_srgb": audit_result.get('current_srgb', ''),
                "current_mipmaps": audit_result.get('current_mips', ''),
                "current_lod_group": audit_result.get('current_lod_group', ''),
                "current_virtual_texture": audit_result.get('current_virtual_texture', ''),
                "current_streaming": audit_result.get('current_streaming', ''),
                "recommendations": '; '.join(recommendations),
                "changes": '; '.join(changes),
                "estimated_memory_savings": estimated_savings,
                "priority": audit_result.get('priority', 'Low'),
                "risk_level": self._calculate_risk_level(recommendations),
                "apply_safely": self._can_apply_safely(recommendations)
            }
            
            return recommendation_result
            
        except Exception as e:
            logger.error(f"Error generating recommendation for texture: {e}")
            return None
    
    def _recommend_size_optimization(self, audit_result: Dict[str, str]) -> tuple:
        """Recommend size optimization"""
        current_size = audit_result.get('current_size', '')
        lod_group = audit_result.get('current_lod_group', '')
        
        # Get preset size limits
        preset = self.preset_config.get('textures', {})
        if lod_group == 'Normal':
            max_size = preset.get('max_size_normal', 2048)
        elif lod_group == 'LUT':
            max_size = preset.get('max_size_mask', 1024)
        else:
            max_size = preset.get('max_size_color', 4096)
        
        try:
            width, height = map(int, current_size.split('x'))
            if width > max_size or height > max_size:
                new_size = f"{min(width, max_size)}x{min(height, max_size)}"
                recommendation = f"Reduce size from {current_size} to {new_size}"
                change = f"Size: {current_size} -> {new_size}"
                savings = f"{((width * height - min(width, max_size) * min(height, max_size)) * 4) // (1024 * 1024)}MB"
                return [recommendation], [change], savings
        except (ValueError, AttributeError):
            pass
        
        return [], [], "0MB"
    
    def _recommend_compression_optimization(self, audit_result: Dict[str, str]) -> tuple:
        """Recommend compression optimization"""
        current_compression = audit_result.get('current_compression', '')
        lod_group = audit_result.get('current_lod_group', '')
        
        # Recommend optimal compression based on LOD group
        if lod_group == 'Normal':
            recommended = 'BC5'
        elif lod_group == 'LUT':
            recommended = 'BC4'
        else:
            recommended = 'BC3'
        
        if current_compression != recommended:
            recommendation = f"Change compression from {current_compression} to {recommended}"
            change = f"Compression: {current_compression} -> {recommended}"
            savings = "2MB"  # Rough estimate
            return [recommendation], [change], savings
        
        return [], [], "0MB"
    
    def _recommend_srgb_optimization(self, audit_result: Dict[str, str]) -> tuple:
        """Recommend SRGB optimization"""
        lod_group = audit_result.get('current_lod_group', '')
        
        if lod_group in ['Normal', 'LUT']:
            recommendation = "Disable SRGB for non-color texture"
            change = "SRGB: Enabled -> Disabled"
            savings = "1MB"  # Rough estimate
            return [recommendation], [change], savings
        
        return [], [], "0MB"
    
    def _recommend_mipmap_optimization(self, audit_result: Dict[str, str]) -> tuple:
        """Recommend mipmap optimization"""
        lod_group = audit_result.get('current_lod_group', '')
        
        if lod_group not in ['UI', 'LUT']:
            recommendation = "Enable mipmaps for better performance"
            change = "Mipmaps: Disabled -> Enabled"
            savings = "1MB"  # Rough estimate
            return [recommendation], [change], savings
        
        return [], [], "0MB"
    
    def _recommend_lod_group_optimization(self, audit_result: Dict[str, str]) -> tuple:
        """Recommend LOD group optimization"""
        current_lod_group = audit_result.get('current_lod_group', '')
        
        # Suggest standard LOD group based on texture type
        if 'normal' in audit_result.get('asset_name', '').lower():
            recommended = 'Normal'
        elif 'lut' in audit_result.get('asset_name', '').lower():
            recommended = 'LUT'
        elif 'ui' in audit_result.get('asset_name', '').lower():
            recommended = 'UI'
        else:
            recommended = 'World'
        
        if current_lod_group != recommended:
            recommendation = f"Change LOD group from {current_lod_group} to {recommended}"
            change = f"LOD Group: {current_lod_group} -> {recommended}"
            savings = "1MB"  # Rough estimate
            return [recommendation], [change], savings
        
        return [], [], "0MB"
    
    def _recommend_virtual_texture_optimization(self, audit_result: Dict[str, str]) -> tuple:
        """Recommend virtual texture optimization"""
        lod_group = audit_result.get('current_lod_group', '')
        
        if lod_group in ['UI', 'LUT']:
            recommendation = "Disable virtual texture for UI/LUT textures"
            change = "Virtual Texture: Enabled -> Disabled"
            savings = "1MB"  # Rough estimate
            return [recommendation], [change], savings
        
        return [], [], "0MB"
    
    def _recommend_streaming_optimization(self, audit_result: Dict[str, str]) -> tuple:
        """Recommend streaming optimization"""
        lod_group = audit_result.get('current_lod_group', '')
        
        if lod_group in ['UI', 'LUT']:
            recommendation = "Disable streaming for UI/LUT textures"
            change = "Streaming: Enabled -> Disabled"
            savings = "1MB"  # Rough estimate
            return [recommendation], [change], savings
        
        return [], [], "0MB"
    
    def _calculate_risk_level(self, recommendations: List[str]) -> str:
        """Calculate risk level for recommendations"""
        if not recommendations:
            return "None"
        
        # Check for high-risk operations
        high_risk_keywords = ['size', 'compression', 'format']
        medium_risk_keywords = ['srgb', 'mipmaps', 'lod group']
        
        high_risk_count = sum(1 for rec in recommendations 
                             if any(keyword in rec.lower() for keyword in high_risk_keywords))
        medium_risk_count = sum(1 for rec in recommendations 
                               if any(keyword in rec.lower() for keyword in medium_risk_keywords))
        
        if high_risk_count > 0:
            return "High"
        elif medium_risk_count > 0:
            return "Medium"
        else:
            return "Low"
    
    def _can_apply_safely(self, recommendations: List[str]) -> str:
        """Determine if recommendations can be applied safely"""
        if not recommendations:
            return "Yes"
        
        # Check for operations that require special handling
        unsafe_keywords = ['size', 'compression']
        unsafe_count = sum(1 for rec in recommendations 
                          if any(keyword in rec.lower() for keyword in unsafe_keywords))
        
        if unsafe_count > 0:
            return "With backup"
        else:
            return "Yes"
    
    def _generate_recommendation_report(self, recommendations: List[Dict[str, str]], report_path: str) -> None:
        """Generate CSV recommendation report"""
        if not recommendations:
            logger.warning("No recommendations to report")
            return
        
        # Define CSV fields
        fieldnames = [
            "asset_path", "asset_name", "current_size", "current_compression",
            "current_srgb", "current_mipmaps", "current_lod_group",
            "current_virtual_texture", "current_streaming", "recommendations",
            "changes", "estimated_memory_savings", "priority", "risk_level", "apply_safely"
        ]
        
        # Write CSV report
        success = io_csv.write_csv(report_path, fieldnames, recommendations)
        if success:
            logger.info(f"Recommendation report written to {report_path}")
        else:
            logger.error(f"Failed to write recommendation report to {report_path}")
    
    def _generate_summary(self, recommendations: List[Dict[str, str]]) -> Dict[str, Any]:
        """Generate recommendation summary"""
        if not recommendations:
            return {"total_recommendations": 0, "risk_breakdown": {}, "priority_breakdown": {}}
        
        # Count by risk level
        risk_counts = {}
        for rec in recommendations:
            risk = rec.get('risk_level', 'Unknown')
            risk_counts[risk] = risk_counts.get(risk, 0) + 1
        
        # Count by priority
        priority_counts = {}
        for rec in recommendations:
            priority = rec.get('priority', 'Unknown')
            priority_counts[priority] = priority_counts.get(priority, 0) + 1
        
        # Calculate total estimated savings
        total_savings = 0
        for rec in recommendations:
            savings_str = rec.get('estimated_memory_savings', '0MB')
            try:
                savings = int(savings_str.replace('MB', ''))
                total_savings += savings
            except (ValueError, AttributeError):
                pass
        
        return {
            "total_recommendations": len(recommendations),
            "risk_breakdown": risk_counts,
            "priority_breakdown": priority_counts,
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
    Run texture recommendations (entry point)
    
    Args:
        config: Configuration dictionary
        output_dir: Output directory for reports
        
    Returns:
        Recommendation results dictionary
    """
    recommender = TextureRecommender(config)
    return recommender.run(config, output_dir)
