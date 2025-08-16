"""
Auto-report configuration access (plugin-shipped): reads UE settings via ue_settings.
"""
from . import ue_settings

def is_auto_reporting_enabled() -> bool:
    try:
        return bool(ue_settings.is_auto_reporting_enabled()) and bool(ue_settings.has_user_consent())
    except Exception:
        return True

def should_report_errors() -> bool:
    try:
        return bool(ue_settings.should_report_errors())
    except Exception:
        return True

def should_report_optimizations() -> bool:
    try:
        return bool(ue_settings.should_report_optimizations())
    except Exception:
        return True

def worker_url() -> str:
    try:
        return ue_settings.get_worker_url() or ''
    except Exception:
        return ''


