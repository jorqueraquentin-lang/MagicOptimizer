// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#include "OptimizerRun.h"
#include "OptimizerSettings.h"
#include "PythonBridge.h"
#include "MagicOptimizerLogging.h"
#include "HAL/PlatformTime.h"
#include "Misc/DateTime.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Async/Async.h"

UOptimizerRun::UOptimizerRun()
{
	Status = EOptimizerRunStatus::Idle;
	bCancelled = false;
	StartTime = FDateTime::Now();
	EndTime = FDateTime::Now();
	
	// Initialize references
	OptimizerSettings = UOptimizerSettings::Get();
	PythonBridge = NewObject<UPythonBridge>();
	
	if (PythonBridge)
	{
		PythonBridge->Initialize();
	}
}

void UOptimizerRun::Run(const FString& Phase, const TArray<FString>& Categories)
{
	if (Status == EOptimizerRunStatus::Running)
	{
		UE_LOG(LogMagicOptimizer, Warning, TEXT("OptimizerRun: Already running, cannot start new run"));
		return;
	}

	// Initialize run
	RunID = GenerateRunID();
	CurrentPhase = Phase;
	SelectedCategories = Categories;
	StartTime = FDateTime::Now();
	bCancelled = false;

	// Reset progress
	Progress = FOptimizerRunProgress();
	Progress.CurrentPhase = Phase;
	Progress.TotalAssets = 100; // TODO: Get actual asset count

	// Set status to running
	SetStatus(EOptimizerRunStatus::Running);

	UE_LOG(LogMagicOptimizer, Log, TEXT("OptimizerRun: Starting %s phase for categories: %s"), *Phase, *FString::Join(Categories, TEXT(", ")));

	// Run optimization asynchronously
	Async(EAsyncExecution::TaskGraphMainThread, [this, Phase, Categories]()
	{
		if (bCancelled)
		{
			CancelRun();
			return;
		}

		// Update progress - starting
		UpdateProgress(0.0f, Phase, TEXT("Initializing..."), 0, Progress.TotalAssets);

		// Check if Python bridge is available
		if (!PythonBridge || !PythonBridge->IsPythonAvailable())
		{
			UE_LOG(LogMagicOptimizer, Error, TEXT("OptimizerRun: Python bridge not available"));
			CompleteRun(false);
			return;
		}

		// Run optimization phase
		FOptimizerResult Result = PythonBridge->RunPhase(Phase, Categories);

		if (bCancelled)
		{
			CancelRun();
			return;
		}

		// Update progress - completed
		UpdateProgress(100.0f, Phase, TEXT("Completed"), Progress.TotalAssets, Progress.TotalAssets);

		// Complete run
		CompleteRun(Result.bSuccess);
	});
}

void UOptimizerRun::Cancel()
{
	if (Status == EOptimizerRunStatus::Running)
	{
		bCancelled = true;
		UE_LOG(LogMagicOptimizer, Log, TEXT("OptimizerRun: Cancellation requested"));
	}
}

FTimespan UOptimizerRun::GetDuration() const
{
	if (Status == EOptimizerRunStatus::Running)
	{
		return FDateTime::Now() - StartTime;
	}
	else
	{
		return EndTime - StartTime;
	}
}

FString UOptimizerRun::GenerateRunID() const
{
	// Generate a unique run ID based on timestamp and random number
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	int32 RandomNum = FMath::RandRange(1000, 9999);
	return FString::Printf(TEXT("Run_%s_%d"), *Timestamp, RandomNum);
}

void UOptimizerRun::UpdateProgress(float NewProgress, const FString& Phase, const FString& Asset, int32 Processed, int32 Total)
{
	Progress.Progress = FMath::Clamp(NewProgress, 0.0f, 100.0f);
	Progress.CurrentPhase = Phase;
	Progress.CurrentAsset = Asset;
	Progress.AssetsProcessed = Processed;
	Progress.TotalAssets = Total;
	Progress.StatusMessage = FString::Printf(TEXT("%s: %s (%d/%d)"), *Phase, *Asset, Processed, Total);

	// Broadcast progress event
	OnProgress.Broadcast(Progress);

	UE_LOG(LogMagicOptimizer, Log, TEXT("OptimizerRun: Progress %.1f%% - %s"), Progress.Progress, *Progress.StatusMessage);
}

void UOptimizerRun::SetStatus(EOptimizerRunStatus NewStatus)
{
	Status = NewStatus;
	UE_LOG(LogMagicOptimizer, Log, TEXT("OptimizerRun: Status changed to %d"), (int32)Status);
}

void UOptimizerRun::CompleteRun(bool bSuccess)
{
	EndTime = FDateTime::Now();
	
	if (bSuccess)
	{
		SetStatus(EOptimizerRunStatus::Completed);
		UE_LOG(LogMagicOptimizer, Log, TEXT("OptimizerRun: Run completed successfully in %s"), *GetDuration().ToString());
	}
	else
	{
		SetStatus(EOptimizerRunStatus::Failed);
		UE_LOG(LogMagicOptimizer, Error, TEXT("OptimizerRun: Run failed after %s"), *GetDuration().ToString());
	}

	// Broadcast completion event
	OnCompleted.Broadcast(bSuccess);
}

void UOptimizerRun::CancelRun()
{
	EndTime = FDateTime::Now();
	SetStatus(EOptimizerRunStatus::Cancelled);
	
	UE_LOG(LogMagicOptimizer, Log, TEXT("OptimizerRun: Run cancelled after %s"), *GetDuration().ToString());

	// Broadcast cancellation event
	OnCancelled.Broadcast(true);
}
