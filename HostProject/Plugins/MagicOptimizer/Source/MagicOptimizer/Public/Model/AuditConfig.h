#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "AuditConfig.generated.h"

/**
 * Quality levels for asset optimization
 */
UENUM(BlueprintType)
enum class EQualityLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra"),
    Custom      UMETA(DisplayName = "Custom")
};

/**
 * Audit status for tracking progress
 */
UENUM(BlueprintType)
enum class EAuditStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    Running     UMETA(DisplayName = "Running"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Cancelled   UMETA(DisplayName = "Cancelled")
};

/**
 * Asset types that can be audited
 */
UENUM(BlueprintType)
enum class EAssetType : uint8
{
    Texture     UMETA(DisplayName = "Texture"),
    Mesh        UMETA(DisplayName = "Mesh"),
    Material    UMETA(DisplayName = "Material"),
    Blueprint   UMETA(DisplayName = "Blueprint"),
    Animation   UMETA(DisplayName = "Animation"),
    Audio       UMETA(DisplayName = "Audio"),
    Level       UMETA(DisplayName = "Level"),
    All         UMETA(DisplayName = "All")
};

/**
 * Configuration structure for asset auditing
 * Contains all settings needed to perform comprehensive asset analysis
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FAuditConfig
{
    GENERATED_BODY()

    FAuditConfig()
    {
        // Default values
        TargetPlatform = TEXT("WindowsEditor");
        QualityLevel = EQualityLevel::High;
        MaxMemoryUsageMB = 1000.0f;
        MaxDrawCalls = 1000;
        MaxTextureResolution = 2048;
        MaxPolygonCount = 10000;
        bEnableAggressiveOptimization = false;
        bCreateBackups = true;
        bDryRunMode = true;
        bGenerateReports = true;
        bIncludeTimestamps = true;
        MaxConcurrentAudits = 4;
        TimeoutSeconds = 300.0f;
        
        // Default asset types to include
        IncludeAssetTypes = {EAssetType::Texture, EAssetType::Mesh, EAssetType::Material, EAssetType::Blueprint};
        ExcludeAssetTypes = {};
        
        // Default paths to include/exclude
        IncludePaths = {TEXT("/Game/")};
        ExcludePaths = {TEXT("/Game/Developers/"), TEXT("/Game/Test/")};
    }

    /** Target platform for optimization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform", meta = (DisplayName = "Target Platform"))
    FString TargetPlatform;

    /** Quality level for optimization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality", meta = (DisplayName = "Quality Level"))
    EQualityLevel QualityLevel;

    /** Maximum memory usage in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (DisplayName = "Max Memory Usage (MB)", ClampMin = "1.0", ClampMax = "10000.0"))
    float MaxMemoryUsageMB;

    /** Maximum draw calls allowed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (DisplayName = "Max Draw Calls", ClampMin = "1", ClampMax = "10000"))
    int32 MaxDrawCalls;

    /** Maximum texture resolution */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (DisplayName = "Max Texture Resolution", ClampMin = "64", ClampMax = "8192"))
    int32 MaxTextureResolution;

    /** Maximum polygon count per mesh */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (DisplayName = "Max Polygon Count", ClampMin = "100", ClampMax = "100000"))
    int32 MaxPolygonCount;

    /** Enable aggressive optimization (may reduce quality) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (DisplayName = "Enable Aggressive Optimization"))
    bool bEnableAggressiveOptimization;

    /** Create backups before making changes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safety", meta = (DisplayName = "Create Backups"))
    bool bCreateBackups;

    /** Run in dry-run mode (no actual changes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safety", meta = (DisplayName = "Dry Run Mode"))
    bool bDryRunMode;

    /** Generate detailed reports */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reporting", meta = (DisplayName = "Generate Reports"))
    bool bGenerateReports;

    /** Include timestamps in reports */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reporting", meta = (DisplayName = "Include Timestamps"))
    bool bIncludeTimestamps;

    /** Maximum number of concurrent audits */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (DisplayName = "Max Concurrent Audits", ClampMin = "1", ClampMax = "16"))
    int32 MaxConcurrentAudits;

    /** Timeout for individual audits in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (DisplayName = "Timeout (Seconds)", ClampMin = "10.0", ClampMax = "3600.0"))
    float TimeoutSeconds;

    /** Asset types to include in audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filtering", meta = (DisplayName = "Include Asset Types"))
    TArray<EAssetType> IncludeAssetTypes;

    /** Asset types to exclude from audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filtering", meta = (DisplayName = "Exclude Asset Types"))
    TArray<EAssetType> ExcludeAssetTypes;

    /** Paths to include in audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filtering", meta = (DisplayName = "Include Paths"))
    TArray<FString> IncludePaths;

    /** Paths to exclude from audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filtering", meta = (DisplayName = "Exclude Paths"))
    TArray<FString> ExcludePaths;

    /** Custom settings for specific asset types */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced", meta = (DisplayName = "Custom Settings"))
    TMap<FString, FString> CustomSettings;

    /**
     * Validate the configuration
     * @return True if configuration is valid, false otherwise
     */
    bool IsValid() const
    {
        if (TargetPlatform.IsEmpty())
        {
            return false;
        }

        if (MaxMemoryUsageMB <= 0.0f)
        {
            return false;
        }

        if (MaxDrawCalls <= 0)
        {
            return false;
        }

        if (MaxConcurrentAudits <= 0)
        {
            return false;
        }

        if (TimeoutSeconds <= 0.0f)
        {
            return false;
        }

        return true;
    }

    /**
     * Get display name for quality level
     * @param Level Quality level
     * @return Display name string
     */
    static FString GetQualityLevelDisplayName(EQualityLevel Level)
    {
        switch (Level)
        {
            case EQualityLevel::Low: return TEXT("Low");
            case EQualityLevel::Medium: return TEXT("Medium");
            case EQualityLevel::High: return TEXT("High");
            case EQualityLevel::Ultra: return TEXT("Ultra");
            case EQualityLevel::Custom: return TEXT("Custom");
            default: return TEXT("Unknown");
        }
    }

    /**
     * Get display name for asset type
     * @param AssetType Asset type
     * @return Display name string
     */
    static FString GetAssetTypeDisplayName(EAssetType AssetType)
    {
        switch (AssetType)
        {
            case EAssetType::Texture: return TEXT("Texture");
            case EAssetType::Mesh: return TEXT("Mesh");
            case EAssetType::Material: return TEXT("Material");
            case EAssetType::Blueprint: return TEXT("Blueprint");
            case EAssetType::Animation: return TEXT("Animation");
            case EAssetType::Audio: return TEXT("Audio");
            case EAssetType::Level: return TEXT("Level");
            case EAssetType::All: return TEXT("All");
            default: return TEXT("Unknown");
        }
    }

    /**
     * Check if an asset type should be included
     * @param AssetType Asset type to check
     * @return True if asset type should be included
     */
    bool ShouldIncludeAssetType(EAssetType AssetType) const
    {
        // Check if explicitly excluded
        if (ExcludeAssetTypes.Contains(AssetType))
        {
            return false;
        }

        // Check if explicitly included
        if (IncludeAssetTypes.Contains(AssetType))
        {
            return true;
        }

        // If no specific includes, include all except explicitly excluded
        return IncludeAssetTypes.Num() == 0;
    }

    /**
     * Check if a path should be included
     * @param AssetPath Asset path to check
     * @return True if path should be included
     */
    bool ShouldIncludePath(const FString& AssetPath) const
    {
        // Check if explicitly excluded
        for (const FString& ExcludePath : ExcludePaths)
        {
            if (AssetPath.StartsWith(ExcludePath))
            {
                return false;
            }
        }

        // Check if explicitly included
        for (const FString& IncludePath : IncludePaths)
        {
            if (AssetPath.StartsWith(IncludePath))
            {
                return true;
            }
        }

        // If no specific includes, include all except explicitly excluded
        return IncludePaths.Num() == 0;
    }
};

/**
 * UI-specific audit configuration
 * Bridge between UI expectations and the audit system
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FAuditUIConfig
{
    GENERATED_BODY()

    FAuditUIConfig()
        : bIncludeTextures(true)
        , bIncludeMeshes(true)
        , bIncludeMaterials(true)
        , bIncludeAnimations(false)
        , bIncludeAudio(false)
        , MaxAssetsToProcess(1000)
    {}

    /** Include textures in audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Types")
    bool bIncludeTextures;

    /** Include meshes in audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Types")
    bool bIncludeMeshes;

    /** Include materials in audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Types")
    bool bIncludeMaterials;

    /** Include animations in audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Types")
    bool bIncludeAnimations;

    /** Include audio assets in audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Types")
    bool bIncludeAudio;

    /** Maximum number of assets to process */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxAssetsToProcess;

    /** Asset path filters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scope")
    TArray<FString> AssetPathFilters;
};