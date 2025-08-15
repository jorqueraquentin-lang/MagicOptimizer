// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "OptimizerRun.generated.h"

class UOptimizerSettings;
class UPythonBridge;

UENUM(BlueprintType)
enum class EOptimizerRunStatus : uint8
{
	Idle,
	Running,
	Completed,
	Failed,
	Cancelled
};

USTRUCT(BlueprintType)
struct FOptimizerRunProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	float Progress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FString CurrentPhase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FString CurrentAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 AssetsProcessed;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Progress")
	int32 TotalAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FString StatusMessage;

	FOptimizerRunProgress()
	{
		Progress = 0.0f;
		AssetsProcessed = 0;
		TotalAssets = 0;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptimizerRunProgress, const FOptimizerRunProgress&, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptimizerRunCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptimizerRunCancelled, bool, bCancelled);

UCLASS(BlueprintType, Blueprintable)
class MAGICOPTIMIZER_API UOptimizerRun : public UObject
{
	GENERATED_BODY()

public:
	UOptimizerRun();

	// Run optimization workflow
	UFUNCTION(BlueprintCallable, Category = "Optimizer Run")
	void Run(const FString& Phase, const TArray<FString>& Categories);

	// Cancel current run
	UFUNCTION(BlueprintCallable, Category = "Optimizer Run")
	void Cancel();

	// Get current status
	UFUNCTION(BlueprintCallable, Category = "Optimizer Run")
	EOptimizerRunStatus GetStatus() const { return Status; }

	// Get current progress
	UFUNCTION(BlueprintCallable, Category = "Optimizer Run")
	FOptimizerRunProgress GetProgress() const { return Progress; }

	// Check if running
	UFUNCTION(BlueprintCallable, Category = "Optimizer Run")
	bool IsRunning() const { return Status == EOptimizerRunStatus::Running; }

	// Get run ID
	UFUNCTION(BlueprintCallable, Category = "Optimizer Run")
	FString GetRunID() const { return RunID; }

	// Get start time
	UFUNCTION(BlueprintCallable, Category = "Optimizer Run")
	FDateTime GetStartTime() const { return StartTime; }

	// Get end time
	UFUNCTION(BlueprintCallable, Category = "Optimizer Run")
	FDateTime GetEndTime() const { return EndTime; }

	// Get duration
	UFUNCTION(BlueprintCallable, Category = "Optimizer Run")
	FTimespan GetDuration() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnOptimizerRunProgress OnProgress;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnOptimizerRunCompleted OnCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnOptimizerRunCancelled OnCancelled;

protected:
	// Run ID for this optimization run
	UPROPERTY(BlueprintReadOnly, Category = "Optimizer Run")
	FString RunID;

	// Current status
	UPROPERTY(BlueprintReadOnly, Category = "Optimizer Run")
	EOptimizerRunStatus Status;

	// Current progress
	UPROPERTY(BlueprintReadOnly, Category = "Optimizer Run")
	FOptimizerRunProgress Progress;

	// Start time
	UPROPERTY(BlueprintReadOnly, Category = "Optimizer Run")
	FDateTime StartTime;

	// End time
	UPROPERTY(BlueprintReadOnly, Category = "Optimizer Run")
	FDateTime EndTime;

	// Settings reference
	UPROPERTY()
	UOptimizerSettings* OptimizerSettings;

	// Python bridge reference
	UPROPERTY()
	UPythonBridge* PythonBridge;

	// Current phase
	UPROPERTY(BlueprintReadOnly, Category = "Optimizer Run")
	FString CurrentPhase;

	// Selected categories
	UPROPERTY(BlueprintReadOnly, Category = "Optimizer Run")
	TArray<FString> SelectedCategories;

	// Whether run was cancelled
	UPROPERTY(BlueprintReadOnly, Category = "Optimizer Run")
	bool bCancelled;

	// Generate run ID
	FString GenerateRunID() const;

	// Update progress
	void UpdateProgress(float NewProgress, const FString& Phase, const FString& Asset, int32 Processed, int32 Total);

	// Set status
	void SetStatus(EOptimizerRunStatus NewStatus);

	// Complete run
	void CompleteRun(bool bSuccess);

	// Cancel run
	void CancelRun();
};
