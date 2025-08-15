<#
  git_commit_push.ps1
  Part of the MagicOptimizer Unreal Engine plugin.
  Helper to stage, commit, and push changes without shell separator issues.
  Usage examples:
    powershell -ExecutionPolicy Bypass -File tools\git_commit_push.ps1 -Message "docs: update log" -All
    powershell -ExecutionPolicy Bypass -File tools\git_commit_push.ps1 -Message "feat: add foo" -Paths @('README.md','src/foo.cpp')
#>
param(
    [Parameter(Mandatory=$true)][string]$Message,
    [switch]$All,
    [string[]]$Paths,
    [switch]$NoPush
)

$ErrorActionPreference = 'Stop'

try {
    if ($All) {
        & git add -A | Out-Null
    } elseif ($Paths -and $Paths.Count -gt 0) {
        foreach ($p in $Paths) { & git add -- $p | Out-Null }
    } else {
        # Default to adding all modified/removed/new files for convenience
        & git add -A | Out-Null
    }

    # Check if there is anything staged
    $staged = & git diff --cached --name-only
    if (-not $staged) {
        Write-Host "No staged changes to commit. Working tree may be clean."
        exit 0
    }

    & git commit -m $Message | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Nothing to commit or commit failed."
        exit $LASTEXITCODE
    }

    if (-not $NoPush) {
        & git push | Out-Null
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Push failed with exit code $LASTEXITCODE"
            exit $LASTEXITCODE
        }
        Write-Host "Committed and pushed."
    } else {
        Write-Host "Committed (push skipped)."
    }
} catch {
    Write-Error $_.Exception.Message
    exit 1
}


