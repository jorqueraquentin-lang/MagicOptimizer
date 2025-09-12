#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "AssetRegistry/AssetData.h"
#include "AuditTypes/AuditConfig.h"
#include "AuditTypes/AuditResult.h"
#include "AssetAuditors/IAssetAuditor.h"
#include "MagicOptimizerAuditManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuditProgress, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuditCompleted, const TArray<FAuditResult>&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuditFailed, const FString&, ErrorMessage);

/**
 * Central manager for asset auditing operations
 * Coordinates all audit activities and manages the audit workflow
 */
UCLASS(BlueprintType, Blueprintable)
class MAGICOPTIMIZER_API UMagicOptimizerAuditManager : public UObject
{
    GENERATED_BODY()

public:
    UMagicOptimizerAuditManager();

    /**
     * Initialize the audit manager
     */
    UFUNCTION(BlueprintCallable, Category = "MagicOptimizer Audit")
    static void Initialize();

    /**
     * Shutdown the audit manager
     */
    UFUNCTION(BlueprintCallable, Category = "MagicOptimizer Audit")
    static void Shutdown();

    /**
     * Start asset audit with given configuration
     * @param Config Audit configuration
     * @return True if audit started successfully
     */
    UFUNCTION(BlueprintCallable, Category = "MagicOptimizer Audit")
    static bool StartAssetAudit(const FAuditConfig& Config);

    /**
     * Stop current asset audit
     */
    UFUNCTION(BlueprintCallable, Category = "MagicOptimizer Audit")
    static void StopAssetAudit();

    /**
     * Check if audit is currently running
     * @return True if audit is running
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MagicOptimizer Audit")
    static bool IsAuditRunning();

    /**
     * Get current audit progress (0.0 = not started, 1.0 = completed)
     * @return Current audit progress
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MagicOptimizer Audit")
    static float GetAuditProgress();

    /**
     * Get current audit status
     * @return Current audit status
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MagicOptimizer Audit")
    static FString GetCurrentAuditStatus();

    /**
     * Get audit results
     * @return Array of audit results
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MagicOptimizer Audit")
    static TArray<FAuditResult> GetAuditResults();

    /**
     * Get high priority recommendations from all results
     * @return Array of high priority recommendations
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MagicOptimizer Audit")
    static TArray<FOptimizationRecommendation> GetHighPriorityRecommendations();

    /**
     * Get critical issues from all results
     * @return Array of critical issues
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MagicOptimizer Audit")
    static TArray<FAuditIssue> GetCriticalIssues();

    /**
     * Generate comprehensive audit report
     * @return Audit report as string
     */
    UFUNCTION(BlueprintCallable, Category = "MagicOptimizer Audit")
    static FString GenerateAuditReport();

    /**
     * Export audit results to file
     * @param FilePath File path to export to
     * @return True if export was successful
     */
    UFUNCTION(BlueprintCallable, Category = "MagicOptimizer Audit")
    static bool ExportAuditResults(const FString& FilePath);

    /**
     * Import audit results from file
     * @param FilePath File path to import from
     * @return True if import was successful
     */
    UFUNCTION(BlueprintCallable, Category = "MagicOptimizer Audit")
    static bool ImportAuditResults(const FString& FilePath);

    /**
     * Clear all audit results
     */
    UFUNCTION(BlueprintCallable, Category = "MagicOptimizer Audit")
    static void ClearAuditResults();

    /**
     * Get total estimated savings from all recommendations
     * @return Total estimated savings in MB
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MagicOptimizer Audit")
    static float GetTotalEstimatedSavings();

    /**
     * Get audit statistics
     * @return Audit statistics as string
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MagicOptimizer Audit")
    static FString GetAuditStatistics();

    // Event delegates (static, not UPROPERTY)
    static FOnAuditProgress OnAuditProgress;
    static FOnAuditCompleted OnAuditCompleted;
    static FOnAuditFailed OnAuditFailed;

private:
    static UMagicOptimizerAuditManager* Instance;
    static TArray<TSharedPtr<IAssetAuditor>> AssetAuditors;
    static FAuditConfig CurrentConfig;
    static TArray<FAuditResult> AuditResults;
    static bool bIsAuditRunning;
    static float CurrentProgress;
    static FString CurrentStatus;

    /**
     * Register all available asset auditors
     */
    static void RegisterAssetAuditors();

    /**
     * Discover assets to audit based on configuration
     * @return Array of assets to audit
     */
    static TArray<FAssetData> DiscoverAssetsToAudit();

    /**
     * Process a single asset for audit
     * @param AssetData Asset data to process
     * @return Audit result
     */
    static FAuditResult ProcessAssetForAudit(const FAssetData& AssetData);

    /**
     * Handle audit error
     * @param ErrorMessage Error message
     */
    static void HandleAuditError(const FString& ErrorMessage);

    /**
     * Update audit progress
     * @param Progress Current progress (0.0 to 1.0)
     */
    static void UpdateAuditProgress(float Progress);

    /**
     * Complete audit process
     */
    static void CompleteAudit();
};
