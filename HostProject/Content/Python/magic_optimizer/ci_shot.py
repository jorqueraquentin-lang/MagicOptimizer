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

	# Direct screenshots to our CI folder
	try:
		unreal.SystemLibrary.execute_console_command(None, f'r.ScreenshotSavePath "{screenshots_dir.replace("\\","/")}"')
	except Exception as e:
		unreal.log_warning(f"CI_SHOT: set ScreenshotSavePath failed: {e}")
	
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
	
	# Helper: list all existing PNGs under Saved/Screenshots
	def _list_shots():
		shots_root = os.path.join(unreal.Paths.project_saved_dir(), "Screenshots")
		paths = []
		for root, _dirs, files in os.walk(shots_root):
			for f in files:
				if f.lower().endswith('.png'):
					paths.append(os.path.join(root, f))
		return set(paths)

	# Helper: from a delta of new files, pick newest and copy as name
	def _copy_newest(delta_paths, name):
		if not delta_paths:
			return False
		try:
			cand = max(delta_paths, key=lambda p: os.path.getmtime(p))
			target = os.path.join(screenshots_dir, name)
			os.makedirs(os.path.dirname(target), exist_ok=True)
			unreal.log(f"CI_SHOT: copying {cand} -> {target}")
			with open(cand, 'rb') as src, open(target, 'wb') as dst:
				dst.write(src.read())
			return True
		except Exception as e:
			unreal.log_warning(f"CI_SHOT: copy failed: {e}")
			return False

	# Helper: wait for UE to finish writing the screenshot and then copy
	def _wait_and_copy(start_set, name, timeout_sec=12.0, poll_sec=0.5):
		import time as _t
		deadline = _t.time() + timeout_sec
		while _t.time() < deadline:
			now = _list_shots()
			delta = now - start_set
			if _copy_newest(delta, name):
				return True
			_t.sleep(poll_sec)
		# final attempt
		now = _list_shots()
		delta = now - start_set
		return _copy_newest(delta, name)

	# Take BEFORE screenshot (initial state) and ensure it ends up in our CI folder
	before_name = "01_before_test.png"
	before_files = _list_shots()
	print(f"Taking BEFORE screenshot: {before_name}")
	# First try console command to force exact name into our folder
	try:
		unreal.SystemLibrary.execute_console_command(None, f'HighResShot 1280x720 {before_name}')
	except Exception as e:
		unreal.log_warning(f"CI_SHOT: HighResShot BEFORE failed: {e}")
	time.sleep(2)
	if not os.path.isfile(os.path.join(screenshots_dir, before_name)):
		# Fallback to automation API and delta copy
		unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, before_name)
		_wait_and_copy(before_files, before_name)
	
	# Step 2: Execute the test (Audit phase)
	print("Executing MagicOptimizer.Run Audit Textures...")
	unreal.SystemLibrary.execute_console_command(None, "MagicOptimizer.Run Audit Textures")
	
	# Wait for test to complete and UI to update
	print("Waiting for test to complete...")
	time.sleep(8)
	
	# Step 3: Take AFTER screenshot (showing results)
	after_name = "02_after_test.png"
	prev = _list_shots()
	print(f"Taking AFTER screenshot: {after_name}")
	try:
		unreal.SystemLibrary.execute_console_command(None, f'HighResShot 1280x720 {after_name}')
	except Exception as e:
		unreal.log_warning(f"CI_SHOT: HighResShot AFTER failed: {e}")
	time.sleep(2)
	if not os.path.isfile(os.path.join(screenshots_dir, after_name)):
		unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, after_name)
		_wait_and_copy(prev, after_name)
	
	print("Screenshots captured successfully!")
	print(f"Before: {os.path.join(screenshots_dir, before_name)}")
	print(f"After: {os.path.join(screenshots_dir, after_name)}")
	
	# Exit editor
	print("Exiting editor...")
	unreal.SystemLibrary.quit_editor()

if __name__ == "__main__":
	main()


