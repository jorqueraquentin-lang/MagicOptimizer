import unreal
import time
import os

def main():
	"""Take screenshots before and after plugin test execution"""
	
	# Ensure we have a stable editor state by loading a template map
	try:
		unreal.EditorLevelLibrary.load_level('/Engine/Maps/Templates/OpenWorld')
	except Exception as e:
		unreal.log_warning(f"CI_SHOT: load_level failed: {e}")
	
	# Create screenshots directory
	# Use absolute path under the project's Saved dir to ensure files are written where we expect
	saved_root = unreal.Paths.project_saved_dir()
	screenshots_dir = os.path.join(saved_root, "MagicOptimizer", "CI")
	os.makedirs(screenshots_dir, exist_ok=True)
	
	# Set a deterministic resolution
	try:
		unreal.SystemLibrary.execute_console_command(None, 'r.setres 1280x720w')
	except Exception as e:
		unreal.log_warning(f"CI_SHOT: setres failed: {e}")
	
	# Step 1: Open the plugin panel via console command and take BEFORE screenshot
	print("Opening MagicOptimizer plugin panel via console command...")
	unreal.SystemLibrary.execute_console_command(None, 'MagicOptimizer.Open')
	
	# Wait for UI to render
	print("Waiting for UI to render...")
	time.sleep(4)
	
	# Take BEFORE screenshot (initial state)
	before_screenshot = os.path.join(screenshots_dir, "01_before_test.png")
	print(f"Taking BEFORE screenshot: {before_screenshot}")
	unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, before_screenshot)
	time.sleep(1)
	
	# Step 2: Execute the test (Audit phase)
	print("Executing MagicOptimizer.Run Audit Textures...")
	unreal.SystemLibrary.execute_console_command(None, "MagicOptimizer.Run Audit Textures")
	
	# Wait for test to complete and UI to update
	print("Waiting for test to complete...")
	time.sleep(8)
	
	# Step 3: Take AFTER screenshot (showing results)
	after_screenshot = os.path.join(screenshots_dir, "02_after_test.png")
	print(f"Taking AFTER screenshot: {after_screenshot}")
	unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, after_screenshot)
	time.sleep(1)
	
	print("Screenshots captured successfully!")
	print(f"Before: {before_screenshot}")
	print(f"After: {after_screenshot}")
	
	# Exit editor
	print("Exiting editor...")
	unreal.SystemLibrary.quit_editor()

if __name__ == "__main__":
	main()


