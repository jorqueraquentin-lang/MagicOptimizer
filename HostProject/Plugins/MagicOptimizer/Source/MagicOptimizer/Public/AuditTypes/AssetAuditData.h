#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "AuditResult.h"
#include "OptimizationRecommendation.h"
#include "AssetAuditData.generated.h"

class UTexture2D;
class UStaticMesh;
class UMaterial;
class UBlueprint;

/**
 * Comprehensive asset audit data structure containing all information
 * about an asset's analysis, performance metrics, and optimization recommendations.
 * 
 * This structure serves as the primary data container for the Magic Optimizer
 * asset audit system, providing a unified interface for asset information
 * across all supported asset types.
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FAssetAuditData
{
    GENERATED_BODY()

    // ============================================================================
    // BASIC ASSET INFORMATION
    // ============================================================================

    /** The full path to the asset in the project */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Info", meta = (DisplayName = "Asset Path"))
    FString AssetPath;

    /** The name of the asset (without path) */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Info", meta = (DisplayName = "Asset Name"))
    FString AssetName;

    /** The type of asset (Texture, StaticMesh, Material, Blueprint, etc.) */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Info", meta = (DisplayName = "Asset Type"))
    FString AssetType;

    /** The size of the asset in bytes */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Info", meta = (DisplayName = "Size (Bytes)"))
    int64 AssetSizeBytes;

    /** The size of the asset in a human-readable format (e.g., "2.5 MB") */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Info", meta = (DisplayName = "Size (Formatted)"))
    FString AssetSizeFormatted;

    // ============================================================================
    // AUDIT RESULTS
    // ============================================================================

    /** List of issues found during the audit */
    UPROPERTY(BlueprintReadOnly, Category = "Audit Results", meta = (DisplayName = "Issues"))
    TArray<FAuditIssue> Issues;

    /** List of optimization recommendations */
    UPROPERTY(BlueprintReadOnly, Category = "Audit Results", meta = (DisplayName = "Recommendations"))
    TArray<FOptimizationRecommendation> Recommendations;

    /** Overall audit status */
    UPROPERTY(BlueprintReadOnly, Category = "Audit Results", meta = (DisplayName = "Audit Status"))
    EAuditStatus AuditStatus;

    /** Number of critical issues found */
    UPROPERTY(BlueprintReadOnly, Category = "Audit Results", meta = (DisplayName = "Critical Issues"))
    int32 CriticalIssuesCount;

    /** Number of warning issues found */
    UPROPERTY(BlueprintReadOnly, Category = "Audit Results", meta = (DisplayName = "Warning Issues"))
    int32 WarningIssuesCount;

    /** Number of info issues found */
    UPROPERTY(BlueprintReadOnly, Category = "Audit Results", meta = (DisplayName = "Info Issues"))
    int32 InfoIssuesCount;

    // ============================================================================
    // PERFORMANCE METRICS
    // ============================================================================

    /** Memory usage in megabytes */
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Memory Usage (MB)"))
    float MemoryUsageMB;

    /** Disk usage in megabytes */
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Disk Usage (MB)"))
    float DiskUsageMB;

    /** Quality score from 0-100 */
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Quality Score"))
    int32 QualityScore;

    /** Performance impact score from 0-100 (higher = more impact) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Performance Impact"))
    int32 PerformanceImpactScore;

    /** Estimated optimization potential (0-100) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Optimization Potential"))
    int32 OptimizationPotential;

    // ============================================================================
    // ASSET-SPECIFIC DATA
    // ============================================================================

    /** Texture-specific data (only valid for texture assets) */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Specific", meta = (DisplayName = "Texture Data"))
    FString TextureData;

    /** StaticMesh-specific data (only valid for mesh assets) */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Specific", meta = (DisplayName = "Mesh Data"))
    FString MeshData;

    /** Material-specific data (only valid for material assets) */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Specific", meta = (DisplayName = "Material Data"))
    FString MaterialData;

    /** Blueprint-specific data (only valid for blueprint assets) */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Specific", meta = (DisplayName = "Blueprint Data"))
    FString BlueprintData;

    // ============================================================================
    // METADATA
    // ============================================================================

    /** When the asset was last modified */
    UPROPERTY(BlueprintReadOnly, Category = "Metadata", meta = (DisplayName = "Last Modified"))
    FDateTime LastModified;

    /** When the audit was performed */
    UPROPERTY(BlueprintReadOnly, Category = "Metadata", meta = (DisplayName = "Audit Timestamp"))
    FDateTime AuditTimestamp;

    /** The platform this audit was performed for */
    UPROPERTY(BlueprintReadOnly, Category = "Metadata", meta = (DisplayName = "Target Platform"))
    FString TargetPlatform;

    /** The quality level used for this audit */
    UPROPERTY(BlueprintReadOnly, Category = "Metadata", meta = (DisplayName = "Quality Level"))
    EQualityLevel QualityLevel;

    /** Whether this was a dry run (no actual changes made) */
    UPROPERTY(BlueprintReadOnly, Category = "Metadata", meta = (DisplayName = "Dry Run"))
    bool bDryRun;

    // ============================================================================
    // CONSTRUCTOR
    // ============================================================================

    /** Default constructor */
    FAssetAuditData()
        : AssetSizeBytes(0)
        , AssetSizeFormatted(TEXT("0 B"))
        , AuditStatus(EAuditStatus::Pending)
        , CriticalIssuesCount(0)
        , WarningIssuesCount(0)
        , InfoIssuesCount(0)
        , MemoryUsageMB(0.0f)
        , DiskUsageMB(0.0f)
        , QualityScore(0)
        , PerformanceImpactScore(0)
        , OptimizationPotential(0)
        , TextureData(TEXT(""))
        , MeshData(TEXT(""))
        , MaterialData(TEXT(""))
        , BlueprintData(TEXT(""))
        , LastModified(FDateTime::Now())
        , AuditTimestamp(FDateTime::Now())
        , TargetPlatform(TEXT("WindowsEditor"))
        , QualityLevel(EQualityLevel::High)
        , bDryRun(true)
    {
        AssetPath = TEXT("");
        AssetName = TEXT("");
        AssetType = TEXT("Unknown");
    }

    // ============================================================================
    // UTILITY FUNCTIONS
    // ============================================================================

    /** Get the total number of issues */
    int32 GetTotalIssuesCount() const
    {
        return CriticalIssuesCount + WarningIssuesCount + InfoIssuesCount;
    }

    /** Get the formatted asset size */
    FString GetFormattedAssetSize() const
    {
        return AssetSizeFormatted;
    }

    /** Check if the asset has any critical issues */
    bool HasCriticalIssues() const
    {
        return CriticalIssuesCount > 0;
    }

    /** Check if the asset has any issues at all */
    bool HasIssues() const
    {
        return GetTotalIssuesCount() > 0;
    }

    /** Get the overall health score (0-100) */
    int32 GetHealthScore() const
    {
        if (QualityScore == 0)
        {
            return 0;
        }
        
        // Calculate health score based on quality and issue count
        float HealthScore = QualityScore;
        
        // Penalize for issues
        HealthScore -= (CriticalIssuesCount * 10.0f);  // -10 points per critical issue
        HealthScore -= (WarningIssuesCount * 5.0f);    // -5 points per warning
        HealthScore -= (InfoIssuesCount * 1.0f);       // -1 point per info issue
        
        // Ensure score is between 0 and 100
        return FMath::Clamp(FMath::RoundToInt(HealthScore), 0, 100);
    }

    /** Get a summary string for display */
    FString GetSummaryString() const
    {
        return FString::Printf(TEXT("%s | %s | %s | %d issues | Score: %d"),
            *AssetName,
            *AssetType,
            *AssetSizeFormatted,
            GetTotalIssuesCount(),
            GetHealthScore()
        );
    }
};
