"""
Event Logger for MagicOptimizer plugin.

Automatically logs user actions, optimization results, and usage patterns
to enable self-learning and plugin improvement over time.
"""

import os
import json
import time
from datetime import datetime
from typing import Dict, Any, Optional, List


class EventLogger:
    """Logs events for self-learning and plugin improvement."""
    
    def __init__(self, project_saved_dir: str):
        """Initialize the event logger.
        
        Args:
            project_saved_dir: Path to the project's Saved directory
        """
        self.project_saved_dir = project_saved_dir
        self.knowledge_dir = os.path.join(project_saved_dir, "MagicOptimizer", "Knowledge")
        self.run_id = datetime.now().strftime('%Y%m%d_%H%M%S')
        
        # Ensure knowledge directory exists
        os.makedirs(self.knowledge_dir, exist_ok=True)
        
        # Event log file
        self.events_file = os.path.join(self.knowledge_dir, "events.jsonl")
        
        # Session tracking
        self.session_start = time.time()
        self.event_count = 0
        
        # Log initialization
        self._log_event("system", "event_logger_initialized", {
            "run_id": self.run_id,
            "session_start": datetime.fromtimestamp(self.session_start).isoformat(),
            "knowledge_dir": self.knowledge_dir
        })
    
    def _log_event(self, category: str, event_type: str, data: Dict[str, Any]) -> None:
        """Log an event to the events.jsonl file.
        
        Args:
            category: Event category (system, user_action, optimization, error, etc.)
            event_type: Specific event type
            data: Event data dictionary
        """
        try:
            event = {
                "timestamp": datetime.now().isoformat(timespec='seconds'),
                "run_id": self.run_id,
                "category": category,
                "event_type": event_type,
                "session_elapsed": time.time() - self.session_start,
                "event_number": self.event_count,
                "data": data
            }
            
            with open(self.events_file, 'a', encoding='utf-8') as f:
                f.write(json.dumps(event, ensure_ascii=False) + "\n")
            
            self.event_count += 1
            
        except Exception as e:
            # Silently fail to avoid breaking plugin functionality
            pass
    
    def log_user_action(self, action: str, phase: str, profile: str, 
                        include_paths: List[str] = None, exclude_paths: List[str] = None,
                        use_selection: bool = False, dry_run: bool = True,
                        max_changes: int = 100, categories: List[str] = None) -> None:
        """Log a user action (phase execution).
        
        Args:
            action: Action performed (Audit, Recommend, Apply, Verify)
            phase: Optimization phase
            profile: Target optimization profile
            include_paths: Paths to include
            exclude_paths: Paths to exclude
            use_selection: Whether to use current selection
            dry_run: Whether this is a dry run
            max_changes: Maximum changes allowed
            categories: Asset categories to process
        """
        self._log_event("user_action", f"{action.lower()}_started", {
            "action": action,
            "phase": phase,
            "profile": profile,
            "include_paths": include_paths or [],
            "exclude_paths": exclude_paths or [],
            "use_selection": use_selection,
            "dry_run": dry_run,
            "max_changes": max_changes,
            "categories": categories or []
        })
    
    def log_optimization_result(self, phase: str, profile: str, 
                               assets_processed: int, assets_modified: int,
                               success: bool, message: str, 
                               processing_time: float, errors: List[str] = None) -> None:
        """Log optimization phase results.
        
        Args:
            phase: Optimization phase
            profile: Target profile
            assets_processed: Number of assets processed
            assets_modified: Number of assets modified
            success: Whether the operation succeeded
            message: Result message
            processing_time: Time taken in seconds
            errors: List of errors encountered
        """
        self._log_event("optimization_result", f"{phase.lower()}_completed", {
            "phase": phase,
            "profile": profile,
            "assets_processed": assets_processed,
            "assets_modified": assets_modified,
            "success": success,
            "message": message,
            "processing_time": processing_time,
            "errors": errors or []
        })
    
    def log_asset_pattern(self, asset_type: str, asset_path: str, 
                          properties: Dict[str, Any], profile: str) -> None:
        """Log asset patterns for analysis.
        
        Args:
            asset_type: Type of asset (Texture2D, StaticMesh, Material, etc.)
            asset_path: Asset path
            properties: Asset properties (size, format, etc.)
            profile: Target optimization profile
        """
        self._log_event("asset_pattern", f"{asset_type.lower()}_observed", {
            "asset_type": asset_type,
            "asset_path": asset_path,
            "properties": properties,
            "profile": profile
        })
    
    def log_error(self, error_type: str, error_message: str, 
                  context: Dict[str, Any] = None) -> None:
        """Log errors for debugging and improvement.
        
        Args:
            error_type: Type of error
            error_message: Error message
            context: Additional context
        """
        self._log_event("error", error_type, {
            "error_message": error_message,
            "context": context or {}
        })
    
    def log_performance(self, operation: str, duration: float, 
                        memory_usage: Optional[int] = None) -> None:
        """Log performance metrics.
        
        Args:
            operation: Operation performed
            duration: Duration in seconds
            memory_usage: Memory usage in bytes (if available)
        """
        self._log_event("performance", f"{operation}_timing", {
            "operation": operation,
            "duration": duration,
            "memory_usage": memory_usage
        })
    
    def log_ui_interaction(self, ui_element: str, action: str, 
                           context: Dict[str, Any] = None) -> None:
        """Log UI interactions for UX improvement.
        
        Args:
            ui_element: UI element interacted with
            action: Action performed
            context: Additional context
        """
        self._log_event("ui_interaction", f"{ui_element}_{action}", {
            "ui_element": ui_element,
            "action": action,
            "context": context or {}
        })
    
    def log_session_end(self) -> None:
        """Log session end with summary statistics."""
        session_duration = time.time() - self.session_start
        
        self._log_event("system", "session_ended", {
            "session_duration": session_duration,
            "total_events": self.event_count,
            "run_id": self.run_id
        })
    
    def get_session_summary(self) -> Dict[str, Any]:
        """Get a summary of the current session.
        
        Returns:
            Dictionary with session summary
        """
        return {
            "run_id": self.run_id,
            "session_start": datetime.fromtimestamp(self.session_start).isoformat(),
            "session_duration": time.time() - self.session_start,
            "event_count": self.event_count,
            "knowledge_dir": self.knowledge_dir
        }
