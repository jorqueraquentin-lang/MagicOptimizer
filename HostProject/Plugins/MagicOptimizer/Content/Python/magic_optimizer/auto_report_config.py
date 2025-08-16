"""
Configuration for the auto-report system.
Controls whether auto-reporting is enabled and configures the Cloudflare Worker endpoint.
"""

import os
import json
from typing import Optional

try:
    import unreal
    UE_AVAILABLE = True
except ImportError:
    UE_AVAILABLE = False


class AutoReportConfig:
    """Configuration manager for the auto-report system."""
    
    def __init__(self, config_dir: str = None):
        """
        Initialize the configuration manager.
        
        Args:
            config_dir: Directory to store configuration files. If None, will use UE Saved directory.
        """
        self.config_dir = config_dir or self._get_default_config_dir()
        self.config_file = os.path.join(self.config_dir, 'auto_report_config.json')
        self._load_config()
    
    def _get_default_config_dir(self) -> str:
        """Get the default configuration directory."""
        try:
            if UE_AVAILABLE and hasattr(unreal, 'Paths'):
                saved_dir = unreal.Paths.project_saved_dir()
                if saved_dir:
                    return os.path.join(saved_dir, 'MagicOptimizer', 'Config')
        except Exception:
            pass
        
        # Fallback to current directory
        return os.path.join(os.getcwd(), 'Saved', 'MagicOptimizer', 'Config')
    
    def _load_config(self):
        """Load configuration from file or create default."""
        try:
            # First try to get settings from UE
            ue_settings = self._get_ue_settings()
            if ue_settings:
                # Use UE settings as base, merge with defaults
                self.config = self._get_default_config()
                self.config.update(ue_settings)
                return
            
            # Fallback to file-based config
            os.makedirs(self.config_dir, exist_ok=True)
            
            if os.path.exists(self.config_file):
                with open(self.config_file, 'r', encoding='utf-8') as f:
                    self.config = json.load(f)
            else:
                # Create default configuration
                self.config = self._get_default_config()
                self._save_config()
        except Exception:
            # Fallback to default config
            self.config = self._get_default_config()
    
    def _get_default_config(self) -> dict:
        """Get the default configuration."""
        return {
            'enabled': True,
            'worker_url': os.environ.get('MAGICOPTIMIZER_WORKER_URL', ''),
            'include_logs': True,
            'include_knowledge': True,
            'max_log_lines': 1000,
            'report_errors': True,
            'report_optimizations': True,
            'report_sessions': False,
            'anonymize_data': True,
            'rate_limit': {
                'max_reports_per_hour': 10,
                'max_reports_per_day': 50
            }
        }
    
    def _get_ue_settings(self) -> dict:
        """Get settings from Unreal Engine if available."""
        try:
            # Try to import and use the UE settings module
            from . import ue_settings
            return {
                'enabled': ue_settings.is_auto_reporting_enabled() and ue_settings.has_user_consent(),
                'worker_url': ue_settings.get_worker_url(),
                'include_logs': ue_settings.should_include_logs(),
                'include_knowledge': ue_settings.should_include_knowledge(),
                'max_log_lines': ue_settings.get_max_log_lines(),
                'report_errors': ue_settings.should_report_errors(),
                'report_optimizations': ue_settings.should_report_optimizations(),
                'report_sessions': ue_settings.should_report_sessions(),
                'anonymize_data': ue_settings.should_anonymize_data()
            }
        except Exception:
            pass
        return {}
    
    def _save_config(self):
        """Save configuration to file."""
        try:
            with open(self.config_file, 'w', encoding='utf-8') as f:
                json.dump(self.config, f, indent=2, ensure_ascii=False)
        except Exception:
            pass  # Don't break functionality if config save fails
    
    def is_enabled(self) -> bool:
        """Check if auto-reporting is enabled."""
        return self.config.get('enabled', True)
    
    def get_worker_url(self) -> str:
        """Get the Cloudflare Worker URL."""
        return self.config.get('worker_url', '')
    
    def set_worker_url(self, url: str):
        """Set the Cloudflare Worker URL."""
        self.config['worker_url'] = url
        self._save_config()
    
    def should_include_logs(self) -> bool:
        """Check if logs should be included in reports."""
        return self.config.get('include_logs', True)
    
    def should_include_knowledge(self) -> bool:
        """Check if knowledge data should be included in reports."""
        return self.config.get('include_knowledge', True)
    
    def get_max_log_lines(self) -> int:
        """Get the maximum number of log lines to include."""
        return self.config.get('max_log_lines', 1000)
    
    def should_report_errors(self) -> bool:
        """Check if errors should be reported."""
        return self.config.get('report_errors', True)
    
    def should_report_optimizations(self) -> bool:
        """Check if optimizations should be reported."""
        return self.config.get('report_optimizations', True)
    
    def should_report_sessions(self) -> bool:
        """Check if session reports should be sent."""
        return self.config.get('report_sessions', False)
    
    def should_anonymize_data(self) -> bool:
        """Check if data should be anonymized."""
        return self.config.get('anonymize_data', True)
    
    def get_rate_limit(self) -> dict:
        """Get rate limiting configuration."""
        return self.config.get('rate_limit', {})
    
    def set_enabled(self, enabled: bool):
        """Enable or disable auto-reporting."""
        self.config['enabled'] = enabled
        self._save_config()
    
    def set_include_logs(self, include: bool):
        """Set whether logs should be included."""
        self.config['include_logs'] = include
        self._save_config()
    
    def set_include_knowledge(self, include: bool):
        """Set whether knowledge data should be included."""
        self.config['include_knowledge'] = include
        self._save_config()
    
    def set_max_log_lines(self, max_lines: int):
        """Set the maximum number of log lines to include."""
        self.config['max_log_lines'] = max_lines
        self._save_config()
    
    def set_report_errors(self, report: bool):
        """Set whether errors should be reported."""
        self.config['report_errors'] = report
        self._save_config()
    
    def set_report_optimizations(self, report: bool):
        """Set whether optimizations should be reported."""
        self.config['report_optimizations'] = report
        self._save_config()
    
    def set_report_sessions(self, report: bool):
        """Set whether session reports should be sent."""
        self.config['report_sessions'] = report
        self._save_config()
    
    def set_anonymize_data(self, anonymize: bool):
        """Set whether data should be anonymized."""
        self.config['anonymize_data'] = anonymize
        self._save_config()
    
    def get_config_summary(self) -> dict:
        """Get a summary of the current configuration."""
        return {
            'enabled': self.is_enabled(),
            'worker_url_configured': bool(self.get_worker_url()),
            'include_logs': self.should_include_logs(),
            'include_knowledge': self.should_include_knowledge(),
            'max_log_lines': self.get_max_log_lines(),
            'report_errors': self.should_report_errors(),
            'report_optimizations': self.should_report_optimizations(),
            'report_sessions': self.should_report_sessions(),
            'anonymize_data': self.should_anonymize_data()
        }
    
    def reset_to_defaults(self):
        """Reset configuration to defaults."""
        self.config = self._get_default_config()
        self._save_config()


# Global configuration instance
_config = None

def get_config() -> AutoReportConfig:
    """Get the global configuration instance."""
    global _config
    if _config is None:
        _config = AutoReportConfig()
    return _config

def is_auto_reporting_enabled() -> bool:
    """Check if auto-reporting is enabled."""
    config = get_config()
    return config.is_enabled()

def get_worker_url() -> str:
    """Get the configured worker URL."""
    config = get_config()
    return config.get_worker_url()

def should_include_logs() -> bool:
    """Check if logs should be included."""
    config = get_config()
    return config.should_include_logs()

def should_include_knowledge() -> bool:
    """Check if knowledge data should be included."""
    config = get_config()
    return config.should_include_knowledge()

def get_max_log_lines() -> int:
    """Get the maximum number of log lines to include."""
    config = get_config()
    return config.get_max_log_lines()

def should_report_errors() -> bool:
    """Check if errors should be reported."""
    config = get_config()
    return config.should_report_errors()

def should_report_optimizations() -> bool:
    """Check if optimizations should be reported."""
    config = get_config()
    return config.should_report_optimizations()

def should_report_sessions() -> bool:
    """Check if session reports should be sent."""
    config = get_config()
    return config.should_report_sessions()

def should_anonymize_data() -> bool:
    """Check if data should be anonymized."""
    config = get_config()
    return config.should_anonymize_data()
