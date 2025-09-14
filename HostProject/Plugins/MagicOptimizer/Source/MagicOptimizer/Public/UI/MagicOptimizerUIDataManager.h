#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Model/ProgressUpdate.h"
#include "Model/AssetAuditData.h"
#include "MagicOptimizerUIDataManager.generated.h"

// Forward declarations
class UMagicOptimizerProgressManager;

/**
 * UI Data Manager for Magic Optimizer Plugin
 * Manages real-time data binding between the audit system and UI components
 */
UCLASS(BlueprintType, Blueprintable)
class MAGICOPTIMIZER_API UMagicOptimizerUIDataManager : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	UMagicOptimizerUIDataManager();

	// Initialize the data manager
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	void Initialize();

	// Shutdown the data manager
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	void Shutdown();

	// Get current progress data
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	FProgressUpdate GetCurrentProgress() const;

	// Get current audit results
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	TArray<FAssetAuditData> GetCurrentAuditResults() const;

	// Get health score (0-100)
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	int32 GetHealthScore() const;

	// Get critical issues count
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	int32 GetCriticalIssuesCount() const;

	// Get total assets scanned
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	int32 GetTotalAssetsScanned() const;

	// Get progress percentage (0-100)
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	int32 GetProgressPercentage() const;

	// Get current status message
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	FString GetCurrentStatusMessage() const;

	// Get current asset being processed
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	FString GetCurrentAssetName() const;

	// Get estimated time remaining
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	FString GetEstimatedTimeRemaining() const;

	// Get total memory usage
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	float GetTotalMemoryUsage() const;

	// Get estimated memory savings
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	float GetEstimatedMemorySavings() const;

	// Check if audit is currently running
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	bool IsAuditRunning() const;

	// Events for UI updates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProgressUpdated, const FProgressUpdate&, ProgressUpdate);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssetProcessed, const FAssetAuditData&, AssetData);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAuditCompleted);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAuditStarted);

	UPROPERTY(BlueprintAssignable, Category = "Magic Optimizer|UI")
	FOnProgressUpdated OnProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Magic Optimizer|UI")
	FOnAssetProcessed OnAssetProcessed;

	UPROPERTY(BlueprintAssignable, Category = "Magic Optimizer|UI")
	FOnAuditCompleted OnAuditCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Magic Optimizer|UI")
	FOnAuditStarted OnAuditStarted;

private:
	// Internal state
	UPROPERTY()
	UMagicOptimizerProgressManager* ProgressManager;

	FProgressUpdate CurrentProgress;
	TArray<FAssetAuditData> CurrentAuditResults;
	bool bIsInitialized;

	// Helper functions
	void BindToProgressManager();
	void UnbindFromProgressManager();
	void OnProgressManagerUpdated(const FProgressUpdate& ProgressUpdate);
	void OnProgressManagerAssetProcessed(const FAssetAuditData& AssetData);
	void OnProgressManagerAuditCompleted();
	void OnProgressManagerAuditCancelled(const FString& Reason);
	void CalculateDerivedMetrics();
};
