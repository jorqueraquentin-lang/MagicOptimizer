param(
  [string]$Phase = "Audit",
  [string]$Profile = "Mobile_Low",
  [switch]$WithRHI,
  [switch]$WithScreenshot
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
$CiOut = Join-Path $RepoRoot ("docs/ci/" + (Get-Date).ToString("yyyyMMdd_HHmmss"))

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


