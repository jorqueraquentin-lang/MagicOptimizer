#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "AssetAuditors/IAssetAuditor.h"
#include "Model/AuditConfig.h"
#include "Model/AuditResult.h"

/**
 * Blueprint auditor implementation for analyzing blueprint assets
 * Provides comprehensive blueprint optimization analysis and recommendations
 */
class MAGICOPTIMIZER_API FBlueprintAuditor : public IAssetAuditor
{
public:
    FBlueprintAuditor();
    virtual ~FBlueprintAuditor() = default;

    // IAssetAuditor interface implementation
    virtual bool CanAuditAsset(const FAssetData& AssetData) const override;
    virtual FAuditResult AuditAsset(const FAssetData& AssetData) const override;
    virtual TFuture<FAuditResult> AuditAssetAsync(const FAssetData& AssetData) const override;
    virtual void SetAuditConfig(const FAuditConfig& Config) override;
    virtual FAuditConfig GetAuditConfig() const override;
    virtual float GetAuditProgress() const override;
    virtual FString GetAuditStatus() const override;
    virtual bool HandleError(const FString& ErrorMessage) override;
    virtual void Reset() override;
    virtual FString GetAuditorName() const override;
    virtual TArray<EAssetType> GetSupportedAssetTypes() const override;

private:
    FAuditConfig CurrentConfig;
    mutable float CurrentProgress;
    mutable FString CurrentStatus;
    mutable int32 ProcessedCount;
    mutable int32 TotalCount;

    /**
     * Analyze blueprint properties and generate recommendations
     * @param Blueprint Blueprint to analyze
     * @param AssetData Asset data for the blueprint
     * @return Audit result with analysis and recommendations
     */
    FAuditResult AnalyzeBlueprint(const UBlueprint* Blueprint, const FAssetData& AssetData) const;

    /**
     * Analyze blueprint complexity and generate complexity recommendations
     * @param Blueprint Blueprint to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeBlueprintComplexity(const UObject* Blueprint, FAuditResult& Result) const;

    /**
     * Analyze blueprint functions and generate function recommendations
     * @param Blueprint Blueprint to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeBlueprintFunctions(const UObject* Blueprint, FAuditResult& Result) const;

    /**
     * Analyze blueprint variables and generate variable recommendations
     * @param Blueprint Blueprint to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeBlueprintVariables(const UObject* Blueprint, FAuditResult& Result) const;

    /**
     * Analyze blueprint performance and generate performance recommendations
     * @param Blueprint Blueprint to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeBlueprintPerformance(const UObject* Blueprint, FAuditResult& Result) const;

    /**
     * Analyze blueprint inheritance and generate inheritance recommendations
     * @param Blueprint Blueprint to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeBlueprintInheritance(const UObject* Blueprint, FAuditResult& Result) const;

    /**
     * Generate platform-specific optimization recommendations
     * @param Blueprint Blueprint to analyze
     * @param Result Audit result to populate
     */
    void GeneratePlatformRecommendations(const UObject* Blueprint, FAuditResult& Result) const;

    /**
     * Calculate estimated blueprint complexity score
     * @param Blueprint Blueprint to analyze
     * @return Blueprint complexity score (0-100)
     */
    float CalculateBlueprintComplexity(const UObject* Blueprint) const;

    /**
     * Check if blueprint is optimal for platform
     * @param Blueprint Blueprint to analyze
     * @param Platform Target platform string
     * @return True if blueprint is optimal for platform
     */
    bool IsOptimalForPlatform(const UObject* Blueprint, const FString& Platform) const;

    /**
     * Get recommended blueprint optimization level for platform
     * @param Blueprint Blueprint to analyze
     * @param Platform Target platform string
     * @return Recommended optimization level
     */
    FString GetRecommendedOptimizationLevel(const UObject* Blueprint, const FString& Platform) const;

    /**
     * Update progress tracking
     * @param Processed Number of processed items
     * @param Total Total number of items
     */
    void UpdateProgress(int32 Processed, int32 Total) const;
};
