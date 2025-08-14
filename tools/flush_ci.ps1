param(
	[int]$Keep = 0,
	[int]$MaxAgeDays = 0,
	[switch]$All
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path | Split-Path -Parent
$CiRoot = Join-Path $RepoRoot "docs/ci"

if (!(Test-Path $CiRoot)) {
	Write-Host "No docs/ci directory to flush."
	exit 0
}

$runs = Get-ChildItem -Directory $CiRoot | Sort-Object Name -Descending
if ($All) {
	foreach ($dir in $runs) {
		Write-Host ("Deleting: {0}" -f $dir.FullName)
		Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $dir.FullName
	}
	exit 0
}

$now = Get-Date
$toKeep = @()
if ($Keep -gt 0) { $toKeep = @($runs | Select-Object -First $Keep) }

foreach ($dir in $runs) {
	$keepByCount = ($toKeep -contains $dir)
	$keepByAge = ($MaxAgeDays -gt 0 -and ($now - $dir.CreationTime).TotalDays -le $MaxAgeDays)
	if (-not $keepByCount -and -not $keepByAge) {
		Write-Host ("Deleting: {0}" -f $dir.FullName)
		Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $dir.FullName
	}
}

Write-Host "Flush complete."


