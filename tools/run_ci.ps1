<#
  run_ci.ps1
  Part of the MagicOptimizer Unreal Engine plugin.
  Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.
#>
param(
  [string]$Phase = "Audit",
  [string]$Profile = "Mobile_Low",
  [switch]$WithRHI,
  [switch]$OpenExplorer,
  [switch]$FreshAudit,
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
$CiPy = Join-Path $RepoRoot "tools/ci_run.py"
$Saved = Join-Path $RepoRoot "HostProject/Saved"
$MOFolder = Join-Path $Saved "MagicOptimizer"
$CiRoot = Join-Path $RepoRoot "docs/ci"
$CiOut = Join-Path $CiRoot ((Get-Date).ToString("yyyyMMdd_HHmmss"))

New-Item -ItemType Directory -Force -Path $CiOut | Out-Null

# Optionally clear previous audit artifacts so BEFORE screenshot is truly empty
if ($FreshAudit) {
  try {
    $PrevAudit = Join-Path $MOFolder "Audit"
    if (Test-Path $PrevAudit) { Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $PrevAudit }
    $PrevLog = Join-Path $MOFolder "MagicOptimizerRuntime.log"
    if (Test-Path $PrevLog) { Remove-Item -Force -ErrorAction SilentlyContinue $PrevLog }
    # Ensure directories exist again for the run
    New-Item -ItemType Directory -Force -Path (Join-Path $MOFolder "Audit") | Out-Null
  } catch { Write-Warning ("FreshAudit cleanup failed: {0}" -f $_) }
}

# Build (fast incremental)
# Try to find MSBuild in common Visual Studio locations
$ms = $null
$vsPaths = @(
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
)

foreach ($path in $vsPaths) {
    if (Test-Path $path) {
        $ms = $path
        break
    }
}

if (!$ms) {
    # Try to find MSBuild via vswhere if available
    try {
        $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
        if (Test-Path $vswhere) {
            $vsPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
            if ($vsPath) {
                $ms = Join-Path $vsPath "MSBuild\Current\Bin\MSBuild.exe"
                if (!(Test-Path $ms)) {
                    $ms = Join-Path $vsPath "MSBuild\15.0\Bin\MSBuild.exe"
                }
            }
        }
    } catch {
        Write-Warning "vswhere detection failed: $_"
    }
}

if (!$ms) {
    throw "MSBuild not found. Please install Visual Studio or Build Tools."
}

Write-Host "Using MSBuild: $ms"

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

# Structured result file for diagnostics
$env:MAGICOPTIMIZER_OUTPUT = Join-Path $CiOut "result.json"

# Try to find UE editor in common locations
$UE = $null
$uePaths = @(
    "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
    "C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
    "C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
    "C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
    "C:\Program Files\Epic Games\UE_5.2\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
    "C:\Program Files\Epic Games\UE_5.1\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
    "C:\Program Files\Epic Games\UE_5.0\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
)

foreach ($path in $uePaths) {
    if (Test-Path $path) {
        $UE = $path
        break
    }
}

if (!$UE) {
    # Try to find UE via registry or environment
    try {
        $uePath = Get-ItemProperty -Path "HKLM:\SOFTWARE\EpicGames\Unreal Engine\*" -Name "InstalledDirectory" -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty "InstalledDirectory"
        if ($uePath) {
            $UE = Join-Path $uePath "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        }
    } catch {
        Write-Warning "Registry detection failed: $_"
    }
}

if (!$UE) {
    throw "UnrealEditor-Cmd.exe not found. Please install Unreal Engine or check the path."
}

Write-Host "Using UE Editor: $UE"

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

# Collect artifacts
Copy-Item -Recurse -Force (Join-Path $MOFolder "*") $CiOut -ErrorAction SilentlyContinue
Copy-Item -Recurse -Force (Join-Path $Saved "Logs/*") $CiOut -ErrorAction SilentlyContinue

Write-Host "CI artifacts written to $CiOut"

# Summary and sanity checks
$AuditDir = Join-Path $CiOut "Audit"
$KnowledgeDir = Join-Path $CiOut "Knowledge"
$RuntimeLog = Join-Path $CiOut "MagicOptimizerRuntime.log"
$ResultJson = Join-Path $CiOut "result.json"

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

$summaryPath = Join-Path $CiOut "summary.md"
$ts = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

$md = @()
$md += "# MagicOptimizer CI Summary"
$md += ""
$md += ("- Phase: {0} | Profile: {1} | Time: {2}" -f $Phase, $Profile, $ts)
$md += ("- Artifacts: {0}" -f $CiOut)
$md += ""
$md += "## Result"
if (Test-Path $ResultJson) {
  try {
    $res = Get-Content -Raw -Path $ResultJson | ConvertFrom-Json
    $md += ("- success: {0}" -f $res.success)
    $md += ("- message: {0}" -f $res.message)
    $md += ("- assetsProcessed: {0}" -f $res.assetsProcessed)
    $md += ("- assetsModified: {0}" -f $res.assetsModified)
  } catch { $md += "- (failed to parse result.json)" }
} else {
  $md += "- (no structured result captured)"
}
$md += ""
$md += "## CSV"
$texNote = ""; if (-not (Test-Path $TexturesCsv)) { $texNote = " (missing)" }
$recNote = ""; if (-not (Test-Path $RecsCsv))     { $recNote = " (missing)" }
$md += "- textures.csv: $texRows rows$texNote"
$md += "- textures_recommend.csv: $recRows rows$recNote"
$md += ""
$md += "## Knowledge"
if (Test-Path $KnowledgeDir) {
  $eventsFile = Join-Path $KnowledgeDir "events.jsonl"
  if (Test-Path $eventsFile) {
    try {
      $eventCount = (Get-Content $eventsFile | Measure-Object -Line).Lines
      $md += ("- Knowledge/events.jsonl: {0} events" -f $eventCount)
    } catch { $md += "- Knowledge/events.jsonl (count unavailable)" }
  }
  (Get-ChildItem $KnowledgeDir -File | Where-Object { $_.Name -ne 'events.jsonl' }) | ForEach-Object { $md += ("- Knowledge/{0} ({1} KB)" -f $_.Name, [math]::Round($_.Length/1KB,1)) }
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

# Generate enhanced CI summary
try {
  & (Join-Path $RepoRoot "tools\generate_ci_summary.ps1") -CiOutputDir $CiOut -Phase $Phase -Profile $Profile
  Write-Host "✅ Enhanced CI summary generated" -ForegroundColor Green
} catch {
  Write-Warning "Failed to generate enhanced summary: $_"
}

# Optionally open Windows Explorer to the artifacts folder for convenience
try {
  if ($OpenExplorer) {
    $openPath = $CiOut
    Start-Process explorer.exe $openPath | Out-Null
  }
} catch {}


