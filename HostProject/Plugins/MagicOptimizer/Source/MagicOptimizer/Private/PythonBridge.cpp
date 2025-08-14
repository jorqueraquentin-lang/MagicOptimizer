// Copyright Epic Games, Inc. All Rights Reserved.

#include "PythonBridge.h"
#include "OptimizerSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "IPythonScriptPlugin.h"
#include "Framework/Application/SlateApplication.h"
#include "Editor.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

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
		UE_LOG(LogTemp, Error, TEXT("Failed to get OptimizerSettings"));
		return false;
	}

	// Initialize Python environment
	if (!InitializePythonEnvironment())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize Python environment"));
		return false;
	}

	bPythonInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("PythonBridge initialized successfully"));
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
		Result.bSuccess = false;
		Result.Message = TEXT("Python bridge not initialized");
		Result.Errors.Add(TEXT("Python bridge not initialized"));
		return Result;
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

	// Execute Python script
	FString Output, Error;
	FString ScriptPath = GetPythonScriptPath() / TEXT("entry.py");
	
	if (ExecutePythonScript(ScriptPath, Arguments, Output, Error))
	{
		Result.bSuccess = true;
		Result.Message = TEXT("Optimization completed successfully");
		Result.OutputPath = Output;
		
		// Parse output for statistics (simplified)
		if (Output.Contains(TEXT("assets processed")))
		{
			Result.AssetsProcessed = 1; // Simplified parsing
		}
		if (Output.Contains(TEXT("assets modified")))
		{
			Result.AssetsModified = 1; // Simplified parsing
		}
	}
	else
	{
		Result.bSuccess = false;
		Result.Message = TEXT("Optimization failed");
		Result.Errors.Add(Error);
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
	if (OptimizerSettings)
	{
		return OptimizerSettings->PythonScriptPath;
	}
	return PythonScriptPath;
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

	// Build command line
	FString CommandLine = FString::Printf(TEXT("python \"%s\""), *ScriptPath);
	for (const FString& Arg : Arguments)
	{
		CommandLine += TEXT(" \"") + Arg + TEXT("\"");
	}

	// Execute command
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

	// Check if Python module is available
	FString Output, Error;
	FString Command = FString::Printf(TEXT("python -c \"import %s; print('OK')\""), *ModuleName);
	
	// Create a temporary non-const instance to call ExecutePythonCommand
	UPythonBridge* NonConstThis = const_cast<UPythonBridge*>(this);
	return NonConstThis->ExecutePythonCommand(Command, Output, Error) && Output.Contains(TEXT("OK"));
}

UOptimizerSettings* UPythonBridge::GetOptimizerSettings() const
{
	return OptimizerSettings;
}

bool UPythonBridge::InitializePythonEnvironment()
{
	// Check if Python is available in the system
	FString Output, Error;
	if (!ExecutePythonCommand(TEXT("python --version"), Output, Error))
	{
		UE_LOG(LogTemp, Warning, TEXT("Python not found in system PATH"));
		return false;
	}

	// Extract Python version
	PythonVersion = Output.TrimStartAndEnd();
	UE_LOG(LogTemp, Log, TEXT("Found Python: %s"), *PythonVersion);

	// Check if required Python modules are available
	TArray<FString> RequiredModules = { TEXT("json"), TEXT("csv"), TEXT("os"), TEXT("sys"), TEXT("pathlib") };
	for (const FString& Module : RequiredModules)
	{
		if (!IsPythonModuleAvailable(Module))
		{
			UE_LOG(LogTemp, Warning, TEXT("Required Python module not available: %s"), *Module);
			return false;
		}
	}

	// Create output directory
	FString OutputDir = OptimizerSettings ? OptimizerSettings->OutputDirectory : TEXT("Saved/MagicOptimizer");
	if (!CreateOutputDirectory(OutputDir))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create output directory: %s"), *OutputDir);
	}

	return true;
}

bool UPythonBridge::ExecutePythonCommand(const FString& Command, FString& Output, FString& Error)
{
	// Execute Python command using system shell
	int32 ReturnCode = 0;
	
	// Use FPlatformProcess::ExecProcess for cross-platform compatibility
	FString StdOut, StdErr;
	const bool bSuccess = FPlatformProcess::ExecProcess(*Command, nullptr, &ReturnCode, &StdOut, &StdErr);
	
	Output = StdOut;
	Error = StdErr;
	
	if (!bSuccess || ReturnCode != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Python command failed: %s (Return code: %d)"), *Command, ReturnCode);
		if (!Error.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("Error: %s"), *Error);
		}
		return false;
	}
	
	return true;
}

bool UPythonBridge::ValidatePythonScript(const FString& ScriptPath) const
{
	// Check if script file exists
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	return PlatformFile.FileExists(*ScriptPath);
}

FString UPythonBridge::GetProjectContentDir() const
{
	// Get project content directory
	return FPaths::ProjectContentDir();
}

bool UPythonBridge::CreateOutputDirectory(const FString& OutputPath) const
{
	// Create output directory if it doesn't exist
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	FString FullPath = FPaths::ProjectSavedDir() / OutputPath;
	if (!PlatformFile.DirectoryExists(*FullPath))
	{
		return PlatformFile.CreateDirectoryTree(*FullPath);
	}
	
	return true;
}
