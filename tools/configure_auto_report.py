#!/usr/bin/env python3
"""
Configuration utility for the MagicOptimizer auto-report system.
Allows users to configure the Cloudflare Worker endpoint and other settings.
"""

import os
import sys
import json

# Add the plugin's Python directory to the path
plugin_python_dir = os.path.join(
    os.path.dirname(__file__), 
    '..', 
    'HostProject', 
    'Content', 
    'Python', 
    'magic_optimizer'
)
sys.path.insert(0, plugin_python_dir)

def get_user_input(prompt: str, default: str = "") -> str:
    """Get user input with a default value."""
    if default:
        user_input = input(f"{prompt} [{default}]: ").strip()
        return user_input if user_input else default
    else:
        return input(f"{prompt}: ").strip()

def get_yes_no_input(prompt: str, default: bool = True) -> bool:
    """Get yes/no input from user."""
    default_str = "Y/n" if default else "y/N"
    while True:
        user_input = input(f"{prompt} [{default_str}]: ").strip().lower()
        if not user_input:
            return default
        if user_input in ['y', 'yes']:
            return True
        if user_input in ['n', 'no']:
            return False
        print("Please enter 'y' or 'n'")

def configure_auto_report():
    """Interactive configuration for the auto-report system."""
    print("MagicOptimizer Auto-Report Configuration")
    print("=" * 50)
    print()
    print("This utility will help you configure the auto-report system.")
    print("The auto-report system sends usage data and error reports to")
    print("a Cloudflare Worker endpoint, which then creates GitHub issues.")
    print()
    
    try:
        from auto_report_config import get_config
        
        config = get_config()
        current_config = config.get_config_summary()
        
        print("Current Configuration:")
        print(f"  Auto-reporting enabled: {'Yes' if current_config['enabled'] else 'No'}")
        print(f"  Worker URL configured: {'Yes' if current_config['worker_url_configured'] else 'No'}")
        print(f"  Include logs: {'Yes' if current_config['include_logs'] else 'No'}")
        print(f"  Include knowledge data: {'Yes' if current_config['include_knowledge'] else 'No'}")
        print(f"  Report errors: {'Yes' if current_config['report_errors'] else 'No'}")
        print(f"  Report optimizations: {'Yes' if current_config['report_optimizations'] else 'No'}")
        print(f"  Report sessions: {'Yes' if current_config['report_sessions'] else 'No'}")
        print(f"  Anonymize data: {'Yes' if current_config['anonymize_data'] else 'No'}")
        print()
        
        # Ask if user wants to enable auto-reporting
        enable = get_yes_no_input("Enable auto-reporting?", current_config['enabled'])
        config.set_enabled(enable)
        
        if enable:
            print("\nConfiguring auto-reporting settings...")
            
            # Worker URL configuration
            current_url = config.get_worker_url()
            if not current_url:
                print("\nYou need to provide a Cloudflare Worker URL.")
                print("This should be the endpoint URL provided by your setup.")
                print("Example: https://your-worker.your-subdomain.workers.dev/report")
                print()
                
                while True:
                    worker_url = get_user_input("Enter your Cloudflare Worker URL")
                    if worker_url.startswith(('http://', 'https://')):
                        config.set_worker_url(worker_url)
                        print(f"✅ Worker URL set to: {worker_url}")
                        break
                    else:
                        print("❌ Please enter a valid URL starting with http:// or https://")
            else:
                print(f"\nCurrent Worker URL: {current_url}")
                change_url = get_yes_no_input("Change the Worker URL?", False)
                if change_url:
                    while True:
                        worker_url = get_user_input("Enter new Cloudflare Worker URL")
                        if worker_url.startswith(('http://', 'https://')):
                            config.set_worker_url(worker_url)
                            print(f"✅ Worker URL updated to: {worker_url}")
                            break
                        else:
                            print("❌ Please enter a valid URL starting with http:// or https://")
            
            # Data inclusion settings
            print("\nData Inclusion Settings:")
            print("These control what information is sent with reports.")
            
            include_logs = get_yes_no_input("Include plugin logs in reports?", current_config['include_logs'])
            config.set_include_logs(include_logs)
            
            include_knowledge = get_yes_no_input("Include knowledge base data in reports?", current_config['include_knowledge'])
            config.set_include_knowledge(include_knowledge)
            
            # Report type settings
            print("\nReport Type Settings:")
            print("These control when reports are sent.")
            
            report_errors = get_yes_no_input("Send reports when errors occur?", current_config['report_errors'])
            config.set_report_errors(report_errors)
            
            report_optimizations = get_yes_no_input("Send reports after optimization phases?", current_config['report_optimizations'])
            config.set_report_optimizations(report_optimizations)
            
            report_sessions = get_yes_no_input("Send reports at the end of sessions?", current_config['report_sessions'])
            config.set_report_sessions(report_sessions)
            
            # Privacy settings
            print("\nPrivacy Settings:")
            print("These control how your data is handled.")
            
            anonymize = get_yes_no_input("Anonymize data in reports?", current_config['anonymize_data'])
            config.set_anonymize_data(anonymize)
            
            if not anonymize:
                print("\n⚠️  Warning: Disabling anonymization means more identifying")
                print("   information may be included in reports.")
                print("   This is generally not recommended for privacy reasons.")
            
            # Log line limit
            print("\nLog Settings:")
            current_max_lines = current_config['max_log_lines']
            max_lines_input = get_user_input(f"Maximum log lines to include in reports", str(current_max_lines))
            try:
                max_lines = int(max_lines_input)
                if max_lines > 0:
                    config.set_max_log_lines(max_lines)
                    print(f"✅ Maximum log lines set to: {max_lines}")
                else:
                    print("❌ Maximum log lines must be greater than 0, keeping current value")
            except ValueError:
                print("❌ Invalid number, keeping current value")
            
            print("\n✅ Auto-report configuration completed!")
            
        else:
            print("\n✅ Auto-reporting disabled.")
        
        # Show final configuration
        print("\nFinal Configuration:")
        final_config = config.get_config_summary()
        print(f"  Auto-reporting enabled: {'Yes' if final_config['enabled'] else 'No'}")
        if final_config['enabled']:
            print(f"  Worker URL configured: {'Yes' if final_config['worker_url_configured'] else 'No'}")
            print(f"  Include logs: {'Yes' if final_config['include_logs'] else 'No'}")
            print(f"  Include knowledge data: {'Yes' if final_config['include_knowledge'] else 'No'}")
            print(f"  Report errors: {'Yes' if final_config['report_errors'] else 'No'}")
            print(f"  Report optimizations: {'Yes' if final_config['report_optimizations'] else 'No'}")
            print(f"  Report sessions: {'Yes' if final_config['report_sessions'] else 'No'}")
            print(f"  Anonymize data: {'Yes' if final_config['anonymize_data'] else 'No'}")
            print(f"  Max log lines: {final_config['max_log_lines']}")
        
        print("\nConfiguration saved to:")
        print(f"  {config.config_file}")
        
        # Test configuration
        if final_config['enabled'] and final_config['worker_url_configured']:
            print("\nTesting configuration...")
            try:
                from auto_report import AutoReporter
                
                reporter = AutoReporter(config.get_worker_url())
                print(f"✅ AutoReporter initialized successfully")
                print(f"✅ Worker URL: {reporter.worker_url}")
                print(f"✅ Plugin version: {reporter.plugin_version}")
                print(f"✅ Project name: {reporter.project_name}")
                
                print("\n🎉 Configuration test successful! Your auto-report system is ready.")
                
            except Exception as e:
                print(f"❌ Configuration test failed: {e}")
                print("   Please check your Worker URL and try again.")
        
    except ImportError as e:
        print(f"❌ Failed to import auto-report modules: {e}")
        print("   Make sure you're running this from the correct directory.")
        return False
    except Exception as e:
        print(f"❌ Configuration failed: {e}")
        return False
    
    return True

def show_help():
    """Show help information."""
    print("MagicOptimizer Auto-Report Configuration Utility")
    print("=" * 50)
    print()
    print("Usage:")
    print("  python tools/configure_auto_report.py")
    print()
    print("This utility will:")
    print("  1. Show your current auto-report configuration")
    print("  2. Allow you to enable/disable auto-reporting")
    print("  3. Configure your Cloudflare Worker endpoint")
    print("  4. Set privacy and data inclusion preferences")
    print("  5. Test the configuration")
    print()
    print("The auto-report system sends:")
    print("  - Error reports when issues occur")
    print("  - Optimization reports after each phase")
    print("  - Session reports at the end of plugin usage")
    print()
    print("All data is sent to your Cloudflare Worker, which then")
    print("creates GitHub issues in your repository.")
    print()
    print("For more information, see the plugin documentation.")

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] in ['-h', '--help', 'help']:
        show_help()
    else:
        success = configure_auto_report()
        sys.exit(0 if success else 1)
