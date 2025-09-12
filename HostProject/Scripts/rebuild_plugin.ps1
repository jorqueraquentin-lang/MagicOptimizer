# Magic Optimizer Plugin Rebuild Script
# Prevents duplicate settings windows by ensuring complete plugin rebuild

Write-Host "🔧 Magic Optimizer Plugin Rebuild Script" -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan

# Check directory
if (-not (Test-Path "HostProject.uproject")) {
    Write-Host "❌ Error: Must run from HostProject directory" -ForegroundColor Red
    exit 1
}

Write-Host "✅ Project directory confirmed" -ForegroundColor Green

# Step 1: Clean build artifacts
Write-Host "🧹 Step 1: Cleaning build artifacts..." -ForegroundColor Yellow

$cleanPaths = @(
    "Plugins\MagicOptimizer\Intermediate",
    "Plugins\MagicOptimizer\Binaries", 
    "Intermediate",
    "Binaries"
)

foreach ($path in $cleanPaths) {
    if (Test-Path $path) {
        Write-Host "   Removing: $path" -ForegroundColor Gray
        Remove-Item -Recurse -Force $path -ErrorAction SilentlyContinue
    }
}

Write-Host "✅ Build artifacts cleaned" -ForegroundColor Green

# Step 2: Check for duplicate configs
Write-Host "🚫 Step 2: Checking for duplicate configs..." -ForegroundColor Yellow

$projectConfig = "Config\DefaultEditor.ini"
if (Test-Path $projectConfig) {
    $content = Get-Content $projectConfig -Raw
    if ($content -match "MagicOptimizer") {
        Write-Host "   ⚠️  WARNING: Project config contains MagicOptimizer settings!" -ForegroundColor Red
        Write-Host "   This will cause duplicate settings windows!" -ForegroundColor Red
        Write-Host "   Removing duplicate settings..." -ForegroundColor Yellow
        
        $newContent = $content -replace "\[/Script/MagicOptimizer\.OptimizerSettings\].*", ""
        Set-Content $projectConfig $newContent -NoNewline
        Write-Host "   ✅ Duplicate settings removed" -ForegroundColor Green
    } else {
        Write-Host "   ✅ Project config is clean" -ForegroundColor Green
    }
}

# Step 3: Build project
Write-Host "🏗️  Step 3: Building project..." -ForegroundColor Yellow

$uePath = "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"

if (-not (Test-Path $uePath)) {
    Write-Host "❌ Error: UnrealBuildTool not found" -ForegroundColor Red
    exit 1
}

$buildArgs = @(
    "HostProject",
    "Win64", 
    "Development",
    "-Project=`"$(Get-Location)\HostProject.uproject`"",
    "-WaitMutex",
    "-FromMsBuild"
)

Write-Host "   Building with UnrealBuildTool..." -ForegroundColor Gray

try {
    & $uePath @buildArgs
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ Build completed successfully!" -ForegroundColor Green
    } else {
        Write-Host "❌ Build failed with exit code: $LASTEXITCODE" -ForegroundColor Red
        exit $LASTEXITCODE
    }
} catch {
    Write-Host "❌ Build failed with error: $_" -ForegroundColor Red
    exit 1
}

# Step 4: Verify plugin build
Write-Host "🔍 Step 4: Verifying plugin build..." -ForegroundColor Yellow

$pluginDll = "Plugins\MagicOptimizer\Binaries\Win64\UnrealEditor-MagicOptimizer.dll"
if (Test-Path $pluginDll) {
    $buildTime = (Get-Item $pluginDll).LastWriteTime
    Write-Host "   ✅ Plugin built successfully at: $buildTime" -ForegroundColor Green
} else {
    Write-Host "❌ Plugin binary not found!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "🎉 REBUILD COMPLETED SUCCESSFULLY!" -ForegroundColor Green
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "🚀 NEXT STEPS:" -ForegroundColor White
Write-Host "   1. Restart Unreal Editor completely" -ForegroundColor Yellow
Write-Host "   2. Check Project Settings → Plugins → Magic Optimizer" -ForegroundColor Yellow
Write-Host "   3. Verify only ONE settings window appears" -ForegroundColor Yellow
Write-Host ""
Write-Host "💡 PREVENTION TIPS:" -ForegroundColor White
Write-Host "   • NEVER manually register settings in plugin code" -ForegroundColor Cyan
Write-Host "   • NEVER create project-level configs for plugin settings" -ForegroundColor Cyan
Write-Host "   • ALWAYS use this script after making plugin changes" -ForegroundColor Cyan
