param(
    [switch]$DryRun
)

function Remove-PathSafe {
    param(
        [Parameter(Mandatory=$true)][string]$Path
    )
    if (Test-Path -LiteralPath $Path) {
        if ($DryRun) {
            Write-Host "[DryRun] Would remove: $Path"
        } else {
            Write-Host "Removing: $Path"
            try {
                Remove-Item -LiteralPath $Path -Recurse -Force -ErrorAction Stop
            } catch {
                Write-Warning "Failed to remove ${Path}: $($_.Exception.Message)"
            }
        }
    } else {
        Write-Host "Skip (not found): $Path"
    }
}

Write-Host "MagicOptimizer workspace cleanup started (DryRun=$($DryRun.IsPresent))"

# Plugin build artifacts
Remove-PathSafe -Path "HostProject/Plugins/MagicOptimizer/Binaries"
Remove-PathSafe -Path "HostProject/Plugins/MagicOptimizer/Intermediate"

# Project build artifacts and caches
Remove-PathSafe -Path "HostProject/Binaries"
Remove-PathSafe -Path "HostProject/Intermediate"
Remove-PathSafe -Path "HostProject/DerivedDataCache"

# Project Saved subfolders that are safe to purge
Remove-PathSafe -Path "HostProject/Saved/Logs"
Remove-PathSafe -Path "HostProject/Saved/Crashes"
Remove-PathSafe -Path "HostProject/Saved/Autosaves"
Remove-PathSafe -Path "HostProject/Saved/ShaderDebugInfo"
Remove-PathSafe -Path "HostProject/Saved/webcache_4430"
Remove-PathSafe -Path "HostProject/Saved/SourceControl"

# Python __pycache__ folders
$pycacheDirs = @(
    "HostProject/Plugins/MagicOptimizer/Content/Python/magic_optimizer/__pycache__"
)
foreach ($d in $pycacheDirs) { Remove-PathSafe -Path $d }

# Optional: remove duplicated Cursor rules under HostProject now that rules exist at repo root
if (Test-Path ".cursor/rules/ue561-plugin-builder.mdc") {
    Remove-PathSafe -Path "HostProject/.cursor"
}

Write-Host "MagicOptimizer workspace cleanup completed."


