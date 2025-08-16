"""
Texture apply system for MagicOptimizer plugin
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

class TextureApplier:
    """Applies texture optimizations based on recommendations"""
    
    def __init__(self, config: Dict[str, Any]):
        self.config = config
        self.preset_config = get_preset_config(config.get('target_profile', 'PC_Balanced'))
        self.output_dir = config.get('paths', {}).get('output_dir', 'Saved/Optimizor/History')
        self.dry_run = config.get('dry_run', True)
        self.max_changes = config.get('max_changes', 1000)
        
    def run(self, config: Dict[str, Any], output_dir: str) -> Dict[str, Any]:
        """
        Run texture optimization application
        
        Args:
            config: Configuration dictionary
            output_dir: Output directory for reports
            
        Returns:
            Apply results dictionary
        """
        logger.info("Starting texture optimization application...")
        
        # Read recommendation results
        recommendation_report_path = os.path.join(output_dir, "textures_recommendations.csv")
        if not os.path.exists(recommendation_report_path):
            logger.warning("No recommendation report found, cannot apply optimizations")
            return self._create_empty_result("apply", "Textures")
        
        recommendations = io_csv.read_csv(recommendation_report_path)
        if not recommendations:
            logger.warning("No recommendations found")
            return self._create_empty_result("apply", "Textures")
        
        # Apply optimizations for each texture
        applied_results = []
        skipped_results = []
        error_results = []
        total_changes = 0
        
        for recommendation in recommendations:
            try:
                # Check if we've hit the max changes limit
                if total_changes >= self.max_changes:
                    logger.info(f"Reached maximum changes limit ({self.max_changes})")
                    break
                
                # Apply optimization
                result = self._apply_texture_optimization(recommendation)
                
                if result:
                    if result.get('success', False):
                        applied_results.append(result)
                        total_changes += result.get('changes_applied', 0)
                    else:
                        skipped_results.append(result)
                else:
                    skipped_results.append(recommendation)
                    
            except Exception as e:
                logger.error(f"Error applying optimization to {recommendation.get('asset_path', 'Unknown')}: {e}")
                error_result = recommendation.copy()
                error_result['error'] = str(e)
                error_result['success'] = False
                error_results.append(error_result)
        
        # Generate apply report
        report_path = os.path.join(output_dir, "textures_apply.csv")
        self._generate_apply_report(applied_results, skipped_results, error_results, report_path)
        
        # Generate summary
        summary = self._generate_summary(applied_results, skipped_results, error_results)
        
        logger.info(f"Texture optimization application completed. Applied: {len(applied_results)}, Skipped: {len(skipped_results)}, Errors: {len(error_results)}")
        
        return {
            "phase": "apply",
            "category": "Textures",
            "scanned": len(recommendations),
            "changed": len(applied_results),
            "skipped": len(skipped_results),
            "errors": len(error_results),
            "artifacts": {
                "apply_report": report_path
            },
            "summary": summary
        }
    
    def _apply_texture_optimization(self, recommendation: Dict[str, str]) -> Optional[Dict[str, Any]]:
        """
        Apply optimization to a single texture
        
        Args:
            recommendation: Recommendation dictionary
            
        Returns:
            Apply result dictionary or None if skipped
        """
        try:
            asset_path = recommendation.get('asset_path', '')
            changes = recommendation.get('changes', '').split('; ')
            risk_level = recommendation.get('risk_level', 'Low')
            
            if not changes or not changes[0]:
                return None
            
            # Check if we should skip based on risk level
            if risk_level == 'High' and self.config.get('conservative_mode', True):
                logger.info(f"Skipping high-risk optimization for {asset_path} (conservative mode)")
                return self._create_skipped_result(recommendation, "High risk - conservative mode")
            
            # Check if we can apply safely
            apply_safely = recommendation.get('apply_safely', 'Yes')
            if apply_safely == 'With backup' and not self.dry_run:
                # Create backup
                backup_created = self._create_backup(asset_path)
                if not backup_created:
                    return self._create_skipped_result(recommendation, "Failed to create backup")
            
            # Apply changes
            applied_changes = []
            changes_applied = 0
            
            for change in changes:
                if not change:
                    continue
                
                if self.dry_run:
                    # Simulate change in dry-run mode
                    applied_changes.append(f"[DRY-RUN] {change}")
                    changes_applied += 1
                else:
                    # Actually apply the change
                    success = self._apply_single_change(asset_path, change)
                    if success:
                        applied_changes.append(change)
                        changes_applied += 1
                    else:
                        logger.warning(f"Failed to apply change: {change}")
            
            if not applied_changes:
                return self._create_skipped_result(recommendation, "No changes could be applied")
            
            # Create apply result
            apply_result = {
                "asset_path": asset_path,
                "asset_name": recommendation.get('asset_name', ''),
                "recommendations": recommendation.get('recommendations', ''),
                "changes_requested": '; '.join(changes),
                "changes_applied": '; '.join(applied_changes),
                "changes_applied_count": changes_applied,
                "estimated_memory_savings": recommendation.get('estimated_memory_savings', '0MB'),
                "risk_level": risk_level,
                "success": True,
                "dry_run": self.dry_run
            }
            
            return apply_result
            
        except Exception as e:
            logger.error(f"Error applying texture optimization: {e}")
            return None
    
    def _create_backup(self, asset_path: str) -> bool:
        """
        Create backup of asset before modification
        
        Args:
            asset_path: Path to asset
            
        Returns:
            True if backup created successfully
        """
        try:
            # This would use UE's asset backup system in a real implementation
            # For now, just log the intention
            logger.info(f"Would create backup of {asset_path}")
            return True
        except Exception as e:
            logger.error(f"Failed to create backup of {asset_path}: {e}")
            return False
    
    def _apply_single_change(self, asset_path: str, change: str) -> bool:
        """
        Apply a single change to a texture
        
        Args:
            asset_path: Path to texture asset
            change: Change description
            
        Returns:
            True if change applied successfully
        """
        try:
            # This would use UE's texture modification API in a real implementation
            # For now, just log the intention
            
            change_lower = change.lower()
            
            if "size:" in change_lower:
                logger.info(f"Would resize texture {asset_path}: {change}")
                return True
            elif "compression:" in change_lower:
                logger.info(f"Would change compression for {asset_path}: {change}")
                return True
            elif "srgb:" in change_lower:
                logger.info(f"Would change SRGB setting for {asset_path}: {change}")
                return True
            elif "mipmaps:" in change_lower:
                logger.info(f"Would change mipmap setting for {asset_path}: {change}")
                return True
            elif "lod group:" in change_lower:
                logger.info(f"Would change LOD group for {asset_path}: {change}")
                return True
            elif "virtual texture:" in change_lower:
                logger.info(f"Would change virtual texture setting for {asset_path}: {change}")
                return True
            elif "streaming:" in change_lower:
                logger.info(f"Would change streaming setting for {asset_path}: {change}")
                return True
            else:
                logger.warning(f"Unknown change type: {change}")
                return False
                
        except Exception as e:
            logger.error(f"Error applying change {change} to {asset_path}: {e}")
            return False
    
    def _create_skipped_result(self, recommendation: Dict[str, str], reason: str) -> Dict[str, Any]:
        """Create result for skipped optimization"""
        return {
            "asset_path": recommendation.get('asset_path', ''),
            "asset_name": recommendation.get('asset_name', ''),
            "recommendations": recommendation.get('recommendations', ''),
            "changes_requested": recommendation.get('changes', ''),
            "changes_applied": "",
            "changes_applied_count": 0,
            "estimated_memory_savings": "0MB",
            "risk_level": recommendation.get('risk_level', 'Unknown'),
            "success": False,
            "skip_reason": reason,
            "dry_run": self.dry_run
        }
    
    def _generate_apply_report(self, applied_results: List[Dict[str, Any]], 
                              skipped_results: List[Dict[str, Any]], 
                              error_results: List[Dict[str, Any]], 
                              report_path: str) -> None:
        """Generate CSV apply report"""
        # Combine all results
        all_results = applied_results + skipped_results + error_results
        
        if not all_results:
            logger.warning("No apply results to report")
            return
        
        # Define CSV fields
        fieldnames = [
            "asset_path", "asset_name", "recommendations", "changes_requested",
            "changes_applied", "changes_applied_count", "estimated_memory_savings",
            "risk_level", "success", "skip_reason", "dry_run"
        ]
        
        # Write CSV report
        success = io_csv.write_csv(report_path, fieldnames, all_results)
        if success:
            logger.info(f"Apply report written to {report_path}")
        else:
            logger.error(f"Failed to write apply report to {report_path}")
    
    def _generate_summary(self, applied_results: List[Dict[str, Any]], 
                         skipped_results: List[Dict[str, Any]], 
                         error_results: List[Dict[str, Any]]) -> Dict[str, Any]:
        """Generate apply summary"""
        # Count by success status
        success_count = len(applied_results)
        skipped_count = len(skipped_results)
        error_count = len(error_results)
        
        # Count by risk level
        risk_counts = {}
        for result in applied_results + skipped_results:
            risk = result.get('risk_level', 'Unknown')
            risk_counts[risk] = risk_counts.get(risk, 0) + 1
        
        # Calculate total changes applied
        total_changes = sum(result.get('changes_applied_count', 0) for result in applied_results)
        
        # Calculate total estimated savings
        total_savings = 0
        for result in applied_results:
            savings_str = result.get('estimated_memory_savings', '0MB')
            try:
                savings = int(savings_str.replace('MB', ''))
                total_savings += savings
            except (ValueError, AttributeError):
                pass
        
        return {
            "total_processed": success_count + skipped_count + error_count,
            "successful": success_count,
            "skipped": skipped_count,
            "errors": error_count,
            "total_changes_applied": total_changes,
            "risk_breakdown": risk_counts,
            "total_estimated_savings": f"{total_savings}MB",
            "dry_run": self.dry_run
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
    Run texture optimization application (entry point)
    
    Args:
        config: Configuration dictionary
        output_dir: Output directory for reports
        
    Returns:
        Apply results dictionary
    """
    applier = TextureApplier(config)
    return applier.run(config, output_dir)
