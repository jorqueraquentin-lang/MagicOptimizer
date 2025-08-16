Param(
  [Parameter(Mandatory=$true)][string]$OutPath,
  [int]$DelayMs = 0,
  [string]$WindowTitle,
  [string]$ProcessName,
  [switch]$FullScreen
)

Add-Type -AssemblyName System.Windows.Forms -ErrorAction SilentlyContinue | Out-Null
Add-Type -AssemblyName System.Drawing -ErrorAction SilentlyContinue | Out-Null

# user32 helpers
$sig = @"
using System;
using System.Runtime.InteropServices;
public static class Win32 {
  [StructLayout(LayoutKind.Sequential)]
  public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
}
"@
try { Add-Type -TypeDefinition $sig -ErrorAction SilentlyContinue | Out-Null } catch {}

function Get-WindowRectFromProc {
  Param([System.Diagnostics.Process]$Proc)
  if ($null -eq $Proc -or $Proc.MainWindowHandle -eq 0) { return $null }
  $r = New-Object Win32+RECT
  if ([Win32]::GetWindowRect($Proc.MainWindowHandle, [ref]$r)) { return $r } else { return $null }
}

try {
  if ($DelayMs -gt 0) { Start-Sleep -Milliseconds $DelayMs }
  $rect = $null
  $candidates = @()
  if ($ProcessName) {
    $candidates += Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.ProcessName -like $ProcessName }
  }
  if (-not $candidates) {
    $candidates += Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowTitle -like '*Unreal Editor*' }
  }
  foreach ($p in $candidates) {
    $r = Get-WindowRectFromProc -Proc $p
    if ($r) { $rect = $r; break }
  }
  if ($WindowTitle) {
    $pt = Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowTitle -like ('*' + $WindowTitle + '*') } | Select-Object -First 1
    $rt = Get-WindowRectFromProc -Proc $pt
    if ($rt) { $rect = $rt }
  }

  $dir = Split-Path -Parent $OutPath
  if ($dir) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }

  if ($rect -and -not $FullScreen) {
    $w = [Math]::Max($rect.Right - $rect.Left, 1)
    $h = [Math]::Max($rect.Bottom - $rect.Top, 1)
    $bmp = New-Object System.Drawing.Bitmap $w, $h
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $srcPt = New-Object System.Drawing.Point $rect.Left, $rect.Top
    $g.CopyFromScreen($srcPt, [System.Drawing.Point]::Empty, (New-Object System.Drawing.Size $w, $h))
    $bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $bmp.Dispose()
    Write-Host ("Saved window region capture to {0}" -f $OutPath)
  } else {
    $bounds = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
    $bmp = New-Object System.Drawing.Bitmap $bounds.Width, $bounds.Height
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)
    $bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $bmp.Dispose()
    Write-Host ("Saved desktop capture to {0}" -f $OutPath)
  }
} catch {
  Write-Error ("windows_capture.ps1 failed: {0}" -f $_)
  exit 1
}


