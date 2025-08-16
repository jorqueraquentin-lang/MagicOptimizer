"""
Auto-report system (plugin-shipped) with basic payload size guards and retry.
"""
import os, json, time, sys
from datetime import datetime
from typing import Optional, Tuple, Dict, Any

try:
    import urllib.request, urllib.parse, urllib.error
    HTTP_AVAILABLE = True
except ImportError:
    HTTP_AVAILABLE = False

try:
    import unreal
    UE_AVAILABLE = True
except ImportError:
    UE_AVAILABLE = False

MAX_LOG_BYTES = 256 * 1024
MAX_KNOWLEDGE_BYTES = 256 * 1024

def _clip(s: str, limit: int) -> str:
    if s is None:
        return ""
    if len(s.encode('utf-8')) <= limit:
        return s
    # approximate clipping on characters
    return s[: max(0, limit // 2)] + "\n...[truncated]...\n" + s[-max(0, limit // 2):]

class AutoReporter:
    def __init__(self, worker_url: Optional[str] = None):
        self.worker_url = worker_url or os.environ.get('MAGICOPTIMIZER_WORKER_URL') or ""
        if not self.worker_url:
            self.worker_url = "https://your-worker.your-subdomain.workers.dev/report"
        self.plugin_version = os.environ.get('MAGICOPTIMIZER_VERSION', '1.0.0')
        try:
            if UE_AVAILABLE and hasattr(unreal, 'MagicOptimizer'):
                self.plugin_version = getattr(unreal.MagicOptimizer, 'version', self.plugin_version)
        except Exception:
            pass

    def _system_info(self) -> Dict[str, Any]:
        info = {
            'timestamp': datetime.now().isoformat(),
            'plugin_version': self.plugin_version,
            'python': f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}",
        }
        try:
            if UE_AVAILABLE:
                info['project'] = os.path.splitext(os.path.basename(unreal.Paths.get_project_file_path() or ""))[0]
        except Exception:
            pass
        return info

    def _read_tail(self, path: str, max_bytes: int) -> str:
        try:
            if not path or not os.path.exists(path):
                return ""
            with open(path, 'rb') as f:
                f.seek(0, os.SEEK_END)
                size = f.tell()
                f.seek(max(0, size - max_bytes))
                data = f.read()
            return data.decode('utf-8', errors='replace')
        except Exception:
            return ""

    def _knowledge_blob(self) -> str:
        try:
            saved = os.environ.get('MAGICOPTIMIZER_SAVED_DIR', 'Saved')
            kb_dir = os.path.join(saved, 'MagicOptimizer', 'Knowledge')
            path = os.path.join(kb_dir, 'events.jsonl')
            return _clip(self._read_tail(path, MAX_KNOWLEDGE_BYTES), MAX_KNOWLEDGE_BYTES)
        except Exception:
            return ""

    def _post(self, metadata: Dict[str, Any], report: Dict[str, Any], logs: str = "", knowledge: str = "") -> Tuple[bool, str, Optional[str]]:
        if not HTTP_AVAILABLE:
            return False, "HTTP unavailable", None
        boundary = f"----MagicOptimizer{int(time.time())}"
        parts = []
        def add_field(name: str, content: str, ctype: str):
            parts.append(f"--{boundary}")
            parts.append(f"Content-Disposition: form-data; name=\"{name}\"")
            parts.append(f"Content-Type: {ctype}")
            parts.append("")
            parts.append(content)
        add_field("metadata", json.dumps(metadata), "application/json")
        add_field("report", json.dumps(report), "application/json")
        if logs:
            parts.append(f"--{boundary}")
            parts.append('Content-Disposition: form-data; name="logs"; filename="plugin_logs.txt"')
            parts.append('Content-Type: text/plain')
            parts.append("")
            parts.append(_clip(logs, MAX_LOG_BYTES))
        if knowledge:
            parts.append(f"--{boundary}")
            parts.append('Content-Disposition: form-data; name="knowledge"; filename="knowledge.jsonl"')
            parts.append('Content-Type: application/json')
            parts.append("")
            parts.append(_clip(knowledge, MAX_KNOWLEDGE_BYTES))
        parts.append(f"--{boundary}--")
        parts.append("")
        payload = "\r\n".join(parts).encode('utf-8')
        headers = {
            'Content-Type': f'multipart/form-data; boundary={boundary}',
            'Content-Length': str(len(payload)),
            'User-Agent': f'MagicOptimizer/{self.plugin_version}'
        }
        req = urllib.request.Request(self.worker_url, data=payload, headers=headers, method='POST')
        # simple retry
        for attempt in range(3):
            try:
                with urllib.request.urlopen(req, timeout=30) as resp:
                    body = resp.read().decode('utf-8', errors='replace')
                    if resp.getcode() == 200:
                        try:
                            j = json.loads(body)
                            return (bool(j.get('success')), j.get('message', ''), j.get('issue_url'))
                        except Exception:
                            return False, body, None
                    return False, f"HTTP {resp.getcode()}: {body}", None
            except Exception as e:
                time.sleep(1.5 * (attempt + 1))
                last = str(e)
        return False, last if 'last' in locals() else 'error', None

    def send_error_report(self, error_type: str, error_message: str, context: str = "",
                          include_logs: bool = True, include_knowledge: bool = True) -> Tuple[bool, str, Optional[str]]:
        meta = {
            'title': f"[{error_type}] {error_message[:100]}...",
            'labels': ['auto-report', 'error', error_type.lower()],
        }
        rep = {
            'type': 'error',
            'context': context,
            'system_info': self._system_info(),
            'error_type': error_type,
            'error_message': error_message,
        }
        logs = self._read_tail(os.environ.get('MAGICOPTIMIZER_LOG', ''), MAX_LOG_BYTES) if include_logs else ""
        kb = self._knowledge_blob() if include_knowledge else ""
        return self._post(meta, rep, logs, kb)

    def send_optimization_report(self, phase: str, profile: str, assets_processed: int, assets_modified: int,
                                 success: bool, duration_seconds: float, context: str = "") -> Tuple[bool, str, Optional[str]]:
        meta = {
            'title': f"{'✅' if success else '❌'} {phase.title()} Phase - {profile}",
            'labels': ['auto-report', 'optimization', phase.lower(), profile.lower()],
        }
        rep = {
            'type': 'optimization',
            'phase': phase,
            'profile': profile,
            'assets_processed': assets_processed,
            'assets_modified': assets_modified,
            'success': success,
            'duration_seconds': duration_seconds,
            'context': context,
            'system_info': self._system_info()
        }
        return self._post(meta, rep)


