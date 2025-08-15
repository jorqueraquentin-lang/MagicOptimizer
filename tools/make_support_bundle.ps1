param(
    [string]$RunDir,
    [switch]$Latest,
    [string]$OutZip
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path | Split-Path -Parent
$CiRoot = Join-Path $RepoRoot "docs/ci"
if (!(Test-Path $CiRoot)) { throw "CI root not found: $CiRoot" }

if ($Latest) {
    $run = Get-ChildItem -Directory $CiRoot | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if (-not $run) { throw "No CI runs found in $CiRoot" }
} elseif ($RunDir) {
    $run = Get-Item $RunDir
} else {
    throw "Provide -Latest or -RunDir <path>"
}

$runPath = $run.FullName
if (-not $OutZip -or [string]::IsNullOrWhiteSpace($OutZip)) {
    $OutZip = "$runPath.zip"
}

# Create a manifest for quick context inside the bundle
$summary = Join-Path $runPath "summary.md"
$manifestPath = Join-Path $runPath "bundle_manifest.txt"
try {
    $lines = @()
    $lines += "MagicOptimizer Support Bundle"
    $lines += ("Run: {0}" -f $runPath)
    $lines += ("Created: {0}" -f (Get-Date).ToString("yyyy-MM-dd HH:mm:ss"))
    if (Test-Path $summary) {
        $lines += ""
        $lines += "--- summary.md (first 50 lines) ---"
        $lines += (Get-Content $summary -First 50)
    }
    Set-Content -Path $manifestPath -Value ($lines -join "`r`n") -Encoding UTF8
} catch { }

if (Test-Path $OutZip) { Remove-Item -Force $OutZip }

Compress-Archive -Path $runPath -DestinationPath $OutZip -Force

Write-Host ("Bundle created: {0}" -f $OutZip)


