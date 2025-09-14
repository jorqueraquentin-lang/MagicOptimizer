#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "OptimizationRecommendation.generated.h"

/**
 * Optimization recommendation structure for the Plan dialog.
 * Represents a batch of changes that can be applied to assets.
 */

USTRUCT(BlueprintType)
struct FOptimizationRecommendation
{
    GENERATED_BODY()

    UPROPERTY()
    FString Id;         // "srgb_off", "normals_bc5", "disable_vt"

    UPROPERTY()
    FString Label;      // "Set sRGB off for N mask textures"

    UPROPERTY()
    FString Description; // Detailed description of the change

    UPROPERTY()
    TArray<FSoftObjectPath> Items;  // Assets to modify

    UPROPERTY()
    int32 EstimatedSavings = 0;     // Estimated memory/disk savings in MB

    UPROPERTY()
    bool bIsSafe = true;            // Can this be safely applied?

    UPROPERTY()
    FString Category;               // "Compression", "Flags", "Size", etc.

    FOptimizationRecommendation() = default;
    FOptimizationRecommendation(const FString& InId, const FString& InLabel, const FString& InDescription)
        : Id(InId), Label(InLabel), Description(InDescription) {}
};
