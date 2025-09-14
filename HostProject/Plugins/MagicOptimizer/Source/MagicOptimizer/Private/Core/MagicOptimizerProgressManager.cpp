#include "Core/MagicOptimizerProgressManager.h"
#include "MagicOptimizerDiagnostics.h"
#include "Model/ProgressUpdate.h"
#include "Model/AssetAuditData.h"
#include "Misc/DateTime.h"

void UMagicOptimizerProgressManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	MAGIC_LOG(General, TEXT("MagicOptimizerProgressManager initialized"));
	
	// Initialize default state
	CurrentProgress = FProgressUpdate();
	bAuditInProgress = false;
	AuditResults.Empty();
}

void UMagicOptimizerProgressManager::Deinitialize()
{
	MAGIC_LOG(General, TEXT("MagicOptimizerProgressManager deinitialized"));
	
	Super::Deinitialize();
}

void UMagicOptimizerProgressManager::StartAuditProgress(int32 TotalAssets)
{
	if (bAuditInProgress)
	{
		MAGIC_LOG(Warning, TEXT("Audit already in progress, cancelling previous audit"));
		CancelAuditProgress();
	}

	FString LogMessage = FString::Printf(TEXT("Starting audit progress tracking for %d assets"), TotalAssets);
	MAGIC_LOG(General, *LogMessage);

	// Initialize progress
	CurrentProgress = FProgressUpdate();
	CurrentProgress.TotalAssets = TotalAssets;
	CurrentProgress.AssetsProcessed = 0;
	CurrentProgress.AssetsWithIssues = 0;
	CurrentProgress.AssetsSuccessful = 0;
	CurrentProgress.AssetsFailed = 0;
	CurrentProgress.ProgressPercentage = 0.0f;
	CurrentProgress.StatusMessage = TEXT("Initializing audit...");
	CurrentProgress.StartTime = FDateTime::Now();
	CurrentProgress.EstimatedEndTime = FDateTime::Now();

	// Clear previous results
	AuditResults.Empty();
	bAuditInProgress = true;
	AuditStartTime = FDateTime::Now();

	// Broadcast initial progress
	BroadcastProgressUpdate();
}

void UMagicOptimizerProgressManager::UpdateProgress(int32 ProcessedAssets, int32 AssetsWithIssues, int32 SuccessfulAssets, int32 FailedAssets, const FString& CurrentAssetName)
{
	if (!bAuditInProgress)
	{
		MAGIC_LOG(Warning, TEXT("UpdateProgress called but no audit in progress"));
		return;
	}

	// Update progress data
	CurrentProgress.AssetsProcessed = ProcessedAssets;
	CurrentProgress.AssetsWithIssues = AssetsWithIssues;
	CurrentProgress.AssetsSuccessful = SuccessfulAssets;
	CurrentProgress.AssetsFailed = FailedAssets;

	// Calculate percentage
	if (CurrentProgress.TotalAssets > 0)
	{
		CurrentProgress.ProgressPercentage = (float)ProcessedAssets / (float)CurrentProgress.TotalAssets * 100.0f;
	}
	else
	{
		CurrentProgress.ProgressPercentage = 0.0f;
	}

	// Update status message
	if (!CurrentAssetName.IsEmpty())
	{
		CurrentProgress.StatusMessage = FString::Printf(TEXT("Processing: %s"), *CurrentAssetName);
	}
	else
	{
		CurrentProgress.StatusMessage = FString::Printf(TEXT("Processing assets... (%d/%d)"), ProcessedAssets, CurrentProgress.TotalAssets);
	}

	// Calculate estimated end time
	CalculateEstimatedEndTime();

	FString ProgressLogMessage = FString::Printf(TEXT("Progress updated: %d/%d assets (%.1f%%) - %s"), 
		ProcessedAssets, CurrentProgress.TotalAssets, CurrentProgress.ProgressPercentage, *CurrentProgress.StatusMessage);
	MAGIC_LOG(Info, *ProgressLogMessage);

	// Broadcast update
	BroadcastProgressUpdate();
}

void UMagicOptimizerProgressManager::CompleteAuditProgress()
{
	if (!bAuditInProgress)
	{
		MAGIC_LOG(Warning, TEXT("CompleteAuditProgress called but no audit in progress"));
		return;
	}

	MAGIC_LOG(General, TEXT("Audit completed successfully"));

	// Finalize progress
	CurrentProgress.ProgressPercentage = 100.0f;
	CurrentProgress.StatusMessage = FString::Printf(TEXT("Audit completed! Processed %d assets in %s"), 
		CurrentProgress.AssetsProcessed, *GetElapsedTimeString());

	bAuditInProgress = false;

	// Broadcast completion
	BroadcastProgressUpdate();
	OnAuditCompleted.Broadcast();
}

void UMagicOptimizerProgressManager::CancelAuditProgress()
{
	if (!bAuditInProgress)
	{
		return;
	}

	MAGIC_LOG(General, TEXT("Audit cancelled"));

	CurrentProgress.StatusMessage = FString::Printf(TEXT("Audit cancelled after processing %d assets"), CurrentProgress.AssetsProcessed);
	bAuditInProgress = false;

	// Broadcast cancellation
	BroadcastProgressUpdate();
	OnAuditCancelled.Broadcast(TEXT("User requested cancellation"));
}

void UMagicOptimizerProgressManager::AddAssetResult(const FAssetAuditData& AssetData)
{
	AuditResults.Add(AssetData);

	FString AssetLogMessage = FString::Printf(TEXT("Added asset result: %s"), *AssetData.AssetName);
	MAGIC_LOG(Info, *AssetLogMessage);

	// Broadcast asset processed
	OnAssetProcessed.Broadcast(AssetData);
}

TArray<FAssetAuditData> UMagicOptimizerProgressManager::GetAuditResults() const
{
	return AuditResults;
}

FProgressUpdate UMagicOptimizerProgressManager::GetCurrentProgress() const
{
	return CurrentProgress;
}

void UMagicOptimizerProgressManager::BroadcastProgressUpdate()
{
	OnProgressUpdated.Broadcast(CurrentProgress);
}

void UMagicOptimizerProgressManager::CalculateEstimatedEndTime()
{
	if (CurrentProgress.AssetsProcessed <= 0 || CurrentProgress.TotalAssets <= 0)
	{
		CurrentProgress.EstimatedEndTime = FDateTime::Now();
		return;
	}

	// Calculate average time per asset
	FTimespan ElapsedTime = FDateTime::Now() - AuditStartTime;
	double AverageTimePerAsset = ElapsedTime.GetTotalSeconds() / CurrentProgress.AssetsProcessed;

	// Estimate remaining time
	int32 RemainingAssets = CurrentProgress.TotalAssets - CurrentProgress.AssetsProcessed;
	double EstimatedRemainingSeconds = RemainingAssets * AverageTimePerAsset;

	CurrentProgress.EstimatedEndTime = FDateTime::Now() + FTimespan::FromSeconds(EstimatedRemainingSeconds);
}

FString UMagicOptimizerProgressManager::GetElapsedTimeString() const
{
	FTimespan ElapsedTime = FDateTime::Now() - AuditStartTime;
	
	int32 TotalSeconds = (int32)ElapsedTime.GetTotalSeconds();
	int32 Minutes = TotalSeconds / 60;
	int32 Seconds = TotalSeconds % 60;

	if (Minutes > 0)
	{
		return FString::Printf(TEXT("%dm %ds"), Minutes, Seconds);
	}
	else
	{
		return FString::Printf(TEXT("%ds"), Seconds);
	}
}
