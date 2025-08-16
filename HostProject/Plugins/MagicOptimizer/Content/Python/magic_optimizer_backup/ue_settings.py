"""
Unreal Engine Settings Access Module
Provides Python access to the plugin's C++ settings
"""

import unreal

def get_optimizer_settings():
    """
    Get the optimizer settings from Unreal Engine.
    
    Returns:
        dict: Dictionary containing all optimizer settings
    """
    try:
        # Try to get the PythonBridge instance
        if hasattr(unreal, 'MagicOptimizer'):
            # Get the PythonBridge from the plugin module
            python_bridge = unreal.MagicOptimizer.get_python_bridge()
            if python_bridge:
                settings = python_bridge.get_optimizer_settings()
                if settings:
                    return {
                        'b_enable_auto_reporting': getattr(settings, 'b_enable_auto_reporting', True),
                        'auto_report_worker_url': getattr(settings, 'auto_report_worker_url', ''),
                        'b_include_logs_in_reports': getattr(settings, 'b_include_logs_in_reports', True),
                        'b_include_knowledge_in_reports': getattr(settings, 'b_include_knowledge_in_reports', True),
                        'max_log_lines': getattr(settings, 'max_log_lines', 1000),
                        'b_report_errors': getattr(settings, 'b_report_errors', True),
                        'b_report_optimizations': getattr(settings, 'b_report_optimizations', True),
                        'b_report_sessions': getattr(settings, 'b_report_sessions', False),
                        'b_anonymize_data': getattr(settings, 'b_anonymize_data', True),
                        'b_user_consent': getattr(settings, 'b_user_consent', True),
                        'category_mask': getattr(settings, 'category_mask', 0),
                        'target_profile': getattr(settings, 'target_profile', 'PC_Balanced'),
                        'run_mode': getattr(settings, 'run_mode', 'Audit'),
                        'b_use_selection': getattr(settings, 'b_use_selection', False),
                        'include_paths_csv': getattr(settings, 'include_paths_csv', ''),
                        'exclude_paths_csv': getattr(settings, 'exclude_paths_csv', ''),
                        'b_dry_run': getattr(settings, 'b_dry_run', True),
                        'max_changes': getattr(settings, 'max_changes', 100),
                        'b_create_backups': getattr(settings, 'b_create_backups', True),
                        'b_close_editor': getattr(settings, 'b_close_editor', False),
                        'output_directory': getattr(settings, 'output_directory', 'Saved/MagicOptimizer'),
                        'b_generate_reports': getattr(settings, 'b_generate_reports', True),
                        'python_script_path': getattr(settings, 'python_script_path', ''),  # Empty to default to plugin-shipped Python
                        'b_enable_python_logging': getattr(settings, 'b_enable_python_logging', True)
                    }
    except Exception as e:
        print(f"Warning: Could not access UE settings: {e}")
    
    # Return default settings if UE access fails
    return {
        'b_enable_auto_reporting': True,
        'auto_report_worker_url': '',
        'b_include_logs_in_reports': True,
        'b_include_knowledge_in_reports': True,
        'max_log_lines': 1000,
        'b_report_errors': True,
        'b_report_optimizations': True,
        'b_report_sessions': False,
        'b_anonymize_data': True,
        'category_mask': 1,  # Textures
        'target_profile': 'PC_Balanced',
        'run_mode': 'Audit',
        'b_use_selection': False,
        'include_paths_csv': '',
        'exclude_paths_csv': '',
        'b_dry_run': True,
        'max_changes': 100,
        'b_create_backups': True,
        'b_close_editor': False,
        'output_directory': 'Saved/MagicOptimizer',
        'b_generate_reports': True,
        'python_script_path': '',  # Empty to default to plugin-shipped Python
        'b_enable_python_logging': True
    }

def is_auto_reporting_enabled():
    """
    Check if auto-reporting is enabled in UE settings.
    
    Returns:
        bool: True if auto-reporting is enabled
    """
    settings = get_optimizer_settings()
    return settings.get('b_enable_auto_reporting', True)

def get_worker_url():
    """
    Get the Cloudflare Worker URL from UE settings.
    
    Returns:
        str: Worker URL or empty string if not configured
    """
    settings = get_optimizer_settings()
    return settings.get('auto_report_worker_url', '')

def should_include_logs():
    """
    Check if logs should be included in reports.
    
    Returns:
        bool: True if logs should be included
    """
    settings = get_optimizer_settings()
    return settings.get('b_include_logs_in_reports', True)

def should_include_knowledge():
    """
    Check if knowledge data should be included in reports.
    
    Returns:
        bool: True if knowledge data should be included
    """
    settings = get_optimizer_settings()
    return settings.get('b_include_knowledge_in_reports', True)

def should_report_errors():
    """
    Check if errors should be reported.
    
    Returns:
        bool: True if errors should be reported
    """
    settings = get_optimizer_settings()
    return settings.get('b_report_errors', True)

def should_report_optimizations():
    """
    Check if optimizations should be reported.
    
    Returns:
        bool: True if optimizations should be reported
    """
    settings = get_optimizer_settings()
    return settings.get('b_report_optimizations', True)

def should_report_sessions():
    """
    Check if sessions should be reported.
    
    Returns:
        bool: True if sessions should be reported
    """
    settings = get_optimizer_settings()
    return settings.get('b_report_sessions', False)

def should_anonymize_data():
    """
    Check if data should be anonymized.
    
    Returns:
        bool: True if data should be anonymized
    """
    settings = get_optimizer_settings()
    return settings.get('b_anonymize_data', True)

def get_max_log_lines():
    """
    Get the maximum number of log lines to include.
    
    Returns:
        int: Maximum log lines
    """
    settings = get_optimizer_settings()
    return settings.get('max_log_lines', 1000)

def has_user_consent():
    """
    Check if the user has consented to auto-reporting.
    
    Returns:
        bool: True if user has consented
    """
    settings = get_optimizer_settings()
    return settings.get('b_user_consent', True)
