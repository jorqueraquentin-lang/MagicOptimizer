#!/usr/bin/env python3
"""
Test script to verify connection to the actual Cloudflare Worker.
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

def test_real_worker():
    """Test the connection to the actual Cloudflare Worker."""
    print("Testing Connection to Real Cloudflare Worker")
    print("=" * 50)
    
    try:
        from auto_report import AutoReporter
        from auto_report_config import get_config
        
        # Get the configured worker URL
        config = get_config()
        worker_url = config.get_worker_url()
        
        print(f"Configured Worker URL: {worker_url}")
        print(f"Auto-reporting enabled: {config.is_enabled()}")
        
        if not worker_url:
            print("❌ No worker URL configured!")
            return False
        
        # Test the worker endpoint
        print(f"\nTesting worker endpoint: {worker_url}")
        
        # Test GET /diag-auth endpoint
        import urllib.request
        
        try:
            diag_url = f"{worker_url}/diag-auth"
            print(f"Testing diagnostic endpoint: {diag_url}")
            
            req = urllib.request.Request(diag_url)
            with urllib.request.urlopen(req, timeout=10) as response:
                response_data = response.read().decode('utf-8')
                response_code = response.getcode()
                
                print(f"✅ Diagnostic endpoint response: {response_code}")
                print(f"Response data: {response_data}")
                
                if response_code == 200:
                    try:
                        diag_info = json.loads(response_data)
                        print(f"✅ GitHub PAT present: {diag_info.get('gh_pat_present')}")
                        print(f"✅ Owner: {diag_info.get('owner')}")
                        print(f"✅ Repo: {diag_info.get('repo')}")
                        print(f"✅ Token auth status: {diag_info.get('token_auth_status')}")
                        print(f"✅ Bearer auth status: {diag_info.get('bearer_auth_status')}")
                        
                        if diag_info.get('token_auth_status') == 200:
                            print("\n🎉 Worker is fully authenticated and ready!")
                        else:
                            print("\n⚠️  Worker authentication issues detected")
                            
                    except json.JSONDecodeError:
                        print("❌ Could not parse diagnostic response")
                        return False
                else:
                    print(f"❌ Diagnostic endpoint returned {response_code}")
                    return False
                    
        except Exception as e:
            print(f"❌ Diagnostic endpoint test failed: {e}")
            return False
        
        # Test the main endpoint with a simple POST
        print(f"\nTesting main endpoint: {worker_url}")
        
        try:
            # Create a simple test payload
            test_metadata = {
                "title": "Test Report",
                "description": "This is a test report from the MagicOptimizer plugin",
                "labels": ["test", "auto-report"],
                "project": "TestProject",
                "plugin_version": "1.0.0"
            }
            
            test_report = {
                "type": "test",
                "system_info": {
                    "timestamp": "2025-08-16T18:25:00",
                    "plugin_version": "1.0.0",
                    "project_name": "TestProject",
                    "platform": "test"
                }
            }
            
            # Create multipart form data
            import time
            boundary = f"----MagicOptimizer{int(time.time())}"
            
            payload_parts = [
                f"--{boundary}",
                'Content-Disposition: form-data; name="metadata"',
                'Content-Type: application/json',
                "",
                json.dumps(test_metadata),
                f"--{boundary}",
                'Content-Disposition: form-data; name="report"',
                'Content-Type: application/json',
                "",
                json.dumps(test_report),
                f"--{boundary}--",
                ""
            ]
            
            payload = "\r\n".join(payload_parts).encode('utf-8')
            
            headers = {
                'Content-Type': f'multipart/form-data; boundary={boundary}',
                'Content-Length': str(len(payload)),
                'User-Agent': 'MagicOptimizer/1.0.0'
            }
            
            req = urllib.request.Request(
                worker_url,
                data=payload,
                headers=headers,
                method='POST'
            )
            
            with urllib.request.urlopen(req, timeout=30) as response:
                response_data = response.read().decode('utf-8')
                response_code = response.getcode()
                
                print(f"✅ Main endpoint response: {response_code}")
                print(f"Response data: {response_data}")
                
                if response_code == 200:
                    try:
                        result = json.loads(response_data)
                        if result.get('success'):
                            print(f"🎉 Test report sent successfully!")
                            print(f"Issue URL: {result.get('issue_url', 'N/A')}")
                            print(f"Gist URL: {result.get('gist_url', 'N/A')}")
                            return True
                        else:
                            print(f"⚠️  Worker returned success=false: {result.get('message', 'Unknown error')}")
                            return False
                    except json.JSONDecodeError:
                        print(f"⚠️  Could not parse response: {response_data}")
                        return False
                else:
                    print(f"❌ Main endpoint returned {response_code}")
                    return False
                    
        except Exception as e:
            print(f"❌ Main endpoint test failed: {e}")
            return False
            
    except ImportError as e:
        print(f"❌ Import failed: {e}")
        return False
    except Exception as e:
        print(f"❌ Test failed: {e}")
        return False

if __name__ == "__main__":
    success = test_real_worker()
    
    print("\n" + "=" * 50)
    if success:
        print("🎉 All tests passed! Your Cloudflare Worker is working correctly.")
        print("The auto-report system is ready to send reports to GitHub.")
    else:
        print("💥 Some tests failed. Please check the output above.")
    
    sys.exit(0 if success else 1)
