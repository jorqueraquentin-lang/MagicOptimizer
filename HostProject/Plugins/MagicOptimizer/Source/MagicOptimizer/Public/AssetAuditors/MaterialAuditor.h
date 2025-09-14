#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "AssetAuditors/IAssetAuditor.h"
#include "Model/AuditConfig.h"
#include "Model/AuditResult.h"

/**
 * Material auditor implementation for analyzing material assets
 * Provides comprehensive material optimization analysis and recommendations
 */
class MAGICOPTIMIZER_API FMaterialAuditor : public IAssetAuditor
{
public:
    FMaterialAuditor();
    virtual ~FMaterialAuditor() = default;

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
     * Analyze material properties and generate recommendations
     * @param MaterialInterface Material to analyze
     * @param AssetData Asset data for the material
     * @return Audit result with analysis and recommendations
     */
    FAuditResult AnalyzeMaterial(const UMaterialInterface* MaterialInterface, const FAssetData& AssetData) const;

    /**
     * Analyze material complexity and generate complexity recommendations
     * @param MaterialInterface Material to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeMaterialComplexity(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const;

    /**
     * Analyze material texture usage and generate texture recommendations
     * @param MaterialInterface Material to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeMaterialTextures(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const;

    /**
     * Analyze material shader complexity and generate shader recommendations
     * @param MaterialInterface Material to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeMaterialShaders(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const;

    /**
     * Analyze material parameters and generate parameter recommendations
     * @param MaterialInterface Material to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeMaterialParameters(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const;

    /**
     * Analyze material performance impact and generate performance recommendations
     * @param MaterialInterface Material to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeMaterialPerformance(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const;

    /**
     * Generate platform-specific optimization recommendations
     * @param MaterialInterface Material to analyze
     * @param Result Audit result to populate
     */
    void GeneratePlatformRecommendations(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const;

    /**
     * Calculate estimated shader complexity score
     * @param MaterialInterface Material to analyze
     * @return Shader complexity score (0-100)
     */
    float CalculateShaderComplexity(const UMaterialInterface* MaterialInterface) const;

    /**
     * Check if material is optimal for platform
     * @param MaterialInterface Material to analyze
     * @param Platform Target platform string
     * @return True if material is optimal for platform
     */
    bool IsOptimalForPlatform(const UMaterialInterface* MaterialInterface, const FString& Platform) const;

    /**
     * Get recommended shader model for platform
     * @param MaterialInterface Material to analyze
     * @param Platform Target platform string
     * @return Recommended shader model
     */
    FString GetRecommendedShaderModel(const UMaterialInterface* MaterialInterface, const FString& Platform) const;

    /**
     * Update progress tracking
     * @param Processed Number of processed items
     * @param Total Total number of items
     */
    void UpdateProgress(int32 Processed, int32 Total) const;
};
