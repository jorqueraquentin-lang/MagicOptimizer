Param(
  [Parameter(Mandatory=$true)][string]$OutPath,
  [int]$DelayMs = 0
)

try {
  if ($DelayMs -gt 0) { Start-Sleep -Milliseconds $DelayMs }
  Add-Type -AssemblyName System.Windows.Forms -ErrorAction SilentlyContinue | Out-Null
  Add-Type -AssemblyName System.Drawing -ErrorAction SilentlyContinue | Out-Null
  $bounds = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
  $bmp = New-Object System.Drawing.Bitmap $bounds.Width, $bounds.Height
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $g.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)
  $dir = Split-Path -Parent $OutPath
  if ($dir) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
  $bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
  $g.Dispose(); $bmp.Dispose()
  Write-Host ("Saved desktop capture to {0}" -f $OutPath)
} catch {
  Write-Error ("windows_capture.ps1 failed: {0}" -f $_)
  exit 1
}


