"""
Texture verification system for MagicOptimizer plugin
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

class TextureVerifier:
    """Verifies texture optimizations were applied correctly"""
    
    def __init__(self, config: Dict[str, Any]):
        self.config = config
        self.preset_config = get_preset_config(config.get('target_profile', 'PC_Balanced'))
        self.output_dir = config.get('paths', {}).get('output_dir', 'Saved/Optimizor/History')
        
    def run(self, config: Dict[str, Any], output_dir: str) -> Dict[str, Any]:
        """
        Run texture verification
        
        Args:
            config: Configuration dictionary
            output_dir: Output directory for reports
            
        Returns:
            Verification results dictionary
        """
        logger.info("Starting texture verification...")
        
        # Read apply results
        apply_report_path = os.path.join(output_dir, "textures_apply.csv")
        if not os.path.exists(apply_report_path):
            logger.warning("No apply report found, cannot verify optimizations")
            return self._create_empty_result("verify", "Textures")
        
        apply_results = io_csv.read_csv(apply_report_path)
        if not apply_results:
            logger.warning("No apply results found")
            return self._create_empty_result("verify", "Textures")
        
        # Filter to only successfully applied optimizations
        applied_assets = [result for result in apply_results if result.get('success', False)]
        
        if not applied_assets:
            logger.info("No successfully applied optimizations to verify")
            return self._create_empty_result("verify", "Textures")
        
        # Verify each optimized texture
        verification_results = []
        passed_count = 0
        failed_count = 0
        
        for applied_asset in applied_assets:
            try:
                verification_result = self._verify_texture_optimization(applied_asset)
                if verification_result:
                    verification_results.append(verification_result)
                    
                    if verification_result.get('verification_status', '') == 'Passed':
                        passed_count += 1
                    else:
                        failed_count += 1
                        
            except Exception as e:
                logger.error(f"Error verifying {applied_asset.get('asset_path', 'Unknown')}: {e}")
                failed_count += 1
        
        # Generate verification report
        report_path = os.path.join(output_dir, "textures_verify.csv")
        self._generate_verification_report(verification_results, report_path)
        
        # Generate summary
        summary = self._generate_summary(verification_results, passed_count, failed_count)
        
        logger.info(f"Texture verification completed. Passed: {passed_count}, Failed: {failed_count}")
        
        return {
            "phase": "verify",
            "category": "Textures",
            "scanned": len(applied_assets),
            "changed": 0,  # Verify doesn't change anything
            "skipped": 0,
            "errors": failed_count,
            "artifacts": {
                "verification_report": report_path
            },
            "summary": summary
        }
    
    def _verify_texture_optimization(self, applied_asset: Dict[str, Any]) -> Optional[Dict[str, Any]]:
        """
        Verify a single texture optimization
        
        Args:
            applied_asset: Applied asset result dictionary
            
        Returns:
            Verification result dictionary
        """
        try:
            asset_path = applied_asset.get('asset_path', '')
            changes_applied = applied_asset.get('changes_applied', '').split('; ')
            
            if not changes_applied or not changes_applied[0]:
                return None
            
            # Get current texture state (this would use UE's texture API in real implementation)
            current_state = self._get_current_texture_state(asset_path)
            
            # Verify each applied change
            verification_details = []
            all_passed = True
            
            for change in changes_applied:
                if not change or change.startswith('[DRY-RUN]'):
                    continue
                
                verification_result = self._verify_single_change(asset_path, change, current_state)
                verification_details.append(verification_result)
                
                if not verification_result.get('passed', False):
                    all_passed = False
            
            # Create verification result
            # Additional property consistency warnings (heuristic, property-based)
            consistency_warnings = self._property_consistency_warnings(current_state)

            verification_result = {
                "asset_path": asset_path,
                "asset_name": applied_asset.get('asset_name', ''),
                "changes_applied": '; '.join(changes_applied),
                "verification_status": "Passed" if all_passed else "Failed",
                "verification_details": '; '.join([f"{v.get('change', '')}: {v.get('status', 'Unknown')}" for v in verification_details]),
                "passed_checks": sum(1 for v in verification_details if v.get('passed', False)),
                "total_checks": len(verification_details),
                "estimated_memory_savings": applied_asset.get('estimated_memory_savings', '0MB'),
                "dry_run": applied_asset.get('dry_run', True)
            }
            if consistency_warnings:
                verification_result["consistency_warnings"] = '; '.join(consistency_warnings)
            
            return verification_result
            
        except Exception as e:
            logger.error(f"Error verifying texture optimization: {e}")
            return None
    
    def _get_current_texture_state(self, asset_path: str) -> Dict[str, Any]:
        """
        Get current texture state for verification
        
        Args:
            asset_path: Path to texture asset
            
        Returns:
            Current texture state dictionary
        """
        # This would use UE's texture asset API in a real implementation
        # For now, return mock data structure
        
        texture_state = {
            'size': '2048x2048',  # Would get from UTexture2D
            'compression': 'BC3',  # Would get from texture settings
            'srgb': True,  # Would get from texture settings
            'mipmaps': True,  # Would get from texture settings
            'lod_group': 'UI',  # Would get from texture settings
            'virtual_texture': False,  # Would get from texture settings
            'streaming': True,  # Would get from texture settings
            'format': 'RGBA8'  # Would get from texture settings
        }
        
        return texture_state
    
    def _verify_single_change(self, asset_path: str, change: str, current_state: Dict[str, Any]) -> Dict[str, Any]:
        """
        Verify a single applied change
        
        Args:
            asset_path: Path to texture asset
            change: Change description
            current_state: Current texture state
            
        Returns:
            Verification result for this change
        """
        try:
            change_lower = change.lower()
            passed = False
            status = "Unknown"
            
            if "size:" in change_lower:
                # Verify size change
                passed, status = self._verify_size_change(change, current_state)
            elif "compression:" in change_lower:
                # Verify compression change
                passed, status = self._verify_compression_change(change, current_state)
            elif "srgb:" in change_lower:
                # Verify SRGB change
                passed, status = self._verify_srgb_change(change, current_state)
            elif "mipmaps:" in change_lower:
                # Verify mipmap change
                passed, status = self._verify_mipmap_change(change, current_state)
            elif "lod group:" in change_lower:
                # Verify LOD group change
                passed, status = self._verify_lod_group_change(change, current_state)
            elif "virtual texture:" in change_lower:
                # Verify virtual texture change
                passed, status = self._verify_virtual_texture_change(change, current_state)
            elif "streaming:" in change_lower:
                # Verify streaming change
                passed, status = self._verify_streaming_change(change, current_state)
            else:
                status = "Unknown change type"
            
            return {
                "change": change,
                "passed": passed,
                "status": status
            }
            
        except Exception as e:
            logger.error(f"Error verifying change {change}: {e}")
            return {
                "change": change,
                "passed": False,
                "status": f"Error: {str(e)}"
            }

    def _property_consistency_warnings(self, state: Dict[str, Any]) -> List[str]:
        """Heuristic property-based consistency checks independent of requested changes."""
        warnings: List[str] = []
        lod = (state.get('lod_group') or '').strip()
        compression = (state.get('compression') or '').upper().strip()
        srgb = bool(state.get('srgb', True))
        streaming = bool(state.get('streaming', True))
        vt = bool(state.get('virtual_texture', False))

        if lod == 'Normal':
            if srgb:
                warnings.append('SRGB should be Disabled for Normal maps')
            if compression != 'BC5':
                warnings.append('Compression should be BC5 for Normal maps')
        elif lod == 'LUT':
            if srgb:
                warnings.append('SRGB should be Disabled for LUT/Mask textures')
            if compression != 'BC4':
                warnings.append('Compression should be BC4 for LUT/Mask textures')
        elif lod == 'UI':
            if streaming:
                warnings.append('Streaming should be Disabled for UI textures')
            if vt:
                warnings.append('Virtual Texture should be Disabled for UI textures')
        # World: no strict checks here

        return warnings
    
    def _verify_size_change(self, change: str, current_state: Dict[str, Any]) -> tuple:
        """Verify size change was applied"""
        try:
            # Parse change like "Size: 4096x4096 -> 2048x2048"
            if "->" in change:
                target_size = change.split("->")[-1].strip()
                current_size = current_state.get('size', '')
                
                if current_size == target_size:
                    return True, f"Size correctly set to {target_size}"
                else:
                    return False, f"Expected {target_size}, got {current_size}"
            else:
                return False, "Invalid size change format"
        except Exception as e:
            return False, f"Error parsing size change: {e}"
    
    def _verify_compression_change(self, change: str, current_state: Dict[str, Any]) -> tuple:
        """Verify compression change was applied"""
        try:
            # Parse change like "Compression: RGBA8 -> BC3"
            if "->" in change:
                target_compression = change.split("->")[-1].strip()
                current_compression = current_state.get('compression', '')
                
                if current_compression == target_compression:
                    return True, f"Compression correctly set to {target_compression}"
                else:
                    return False, f"Expected {target_compression}, got {current_compression}"
            else:
                return False, "Invalid compression change format"
        except Exception as e:
            return False, f"Error parsing compression change: {e}"
    
    def _verify_srgb_change(self, change: str, current_state: Dict[str, Any]) -> tuple:
        """Verify SRGB change was applied"""
        try:
            # Parse change like "SRGB: Enabled -> Disabled"
            if "->" in change:
                target_srgb = change.split("->")[-1].strip()
                current_srgb = current_state.get('srgb', True)
                
                expected_srgb = target_srgb.lower() == 'enabled'
                if current_srgb == expected_srgb:
                    return True, f"SRGB correctly set to {target_srgb}"
                else:
                    return False, f"Expected {target_srgb}, got {'Enabled' if current_srgb else 'Disabled'}"
            else:
                return False, "Invalid SRGB change format"
        except Exception as e:
            return False, f"Error parsing SRGB change: {e}"
    
    def _verify_mipmap_change(self, change: str, current_state: Dict[str, Any]) -> tuple:
        """Verify mipmap change was applied"""
        try:
            # Parse change like "Mipmaps: Disabled -> Enabled"
            if "->" in change:
                target_mipmaps = change.split("->")[-1].strip()
                current_mipmaps = current_state.get('mipmaps', True)
                
                expected_mipmaps = target_mipmaps.lower() == 'enabled'
                if current_mipmaps == expected_mipmaps:
                    return True, f"Mipmaps correctly set to {target_mipmaps}"
                else:
                    return False, f"Expected {target_mipmaps}, got {'Enabled' if current_mipmaps else 'Disabled'}"
            else:
                return False, "Invalid mipmap change format"
        except Exception as e:
            return False, f"Error parsing mipmap change: {e}"
    
    def _verify_lod_group_change(self, change: str, current_state: Dict[str, Any]) -> tuple:
        """Verify LOD group change was applied"""
        try:
            # Parse change like "LOD Group: Custom -> UI"
            if "->" in change:
                target_lod_group = change.split("->")[-1].strip()
                current_lod_group = current_state.get('lod_group', '')
                
                if current_lod_group == target_lod_group:
                    return True, f"LOD Group correctly set to {target_lod_group}"
                else:
                    return False, f"Expected {target_lod_group}, got {current_lod_group}"
            else:
                return False, "Invalid LOD group change format"
        except Exception as e:
            return False, f"Error parsing LOD group change: {e}"
    
    def _verify_virtual_texture_change(self, change: str, current_state: Dict[str, Any]) -> tuple:
        """Verify virtual texture change was applied"""
        try:
            # Parse change like "Virtual Texture: Enabled -> Disabled"
            if "->" in change:
                target_vt = change.split("->")[-1].strip()
                current_vt = current_state.get('virtual_texture', False)
                
                expected_vt = target_vt.lower() == 'enabled'
                if current_vt == expected_vt:
                    return True, f"Virtual Texture correctly set to {target_vt}"
                else:
                    return False, f"Expected {target_vt}, got {'Enabled' if current_vt else 'Disabled'}"
            else:
                return False, "Invalid virtual texture change format"
        except Exception as e:
            return False, f"Error parsing virtual texture change: {e}"
    
    def _verify_streaming_change(self, change: str, current_state: Dict[str, Any]) -> tuple:
        """Verify streaming change was applied"""
        try:
            # Parse change like "Streaming: Enabled -> Disabled"
            if "->" in change:
                target_streaming = change.split("->")[-1].strip()
                current_streaming = current_state.get('streaming', True)
                
                expected_streaming = target_streaming.lower() == 'enabled'
                if current_streaming == expected_streaming:
                    return True, f"Streaming correctly set to {target_streaming}"
                else:
                    return False, f"Expected {target_streaming}, got {'Enabled' if current_streaming else 'Disabled'}"
            else:
                return False, "Invalid streaming change format"
        except Exception as e:
            return False, f"Error parsing streaming change: {e}"
    
    def _generate_verification_report(self, verification_results: List[Dict[str, Any]], report_path: str) -> None:
        """Generate CSV verification report"""
        if not verification_results:
            logger.warning("No verification results to report")
            return
        
        # Define CSV fields
        fieldnames = [
            "asset_path", "asset_name", "changes_applied", "verification_status",
            "verification_details", "passed_checks", "total_checks",
            "estimated_memory_savings", "dry_run"
        ]
        
        # Write CSV report
        success = io_csv.write_csv(report_path, fieldnames, verification_results)
        if success:
            logger.info(f"Verification report written to {report_path}")
        else:
            logger.error(f"Failed to write verification report to {report_path}")
    
    def _generate_summary(self, verification_results: List[Dict[str, Any]], 
                         passed_count: int, failed_count: int) -> Dict[str, Any]:
        """Generate verification summary"""
        if not verification_results:
            return {"total_verified": 0, "passed": 0, "failed": 0, "success_rate": "0%"}
        
        # Calculate success rate
        total_verified = len(verification_results)
        success_rate = f"{(passed_count / total_verified) * 100:.1f}%" if total_verified > 0 else "0%"
        
        # Count by verification status
        status_counts = {}
        for result in verification_results:
            status = result.get('verification_status', 'Unknown')
            status_counts[status] = status_counts.get(status, 0) + 1
        
        # Calculate total checks
        total_checks = sum(result.get('total_checks', 0) for result in verification_results)
        passed_checks = sum(result.get('passed_checks', 0) for result in verification_results)
        
        return {
            "total_verified": total_verified,
            "passed": passed_count,
            "failed": failed_count,
            "success_rate": success_rate,
            "status_breakdown": status_counts,
            "total_checks": total_checks,
            "passed_checks": passed_checks,
            "check_success_rate": f"{(passed_checks / total_checks) * 100:.1f}%" if total_checks > 0 else "0%"
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
    Run texture verification (entry point)
    
    Args:
        config: Configuration dictionary
        output_dir: Output directory for reports
        
    Returns:
        Verification results dictionary
    """
    verifier = TextureVerifier(config)
    return verifier.run(config, output_dir)
