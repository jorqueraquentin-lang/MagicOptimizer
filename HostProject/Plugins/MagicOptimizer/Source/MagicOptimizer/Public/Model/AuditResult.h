#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "AssetRegistry/AssetData.h"
#include "AuditResult.generated.h"

/**
 * Backend audit result structure - can evolve freely without breaking UI.
 * The UIDataProvider adapter converts this to UI-facing types.
 */

UENUM()
enum class EAuditIssueLevel : uint8
{
    Info    UMETA(DisplayName = "Info"),
    Should  UMETA(DisplayName = "Should Fix"),
    Must    UMETA(DisplayName = "Must Fix")
};

UENUM()
enum class EAuditIssueType : uint8
{
    Info    UMETA(DisplayName = "Info"),
    Warning UMETA(DisplayName = "Warning"),
    Error   UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FAuditIssue
{
    GENERATED_BODY()

    UPROPERTY()
    EAuditIssueLevel Level = EAuditIssueLevel::Info;

    UPROPERTY()
    FString Code;        // "SRGB_MASKS_ON", "NORMAL_NOT_BC5", etc.

    UPROPERTY()
    FString Message;     // Human-readable description

    FAuditIssue() = default;
    FAuditIssue(EAuditIssueLevel InLevel, const FString& InCode, const FString& InMessage)
        : Level(InLevel), Code(InCode), Message(InMessage) {}
};

USTRUCT(BlueprintType)
struct FAuditResult
{
    GENERATED_BODY()

    UPROPERTY()
    FGuid Guid;

    UPROPERTY()
    FSoftObjectPath AssetPath;

    UPROPERTY()
    FName AssetClass;        // e.g., "Texture2D"

    UPROPERTY()
    int32 MaxSize = 0;       // Inferred size (512/1k/2k/4k)

    UPROPERTY()
    int32 LODBias = 0;

    UPROPERTY()
    TSet<FString> Flags;     // "sRGBOn", "NoMips", "VT"

    UPROPERTY()
    TMap<FString, FString> Context;  // "TextureGroup", "Compression", etc.

    UPROPERTY()
    TArray<FString> Referencers;

    UPROPERTY()
    TArray<FAuditIssue> Issues;

    UPROPERTY()
    int32 NumIssues = 0;

    // Helper methods for easy access
    FString GetAssetName() const
    {
        return AssetPath.GetAssetName();
    }

    FString GetAssetPath() const
    {
        return AssetPath.ToString();
    }

    FString GetMetadata(const FString& Key, const FString& DefaultValue = TEXT("")) const
    {
        return Context.FindRef(Key, DefaultValue);
    }

    void AddIssue(EAuditIssueLevel Level, const FString& Code, const FString& Message)
    {
        Issues.Add(FAuditIssue(Level, Code, Message));
        NumIssues = Issues.Num();
    }

    FAuditResult() = default;
};
