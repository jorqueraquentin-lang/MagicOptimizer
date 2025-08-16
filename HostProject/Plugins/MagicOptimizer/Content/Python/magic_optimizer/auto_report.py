"""
Auto-report system for MagicOptimizer plugin.
Sends issue reports to Cloudflare Worker endpoint for GitHub issue creation.
"""

import os
import json
import time
from datetime import datetime
from typing import Optional, Tuple, Dict, Any
import sys

try:
    import urllib.request
    import urllib.parse
    import urllib.error
    HTTP_AVAILABLE = True
except ImportError:
    HTTP_AVAILABLE = False

try:
    import unreal
    UE_AVAILABLE = True
except ImportError:
    UE_AVAILABLE = False


class AutoReporter:
    """Handles automatic issue reporting to Cloudflare Worker endpoint."""
    
    def __init__(self, worker_url: str = None):
        """
        Initialize the auto-reporter.
        
        Args:
            worker_url: Cloudflare Worker endpoint URL. If None, will try to get from environment.
        """
        self.worker_url = worker_url or os.environ.get('MAGICOPTIMIZER_WORKER_URL')
        if not self.worker_url:
            # Default to a placeholder - should be configured via environment
            self.worker_url = "https://your-worker.your-subdomain.workers.dev/report"
        
        self.plugin_version = self._get_plugin_version()
        self.project_name = self._get_project_name()
        self.user_id = self._get_user_id()
    
    def _get_plugin_version(self) -> str:
        """Get the current plugin version."""
        try:
            if UE_AVAILABLE and hasattr(unreal, 'MagicOptimizer'):
                # Try to get version from plugin module
                return getattr(unreal.MagicOptimizer, 'version', 'unknown')
        except Exception:
            pass
        
        # Fallback to environment or default
        return os.environ.get('MAGICOPTIMIZER_VERSION', '1.0.0')
    
    def _get_project_name(self) -> str:
        """Get the current project name."""
        try:
            if UE_AVAILABLE and hasattr(unreal, 'Paths'):
                project_path = unreal.Paths.get_project_file_path()
                if project_path:
                    return os.path.splitext(os.path.basename(project_path))[0]
        except Exception:
            pass
        
        # Fallback to environment or current directory
        return os.environ.get('MAGICOPTIMIZER_PROJECT', os.path.basename(os.getcwd()))
    
    def _get_user_id(self) -> str:
        """Get a user identifier (anonymized)."""
        try:
            if UE_AVAILABLE and hasattr(unreal, 'Paths'):
                # Use project saved directory as a stable identifier
                saved_dir = unreal.Paths.project_saved_dir()
                if saved_dir:
                    # Create a hash of the project path for anonymity
                    import hashlib
                    return hashlib.md5(saved_dir.encode()).hexdigest()[:8]
        except Exception:
            pass
        
        # Fallback to environment or default
        return os.environ.get('MAGICOPTIMIZER_USER_ID', 'unknown')
    
    def _gather_system_info(self) -> Dict[str, Any]:
        """Gather system and environment information."""
        info = {
            'timestamp': datetime.now().isoformat(),
            'plugin_version': self.plugin_version,
            'project_name': self.project_name,
            'user_id': self.user_id,
            'platform': 'unknown',
            'ue_version': 'unknown',
            'python_version': f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}",
        }
        
        try:
            if UE_AVAILABLE:
                # Get UE version
                if hasattr(unreal, 'Engine'):
                    info['ue_version'] = getattr(unreal.Engine, 'get_version', lambda: 'unknown')()
                
                # Get platform
                if hasattr(unreal, 'PlatformMisc'):
                    info['platform'] = getattr(unreal.PlatformMisc, 'get_platform_name', lambda: 'unknown')()
        except Exception:
            pass
        
        return info
    
    def _gather_logs(self, max_lines: int = 1000) -> str:
        """Gather recent logs for the report."""
        try:
            log_path = os.environ.get('MAGICOPTIMIZER_LOG')
            if not log_path or not os.path.exists(log_path):
                return "No log file available"
            
            with open(log_path, 'r', encoding='utf-8') as f:
                lines = f.readlines()
            
            # Get last N lines
            recent_lines = lines[-max_lines:] if len(lines) > max_lines else lines
            return ''.join(recent_lines)
        except Exception as e:
            return f"Error reading logs: {e}"
    
    def _gather_knowledge_data(self) -> Dict[str, Any]:
        """Gather knowledge base data if available."""
        try:
            kb_dir = os.path.join(
                os.environ.get('MAGICOPTIMIZER_SAVED_DIR', 'Saved'),
                'MagicOptimizer',
                'Knowledge'
            )
            
            if not os.path.exists(kb_dir):
                return {}
            
            data = {}
            
            # Get events.jsonl
            events_path = os.path.join(kb_dir, 'events.jsonl')
            if os.path.exists(events_path):
                with open(events_path, 'r', encoding='utf-8') as f:
                    lines = f.readlines()
                    data['recent_events'] = lines[-50:] if len(lines) > 50 else lines
            
            # Get any CSV files
            csv_files = [f for f in os.listdir(kb_dir) if f.endswith('.csv')]
            if csv_files:
                data['available_csvs'] = csv_files
            
            return data
        except Exception:
            return {}
    
    def send_error_report(self, 
                         error_type: str, 
                         error_message: str, 
                         context: str = "",
                         include_logs: bool = True,
                         include_knowledge: bool = True) -> Tuple[bool, str, Optional[str]]:
        """
        Send an error report to the Cloudflare Worker.
        
        Args:
            error_type: Type/category of the error
            error_message: Detailed error message
            context: Additional context information
            include_logs: Whether to include log files
            include_knowledge: Whether to include knowledge base data
            
        Returns:
            Tuple of (success, message, issue_url)
        """
        if not HTTP_AVAILABLE:
            return False, "HTTP libraries not available", None
        
        try:
            # Prepare the report data
            report_data = {
                'type': 'error',
                'error_type': error_type,
                'error_message': error_message,
                'context': context,
                'system_info': self._gather_system_info()
            }
            
            # Add logs if requested
            if include_logs:
                report_data['logs'] = self._gather_logs()
            
            # Add knowledge data if requested
            if include_knowledge:
                report_data['knowledge'] = self._gather_knowledge_data()
            
            # Create multipart form data
            boundary = f"----MagicOptimizer{int(time.time())}"
            
            # Build the multipart payload
            payload_parts = []
            
            # Add JSON metadata
            metadata = {
                'title': f"[{error_type}] {error_message[:100]}...",
                'description': f"**Error Type:** {error_type}\n\n**Error Message:** {error_message}\n\n**Context:** {context}\n\n**System Info:** {json.dumps(report_data['system_info'], indent=2)}",
                'labels': ['auto-report', 'error', error_type.lower()],
                'project': self.project_name,
                'plugin_version': self.plugin_version
            }
            
            payload_parts.append(f"--{boundary}")
            payload_parts.append('Content-Disposition: form-data; name="metadata"')
            payload_parts.append('Content-Type: application/json')
            payload_parts.append("")
            payload_parts.append(json.dumps(metadata))
            
            # Add full report data
            payload_parts.append(f"--{boundary}")
            payload_parts.append('Content-Disposition: form-data; name="report"')
            payload_parts.append('Content-Type: application/json')
            payload_parts.append("")
            payload_parts.append(json.dumps(report_data))
            
            # Add logs as separate file if present
            if include_logs and 'logs' in report_data:
                payload_parts.append(f"--{boundary}")
                payload_parts.append('Content-Disposition: form-data; name="logs"; filename="plugin_logs.txt"')
                payload_parts.append('Content-Type: text/plain')
                payload_parts.append("")
                payload_parts.append(report_data['logs'])
            
            # Add knowledge data as separate file if present
            if include_knowledge and 'knowledge' in report_data:
                payload_parts.append(f"--{boundary}")
                payload_parts.append('Content-Disposition: form-data; name="knowledge"; filename="knowledge_data.json"')
                payload_parts.append('Content-Type: application/json')
                payload_parts.append("")
                payload_parts.append(json.dumps(report_data['knowledge']))
            
            payload_parts.append(f"--{boundary}--")
            payload_parts.append("")
            
            # Join all parts
            payload = "\r\n".join(payload_parts).encode('utf-8')
            
            # Prepare the request
            headers = {
                'Content-Type': f'multipart/form-data; boundary={boundary}',
                'Content-Length': str(len(payload)),
                'User-Agent': f'MagicOptimizer/{self.plugin_version}'
            }
            
            # Send the request
            req = urllib.request.Request(
                self.worker_url,
                data=payload,
                headers=headers,
                method='POST'
            )
            
            with urllib.request.urlopen(req, timeout=30) as response:
                response_data = response.read().decode('utf-8')
                response_code = response.getcode()
                
                if response_code == 200:
                    try:
                        result = json.loads(response_data)
                        if result.get('success'):
                            issue_url = result.get('issue_url')
                            return True, "Report sent successfully", issue_url
                        else:
                            return False, f"Worker error: {result.get('message', 'Unknown error')}", None
                    except json.JSONDecodeError:
                        return False, f"Invalid response from worker: {response_data}", None
                else:
                    return False, f"HTTP {response_code}: {response_data}", None
                    
        except urllib.error.URLError as e:
            return False, f"Network error: {e.reason}", None
        except urllib.error.HTTPError as e:
            return False, f"HTTP error {e.code}: {e.reason}", None
        except Exception as e:
            return False, f"Unexpected error: {e}", None
    
    def send_optimization_report(self,
                                phase: str,
                                profile: str,
                                assets_processed: int,
                                assets_modified: int,
                                success: bool,
                                duration_seconds: float,
                                context: str = "") -> Tuple[bool, str, Optional[str]]:
        """
        Send an optimization report to the Cloudflare Worker.
        
        Args:
            phase: Optimization phase (audit, recommend, apply, verify)
            profile: Profile used for optimization
            assets_processed: Number of assets processed
            assets_modified: Number of assets modified
            success: Whether the optimization was successful
            duration_seconds: How long the optimization took
            context: Additional context information
            
        Returns:
            Tuple of (success, message, issue_url)
        """
        if not HTTP_AVAILABLE:
            return False, "HTTP libraries not available", None
        
        try:
            # Prepare the report data
            report_data = {
                'type': 'optimization',
                'phase': phase,
                'profile': profile,
                'assets_processed': assets_processed,
                'assets_modified': assets_modified,
                'success': success,
                'duration_seconds': duration_seconds,
                'context': context,
                'system_info': self._gather_system_info()
            }
            
            # Create multipart form data
            boundary = f"----MagicOptimizer{int(time.time())}"
            
            # Build the multipart payload
            payload_parts = []
            
            # Add JSON metadata
            status_emoji = "✅" if success else "❌"
            metadata = {
                'title': f"{status_emoji} {phase.title()} Phase - {profile}",
                'description': f"**Phase:** {phase}\n**Profile:** {profile}\n**Assets Processed:** {assets_processed}\n**Assets Modified:** {assets_modified}\n**Success:** {success}\n**Duration:** {duration_seconds:.2f}s\n\n**Context:** {context}\n\n**System Info:** {json.dumps(report_data['system_info'], indent=2)}",
                'labels': ['auto-report', 'optimization', phase.lower(), profile.lower()],
                'project': self.project_name,
                'plugin_version': self.plugin_version
            }
            
            payload_parts.append(f"--{boundary}")
            payload_parts.append('Content-Disposition: form-data; name="metadata"')
            payload_parts.append('Content-Type: application/json')
            payload_parts.append("")
            payload_parts.append(json.dumps(metadata))
            
            # Add full report data
            payload_parts.append(f"--{boundary}")
            payload_parts.append('Content-Disposition: form-data; name="report"')
            payload_parts.append('Content-Type: application/json')
            payload_parts.append("")
            payload_parts.append(json.dumps(report_data))
            
            # Join all parts
            payload = "\r\n".join(payload_parts).encode('utf-8')
            
            # Prepare the request
            headers = {
                'Content-Type': f'multipart/form-data; boundary={boundary}',
                'Content-Length': str(len(payload)),
                'User-Agent': f'MagicOptimizer/{self.plugin_version}'
            }
            
            # Send the request
            req = urllib.request.Request(
                self.worker_url,
                data=payload,
                headers=headers,
                method='POST'
            )
            
            with urllib.request.urlopen(req, timeout=30) as response:
                response_data = response.read().decode('utf-8')
                response_code = response.getcode()
                
                if response_code == 200:
                    try:
                        result = json.loads(response_data)
                        if result.get('success'):
                            issue_url = result.get('issue_url')
                            return True, "Report sent successfully", issue_url
                        else:
                            return False, f"Worker error: {result.get('message', 'Unknown error')}", None
                    except json.JSONDecodeError:
                        return False, f"Invalid response from worker: {response_data}", None
                else:
                    return False, f"HTTP {response_code}: {response_data}", None
                    
        except urllib.error.URLError as e:
            return False, f"Network error: {e.reason}", None
        except urllib.error.HTTPError as e:
            return False, f"HTTP error {e.code}: {e.reason}", None
        except Exception as e:
            return False, f"Unexpected error: {e}", None


# Global instance
_auto_reporter = None

def get_auto_reporter() -> Optional[AutoReporter]:
    """Get the global auto-reporter instance."""
    global _auto_reporter
    if _auto_reporter is None:
        _auto_reporter = AutoReporter()
    return _auto_reporter

def send_error_report(error_type: str, error_message: str, context: str = "") -> Tuple[bool, str, Optional[str]]:
    """Convenience function to send an error report."""
    reporter = get_auto_reporter()
    if reporter:
        return reporter.send_error_report(error_type, error_message, context)
    return False, "Auto-reporter not available", None

def send_optimization_report(phase: str, profile: str, assets_processed: int, 
                           assets_modified: int, success: bool, duration_seconds: float, 
                           context: str = "") -> Tuple[bool, str, Optional[str]]:
    """Convenience function to send an optimization report."""
    reporter = get_auto_reporter()
    if reporter:
        return reporter.send_optimization_report(phase, profile, assets_processed, 
                                              assets_modified, success, duration_seconds, context)
    return False, "Auto-reporter not available", None
