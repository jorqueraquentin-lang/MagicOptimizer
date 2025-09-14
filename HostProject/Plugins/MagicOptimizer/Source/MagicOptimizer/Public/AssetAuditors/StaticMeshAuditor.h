#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/StaticMesh.h"
#include "AssetAuditors/IAssetAuditor.h"
#include "Model/AuditConfig.h"
#include "Model/AuditResult.h"

/**
 * Static mesh auditor implementation for analyzing static mesh assets
 * Provides comprehensive mesh optimization analysis and recommendations
 */
class MAGICOPTIMIZER_API FStaticMeshAuditor : public IAssetAuditor
{
public:
    FStaticMeshAuditor();
    virtual ~FStaticMeshAuditor() = default;

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
     * Analyze mesh polygon count and generate LOD recommendations
     * @param StaticMesh Static mesh to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeMeshComplexity(const UStaticMesh* StaticMesh, FAuditResult& Result) const;

    /**
     * Analyze mesh LODs and generate LOD optimization recommendations
     * @param StaticMesh Static mesh to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeMeshLODs(const UStaticMesh* StaticMesh, FAuditResult& Result) const;

    /**
     * Analyze mesh collision complexity and generate collision recommendations
     * @param StaticMesh Static mesh to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeMeshCollision(const UStaticMesh* StaticMesh, FAuditResult& Result) const;

    /**
     * Analyze mesh material usage and generate material recommendations
     * @param StaticMesh Static mesh to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeMeshMaterials(const UStaticMesh* StaticMesh, FAuditResult& Result) const;

    /**
     * Analyze mesh UV mapping and generate UV optimization recommendations
     * @param StaticMesh Static mesh to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeMeshUVs(const UStaticMesh* StaticMesh, FAuditResult& Result) const;

    /**
     * Generate platform-specific optimization recommendations
     * @param StaticMesh Static mesh to analyze
     * @param Result Audit result to populate
     */
    void GeneratePlatformRecommendations(const UStaticMesh* StaticMesh, FAuditResult& Result) const;

    /**
     * Calculate estimated memory usage for mesh
     * @param StaticMesh Static mesh to analyze
     * @return Estimated memory usage in MB
     */
    float CalculateMeshMemoryUsage(const UStaticMesh* StaticMesh) const;

    /**
     * Check if polygon count is optimal for platform
     * @param PolygonCount Number of polygons
     * @param Platform Target platform string
     * @return True if polygon count is optimal
     */
    bool IsOptimalPolygonCount(int32 PolygonCount, const FString& Platform) const;

    /**
     * Get recommended LOD count for platform
     * @param StaticMesh Static mesh to analyze
     * @param Platform Target platform string
     * @return Recommended LOD count
     */
    int32 GetRecommendedLODCount(const UStaticMesh* StaticMesh, const FString& Platform) const;

    /**
     * Update progress tracking
     * @param Processed Number of processed items
     * @param Total Total number of items
     */
    void UpdateProgress(int32 Processed, int32 Total) const;
};
