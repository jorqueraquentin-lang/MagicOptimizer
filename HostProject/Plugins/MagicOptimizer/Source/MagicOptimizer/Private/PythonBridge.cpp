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
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "JsonObjectConverter.h"
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
	UE_LOG(LogTemp, Log, TEXT("PythonBridge initialized successfully (Version: %s)"), *PythonVersion);
	return true;
}

bool UPythonBridge::IsPythonAvailable() const
{
	// Consider embedded Python presence as available
	return bPythonInitialized || (IPythonScriptPlugin::Get() != nullptr);
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

	// Execute Python script if present, otherwise run embedded hello world
    FString Output, Error;
    const FString BasePath = GetPythonScriptPath();
    const FString ScriptPath = BasePath / TEXT("entry.py");
    {
        const bool bExists = FPlatformFileManager::Get().GetPlatformFile().FileExists(*ScriptPath);
        UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Resolved Python path: %s (entry: %s) Exists=%s"), *BasePath, *ScriptPath, bExists ? TEXT("true") : TEXT("false"));
    }

	const bool bHasScript = ValidatePythonScript(ScriptPath);
	bool bRan = false;

	// If we have embedded Python and a script, execute it within embedded Python and read a result file
	if (bHasScript && IPythonScriptPlugin::Get())
	{
		// Ensure output directory exists and build an output file path
		const FString OutputDir = OptimizerSettings ? OptimizerSettings->OutputDirectory : TEXT("Saved/MagicOptimizer");
		const FString FullOutputDir = FPaths::ProjectSavedDir() / OutputDir;
		IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
		PF.CreateDirectoryTree(*FullOutputDir);
		const FString ResultFile = FullOutputDir / TEXT("last_result.json");

		// Build a Python snippet to set sys.argv and exec the file
		FString PyArgList = TEXT("[");
		// argv[0] is the script path
		PyArgList += FString::Printf(TEXT("r'%s'"), *ScriptPath);
		for (const FString& Arg : Arguments)
		{
			PyArgList += TEXT(", r'") + Arg.ReplaceCharWithEscapedChar() + TEXT("'");
		}
		PyArgList += TEXT("]");

		const FString PyCode = FString::Printf(
			TEXT("import sys, os\n")
			TEXT("sys.argv = %s\n")
			TEXT("os.environ['MAGICOPTIMIZER_OUTPUT']=r'%s'\n")
			TEXT("_p=r'%s'\n")
			TEXT("_c=open(_p,'r',encoding='utf-8').read()\n")
			TEXT("exec(compile(_c,_p,'exec'),{'__name__':'__main__'})\n"),
			*PyArgList, *ResultFile, *ScriptPath);

		UE_LOG(LogTemp, Log, TEXT("Running embedded Python script: %s"), *ScriptPath);
		bRan = IPythonScriptPlugin::Get()->ExecPythonCommand(*PyCode);
		// Try to read the result file
		FString FileContents;
		if (bRan && PF.FileExists(*ResultFile) && FFileHelper::LoadFileToString(FileContents, *ResultFile))
		{
			Output = FileContents;
		}
	}
	else if (bHasScript)
	{
		UE_LOG(LogTemp, Log, TEXT("Running system python script: %s"), *ScriptPath);
		bRan = ExecutePythonScript(ScriptPath, Arguments, Output, Error);
	}
    else if (IPythonScriptPlugin::Get())
	{
        UE_LOG(LogTemp, Log, TEXT("Running embedded Python Hello World (no entry.py found)"));
		IPythonScriptPlugin::Get()->ExecPythonCommand(TEXT("print('MagicOptimizer: Hello from embedded Python')"));
		bRan = true;
		Output = TEXT("Embedded Python Hello World executed");
	}

	if (bRan)
	{
		Result.bSuccess = true;
		Result.Message = bHasScript ? TEXT("Optimization completed successfully") : TEXT("Embedded Python executed (Hello World)");
		Result.OutputPath = Output;
		Result.StdOut = Output;
		Result.StdErr = Error;

		// Attempt to parse structured JSON from stdout if present
		TSharedPtr<FJsonObject> JsonObj;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Output);
		if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
		{
			// Optional fields we care about
			FString Msg;
			int32 AssetsProcessed = 0;
			int32 AssetsModified = 0;
			bool bOk = false;
			JsonObj->TryGetStringField(TEXT("message"), Msg);
			JsonObj->TryGetNumberField(TEXT("assetsProcessed"), AssetsProcessed);
			JsonObj->TryGetNumberField(TEXT("assetsModified"), AssetsModified);
			JsonObj->TryGetBoolField(TEXT("success"), bOk);
			if (!Msg.IsEmpty())
			{
				Result.Message = Msg;
			}
			if (AssetsProcessed > 0) { Result.AssetsProcessed = AssetsProcessed; }
			if (AssetsModified > 0) { Result.AssetsModified = AssetsModified; }
			if (!bOk) { Result.bSuccess = false; }
		}
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
    FString Path = OptimizerSettings ? OptimizerSettings->PythonScriptPath : PythonScriptPath;
    if (FPaths::IsRelative(Path))
    {
        // Resolve relative to project directory so "Content/..." works
        return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / Path);
    }
    return Path;
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

	// Prefer embedded Python
	if (IPythonScriptPlugin::Get())
	{
		const FString Code = FString::Printf(TEXT("import %s"), *ModuleName);
		return IPythonScriptPlugin::Get()->ExecPythonCommand(*Code);
	}

	// Fallback to system Python
	FString Output, Error;
	FString Command = FString::Printf(TEXT("python -c \"import %s; print('OK')\""), *ModuleName);
	UPythonBridge* NonConstThis = const_cast<UPythonBridge*>(this);
	return NonConstThis->ExecutePythonCommand(Command, Output, Error) && Output.Contains(TEXT("OK"));
}

UOptimizerSettings* UPythonBridge::GetOptimizerSettings() const
{
	return OptimizerSettings;
}

bool UPythonBridge::InitializePythonEnvironment()
{
    // Prefer Unreal's embedded Python if available (avoid executing any code here to prevent crashes)
    if (IPythonScriptPlugin::Get() != nullptr)
    {
        PythonVersion = TEXT("UE Embedded Python");
        UE_LOG(LogTemp, Log, TEXT("Using embedded Python"));
        const FString OutputDir = OptimizerSettings ? OptimizerSettings->OutputDirectory : TEXT("Saved/MagicOptimizer");
        CreateOutputDirectory(OutputDir);
        return true;
    }

	// Fallback: system Python
	FString Output, Error;
    if (!ExecutePythonCommand(TEXT("python --version"), Output, Error))
	{
		UE_LOG(LogTemp, Warning, TEXT("Python not found in system PATH"));
		return false;
	}

	PythonVersion = Output.TrimStartAndEnd();
	UE_LOG(LogTemp, Log, TEXT("Found Python: %s"), *PythonVersion);

	TArray<FString> RequiredModules = { TEXT("json"), TEXT("csv"), TEXT("os"), TEXT("sys"), TEXT("pathlib") };
	for (const FString& Module : RequiredModules)
	{
		if (!IsPythonModuleAvailable(Module))
		{
			UE_LOG(LogTemp, Warning, TEXT("Required Python module not available: %s"), *Module);
			return false;
		}
	}

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
