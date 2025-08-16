"""
Main entry point for MagicOptimizer plugin
"""

import os
import json
import logging
from pathlib import Path
from typing import Dict, List, Any, Optional
from datetime import datetime

# Import category modules
from . import io_csv
from . import utils
from .presets import get_preset_config, validate_preset

# Import optimization modules
from .textures import audit as textures_audit
from .textures import recommend as textures_recommend
from .textures import apply as textures_apply
from .textures import verify as textures_verify

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Optional knowledge and auto-report (now shipped with plugin)
try:
    from .knowledge.event_logger import EventLogger
    KNOWLEDGE_AVAILABLE = True
except Exception:
    KNOWLEDGE_AVAILABLE = False

try:
    from .auto_report import AutoReporter
    from .auto_report_config import is_auto_reporting_enabled, should_report_errors, should_report_optimizations
    AUTO_REPORT_AVAILABLE = True
except Exception:
    AUTO_REPORT_AVAILABLE = False

class MagicOptimizer:
    """Main orchestrator for optimization workflows"""
    
    def __init__(self, config_path: str = None):
        self.config_path = config_path
        self.config = {}
        self.run_id = None
        self.output_dir = None
        
        if config_path:
            self.load_config(config_path)
    
    def load_config(self, config_path: str) -> bool:
        """
        Load configuration from file
        
        Args:
            config_path: Path to configuration file
            
        Returns:
            True if successful
        """
        try:
            if os.path.exists(config_path):
                with open(config_path, 'r', encoding='utf-8') as f:
                    self.config = json.load(f)
                logger.info(f"Configuration loaded from {config_path}")
                return True
            else:
                logger.warning(f"Configuration file not found: {config_path}")
                return False
        except Exception as e:
            logger.error(f"Error loading configuration: {e}")
            return False
    
    def create_run_id(self) -> str:
        """Create unique run ID for this optimization session"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.run_id = f"run_{timestamp}"
        return self.run_id
    
    def setup_output_directory(self) -> str:
        """Setup output directory for this run"""
        if not self.run_id:
            self.create_run_id()
        
        # Get base output directory from config or use default
        base_output = self.config.get('paths', {}).get('output_dir', 'Saved/Optimizor/History')
        self.output_dir = os.path.join(base_output, self.run_id)
        
        # Ensure directory exists
        utils.ensure_directory_exists(self.output_dir)
        
        logger.info(f"Output directory: {self.output_dir}")
        return self.output_dir
    
    def validate_config(self) -> bool:
        """Validate configuration settings"""
        if not self.config:
            logger.error("No configuration loaded")
            return False
        
        # Check required fields
        required_fields = ['target_profile', 'categories']
        for field in required_fields:
            if field not in self.config:
                logger.error(f"Missing required field: {field}")
                return False
        
        # Validate target profile
        target_profile = self.config['target_profile']
        if not validate_preset(target_profile):
            logger.error(f"Invalid target profile: {target_profile}")
            return False
        
        # Validate categories
        valid_categories = ['Textures', 'Meshes', 'Materials', 'Levels']
        categories = self.config['categories']
        for category in categories:
            if category not in valid_categories:
                logger.error(f"Invalid category: {category}")
                return False
        
        logger.info("Configuration validation passed")
        return True
    
    def run_phase(self, phase: str, categories: List[str]) -> Dict[str, Any]:
        """
        Run a specific optimization phase
        
        Args:
            phase: Phase to run (audit, recommend, apply, verify)
            categories: Categories to process
            
        Returns:
            Results dictionary
        """
        if not self.validate_config():
            return self._create_error_result(phase, "Configuration validation failed")
        
        if not self.output_dir:
            self.setup_output_directory()
        
        logger.info(f"Running {phase} phase for categories: {categories}")
        
        results = {}
        total_scanned = 0
        total_changed = 0
        total_skipped = 0
        total_errors = 0
        
        # Initialize optional systems
        reporter = AutoReporter() if AUTO_REPORT_AVAILABLE and is_auto_reporting_enabled() else None
        event_logger = EventLogger(Path(self.output_dir).parent.parent.as_posix()) if KNOWLEDGE_AVAILABLE else None

        # Process each category
        for category in categories:
            try:
                if category == "Textures":
                    if phase == "audit":
                        result = textures_audit.run(self.config, self.output_dir)
                    elif phase == "recommend":
                        result = textures_recommend.run(self.config, self.output_dir)
                    elif phase == "apply":
                        result = textures_apply.run(self.config, self.output_dir)
                    elif phase == "verify":
                        result = textures_verify.run(self.config, self.output_dir)
                    else:
                        result = self._create_error_result(phase, category, f"Unknown phase: {phase}")
                
                elif category == "Meshes":
                    # TODO: Implement mesh optimization modules
                    result = self._create_placeholder_result(phase, category, "Mesh optimization not yet implemented")
                
                elif category == "Materials":
                    # TODO: Implement material optimization modules
                    result = self._create_placeholder_result(phase, category, "Material optimization not yet implemented")
                
                elif category == "Levels":
                    # TODO: Implement level optimization modules
                    result = self._create_placeholder_result(phase, category, "Level optimization not yet implemented")
                
                else:
                    result = self._create_error_result(phase, category, f"Unknown category: {category}")
                
                results[category] = result
                
                # Accumulate totals
                total_scanned += result.get('scanned', 0)
                total_changed += result.get('changed', 0)
                total_skipped += result.get('skipped', 0)
                total_errors += result.get('errors', 0)
                
            except Exception as e:
                logger.error(f"Error processing {category} in {phase} phase: {e}")
                if reporter and should_report_errors():
                    reporter.send_error_report("phase_error", str(e), context=f"phase={phase} category={category}")
                result = self._create_error_result(phase, category, str(e))
                results[category] = result
                total_errors += 1
        
        # Create summary result
        summary_result = {
            "phase": phase,
            "categories": categories,
            "scanned": total_scanned,
            "changed": total_changed,
            "skipped": total_skipped,
            "errors": total_errors,
            "artifacts": {
                "run_id": self.run_id,
                "output_dir": self.output_dir,
                "category_results": results
            },
            "summary": {
                "success": total_errors == 0,
                "total_categories": len(categories),
                "categories_with_errors": sum(1 for r in results.values() if r.get('errors', 0) > 0)
            }
        }
        
        # Save summary to file (schema_version for durability)
        self._save_summary(phase, summary_result)
        
        logger.info(f"{phase} phase completed. Scanned: {total_scanned}, Changed: {total_changed}, Errors: {total_errors}")
        # Report optimization summary
        if reporter and should_report_optimizations():
            try:
                reporter.send_optimization_report(
                    phase=phase,
                    profile=self.config.get('target_profile', ''),
                    assets_processed=total_scanned,
                    assets_modified=total_changed,
                    success=(total_errors == 0),
                    duration_seconds=0.0,
                    context=f"categories={','.join(categories)}"
                )
            except Exception:
                pass
        
        return summary_result
    
    def run_audit(self, categories: List[str] = None) -> Dict[str, Any]:
        """Run audit phase"""
        if categories is None:
            categories = self.config.get('categories', ['Textures'])
        return self.run_phase("audit", categories)
    
    def run_recommend(self, categories: List[str] = None) -> Dict[str, Any]:
        """Run recommend phase"""
        if categories is None:
            categories = self.config.get('categories', ['Textures'])
        return self.run_phase("recommend", categories)
    
    def run_apply(self, categories: List[str] = None) -> Dict[str, Any]:
        """Run apply phase"""
        if categories is None:
            categories = self.config.get('categories', ['Textures'])
        return self.run_phase("apply", categories)
    
    def run_verify(self, categories: List[str] = None) -> Dict[str, Any]:
        """Run verify phase"""
        if categories is None:
            categories = self.config.get('categories', ['Textures'])
        return self.run_phase("verify", categories)
    
    def run_full_workflow(self, categories: List[str] = None) -> Dict[str, Any]:
        """Run complete optimization workflow"""
        if categories is None:
            categories = self.config.get('categories', ['Textures'])
        
        logger.info("Starting full optimization workflow")
        
        # Run all phases
        audit_result = self.run_audit(categories)
        recommend_result = self.run_recommend(categories)
        apply_result = self.run_apply(categories)
        verify_result = self.run_verify(categories)
        
        # Create workflow summary
        workflow_result = {
            "workflow": "full",
            "run_id": self.run_id,
            "categories": categories,
            "phases": {
                "audit": audit_result,
                "recommend": recommend_result,
                "apply": apply_result,
                "verify": verify_result
            },
            "summary": {
                "total_scanned": sum(r.get('scanned', 0) for r in [audit_result, recommend_result, apply_result, verify_result]),
                "total_changed": sum(r.get('changed', 0) for r in [audit_result, recommend_result, apply_result, verify_result]),
                "total_errors": sum(r.get('errors', 0) for r in [audit_result, recommend_result, apply_result, verify_result]),
                "success": all(r.get('summary', {}).get('success', False) for r in [audit_result, recommend_result, apply_result, verify_result])
            }
        }
        
        # Save workflow summary
        self._save_workflow_summary(workflow_result)
        
        logger.info("Full optimization workflow completed")
        return workflow_result
    
    def _save_summary(self, phase: str, result: Dict[str, Any]) -> None:
        """Save phase summary to file"""
        if not self.output_dir:
            return
        
        summary_file = os.path.join(self.output_dir, f"{phase}_summary.json")
        utils.save_json_config(summary_file, result)
    
    def _save_workflow_summary(self, workflow_result: Dict[str, Any]) -> None:
        """Save workflow summary to file"""
        if not self.output_dir:
            return
        
        summary_file = os.path.join(self.output_dir, "workflow_summary.json")
        utils.save_json_config(summary_file, workflow_result)
    
    def _create_error_result(self, phase: str, category: str, error_message: str) -> Dict[str, Any]:
        """Create error result structure"""
        return {
            "phase": phase,
            "category": category,
            "scanned": 0,
            "changed": 0,
            "skipped": 0,
            "errors": 1,
            "artifacts": {},
            "summary": {
                "success": False,
                "error": error_message
            }
        }
    
    def _create_placeholder_result(self, phase: str, category: str, message: str) -> Dict[str, Any]:
        """Create placeholder result for unimplemented features"""
        return {
            "phase": phase,
            "category": category,
            "scanned": 0,
            "changed": 0,
            "skipped": 0,
            "errors": 0,
            "artifacts": {},
            "summary": {
                "success": True,
                "message": message
            }
        }

def run(config_path: str, phase: str, category: str) -> Dict[str, Any]:
    """
    Main entry point function
    
    Args:
        config_path: Path to configuration file
        phase: Phase to run (audit, recommend, apply, verify)
        category: Category to process (Textures, Meshes, Materials, Levels)
        
    Returns:
        Results dictionary
    """
    try:
        optimizer = MagicOptimizer(config_path)
        
        if phase == "audit":
            return optimizer.run_audit([category])
        elif phase == "recommend":
            return optimizer.run_recommend([category])
        elif phase == "apply":
            return optimizer.run_apply([category])
        elif phase == "verify":
            return optimizer.run_verify([category])
        else:
            return {
                "phase": phase,
                "category": category,
                "scanned": 0,
                "changed": 0,
                "skipped": 0,
                "errors": 1,
                "artifacts": {},
                "summary": {
                    "success": False,
                    "error": f"Unknown phase: {phase}"
                }
            }
    
    except Exception as e:
        logger.error(f"Error in main entry point: {e}")
        return {
            "phase": phase,
            "category": category,
            "scanned": 0,
            "changed": 0,
            "skipped": 0,
            "errors": 1,
            "artifacts": {},
            "summary": {
                "success": False,
                "error": str(e)
            }
        }

def create_default_config(output_path: str, target_profile: str = "PC_Balanced") -> bool:
    """
    Create default configuration file
    
    Args:
        output_path: Path to output configuration file
        target_profile: Target optimization profile
        
    Returns:
        True if successful
    """
    default_config = {
        "target_profile": target_profile,
        "categories": ["Textures", "Meshes", "Materials", "Levels"],
        "use_selection": False,
        "include_paths": [],
        "exclude_paths": [],
        "dry_run": True,
        "max_changes": 1000,
        "conservative_mode": True,
        "paths": {
            "output_dir": "Saved/Optimizor/History"
        },
        "safety": {
            "create_backups": True,
            "source_control_checkout": False,
            "close_editor": False
        }
    }
    
    return utils.save_json_config(output_path, default_config)

if __name__ == "__main__":
    # Example usage
    import sys
    
    if len(sys.argv) < 4:
        print("Usage: python entry.py <config_path> <phase> <category>")
        print("Phases: audit, recommend, apply, verify")
        print("Categories: Textures, Meshes, Materials, Levels")
        sys.exit(1)
    
    config_path = sys.argv[1]
    phase = sys.argv[2]
    category = sys.argv[3]
    
    result = run(config_path, phase, category)
    print(json.dumps(result, indent=2))
