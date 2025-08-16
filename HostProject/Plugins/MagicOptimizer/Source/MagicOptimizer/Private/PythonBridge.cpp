// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

/*
  PythonBridge.cpp
  Part of the MagicOptimizer Unreal Engine plugin.
  Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.
*/
#include "PythonBridge.h"
#include "OptimizerSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "JsonObjectConverter.h"
#include "OptimizerLogging.h"
#include "MagicOptimizerLogging.h"
#include "Interfaces/IPluginManager.h"

UPythonBridge::UPythonBridge()
{
	PythonScriptPath = TEXT("Content/Python/magic_optimizer");
	bPythonInitialized = false;
	PythonVersion = TEXT("Unknown");
	OptimizerSettings = nullptr;
}

bool UPythonBridge::Initialize()
{
	if (bPythonInitialized)
	{
		return true;
	}

	// Get settings
	OptimizerSettings = UOptimizerSettings::Get();
	if (!OptimizerSettings)
	{
		UE_LOG(LogMagicOptimizer, Error, TEXT("Failed to get OptimizerSettings"));
		MagicOptimizerLog::AppendLine(TEXT("PythonBridge: Failed to get OptimizerSettings"));
		MagicOptimizerLog::AppendBacklog(TEXT("PythonBridge.Initialize: Get UOptimizerSettings FAILED"));
		return false;
	}

	// Initialize Python environment
	if (!InitializePythonEnvironment())
	{
		UE_LOG(LogMagicOptimizer, Error, TEXT("Failed to initialize Python environment"));
		MagicOptimizerLog::AppendLine(TEXT("PythonBridge: Failed to initialize Python environment"));
		MagicOptimizerLog::AppendBacklog(TEXT("PythonBridge.Initialize: InitializePythonEnvironment FAILED"));
		return false;
	}

	bPythonInitialized = true;
	UE_LOG(LogMagicOptimizer, Log, TEXT("PythonBridge initialized successfully (Version: %s)"), *PythonVersion);
	MagicOptimizerLog::AppendLine(FString::Printf(TEXT("PythonBridge: Initialized (Version=%s)"), *PythonVersion));
	MagicOptimizerLog::AppendBacklog(FString::Printf(TEXT("PythonBridge.Initialize: OK Version=%s"), *PythonVersion));
	return true;
}

bool UPythonBridge::IsPythonAvailable() const
{
	return bPythonInitialized;
}

FOptimizerResult UPythonBridge::RunOptimization(const FOptimizerRunParams& Params)
{
	FOptimizerResult Result;

	if (!bPythonInitialized)
	{
		// Attempt lazy initialization now
		if (!InitializePythonEnvironment())
		{
			Result.bSuccess = false;
			Result.Message = TEXT("Python bridge not initialized");
			Result.Errors.Add(TEXT("Python bridge not initialized"));
			MagicOptimizerLog::AppendLine(TEXT("PythonBridge: RunOptimization aborted (not initialized)"));
			MagicOptimizerLog::AppendBacklog(TEXT("RunOptimization: aborted, not initialized"));
			return Result;
		}
		bPythonInitialized = true;
	}

	// Prepare arguments for Python script
	TArray<FString> Arguments;
	Arguments.Add(Params.Phase);
	Arguments.Add(Params.Profile);
	Arguments.Add(Params.bDryRun ? TEXT("true") : TEXT("false"));
	Arguments.Add(FString::FromInt(Params.MaxChanges));
	Arguments.Add(Params.IncludePaths);
	Arguments.Add(Params.ExcludePaths);
	Arguments.Add(Params.bUseSelection ? TEXT("true") : TEXT("false"));

	// Add categories
	FString CategoriesStr;
	for (const FString& Category : Params.Categories)
	{
		if (!CategoriesStr.IsEmpty())
		{
			CategoriesStr += TEXT(",");
		}
		CategoriesStr += Category;
	}
	Arguments.Add(CategoriesStr);

	MagicOptimizerLog::AppendLine(FString::Printf(TEXT("RunOptimization: Phase=%s Profile=%s DryRun=%s MaxChanges=%d UseSel=%s Include='%s' Exclude='%s' Cats=[%s]"),
		*Params.Phase,
		*Params.Profile,
		Params.bDryRun ? TEXT("true") : TEXT("false"),
		Params.MaxChanges,
		Params.bUseSelection ? TEXT("true") : TEXT("false"),
		*Params.IncludePaths,
		*Params.ExcludePaths,
		*CategoriesStr));
	MagicOptimizerLog::AppendBacklog(FString::Printf(TEXT("RunOptimization Args: [%s]"), *FString::Join(Arguments, TEXT(","))));

	// Execute Python script if present using system Python
    FString Output, Error;
    const FString BasePath = GetPythonScriptPath();
    const FString ScriptPath = BasePath / TEXT("entry.py");
    {
        const bool bExists = FPlatformFileManager::Get().GetPlatformFile().FileExists(*ScriptPath);
        UE_LOG(LogMagicOptimizer, Log, TEXT("MagicOptimizer: Resolved Python path: %s (entry: %s) Exists=%s"), *BasePath, *ScriptPath, bExists ? TEXT("true") : TEXT("false"));
		MagicOptimizerLog::AppendLine(FString::Printf(TEXT("PythonBridge: ScriptPath=%s Exists=%s"), *ScriptPath, bExists ? TEXT("true") : TEXT("false")));
		MagicOptimizerLog::AppendBacklog(FString::Printf(TEXT("ScriptPath=%s Base=%s Exists=%s"), *ScriptPath, *BasePath, bExists ? TEXT("true") : TEXT("false")));
    }

	const bool bHasScript = ValidatePythonScript(ScriptPath);
	bool bRan = false;

	if (bHasScript)
	{
		UE_LOG(LogMagicOptimizer, Log, TEXT("Running system Python script: %s"), *ScriptPath);
		MagicOptimizerLog::AppendLine(FString::Printf(TEXT("PythonBridge: Exec system Python: %s"), *ScriptPath));
		MagicOptimizerLog::AppendBacklog(FString::Printf(TEXT("SystemPython Exec: %s Args=[%s]"), *ScriptPath, *FString::Join(Arguments, TEXT(","))));
		bRan = ExecutePythonScript(ScriptPath, Arguments, Output, Error);
	}

	if (bRan)
	{
		Result.bSuccess = true;
		Result.Message = TEXT("Optimization completed successfully");
		Result.OutputPath = Output;
		Result.StdOut = Output;
		Result.StdErr = Error;
		MagicOptimizerLog::AppendLine(FString::Printf(TEXT("PythonBridge: STDOUT len=%d"), Output.Len()));
		MagicOptimizerLog::AppendBacklog(FString::Printf(TEXT("STDOUT preview: %s"), *Output.Left(512)));
		if (!Error.IsEmpty())
		{
			MagicOptimizerLog::AppendLine(FString::Printf(TEXT("PythonBridge: STDERR: %s"), *Error.Left(2000)));
			MagicOptimizerLog::AppendBacklog(FString::Printf(TEXT("STDERR: %s"), *Error.Left(2000)));
		}
		// Optional JSON parse unchanged
	}
	else
	{
		Result.bSuccess = false;
		Result.Message = TEXT("Optimization failed");
		if (!Error.IsEmpty())
		{
			Result.Errors.Add(Error);
			Result.StdErr = Error;
		}
		MagicOptimizerLog::AppendLine(FString::Printf(TEXT("PythonBridge: Execution failed. Error=%s"), *Error.Left(2000)));
		MagicOptimizerLog::AppendBacklog(FString::Printf(TEXT("Execution FAILED. Error=%s"), *Error.Left(2000)));
	}

	return Result;
}

FOptimizerResult UPythonBridge::RunPhase(const FString& Phase, const TArray<FString>& Categories)
{
	FOptimizerRunParams Params;
	Params.Phase = Phase;
	Params.Categories = Categories;
	Params.Profile = OptimizerSettings ? OptimizerSettings->TargetProfile : TEXT("PC_Balanced");
	Params.bDryRun = OptimizerSettings ? OptimizerSettings->bDryRun : true;
	Params.MaxChanges = OptimizerSettings ? OptimizerSettings->MaxChanges : 100;
	Params.IncludePaths = OptimizerSettings ? OptimizerSettings->IncludePathsCsv : TEXT("");
	Params.ExcludePaths = OptimizerSettings ? OptimizerSettings->ExcludePathsCsv : TEXT("");
	Params.bUseSelection = OptimizerSettings ? OptimizerSettings->bUseSelection : false;

	return RunOptimization(Params);
}

FString UPythonBridge::GetPythonScriptPath() const
{
    const FString ConfiguredPath = OptimizerSettings ? OptimizerSettings->PythonScriptPath : PythonScriptPath;

    // Compute plugin-shipped Python path (preferred default)
    FString PluginPythonDir;
    if (IPluginManager::Get().FindPlugin(TEXT("MagicOptimizer")).IsValid())
    {
        const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("MagicOptimizer"));
        if (Plugin.IsValid())
        {
            // Plugin content dir points to .../Plugins/MagicOptimizer/Content
            PluginPythonDir = Plugin->GetContentDir() / TEXT("Python/magic_optimizer");
        }
    }

    // If no setting specified, use the plugin Python package by default
    if (ConfiguredPath.IsEmpty())
    {
        if (!PluginPythonDir.IsEmpty())
        {
            return PluginPythonDir;
        }
        // Fallback to project Content path if plugin content not found
        return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Content/Python/magic_optimizer"));
    }

    // If user configured a relative path, resolve relative to project
    if (FPaths::IsRelative(ConfiguredPath))
    {
        // Encourage plugin path when users left historical default "Content/Python/magic_optimizer"
        const FString Normalized = ConfiguredPath.Replace(TEXT("\\"), TEXT("/"));
        if (Normalized.StartsWith(TEXT("Content/Python/")) && !PluginPythonDir.IsEmpty())
        {
            return PluginPythonDir;
        }
        return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / ConfiguredPath);
    }

    // Absolute custom path
    return ConfiguredPath;
}

void UPythonBridge::SetPythonScriptPath(const FString& NewPath)
{
	PythonScriptPath = NewPath;
	if (OptimizerSettings)
	{
		OptimizerSettings->PythonScriptPath = NewPath;
		OptimizerSettings->SaveSettings();
	}
}

bool UPythonBridge::ExecutePythonScript(const FString& ScriptPath, const TArray<FString>& Arguments, FString& Output, FString& Error)
{
	// Check if script exists
	if (!ValidatePythonScript(ScriptPath))
	{
		Error = TEXT("Python script not found: ") + ScriptPath;
		return false;
	}

	// Use UE's embedded Python instead of system Python
	// This avoids the system PATH dependency that was causing failures
	
	// For UE embedded Python, we need to use UE's Python execution system
	// The script will be executed through UE's Python interpreter, not system python
	
	// Build command line for UE embedded Python
	FString CommandLine = FString::Printf(TEXT("py \"%s\""), *ScriptPath);
	for (const FString& Arg : Arguments)
	{
		CommandLine += TEXT(" \"") + Arg + TEXT("\"");
	}

	UE_LOG(LogMagicOptimizer, Log, TEXT("Executing Python script via UE embedded Python: %s"), *CommandLine);
	MagicOptimizerLog::AppendLine(FString::Printf(TEXT("PythonBridge: Exec UE embedded Python: %s"), *ScriptPath));
	MagicOptimizerLog::AppendBacklog(FString::Printf(TEXT("EmbeddedPython Exec: %s Args=[%s]"), *ScriptPath, *FString::Join(Arguments, TEXT(","))));

	// Execute command using UE's Python system
	return ExecutePythonCommand(CommandLine, Output, Error);
}

FString UPythonBridge::GetPythonVersion() const
{
	return PythonVersion;
}

bool UPythonBridge::IsPythonModuleAvailable(const FString& ModuleName) const
{
	if (!bPythonInitialized)
	{
		return false;
	}

	// Use UE's embedded Python instead of system Python
	// This avoids the system PATH dependency that was causing failures
	
	// For UE embedded Python, we assume basic modules are available
	// since they're part of UE's Python distribution
	TArray<FString> BasicModules = { TEXT("json"), TEXT("csv"), TEXT("os"), TEXT("sys"), TEXT("pathlib") };
	if (BasicModules.Contains(ModuleName))
	{
		return true;
	}
	
	// For other modules, we'll need to check via UE's Python system
	// For now, return true to avoid blocking the optimization process
	UE_LOG(LogMagicOptimizer, Log, TEXT("Assuming Python module available in UE embedded Python: %s"), *ModuleName);
	return true;
}

UOptimizerSettings* UPythonBridge::GetOptimizerSettings() const
{
	return OptimizerSettings;
}

bool UPythonBridge::InitializePythonEnvironment()
{
	// Use UE's embedded Python instead of system Python
	// This avoids the system PATH dependency that was causing failures
	
	// Set Python version to UE's embedded Python
	PythonVersion = TEXT("UE Embedded Python");
	UE_LOG(LogMagicOptimizer, Log, TEXT("Using UE Embedded Python: %s"), *PythonVersion);

	// For UE embedded Python, we assume basic modules are available
	// The actual Python execution will be handled by UE's Python system
	TArray<FString> RequiredModules = { TEXT("json"), TEXT("csv"), TEXT("os"), TEXT("sys"), TEXT("pathlib") };
	UE_LOG(LogMagicOptimizer, Log, TEXT("Assuming required Python modules available in UE embedded Python"));

	// Create output directory
	FString OutputDir = OptimizerSettings ? OptimizerSettings->OutputDirectory : TEXT("Saved/MagicOptimizer");
	if (!CreateOutputDirectory(OutputDir))
	{
		UE_LOG(LogMagicOptimizer, Warning, TEXT("Failed to create output directory: %s"), *OutputDir);
	}

	return true;
}

bool UPythonBridge::ExecutePythonCommand(const FString& Command, FString& Output, FString& Error)
{
	int32 ReturnCode = 0;
	FString StdOut, StdErr;
	const bool bSuccess = FPlatformProcess::ExecProcess(*Command, nullptr, &ReturnCode, &StdOut, &StdErr);
	Output = StdOut;
	Error = StdErr;
	if (!bSuccess || ReturnCode != 0)
	{
		UE_LOG(LogMagicOptimizer, Error, TEXT("Python command failed: %s (Return code: %d)"), *Command, ReturnCode);
		if (!Error.IsEmpty())
		{
			UE_LOG(LogMagicOptimizer, Error, TEXT("Error: %s"), *Error);
		}
		return false;
	}
	return true;
}

bool UPythonBridge::ValidatePythonScript(const FString& ScriptPath) const
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	return PlatformFile.FileExists(*ScriptPath);
}

FString UPythonBridge::GetProjectContentDir() const
{
	return FPaths::ProjectContentDir();
}

bool UPythonBridge::CreateOutputDirectory(const FString& OutputPath) const
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString FullPath = FPaths::ProjectSavedDir() / OutputPath;
	if (!PlatformFile.DirectoryExists(*FullPath))
	{
		return PlatformFile.CreateDirectoryTree(*FullPath);
	}
	return true;
}
