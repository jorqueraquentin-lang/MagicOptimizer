@echo off
setlocal
set UE_ROOT=D:\Epic\UE_5.6.1
powershell -ExecutionPolicy Bypass -File "%~dp0PackagePlugin.ps1" -EngineRoot "%UE_ROOT%" -PluginName "MagicOptimizer" -TargetPlatforms "Win64" -Clean -BumpPatch
endlocal


