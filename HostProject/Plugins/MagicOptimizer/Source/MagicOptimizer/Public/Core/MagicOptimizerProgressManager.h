#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Model/ProgressUpdate.h"
#include "Model/AssetAuditData.h"
#include "MagicOptimizerProgressManager.generated.h"

// Forward declarations
class UMagicOptimizerAuditManager;

/**
 * Progress Manager for Magic Optimizer Plugin
 * Handles real-time progress tracking and updates during asset audits
 */
UCLASS(BlueprintType, Blueprintable)
class MAGICOPTIMIZER_API UMagicOptimizerProgressManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Progress Management
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	void StartAuditProgress(int32 TotalAssets);

	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	void UpdateProgress(int32 ProcessedAssets, int32 AssetsWithIssues, int32 SuccessfulAssets, int32 FailedAssets, const FString& CurrentAssetName = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	void CompleteAuditProgress();

	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	void CancelAuditProgress();

	// Data Management
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	void AddAssetResult(const FAssetAuditData& AssetData);

	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	TArray<FAssetAuditData> GetAuditResults() const;

	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	FProgressUpdate GetCurrentProgress() const;

	// Statistics
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	int32 GetTotalAssetsProcessed() const { return CurrentProgress.TotalAssets; }

	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	int32 GetAssetsWithIssues() const { return CurrentProgress.AssetsWithIssues; }

	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	int32 GetSuccessfulAssets() const { return CurrentProgress.AssetsSuccessful; }

	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	int32 GetFailedAssets() const { return CurrentProgress.AssetsFailed; }

	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	float GetProgressPercentage() const { return CurrentProgress.ProgressPercentage; }

	// Status
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	bool IsAuditInProgress() const { return bAuditInProgress; }

	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Progress")
	FString GetCurrentStatus() const { return CurrentProgress.StatusMessage; }

	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProgressUpdated, const FProgressUpdate&, ProgressUpdate);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssetProcessed, const FAssetAuditData&, AssetData);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAuditCompleted);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuditCancelled, const FString&, Reason);

	UPROPERTY(BlueprintAssignable, Category = "Magic Optimizer|Progress")
	FOnProgressUpdated OnProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Magic Optimizer|Progress")
	FOnAssetProcessed OnAssetProcessed;

	UPROPERTY(BlueprintAssignable, Category = "Magic Optimizer|Progress")
	FOnAuditCompleted OnAuditCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Magic Optimizer|Progress")
	FOnAuditCancelled OnAuditCancelled;

private:
	// Internal state
	FProgressUpdate CurrentProgress;
	TArray<FAssetAuditData> AuditResults;
	bool bAuditInProgress;
	FDateTime AuditStartTime;

	// Helper functions
	void BroadcastProgressUpdate();
	void CalculateEstimatedEndTime();
	FString GetElapsedTimeString() const;
};
