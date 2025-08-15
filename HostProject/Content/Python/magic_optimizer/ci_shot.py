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
	
	# Take BEFORE screenshot (initial state) and ensure it ends up in our CI folder
	before_name = "01_before_test.png"
	print(f"Taking BEFORE screenshot: {before_name}")
	unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, before_name)
	time.sleep(1)
	# Resolve actual output location and copy to CI folder
	def _try_copy_shot(name: str):
		# Many UE builds place screenshots under Saved/Screenshots/<Platform>
		shots_root = os.path.join(unreal.Paths.project_saved_dir(), "Screenshots")
		candidates = []
		for root, _dirs, files in os.walk(shots_root):
			if name in files:
				candidates.append(os.path.join(root, name))
		# Also consider current working dir
		if os.path.isfile(name):
			candidates.append(os.path.abspath(name))
		# Copy the newest candidate if present
		if candidates:
			cand = max(candidates, key=lambda p: os.path.getmtime(p))
			target = os.path.join(screenshots_dir, name)
			try:
				unreal.log(f"CI_SHOT: copying {cand} -> {target}")
				os.makedirs(os.path.dirname(target), exist_ok=True)
				with open(cand, 'rb') as src, open(target, 'wb') as dst:
					dst.write(src.read())
			except Exception as e:
				unreal.log_warning(f"CI_SHOT: copy failed: {e}")

	_try_copy_shot(before_name)
	
	# Step 2: Execute the test (Audit phase)
	print("Executing MagicOptimizer.Run Audit Textures...")
	unreal.SystemLibrary.execute_console_command(None, "MagicOptimizer.Run Audit Textures")
	
	# Wait for test to complete and UI to update
	print("Waiting for test to complete...")
	time.sleep(8)
	
	# Step 3: Take AFTER screenshot (showing results)
	after_name = "02_after_test.png"
	print(f"Taking AFTER screenshot: {after_name}")
	unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, after_name)
	time.sleep(1)
	_try_copy_shot(after_name)
	
	print("Screenshots captured successfully!")
	print(f"Before: {os.path.join(screenshots_dir, before_name)}")
	print(f"After: {os.path.join(screenshots_dir, after_name)}")
	
	# Exit editor
	print("Exiting editor...")
	unreal.SystemLibrary.quit_editor()

if __name__ == "__main__":
	main()


