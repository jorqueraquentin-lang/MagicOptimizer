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
  [switch]$OpenExplorer,
  [switch]$WinDesktopShot,
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

# Ensure screenshots are present under the CI/<ts>/CI folder
$SavedCiDir = Join-Path $MOFolder "CI"
try {
  New-Item -ItemType Directory -Force -Path $CiShotsDir | Out-Null
  # 1) Copy from MagicOptimizer/CI if present
  if (Test-Path $SavedCiDir) {
    @("01_before_test.png","02_after_test.png") | ForEach-Object {
      $src = Join-Path $SavedCiDir $_
      if (Test-Path $src) { Copy-Item -Force $src (Join-Path $CiShotsDir $_) }
    }
  }
  # 2) Fallback: copy newest matching files from Saved/Screenshots
  $ScreensRoot = Join-Path $Saved "Screenshots"
  if (Test-Path $ScreensRoot) {
    $pairs = @(
      @{Name="01_before_test.png"; Pattern="01_before_test.png"},
      @{Name="02_after_test.png"; Pattern="02_after_test.png"}
    )
    foreach ($p in $pairs) {
      $dst = Join-Path $CiShotsDir $p.Name
      if (-not (Test-Path $dst)) {
        $match = Get-ChildItem -Recurse -File -ErrorAction SilentlyContinue $ScreensRoot -Filter $p.Pattern | Sort-Object LastWriteTime -Descending | Select-Object -First 1
        if ($match) { Copy-Item -Force $match.FullName $dst }
      }
    }
  }
  # 3) Windows desktop fallback capture if still missing
  if ($WinDesktopShot) {
    $cap = Join-Path $RepoRoot "tools/windows_capture.ps1"
    if (Test-Path $cap) {
      if (-not (Test-Path (Join-Path $CiShotsDir "01_before_test.png"))) {
        powershell -ExecutionPolicy Bypass -File $cap -OutPath (Join-Path $CiShotsDir "01_before_test.png") -DelayMs 500 | Write-Host
      }
      if (-not (Test-Path (Join-Path $CiShotsDir "02_after_test.png"))) {
        powershell -ExecutionPolicy Bypass -File $cap -OutPath (Join-Path $CiShotsDir "02_after_test.png") -DelayMs 500 | Write-Host
      }
    }
  }
} catch {
  Write-Warning ("Screenshot copy step failed: {0}" -f $_)
}

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
 
# Try to generate thumbnails for easier preview in editors
function New-Thumbnail($src, $dst, $maxW, $maxH) {
  try {
    Add-Type -AssemblyName System.Drawing -ErrorAction SilentlyContinue | Out-Null
    if (!(Test-Path $src)) { return }
    $img = [System.Drawing.Image]::FromFile($src)
    $w = [double]$img.Width; $h = [double]$img.Height
    if ($w -le 0 -or $h -le 0) { $img.Dispose(); return }
    $scale = [Math]::Min($maxW / $w, $maxH / $h)
    $nw = [int]([Math]::Round($w * $scale)); $nh = [int]([Math]::Round($h * $scale))
    $thumb = New-Object System.Drawing.Bitmap($nw, $nh)
    $g = [System.Drawing.Graphics]::FromImage($thumb)
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $g.DrawImage($img, 0, 0, $nw, $nh)
    $thumb.Save($dst, [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $thumb.Dispose(); $img.Dispose()
  } catch { }
}

$thumbBefore = Join-Path $CiShotsDir "thumb_before.png"
$thumbAfter  = Join-Path $CiShotsDir "thumb_after.png"
if (Test-Path $beforePng) { New-Thumbnail -src $beforePng -dst $thumbBefore -maxW 480 -maxH 270 }
if (Test-Path $afterPng)  { New-Thumbnail -src $afterPng  -dst $thumbAfter  -maxW 480 -maxH 270 }

$summaryPath = Join-Path $CiOut "summary.md"
$ts = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

$md = @()
$md += "# MagicOptimizer CI Summary"
$md += ""
$md += ("- Phase: {0} | Profile: {1} | Time: {2}" -f $Phase, $Profile, $ts)
$md += ("- Artifacts: {0}" -f $CiOut)
$md += ""
$md += "## CSV"
$texNote = ""; if (-not (Test-Path $TexturesCsv)) { $texNote = " (missing)" }
$recNote = ""; if (-not (Test-Path $RecsCsv))     { $recNote = " (missing)" }
$md += "- textures.csv: $texRows rows$texNote"
$md += "- textures_recommend.csv: $recRows rows$recNote"
$md += ""
$md += "## Screenshots"
$beforeLine = "- BEFORE: (missing)"; $afterLine = "- AFTER: (missing)"
try {
  Add-Type -AssemblyName System.Drawing -ErrorAction SilentlyContinue | Out-Null
  if (Test-Path $beforePng) {
    $imgB = [System.Drawing.Image]::FromFile($beforePng)
    $beforeLine = "- BEFORE: CI/01_before_test.png (${($imgB.Width)}x${($imgB.Height)})"
    $imgB.Dispose()
  }
  if (Test-Path $afterPng) {
    $imgA = [System.Drawing.Image]::FromFile($afterPng)
    $afterLine = "- AFTER: CI/02_after_test.png (${($imgA.Width)}x${($imgA.Height)})"
    $imgA.Dispose()
  }
} catch { }
$md += $beforeLine
$md += $afterLine
$md += ""
$md += "### Thumbnails"
if (Test-Path $thumbBefore) { $md += "![before](CI/thumb_before.png)" } else { $md += "(no before thumbnail)" }
if (Test-Path $thumbAfter)  { $md += "![after](CI/thumb_after.png)" }  else { $md += "(no after thumbnail)" }
$md += ""
$md += "## Knowledge"
if (Test-Path $KnowledgeDir) {
  (Get-ChildItem $KnowledgeDir) | ForEach-Object { $md += ("- Knowledge/{0} ({1} KB)" -f $_.Name, [math]::Round($_.Length/1KB,1)) }
} else {
  $md += "- (none)"
}
$md += ""
$md += "## Logs"
if (Test-Path $RuntimeLog) { $md += "- MagicOptimizerRuntime.log (errors matched: $errorsFound)" } else { $md += "- MagicOptimizerRuntime.log (missing)" }

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

# Optionally open Windows Explorer to the screenshots folder for convenience
try {
  if ($OpenExplorer) {
    $openPath = if (Test-Path $CiShotsDir) { $CiShotsDir } else { $CiOut }
    Start-Process explorer.exe $openPath | Out-Null
  }
} catch {}


