<#
  run_ci.ps1
  Part of the MagicOptimizer Unreal Engine plugin.
  Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.
#>
param(
  [string]$Phase = "Audit",
  [string]$Profile = "Mobile_Low",
  [switch]$WithRHI,
  [switch]$WithScreenshot,
  [int]$Keep = 5,
  [int]$MaxAgeDays = 7,
  [switch]$FailOnError,
  [int]$MinRows = 0
)

$ErrorActionPreference = "Stop"

# Paths
$RepoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path | Split-Path -Parent
$Sln = Join-Path $RepoRoot "HostProject/HostProject.sln"
$UProject = Join-Path $RepoRoot "HostProject/HostProject.uproject"
$CiPy = Join-Path $RepoRoot "HostProject/Content/Python/magic_optimizer/ci_run.py"
$CiShotPy = Join-Path $RepoRoot "HostProject/Content/Python/magic_optimizer/ci_shot.py"
$Saved = Join-Path $RepoRoot "HostProject/Saved"
$MOFolder = Join-Path $Saved "MagicOptimizer"
$CiRoot = Join-Path $RepoRoot "docs/ci"
$CiOut = Join-Path $CiRoot ((Get-Date).ToString("yyyyMMdd_HHmmss"))

New-Item -ItemType Directory -Force -Path $CiOut | Out-Null

# Build (fast incremental)
$ms = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe"
if (!(Test-Path $ms)) { $ms = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\MSBuild\\Current\\Bin\\MSBuild.exe" }
if (!(Test-Path $ms)) { $ms = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\MSBuild\\Current\\Bin\\MSBuild.exe" }
if (!(Test-Path $ms)) { $ms = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\MSBuild\\Current\\Bin\\MSBuild.exe" }

& $ms $Sln /p:Configuration="Development Editor" /p:Platform=Win64 /m | Write-Host

# Set env for Python script
$env:MO_PHASE = $Phase
$env:MO_PROFILE = $Profile
$env:MO_DRYRUN = "true"
$env:MO_MAXCHANGES = "100"
$env:MO_INCLUDE = ""
$env:MO_EXCLUDE = ""
$env:MO_USESELECTION = "false"
$env:MO_CATEGORIES = "Textures,Meshes,Materials"

# Private runtime log for Python
$env:MAGICOPTIMIZER_LOG = Join-Path $MOFolder "MagicOptimizerRuntime.log"

$UE = "C:\\Program Files\\Epic Games\\UE_5.6\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe"
if (!(Test-Path $UE)) { throw "UnrealEditor-Cmd.exe not found at $UE" }

# Clear editor layout/state that can cause headless tab restore crashes (e.g., Fab browser)
$ProjSavedConfigWin = Join-Path $RepoRoot "HostProject/Saved/Config/Windows"
$ProjSavedConfigWinEditor = Join-Path $RepoRoot "HostProject/Saved/Config/WindowsEditor"
Remove-Item -Force -ErrorAction SilentlyContinue (Join-Path $ProjSavedConfigWin "EditorLayout.ini")
Remove-Item -Force -ErrorAction SilentlyContinue (Join-Path $ProjSavedConfigWin "EditorPerProjectUserSettings.ini")
Remove-Item -Force -ErrorAction SilentlyContinue (Join-Path $ProjSavedConfigWinEditor "EditorLayout.ini")
Remove-Item -Force -ErrorAction SilentlyContinue (Join-Path $ProjSavedConfigWinEditor "EditorPerProjectUserSettings.ini")
Remove-Item -Recurse -Force -ErrorAction SilentlyContinue (Join-Path $Saved "Autosaves")

$commonArgs = @(
  "`"$UProject`"",
  "-ExecutePythonScript=`"$CiPy`"",
  "-nop4","-nosplash","-Unattended","-stdout","-FullStdOutLogOutput",
  "-NoLiveCoding","-NoSound","-NoXR","-NoSteam","-ResetLayout"
)
# Minimize plugin surface to avoid Fab/CEF and other tabs during CI
$commonArgs += "-DisableAllPlugins"
$commonArgs += "-EnablePlugins=MagicOptimizer,PythonScriptPlugin"
$commonArgs += "-DisablePlugins=Fab"
if (-not $WithRHI) { $commonArgs += "-nullrhi" }

& $UE $commonArgs | Write-Host

# Optional screenshot pass with RHI
if ($WithScreenshot) {
  $shotArgs = @(
    "`"$UProject`"",
    "-ExecutePythonScript=`"$CiShotPy`"",
    "-nop4","-nosplash","-Unattended","-stdout","-FullStdOutLogOutput",
    "-NoLiveCoding","-NoSound","-NoXR","-NoSteam","-ResetLayout"
  )
  if ($WithRHI) {
    $shotArgs += "-d3d11"
  }
  & $UE $shotArgs | Write-Host
}

# Collect artifacts
Copy-Item -Recurse -Force (Join-Path $MOFolder "*") $CiOut -ErrorAction SilentlyContinue
Copy-Item -Recurse -Force (Join-Path $Saved "Logs/*") $CiOut -ErrorAction SilentlyContinue

Write-Host "CI artifacts written to $CiOut"

# Summary and sanity checks
$AuditDir = Join-Path $CiOut "Audit"
$CiShotsDir = Join-Path $CiOut "CI"
$KnowledgeDir = Join-Path $CiOut "Knowledge"
$RuntimeLog = Join-Path $CiOut "MagicOptimizerRuntime.log"

$TexturesCsv = Join-Path $AuditDir "textures.csv"
$RecsCsv = Join-Path $AuditDir "textures_recommend.csv"

$texRows = 0
if (Test-Path $TexturesCsv) {
  try { $lines = Get-Content $TexturesCsv -ErrorAction Stop; if ($lines.Count -gt 0) { $texRows = [Math]::Max($lines.Count - 1, 0) } } catch {}
}

$recRows = 0
if (Test-Path $RecsCsv) {
  try { $r = Get-Content $RecsCsv -ErrorAction Stop; if ($r.Count -gt 0) { $recRows = [Math]::Max($r.Count - 1, 0) } } catch {}
}

$errorsFound = 0
if (Test-Path $RuntimeLog) {
  try {
    $errMatches = Select-String -Path $RuntimeLog -Pattern "Traceback|Error:" -SimpleMatch -ErrorAction SilentlyContinue
    $errorsFound = if ($null -ne $errMatches) { $errMatches.Count } else { 0 }
  } catch { $errorsFound = 0 }
}

$beforePng = Join-Path $CiShotsDir "01_before_test.png"
$afterPng  = Join-Path $CiShotsDir "02_after_test.png"

$summaryPath = Join-Path $CiOut "summary.md"
$ts = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

$md = @()
$md += "# MagicOptimizer CI Summary"
$md += ""
$md += "- Phase: `$Phase | Profile: `$Profile | Time: $ts"
$md += "- Artifacts: `$CiOut"
$md += ""
$md += "## CSV"
$md += "- textures.csv: $texRows rows" + (if(Test-Path $TexturesCsv){""}else{" (missing)"})
$md += "- textures_recommend.csv: $recRows rows" + (if(Test-Path $RecsCsv){""}else{" (missing)"})
$md += ""
$md += "## Screenshots"
$md += (if(Test-Path $beforePng){"- BEFORE: CI/01_before_test.png"}else{"- BEFORE: (missing)"})
$md += (if(Test-Path $afterPng){"- AFTER: CI/02_after_test.png"}else{"- AFTER: (missing)"})
$md += ""
$md += "## Knowledge"
$md += (if(Test-Path $KnowledgeDir){ (Get-ChildItem $KnowledgeDir | ForEach-Object { "- Knowledge/" + $_.Name + " (`$([math]::Round($_.Length/1KB,1)) KB)" }) } else { @("- (none)") })
$md += ""
$md += "## Logs"
$md += (if(Test-Path $RuntimeLog){"- MagicOptimizerRuntime.log (errors matched: $errorsFound)"}else{"- MagicOptimizerRuntime.log (missing)"})

Set-Content -Path $summaryPath -Value ($md -join "`r`n") -Encoding UTF8

$shouldFail = $false
$failReasons = @()
if ($FailOnError) {
  if ($Phase -ieq "Audit" -and -not (Test-Path $TexturesCsv)) { $failReasons += "missing textures.csv" }
  if ($MinRows -gt 0 -and $texRows -lt $MinRows) { $failReasons += "textures.csv rows<$MinRows" }
  if ($errorsFound -gt 0) { $failReasons += "errors in MagicOptimizerRuntime.log" }
  if ($failReasons.Count -gt 0) { $shouldFail = $true }
}

# Prune older CI runs (retention)
try {
  if (Test-Path $CiRoot) {
    $runs = Get-ChildItem -Directory $CiRoot | Sort-Object Name -Descending
    $toKeep = @($runs | Select-Object -First $Keep)
    $now = Get-Date
    foreach ($dir in $runs) {
      $isKept = $toKeep -contains $dir
      $ageDays = ($now - $dir.CreationTime).TotalDays
      if (-not $isKept -and $ageDays -gt $MaxAgeDays) {
        Write-Host ("Pruning old CI run: {0}" -f $dir.FullName)
        Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $dir.FullName
      }
    }
  }
} catch {
  Write-Warning ("CI prune failed: {0}" -f $_)
}

if ($shouldFail) {
  Write-Host ("CI checks failed: {0}" -f ($failReasons -join ", "))
  exit 1
}


