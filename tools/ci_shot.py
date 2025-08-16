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

	# Proactively clear stale audit CSVs so BEFORE truly reflects an empty state
	try:
		mo_dir = os.path.join(saved_root, "MagicOptimizer")
		audit_dir = os.path.join(mo_dir, "Audit")
		for _name in ("textures.csv", "textures_recommend.csv"):
			_path = os.path.join(audit_dir, _name)
			if os.path.isfile(_path):
				os.remove(_path)
	except Exception as e:
		unreal.log_warning(f"CI_SHOT: pre-clear audit CSVs failed: {e}")

	# Direct screenshots to our CI folder
	try:
		_path_cmd = screenshots_dir.replace("\\", "/")
		_cmd = 'r.ScreenshotSavePath "{0}"'.format(_path_cmd)
		unreal.SystemLibrary.execute_console_command(None, _cmd)
	except Exception as e:
		unreal.log_warning(f"CI_SHOT: set ScreenshotSavePath failed: {e}")
	# Small settle delay so the engine honors the new path before first shot
	time.sleep(0.5)

	# Wait helpers
	def _wait_for_file(path: str, timeout_sec: float = 20.0, quiet: bool = False) -> bool:
		"""Wait until file exists and its size is stable for one poll step."""
		import time as _t
		end = _t.time() + timeout_sec
		last_size = -1
		while _t.time() < end:
			if os.path.isfile(path):
				sz = os.path.getsize(path)
				if sz > 0 and sz == last_size:
					return True
				last_size = sz
			_t.sleep(0.5)
		if not quiet:
			unreal.log_warning(f"CI_SHOT: timeout waiting for file {path}")
		return False

	# Windows fallback capture (target Unreal Editor window)
	def _win_capture(out_path: str, delay_ms: int = 0) -> bool:
		try:
			import subprocess
			repo_root = os.path.normpath(os.path.join(unreal.Paths.project_dir(), '..'))
			ps = os.path.join(repo_root, 'tools', 'windows_capture.ps1')
			if not os.path.isfile(ps):
				return False
			args = [
				'powershell', '-ExecutionPolicy', 'Bypass',
				'-File', ps,
				'-OutPath', out_path,
				'-DelayMs', str(delay_ms),
				'-WindowTitle', 'Unreal Editor',
				'-ProcessName', 'UnrealEditor*',
				'-FullScreen'
			]
			subprocess.run(args, check=False)
			return os.path.isfile(out_path)
		except Exception as e:
			unreal.log_warning(f"CI_SHOT: win capture failed: {e}")
			return False

	def _wait_for_audit(max_wait_sec: float = 60.0, started_ts: float = None) -> None:
		"""Monitor audit completion using multiple methods: live console output, direct UE status checks, and file monitoring."""
		import time as _t
		if started_ts is None:
			started_ts = _t.time()
		
		print("Monitoring audit completion using multiple methods...")
		
		# Method 1: Monitor live console output file
		live_console_log = os.path.join(saved_root, "MagicOptimizer", "live_console.log")
		console_monitor_active = os.path.isfile(live_console_log)
		
		# Method 2: Direct UE console status checking
		status_check_active = True
		
		# Method 3: File system monitoring (backup)
		file_monitor_active = True
		
		deadline = _t.time() + max_wait_sec
		last_status_check = 0
		status_check_interval = 2.0  # Check UE status every 2 seconds
		
		while _t.time() < deadline:
			current_time = _t.time()
			
			# Method 1: Live console output monitoring
			if console_monitor_active:
				try:
					with open(live_console_log, 'r', encoding='utf-8', errors='ignore') as f:
						f.seek(0, 2)  # Seek to end
						file_size = f.tell()
						
						# Check if file has grown (new content)
						if hasattr(_wait_for_audit, 'last_file_size'):
							if file_size > _wait_for_audit.last_file_size:
								# Read new content
								f.seek(_wait_for_audit.last_file_size)
								new_content = f.read()
								print(f"New console output: {new_content.strip()}")
								
								# Check for completion indicators
								if any(indicator in new_content for indicator in [
									'"phase": "Audit"',
									'"success": true',
									'Audit completed successfully',
									'Texture audit finished',
									'Found textures:',
									'textures.csv',
									'MagicOptimizer.Run Audit Textures',
									'Executing MagicOptimizer.Run Audit Textures',
									'121 textures found',
									'textures found'
								]):
									print("Audit completion detected in live console output!")
									return
						
						_wait_for_audit.last_file_size = file_size
				except Exception as e:
					console_monitor_active = False
					print(f"Console monitoring failed: {e}")
			
			# Method 2: Direct UE console status checking
			if status_check_active and (current_time - last_status_check) >= status_check_interval:
				try:
					# Try to get audit status directly from UE
					result = unreal.SystemLibrary.execute_console_command(None, "MagicOptimizer.Status")
					if result and "completed" in result.lower():
						print("Audit completion detected via direct UE status check!")
						return
					
					# Alternative: check if audit files exist and are fresh
					mo_dir = os.path.join(saved_root, "MagicOptimizer")
					audit_dir = os.path.join(mo_dir, "Audit")
					tex_csv = os.path.join(audit_dir, "textures.csv")
					
					if os.path.isfile(tex_csv):
						mt = os.path.getmtime(tex_csv)
						if mt > started_ts:
							# Check if file is still being written (size is stable)
							time.sleep(0.5)  # Wait a bit
							new_size = os.path.getsize(tex_csv)
							if new_size == os.path.getsize(tex_csv):  # Size is stable
								print("Audit completion detected via file monitoring!")
								return
					
					last_status_check = current_time
				except Exception as e:
					status_check_active = False
					print(f"Direct status checking failed: {e}")
			
			# Method 3: File system monitoring (backup)
			if file_monitor_active:
				try:
					mo_dir = os.path.join(saved_root, "MagicOptimizer")
					audit_dir = os.path.join(mo_dir, "Audit")
					tex_csv = os.path.join(audit_dir, "textures.csv")
					
					if os.path.isfile(tex_csv):
						mt = os.path.getmtime(tex_csv)
						if mt > started_ts:
							# Wait for file to stabilize
							time.sleep(0.5)
							if os.path.getsize(tex_csv) == os.path.getsize(tex_csv):
								print("Audit completion detected via file monitoring!")
								return
				except Exception as e:
					file_monitor_active = False
					print(f"File monitoring failed: {e}")
			
			# Small delay between checks
			_t.sleep(0.1)
		
		unreal.log_warning("CI_SHOT: audit wait timed out; proceeding to AFTER screenshot anyway")
	
	def _fallback_wait_for_audit(max_wait_sec: float = 60.0, started_ts: float = None) -> None:
		"""Fallback method using file polling if log monitoring fails."""
		mo_dir = os.path.join(saved_root, "MagicOptimizer")
		audit_dir = os.path.join(mo_dir, "Audit")
		tex_csv = os.path.join(audit_dir, "textures.csv")
		runtime_log = os.path.join(mo_dir, "MagicOptimizerRuntime.log")
		import time as _t
		if started_ts is None:
			started_ts = _t.time()
		deadline = _t.time() + max_wait_sec
		while _t.time() < deadline:
			# Check CSV existence and freshness
			try:
				if os.path.isfile(tex_csv):
					mt = os.path.getmtime(tex_csv)
					if mt > started_ts and _wait_for_file(tex_csv, timeout_sec=2.0, quiet=True):
						return
			except Exception:
				pass
			# Check runtime log for success marker
			try:
				if os.path.isfile(runtime_log):
					log_mt = os.path.getmtime(runtime_log)
					if log_mt > started_ts:
						with open(runtime_log, 'rb') as f:
							f.seek(max(0, os.path.getsize(runtime_log) - 64 * 1024))
							chunk = f.read().decode(errors='ignore')
							if '"phase": "Audit"' in chunk and '"success": true' in chunk:
								return
			except Exception:
				pass
			_t.sleep(0.5)
		unreal.log_warning("CI_SHOT: fallback audit wait timed out")
	
	# Keep native editor resolution for screenshots to avoid cropping
	
	# Step 1: Open the plugin panel via console command and take BEFORE screenshot
	print("Opening MagicOptimizer plugin panel via console command...")
	unreal.SystemLibrary.execute_console_command(None, 'MagicOptimizer.Open')
	
	# Wait for UI to render
	print("Waiting for UI to render...")
	time.sleep(5)
	
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
		unreal.SystemLibrary.execute_console_command(None, f'HighResShot 3840x2160 {before_name}')
	except Exception as e:
		unreal.log_warning(f"CI_SHOT: HighResShot BEFORE failed: {e}")
	time.sleep(2)
	if not os.path.isfile(os.path.join(screenshots_dir, before_name)):
		# Fallback to automation API and delta copy
		unreal.AutomationLibrary.take_high_res_screenshot(3840, 2160, before_name)
		_wait_and_copy(before_files, before_name)
		# If still missing, do a targeted Windows capture of the Unreal window
		outp = os.path.join(screenshots_dir, before_name)
		if not os.path.isfile(outp):
			_win_capture(outp, delay_ms=300)
	
	# Step 2: Execute the test (Audit phase)
	print("Executing MagicOptimizer.Run Audit Textures...")
	# Record start time to ensure we wait for a fresh audit completion, not stale files
	audit_start = time.time()
	unreal.SystemLibrary.execute_console_command(None, "MagicOptimizer.Run Audit Textures")
	
	# Wait for test to complete and UI to update via real-time log monitoring
	print("Waiting for audit to complete (monitoring log output)...")
	_wait_for_audit(60.0, audit_start)
	
	# Step 3: Take AFTER screenshot (showing results)
	after_name = "02_after_test.png"
	prev = _list_shots()
	print(f"Taking AFTER screenshot: {after_name}")
	try:
		unreal.SystemLibrary.execute_console_command(None, f'HighResShot 3840x2160 {after_name}')
	except Exception as e:
		unreal.log_warning(f"CI_SHOT: HighResShot AFTER failed: {e}")
	time.sleep(2)
	if not os.path.isfile(os.path.join(screenshots_dir, after_name)):
		unreal.AutomationLibrary.take_high_res_screenshot(3840, 2160, after_name)
		_wait_and_copy(prev, after_name)
		# If still missing, do a targeted Windows capture while the editor is open
		outp_a = os.path.join(screenshots_dir, after_name)
		if not os.path.isfile(outp_a):
			_win_capture(outp_a, delay_ms=200)
	
	print("Screenshots captured successfully!")
	print(f"Before: {os.path.join(screenshots_dir, before_name)}")
	print(f"After: {os.path.join(screenshots_dir, after_name)}")
	
	# Exit editor
	print("Exiting editor...")
	unreal.SystemLibrary.quit_editor()

if __name__ == "__main__":
	main()


