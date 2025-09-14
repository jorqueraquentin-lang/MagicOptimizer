#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UIDataTypes.generated.h"

/**
 * UI-facing data contracts that mirror the React mock exactly.
 * These are stable interfaces that the Slate UI expects.
 * Backend can evolve freely without breaking UI.
 */

USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FUITextureRow
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Id;

    UPROPERTY(BlueprintReadOnly)
    FString Path;

    UPROPERTY(BlueprintReadOnly)
    FString Size;           // "512", "1K", "2K", "4K"

    UPROPERTY(BlueprintReadOnly)
    FString Group;          // "World", "Effects", "Props", etc.

    UPROPERTY(BlueprintReadOnly)
    FString Compression;    // "Default", "Masks", "Normal", etc.

    UPROPERTY(BlueprintReadOnly)
    bool bSRGB = false;

    UPROPERTY(BlueprintReadOnly)
    bool bMips = true;

    UPROPERTY(BlueprintReadOnly)
    bool bVT = false;       // Virtual Texture

    UPROPERTY(BlueprintReadOnly)
    int32 LOD = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 Referencers = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 Issues = 0;       // 0=Info, 1=Should, 2+=Must

    FUITextureRow() = default;
};

USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FUIHealthPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 T;        // Time/timestamp

    UPROPERTY(BlueprintReadOnly)
    int32 Score;    // Health score

    FUIHealthPoint() = default;
    FUIHealthPoint(int32 InT, int32 InScore) : T(InT), Score(InScore) {}
};

struct FUIBeforeAfter
{
    FString Name;
    double Before = 0.0;
    double After = 0.0;
};

struct FUIBudgetGauges
{
    int32 PoolUsePct = 0;   // Texture pool usage %
    int32 VTPct = 0;        // Virtual Texture usage %
    int32 DiskPct = 0;      // Disk usage %
};

struct FUIRunInfo
{
    FString Id;
    FString Date;
    FString Preset;
    FString Notes;
    TArray<FString> Artifacts;  // "CSV", "JSON", "HTML"
};

struct FTopOffender
{
    FString Name;
    FString Group;
    int32 Count;
    double Impact;  // Estimated impact score
};

// Additional UI data structures
struct FTargetChip {
    FString Id;
    FString Label;
    FString Icon;
    
    FTargetChip() = default;
    FTargetChip(const FString& InId, const FString& InLabel, const FString& InIcon)
        : Id(InId), Label(InLabel), Icon(InIcon) {}
};

struct FPresetOption {
    FString Id;
    FString Label;
    FString Description;
    
    FPresetOption() = default;
    FPresetOption(const FString& InId, const FString& InLabel, const FString& InDescription)
        : Id(InId), Label(InLabel), Description(InDescription) {}
};

USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FHealthData {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 Score = 0;

    UPROPERTY(BlueprintReadOnly)
    FString Status = TEXT("Good");

    UPROPERTY(BlueprintReadOnly)
    TArray<FUIHealthPoint> History;
    
    FHealthData() = default;
};

USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FProgressData {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float Value = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FString Status = TEXT("Idle");

    UPROPERTY(BlueprintReadOnly)
    bool bIsRunning = false;
    
    FProgressData() = default;
};

USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FTaskData {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Id;

    UPROPERTY(BlueprintReadOnly)
    FString Label;

    UPROPERTY(BlueprintReadOnly)
    FString Status;

    UPROPERTY(BlueprintReadOnly)
    float Progress = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FDateTime StartTime;
    
    FTaskData() = default;
    FTaskData(const FString& InId, const FString& InLabel, const FString& InStatus, float InProgress)
        : Id(InId), Label(InLabel), Status(InStatus), Progress(InProgress), StartTime(FDateTime::Now()) {}
    
    bool operator==(const FTaskData& Other) const {
        return Id == Other.Id && Label == Other.Label && Status == Other.Status &&
               FMath::IsNearlyEqual(Progress, Other.Progress) && StartTime == Other.StartTime;
    }
};

struct FTextureSettings {
    FString Compression;
    bool bSRGB = false;
    bool bMips = true;
    bool bVT = false;
    int32 LOD = 0;
    
    FTextureSettings() = default;
};

// Forward declarations for types defined in other files
struct FPlaybookRecommendation;
struct FProposedChange;
struct FOptimizationSummary;