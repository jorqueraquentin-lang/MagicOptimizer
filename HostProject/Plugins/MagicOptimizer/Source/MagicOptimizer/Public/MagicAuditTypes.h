#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "MagicAuditTypes.generated.h"

/**
 * UHT-Safe Audit Result for UI integration
 * This struct is designed to work with DECLARE_DYNAMIC_MULTICAST_DELEGATE
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FMagicAuditResult
{
    GENERATED_BODY()

    /** Asset data for this result */
    UPROPERTY(BlueprintReadOnly, Category = "Asset")
    FAssetData AssetData;

    /** Additional context data - used by UI for display */
    UPROPERTY(BlueprintReadOnly, Category = "Context")
    TMap<FString, FString> Context;

    /** Asset path for quick access */
    UPROPERTY(BlueprintReadOnly, Category = "Asset")
    FString AssetPath;

    /** Asset class for filtering */
    UPROPERTY(BlueprintReadOnly, Category = "Asset")
    FString AssetClass;

    /** Number of issues found */
    UPROPERTY(BlueprintReadOnly, Category = "Issues")
    int32 IssueCount;

    /** Issues array for compatibility */
    UPROPERTY(BlueprintReadOnly, Category = "Issues")
    TArray<FString> Issues;

    /** Audit status */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    FString Status;

    /** Error message if any */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    FString ErrorMessage;

    FMagicAuditResult()
        : AssetPath(TEXT(""))
        , AssetClass(TEXT(""))
        , IssueCount(0)
        , Status(TEXT("Pending"))
        , ErrorMessage(TEXT(""))
    {}

    // Helper to populate Context with texture properties
    void SetTextureProperties(const FString& Group, const FString& Size, const FString& Compression, bool bSRGB, bool bMips)
    {
        Context.Add(TEXT("TextureGroup"), Group);
        Context.Add(TEXT("Resolution"), Size);
        Context.Add(TEXT("CompressionSettings"), Compression);
        Context.Add(TEXT("sRGB"), bSRGB ? TEXT("true") : TEXT("false"));
        Context.Add(TEXT("MipGenSettings"), bMips ? TEXT("FromTextureGroup") : TEXT("NoMipmaps"));
    }

    // Static conversion from legacy FAuditResult (if needed)
    static FMagicAuditResult FromLegacyResult(const struct FAuditResult& LegacyResult);
};

/**
 * UI Configuration for audit operations
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FMagicAuditConfig
{
    GENERATED_BODY()

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

    FMagicAuditConfig()
        : bIncludeTextures(true)
        , bIncludeMeshes(true)
        , bIncludeMaterials(true)
        , bIncludeAnimations(false)
        , bIncludeAudio(false)
        , MaxAssetsToProcess(1000)
    {}
};

// Forward declaration for conversion function
struct FAuditResult;

// Utility function to convert arrays
MAGICOPTIMIZER_API TArray<FMagicAuditResult> ConvertLegacyAuditResults(const TArray<FAuditResult>& LegacyResults);
