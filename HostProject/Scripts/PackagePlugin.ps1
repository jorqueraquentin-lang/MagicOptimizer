param(
  [Parameter(Mandatory = $true)] [string]$EngineRoot,
  [string]$PluginName = "MagicOptimizer",
  [string]$TargetPlatforms = "Win64",
  [switch]$Clean,
  [switch]$BumpPatch
)
$ErrorActionPreference = 'Stop'
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot '..')
$pluginRoot = Join-Path $repoRoot "Plugins\$PluginName"
$uplugin = Join-Path $pluginRoot "$PluginName.uplugin"
if (!(Test-Path $uplugin)) {
  $found = Get-ChildItem -Path (Join-Path $repoRoot 'Plugins') -Filter *.uplugin -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
  if ($found) { $uplugin = $found.FullName; $pluginRoot = Split-Path $uplugin -Parent } else { throw "Could not find any .uplugin under Plugins" }
}
$uat = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
if (!(Test-Path $uat)) { throw "RunUAT not found at $uat. Check -EngineRoot" }
function Bump-UPluginVersionPatch([string]$path) {
  $json = Get-Content -Raw -Path $path | ConvertFrom-Json
  if ($null -eq $json.VersionName) { return }
  $parts = "$($json.VersionName)".Split('.')
  if ($parts.Count -lt 3) { $parts = @($parts + (0..(2 - ($parts.Count - 1)) | ForEach-Object { '0' })) }
  $major = [int]$parts[0]; $minor = [int]$parts[1]; $patch = [int]$parts[2] + 1
  $json.VersionName = "$major.$minor.$patch"
  if ($null -ne $json.Version) { $json.Version = [int]$json.Version + 1 }
  ($json | ConvertTo-Json -Depth 100) | Set-Content -Path $path -Encoding UTF8
}
Write-Host "Plugin" $PluginName
Write-Host "EngineRoot" $EngineRoot
Write-Host "Descriptor" $uplugin
if ($Clean) {
  @('Binaries','Intermediate') | ForEach-Object {
    $p = Join-Path $pluginRoot $_
    if (Test-Path $p) { Write-Host "Cleaning" $p; Remove-Item -Recurse -Force $p }
  }
}
if ($BumpPatch) { Write-Host "Bumping VersionName in .uplugin"; Bump-UPluginVersionPatch $uplugin }
$artifactsRoot = Join-Path $repoRoot 'Artifacts'
if (!(Test-Path $artifactsRoot)) { New-Item -ItemType Directory -Path $artifactsRoot | Out-Null }
$stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
$packageDir = Join-Path $artifactsRoot ("{0}_{1}" -f $PluginName,$stamp)
New-Item -ItemType Directory -Path $packageDir | Out-Null
Write-Host "Packaging to" $packageDir
$args = @('BuildPlugin', "-plugin=$uplugin", "-package=$packageDir", "-TargetPlatforms=$TargetPlatforms", '-VS2022')
$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $uat
$psi.WorkingDirectory = $EngineRoot
$psi.Arguments = $args -join ' '
$psi.UseShellExecute = $false
$proc = [System.Diagnostics.Process]::Start($psi)
$proc.WaitForExit()
if ($proc.ExitCode -ne 0) { throw "RunUAT BuildPlugin failed with exit code $($proc.ExitCode)" }
Write-Host "Done. Packaged plugin at" $packageDir


