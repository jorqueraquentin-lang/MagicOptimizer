"""
Event Logger for MagicOptimizer (plugin-shipped).
Adds light rotation and schema tagging for durability.
"""
import os, json, time
from datetime import datetime
from typing import Dict, Any, Optional, List

SCHEMA_VERSION = 1
MAX_EVENTS_JSONL_BYTES = 5 * 1024 * 1024  # 5 MB

class EventLogger:
    def __init__(self, project_saved_dir: str):
        self.project_saved_dir = project_saved_dir
        self.knowledge_dir = os.path.join(project_saved_dir, "MagicOptimizer", "Knowledge")
        os.makedirs(self.knowledge_dir, exist_ok=True)
        self.events_file = os.path.join(self.knowledge_dir, "events.jsonl")
        self.run_id = datetime.now().strftime('%Y%m%d_%H%M%S')
        self.session_start = time.time()
        self.event_count = 0
        self._log("system", "event_logger_initialized", {
            "run_id": self.run_id,
            "session_start": datetime.fromtimestamp(self.session_start).isoformat(),
            "knowledge_dir": self.knowledge_dir
        })

    def _rotate_if_needed(self):
        try:
            if os.path.exists(self.events_file) and os.path.getsize(self.events_file) > MAX_EVENTS_JSONL_BYTES:
                ts = datetime.now().strftime('%Y%m%d_%H%M%S')
                os.replace(self.events_file, self.events_file.replace('events.jsonl', f'events_{ts}.jsonl'))
        except Exception:
            pass

    def _log(self, category: str, event_type: str, data: Dict[str, Any]) -> None:
        try:
            event = {
                "schema": SCHEMA_VERSION,
                "timestamp": datetime.now().isoformat(timespec='seconds'),
                "run_id": self.run_id,
                "category": category,
                "event_type": event_type,
                "session_elapsed": time.time() - self.session_start,
                "event_number": self.event_count,
                "data": data or {}
            }
            self._rotate_if_needed()
            with open(self.events_file, 'a', encoding='utf-8') as f:
                f.write(json.dumps(event, ensure_ascii=False) + "\n")
            self.event_count += 1
        except Exception:
            pass

    def log_user_action(self, action: str, phase: str, profile: str, include_paths: List[str] = None,
                        exclude_paths: List[str] = None, use_selection: bool = False, dry_run: bool = True,
                        max_changes: int = 100, categories: List[str] = None) -> None:
        self._log("user_action", f"{action.lower()}_started", {
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

    def log_optimization_result(self, phase: str, profile: str, assets_processed: int, assets_modified: int,
                                success: bool, message: str, processing_time: float, errors: List[str] = None) -> None:
        self._log("optimization_result", f"{phase.lower()}_completed", {
            "phase": phase,
            "profile": profile,
            "assets_processed": assets_processed,
            "assets_modified": assets_modified,
            "success": success,
            "message": message,
            "processing_time": processing_time,
            "errors": errors or []
        })

    def log_error(self, error_type: str, error_message: str, context: Dict[str, Any] = None) -> None:
        self._log("error", error_type, {"error_message": error_message, "context": context or {}})

    def log_session_end(self) -> None:
        self._log("system", "session_ended", {
            "session_duration": time.time() - self.session_start,
            "total_events": self.event_count,
            "run_id": self.run_id
        })


