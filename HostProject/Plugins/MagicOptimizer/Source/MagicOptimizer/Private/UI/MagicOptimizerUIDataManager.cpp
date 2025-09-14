#include "UI/MagicOptimizerUIDataManager.h"
#include "Core/MagicOptimizerProgressManager.h"
#include "MagicOptimizerDiagnostics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UMagicOptimizerUIDataManager::UMagicOptimizerUIDataManager()
{
	ProgressManager = nullptr;
	bIsInitialized = false;
	CurrentProgress = FProgressUpdate();
	CurrentAuditResults.Empty();
}

void UMagicOptimizerUIDataManager::Initialize()
{
	if (bIsInitialized)
	{
		return;
	}

	MAGIC_LOG(General, TEXT("Initializing UI Data Manager"));

	// Get the Progress Manager instance
	if (GEngine && GWorld && GWorld->GetGameInstance())
	{
		ProgressManager = GWorld->GetGameInstance()->GetSubsystem<UMagicOptimizerProgressManager>();
		if (ProgressManager)
		{
			BindToProgressManager();
			MAGIC_LOG(General, TEXT("UI Data Manager connected to Progress Manager"));
		}
		else
		{
			MAGIC_LOG(Warning, TEXT("Failed to get Progress Manager instance"));
		}
	}

	bIsInitialized = true;
	MAGIC_LOG(General, TEXT("UI Data Manager initialized"));
}

void UMagicOptimizerUIDataManager::Shutdown()
{
	if (!bIsInitialized)
	{
		return;
	}

	MAGIC_LOG(General, TEXT("Shutting down UI Data Manager"));

	UnbindFromProgressManager();
	ProgressManager = nullptr;
	bIsInitialized = false;

	MAGIC_LOG(General, TEXT("UI Data Manager shutdown complete"));
}

FProgressUpdate UMagicOptimizerUIDataManager::GetCurrentProgress() const
{
	return CurrentProgress;
}

TArray<FAssetAuditData> UMagicOptimizerUIDataManager::GetCurrentAuditResults() const
{
	return CurrentAuditResults;
}

int32 UMagicOptimizerUIDataManager::GetHealthScore() const
{
	if (CurrentAuditResults.Num() == 0)
	{
		return 100; // No data means no issues
	}

	// Calculate health score based on issues found
	int32 TotalIssues = 0;
	int32 CriticalIssues = 0;
	float TotalMemoryUsage = 0.0f;

	for (const FAssetAuditData& AssetData : CurrentAuditResults)
	{
		TotalIssues += AssetData.Issues.Num();
		TotalMemoryUsage += AssetData.MemoryUsageMB;

		// Count critical issues (severity >= 0.8)
		for (const FAuditIssue& Issue : AssetData.Issues)
		{
			if (Issue.Severity >= 0.8f)
			{
				CriticalIssues++;
			}
		}
	}

	// Health score calculation: 100 - (critical issues * 10) - (total issues * 2)
	// But never go below 0
	int32 HealthScore = FMath::Max(0, 100 - (CriticalIssues * 10) - (TotalIssues * 2));
	
	// Bonus for good memory usage (if under 100MB total)
	if (TotalMemoryUsage < 100.0f)
	{
		HealthScore = FMath::Min(100, HealthScore + 10);
	}

	return HealthScore;
}

int32 UMagicOptimizerUIDataManager::GetCriticalIssuesCount() const
{
	int32 CriticalIssues = 0;
	
	for (const FAssetAuditData& AssetData : CurrentAuditResults)
	{
		for (const FAuditIssue& Issue : AssetData.Issues)
		{
			if (Issue.Severity >= 0.8f)
			{
				CriticalIssues++;
			}
		}
	}
	
	return CriticalIssues;
}

int32 UMagicOptimizerUIDataManager::GetTotalAssetsScanned() const
{
	return CurrentProgress.TotalAssets;
}

int32 UMagicOptimizerUIDataManager::GetProgressPercentage() const
{
	return FMath::RoundToInt(CurrentProgress.ProgressPercentage * 100.0f);
}

FString UMagicOptimizerUIDataManager::GetCurrentStatusMessage() const
{
	return CurrentProgress.StatusMessage;
}

FString UMagicOptimizerUIDataManager::GetCurrentAssetName() const
{
	return CurrentProgress.CurrentAssetName;
}

FString UMagicOptimizerUIDataManager::GetEstimatedTimeRemaining() const
{
	return CurrentProgress.TimeRemaining;
}

float UMagicOptimizerUIDataManager::GetTotalMemoryUsage() const
{
	float TotalMemory = 0.0f;
	
	for (const FAssetAuditData& AssetData : CurrentAuditResults)
	{
		TotalMemory += AssetData.MemoryUsageMB;
	}
	
	return TotalMemory;
}

float UMagicOptimizerUIDataManager::GetEstimatedMemorySavings() const
{
	float TotalSavings = 0.0f;
	
	for (const FAssetAuditData& AssetData : CurrentAuditResults)
	{
		for (const FOptimizationRecommendation& Recommendation : AssetData.Recommendations)
		{
			TotalSavings += Recommendation.EstimatedMemorySavingsMB;
		}
	}
	
	return TotalSavings;
}

bool UMagicOptimizerUIDataManager::IsAuditRunning() const
{
	return ProgressManager ? ProgressManager->IsAuditInProgress() : false;
}

void UMagicOptimizerUIDataManager::BindToProgressManager()
{
	if (!ProgressManager)
	{
		return;
	}

	// Unbind first to prevent duplicate bindings
	ProgressManager->OnProgressUpdated.RemoveDynamic(this, &UMagicOptimizerUIDataManager::OnProgressManagerUpdated);
	ProgressManager->OnAssetProcessed.RemoveDynamic(this, &UMagicOptimizerUIDataManager::OnProgressManagerAssetProcessed);
	ProgressManager->OnAuditCompleted.RemoveDynamic(this, &UMagicOptimizerUIDataManager::OnProgressManagerAuditCompleted);
	ProgressManager->OnAuditCancelled.RemoveDynamic(this, &UMagicOptimizerUIDataManager::OnProgressManagerAuditCancelled);

	// Bind to progress manager events
	ProgressManager->OnProgressUpdated.AddDynamic(this, &UMagicOptimizerUIDataManager::OnProgressManagerUpdated);
	ProgressManager->OnAssetProcessed.AddDynamic(this, &UMagicOptimizerUIDataManager::OnProgressManagerAssetProcessed);
	ProgressManager->OnAuditCompleted.AddDynamic(this, &UMagicOptimizerUIDataManager::OnProgressManagerAuditCompleted);
	ProgressManager->OnAuditCancelled.AddDynamic(this, &UMagicOptimizerUIDataManager::OnProgressManagerAuditCancelled);
}

void UMagicOptimizerUIDataManager::UnbindFromProgressManager()
{
	if (!ProgressManager)
	{
		return;
	}

	// Unbind from progress manager events
	ProgressManager->OnProgressUpdated.RemoveDynamic(this, &UMagicOptimizerUIDataManager::OnProgressManagerUpdated);
	ProgressManager->OnAssetProcessed.RemoveDynamic(this, &UMagicOptimizerUIDataManager::OnProgressManagerAssetProcessed);
	ProgressManager->OnAuditCompleted.RemoveDynamic(this, &UMagicOptimizerUIDataManager::OnProgressManagerAuditCompleted);
	ProgressManager->OnAuditCancelled.RemoveDynamic(this, &UMagicOptimizerUIDataManager::OnProgressManagerAuditCancelled);
}

void UMagicOptimizerUIDataManager::OnProgressManagerUpdated(const FProgressUpdate& ProgressUpdate)
{
	CurrentProgress = ProgressUpdate;
	CalculateDerivedMetrics();
	OnProgressUpdated.Broadcast(ProgressUpdate);
}

void UMagicOptimizerUIDataManager::OnProgressManagerAssetProcessed(const FAssetAuditData& AssetData)
{
	CurrentAuditResults.Add(AssetData);
	CalculateDerivedMetrics();
	OnAssetProcessed.Broadcast(AssetData);
}

void UMagicOptimizerUIDataManager::OnProgressManagerAuditCompleted()
{
	CalculateDerivedMetrics();
	OnAuditCompleted.Broadcast();
}

void UMagicOptimizerUIDataManager::OnProgressManagerAuditCancelled(const FString& Reason)
{
	FString LogMessage = FString::Printf(TEXT("Audit cancelled: %s"), *Reason);
	MAGIC_LOG(General, *LogMessage);
	CalculateDerivedMetrics();
}

void UMagicOptimizerUIDataManager::CalculateDerivedMetrics()
{
	// This function can be used to calculate any derived metrics
	// that might be needed for the UI
	// For now, we'll just log the current state
	
	if (CurrentAuditResults.Num() > 0)
	{
		FString LogMessage = FString::Printf(TEXT("UI Data Manager: %d assets processed, %d critical issues, Health Score: %d"), 
			CurrentAuditResults.Num(), GetCriticalIssuesCount(), GetHealthScore());
		MAGIC_LOG(Info, *LogMessage);
	}
}
