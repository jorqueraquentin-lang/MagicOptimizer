import os
import time
import unreal


def ensure_dir(path: str) -> None:
    if not os.path.isdir(path):
        os.makedirs(path, exist_ok=True)


def main() -> None:
    # Prepare output path under Saved/MagicOptimizer so CI collector grabs it
    proj_saved = unreal.Paths.project_saved_dir()
    out_dir = os.path.join(proj_saved, "MagicOptimizer", "CI")
    ensure_dir(out_dir)
    shot_path = os.path.join(out_dir, "screenshot_editor.png")

    # Give the editor a moment to finish layout restore
    unreal.log("CI_SHOT: Waiting for UI to settle...")
    time.sleep(1.0)

    # Optional: set a deterministic resolution
    try:
        unreal.SystemLibrary.execute_console_command(None, "r.setres 1280x720w")
    except Exception as e:  # noqa: BLE001
        unreal.log_warning(f"CI_SHOT: setres failed: {e}")

    # Take screenshot of the editor viewport
    unreal.log(f"CI_SHOT: Capturing screenshot → {shot_path}")
    unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, shot_path)

    # Flush and quit
    unreal.log("CI_SHOT: Screenshot captured. Requesting editor exit...")
    unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


if __name__ == "__main__":
    main()


