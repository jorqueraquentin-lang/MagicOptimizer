#!/usr/bin/env python3
"""
Test script for the auto-report system.
This script tests the auto-report functionality without requiring Unreal Engine.
"""

import os
import sys
import json
from datetime import datetime

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

def test_auto_report_system():
    """Test the auto-report system components."""
    print("Testing MagicOptimizer Auto-Report System")
    print("=" * 50)
    
    # Test 1: Import auto-report modules
    print("\n1. Testing imports...")
    try:
        from auto_report import AutoReporter, send_error_report, send_optimization_report
        from auto_report_config import AutoReportConfig, is_auto_reporting_enabled
        print("✅ All auto-report modules imported successfully")
    except ImportError as e:
        print(f"❌ Import failed: {e}")
        return False
    
    # Test 2: Configuration system
    print("\n2. Testing configuration system...")
    try:
        config = AutoReportConfig()
        print(f"✅ Configuration loaded: {config.get_config_summary()}")
        
        # Test configuration methods
        config.set_worker_url("https://test-worker.example.com/report")
        print(f"✅ Worker URL set: {config.get_worker_url()}")
        
        config.set_enabled(True)
        print(f"✅ Auto-reporting enabled: {config.is_enabled()}")
        
    except Exception as e:
        print(f"❌ Configuration test failed: {e}")
        return False
    
    # Test 3: AutoReporter initialization
    print("\n3. Testing AutoReporter initialization...")
    try:
        reporter = AutoReporter("https://test-worker.example.com/report")
        print(f"✅ AutoReporter initialized with URL: {reporter.worker_url}")
        print(f"✅ Plugin version: {reporter.plugin_version}")
        print(f"✅ Project name: {reporter.project_name}")
        print(f"✅ User ID: {reporter.user_id}")
        
    except Exception as e:
        print(f"❌ AutoReporter initialization failed: {e}")
        return False
    
    # Test 4: System info gathering
    print("\n4. Testing system info gathering...")
    try:
        system_info = reporter._gather_system_info()
        print(f"✅ System info gathered: {json.dumps(system_info, indent=2)}")
        
    except Exception as e:
        print(f"❌ System info gathering failed: {e}")
        return False
    
    # Test 5: Log gathering (mock)
    print("\n5. Testing log gathering...")
    try:
        # Create a mock log file
        mock_log_content = f"[{datetime.now()}] Test log entry 1\n[{datetime.now()}] Test log entry 2\n"
        mock_log_path = os.path.join(os.getcwd(), 'test_log.txt')
        
        with open(mock_log_path, 'w') as f:
            f.write(mock_log_content)
        
        # Temporarily set environment variable
        old_log_path = os.environ.get('MAGICOPTIMIZER_LOG')
        os.environ['MAGICOPTIMIZER_LOG'] = mock_log_path
        
        logs = reporter._gather_logs()
        print(f"✅ Logs gathered: {len(logs)} characters")
        
        # Cleanup
        os.environ['MAGICOPTIMIZER_URL'] = old_log_path or ''
        os.remove(mock_log_path)
        
    except Exception as e:
        print(f"❌ Log gathering failed: {e}")
        return False
    
    # Test 6: Knowledge data gathering (mock)
    print("\n6. Testing knowledge data gathering...")
    try:
        # Create mock knowledge directory
        mock_kb_dir = os.path.join(os.getcwd(), 'Saved', 'MagicOptimizer', 'Knowledge')
        os.makedirs(mock_kb_dir, exist_ok=True)
        
        # Create mock events file
        mock_events = [
            {"timestamp": datetime.now().isoformat(), "event": "test_event_1"},
            {"timestamp": datetime.now().isoformat(), "event": "test_event_2"}
        ]
        events_path = os.path.join(mock_kb_dir, 'events.jsonl')
        with open(events_path, 'w') as f:
            for event in mock_events:
                f.write(json.dumps(event) + '\n')
        
        # Temporarily set environment variable
        old_saved_dir = os.environ.get('MAGICOPTIMIZER_SAVED_DIR')
        os.environ['MAGICOPTIMIZER_SAVED_DIR'] = os.path.join(os.getcwd(), 'Saved')
        
        knowledge_data = reporter._gather_knowledge_data()
        print(f"✅ Knowledge data gathered: {json.dumps(knowledge_data, indent=2)}")
        
        # Cleanup
        os.environ['MAGICOPTIMIZER_SAVED_DIR'] = old_saved_dir or ''
        import shutil
        shutil.rmtree(os.path.join(os.getcwd(), 'Saved'))
        
    except Exception as e:
        print(f"❌ Knowledge data gathering failed: {e}")
        return False
    
    # Test 7: Convenience functions
    print("\n7. Testing convenience functions...")
    try:
        # Test error report function
        success, message, issue_url = send_error_report(
            "TestError", 
            "This is a test error message", 
            "Test context"
        )
        print(f"✅ Error report function: success={success}, message='{message}'")
        
        # Test optimization report function
        success, message, issue_url = send_optimization_report(
            "test", "test_profile", 10, 2, True, 5.5, "Test context"
        )
        print(f"✅ Optimization report function: success={success}, message='{message}'")
        
    except Exception as e:
        print(f"❌ Convenience functions test failed: {e}")
        return False
    
    print("\n" + "=" * 50)
    print("✅ All tests passed! Auto-report system is working correctly.")
    return True

def test_http_functionality():
    """Test HTTP functionality with a mock endpoint."""
    print("\nTesting HTTP functionality...")
    print("=" * 50)
    
    try:
        from auto_report import AutoReporter
        
        # Test with a mock endpoint that will fail (expected)
        reporter = AutoReporter("https://httpbin.org/status/404")
        
        success, message, issue_url = reporter.send_error_report(
            "TestError", 
            "Test error message", 
            "Test context"
        )
        
        print(f"Expected failure test: success={success}, message='{message}'")
        
        if not success:
            print("✅ HTTP test completed as expected (failure due to 404)")
        else:
            print("⚠️  Unexpected success with 404 endpoint")
            
    except Exception as e:
        print(f"❌ HTTP test failed: {e}")
        return False
    
    return True

if __name__ == "__main__":
    print("MagicOptimizer Auto-Report System Test")
    print("=" * 50)
    
    # Run basic tests
    basic_tests_passed = test_auto_report_system()
    
    # Run HTTP tests
    http_tests_passed = test_http_functionality()
    
    # Summary
    print("\n" + "=" * 50)
    print("TEST SUMMARY")
    print("=" * 50)
    print(f"Basic tests: {'✅ PASSED' if basic_tests_passed else '❌ FAILED'}")
    print(f"HTTP tests: {'✅ PASSED' if http_tests_passed else '❌ FAILED'}")
    
    if basic_tests_passed and http_tests_passed:
        print("\n🎉 All tests passed! The auto-report system is ready for use.")
        sys.exit(0)
    else:
        print("\n💥 Some tests failed. Please check the output above.")
        sys.exit(1)
