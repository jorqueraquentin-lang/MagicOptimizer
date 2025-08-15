// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PythonBridge.generated.h"

class UOptimizerSettings;

USTRUCT(BlueprintType)
struct FOptimizerResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bSuccess;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FString OutputPath;

	// Captured standard output from Python execution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FString StdOut;

	// Captured standard error from Python execution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FString StdErr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FString> Errors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FString> Warnings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 AssetsProcessed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 AssetsModified;

	FOptimizerResult()
	{
		bSuccess = false;
		AssetsProcessed = 0;
		AssetsModified = 0;
	}
};

USTRUCT(BlueprintType)
struct FOptimizerRunParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Run")
	FString Phase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Run")
	TArray<FString> Categories;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Run")
	FString Profile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Run")
	bool bDryRun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Run")
	int32 MaxChanges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Run")
	FString IncludePaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Run")
	FString ExcludePaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Run")
	bool bUseSelection;

	FOptimizerRunParams()
	{
		Phase = TEXT("audit");
		bDryRun = true;
		MaxChanges = 100;
		bUseSelection = false;
	}
};

UCLASS(BlueprintType, Blueprintable)
class MAGICOPTIMIZER_API UPythonBridge : public UObject
{
	GENERATED_BODY()

public:
	UPythonBridge();

	// Initialize Python bridge
	UFUNCTION(BlueprintCallable, Category = "Python Bridge")
	bool Initialize();

	// Check if Python is available
	UFUNCTION(BlueprintCallable, Category = "Python Bridge")
	bool IsPythonAvailable() const;

	// Run optimization workflow
	UFUNCTION(BlueprintCallable, Category = "Python Bridge")
	FOptimizerResult RunOptimization(const FOptimizerRunParams& Params);

	// Run specific phase
	UFUNCTION(BlueprintCallable, Category = "Python Bridge")
	FOptimizerResult RunPhase(const FString& Phase, const TArray<FString>& Categories);

	// Get Python script path
	UFUNCTION(BlueprintCallable, Category = "Python Bridge")
	FString GetPythonScriptPath() const;

	// Set Python script path
	UFUNCTION(BlueprintCallable, Category = "Python Bridge")
	void SetPythonScriptPath(const FString& NewPath);

	// Execute Python script
	UFUNCTION(BlueprintCallable, Category = "Python Bridge")
	bool ExecutePythonScript(const FString& ScriptPath, const TArray<FString>& Arguments, FString& Output, FString& Error);

	// Get Python version
	UFUNCTION(BlueprintCallable, Category = "Python Bridge")
	FString GetPythonVersion() const;

	// Check if specific Python module is available
	UFUNCTION(BlueprintCallable, Category = "Python Bridge")
	bool IsPythonModuleAvailable(const FString& ModuleName) const;

	// Get optimization settings
	UFUNCTION(BlueprintCallable, Category = "Python Bridge")
	UOptimizerSettings* GetOptimizerSettings() const;

protected:
	// Python script path
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Python Bridge")
	FString PythonScriptPath;

	// Whether Python is initialized
	UPROPERTY(BlueprintReadOnly, Category = "Python Bridge")
	bool bPythonInitialized;

	// Python version string
	UPROPERTY(BlueprintReadOnly, Category = "Python Bridge")
	FString PythonVersion;

	// Settings reference
	UPROPERTY()
	UOptimizerSettings* OptimizerSettings;

	// Initialize Python environment
	bool InitializePythonEnvironment();

	// Execute Python command
	bool ExecutePythonCommand(const FString& Command, FString& Output, FString& Error);

	// Validate Python script
	bool ValidatePythonScript(const FString& ScriptPath) const;

	// Get project content directory
	FString GetProjectContentDir() const;

	// Create output directory
	bool CreateOutputDirectory(const FString& OutputPath) const;
};
