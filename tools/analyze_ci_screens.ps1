Param(
  [string]$CiRoot = "docs\ci"
)

function Get-LatestCiRun {
  Param([string]$Root)
  if (-not (Test-Path $Root)) { return $null }
  $dir = Get-ChildItem -Directory $Root | Sort-Object Name -Descending | Select-Object -First 1
  if ($dir) { return $dir.FullName } else { return $null }
}

function Get-ImgInfo {
  Param([string]$Path)
  $result = @{ Path = $Path; Exists = (Test-Path $Path) }
  if (-not $result.Exists) { return $result }
  try {
    Add-Type -AssemblyName System.Drawing -ErrorAction SilentlyContinue | Out-Null
    $img = [System.Drawing.Image]::FromFile($Path)
    $bmp = New-Object System.Drawing.Bitmap $img
    $w = [int]$img.Width; $h = [int]$img.Height
    $sum = 0.0; $count = 0
    $stepX = [Math]::Max([int]($w/64),1)
    $stepY = [Math]::Max([int]($h/64),1)
    for ($y = 0; $y -lt $h; $y += $stepY) {
      for ($x = 0; $x -lt $w; $x += $stepX) {
        $px = $bmp.GetPixel($x,$y)
        $br = ($px.R*0.299 + $px.G*0.587 + $px.B*0.114)
        $sum += $br
        $count++
      }
    }
    $avg = if ($count -gt 0) { [math]::Round($sum/$count,1) } else { 0 }
    $bmp.Dispose(); $img.Dispose()
    $file = Get-Item $Path
    $result.Width = $w
    $result.Height = $h
    $result.AvgBrightness = $avg
    $result.FileKB = [math]::Round($file.Length/1KB,1)
  } catch {
    $result.Error = $_.Exception.Message
  }
  return $result
}

$latest = Get-LatestCiRun -Root $CiRoot
if (-not $latest) { Write-Error "No CI run found under $CiRoot"; exit 1 }

$ciDir = Join-Path $latest 'CI'
$before = Join-Path $ciDir '01_before_test.png'
$after  = Join-Path $ciDir '02_after_test.png'

$b = Get-ImgInfo -Path $before
$a = Get-ImgInfo -Path $after

$delta = $null
if ($b.Exists -and $a.Exists -and $b.AvgBrightness -ne $null -and $a.AvgBrightness -ne $null) {
  $delta = [math]::Round(($a.AvgBrightness - $b.AvgBrightness),1)
}

$metrics = [pscustomobject]@{
  CiRun = Split-Path $latest -Leaf
  Before = $b
  After = $a
  BrightnessDelta = $delta
}

$outJson = $metrics | ConvertTo-Json -Depth 6
$metricsPath = Join-Path $latest 'image_metrics.json'
Set-Content -Path $metricsPath -Value $outJson -Encoding UTF8

$outJson


