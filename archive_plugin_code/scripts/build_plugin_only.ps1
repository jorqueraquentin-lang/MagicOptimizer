# Magic Optimizer Plugin Build Script
# Builds only the plugin, not the entire project

Write-Host "Magic Optimizer Plugin Build Script" -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan

# Check directory
if (-not (Test-Path "HostProject.uproject")) {
    Write-Host "Error: Must run from HostProject directory" -ForegroundColor Red
    exit 1
}

Write-Host "Project directory confirmed" -ForegroundColor Green

# Step 1: Clean plugin build artifacts
Write-Host "Step 1: Cleaning plugin build artifacts..." -ForegroundColor Yellow

$cleanPaths = @(
    "Plugins\MagicOptimizer\Intermediate",
    "Plugins\MagicOptimizer\Binaries"
)

foreach ($path in $cleanPaths) {
    if (Test-Path $path) {
        Write-Host "   Removing: $path" -ForegroundColor Gray
        Remove-Item -Recurse -Force $path -ErrorAction SilentlyContinue
    }
}

Write-Host "Plugin build artifacts cleaned" -ForegroundColor Green

# Step 2: Build the plugin using UnrealBuildTool
Write-Host "Step 2: Building MagicOptimizer plugin..." -ForegroundColor Yellow

$uePath = "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"

if (-not (Test-Path $uePath)) {
    Write-Host "Error: UnrealBuildTool not found" -ForegroundColor Red
    exit 1
}

# Build the plugin specifically
$buildArgs = @(
    "MagicOptimizer",
    "Win64", 
    "Development",
    "-Project=`"$(Get-Location)\HostProject.uproject`"",
    "-WaitMutex",
    "-FromMsBuild"
)

Write-Host "   Building MagicOptimizer plugin with UnrealBuildTool..." -ForegroundColor Gray

try {
    & $uePath @buildArgs
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Plugin build completed successfully!" -ForegroundColor Green
    } else {
        Write-Host "Plugin build failed with exit code: $LASTEXITCODE" -ForegroundColor Red
        exit $LASTEXITCODE
    }
} catch {
    Write-Host "Plugin build failed with error: $_" -ForegroundColor Red
    exit 1
}

# Step 3: Verify plugin build
Write-Host "Step 3: Verifying plugin build..." -ForegroundColor Yellow

$pluginDll = "Plugins\MagicOptimizer\Binaries\Win64\UnrealEditor-MagicOptimizer.dll"
if (Test-Path $pluginDll) {
    $buildTime = (Get-Item $pluginDll).LastWriteTime
    Write-Host "   Plugin built successfully at: $buildTime" -ForegroundColor Green
} else {
    Write-Host "Plugin binary not found!" -ForegroundColor Red
    Write-Host "Checking what was built..." -ForegroundColor Yellow
    
    # Check what's in the Binaries folder
    $binariesPath = "Plugins\MagicOptimizer\Binaries"
    if (Test-Path $binariesPath) {
        Write-Host "Contents of Binaries folder:" -ForegroundColor Gray
        Get-ChildItem -Recurse $binariesPath | ForEach-Object {
            Write-Host "   $($_.FullName)" -ForegroundColor Gray
        }
    } else {
        Write-Host "Binaries folder does not exist!" -ForegroundColor Red
    }
    exit 1
}

Write-Host ""
Write-Host "PLUGIN BUILD COMPLETED SUCCESSFULLY!" -ForegroundColor Green
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "NEXT STEPS:" -ForegroundColor White
Write-Host "   1. Restart Unreal Editor completely" -ForegroundColor Yellow
Write-Host "   2. Check if Magic Optimizer appears in Windows menu" -ForegroundColor Yellow
Write-Host "   3. Try console command: MagicOptimizer.Test" -ForegroundColor Yellow
