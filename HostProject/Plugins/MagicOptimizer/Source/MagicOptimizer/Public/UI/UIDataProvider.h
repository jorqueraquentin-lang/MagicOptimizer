#pragma once

#include "CoreMinimal.h"
#include "UI/UIDataTypes.h"

// Forward declarations to avoid UHT conflicts
struct FAuditResult;
struct FAuditIssue;

/**
 * Adapter that converts backend audit results to UI-facing data structures.
 * This is the only place that translates FAuditResult -> UI shapes.
 * Keeps the UI decoupled from backend implementation details.
 */
class MAGICOPTIMIZER_API FUIDataProvider
{
public:
    /**
     * Convert audit results to UI data structures
     */
    static void FromAuditResults(
        const TArray<FAuditResult>& InResults,
        TArray<FUITextureRow>& OutRows,
        TArray<FUIHealthPoint>& OutHealth,
        TArray<FUIBeforeAfter>& OutBeforeAfter,
        FUIBudgetGauges& OutGauges);

    /**
     * Generate mock data for demo purposes
     */
    static void MockIfEmpty(
        TArray<FUITextureRow>& OutRows,
        TArray<FUIHealthPoint>& OutHealth,
        TArray<FUIBeforeAfter>& OutBeforeAfter,
        FUIBudgetGauges& OutGauges);

private:
    /**
     * Convert a single audit result to a texture row
     */
    static FUITextureRow ConvertAuditResultToTexture(const FAuditResult& Result);

    /**
     * Convert size in pixels to size tag (512/1K/2K/4K)
     */
    static FString ToSizeTag(int32 Size);

    /**
     * Count referencers for an asset using UE 5.6 compatible API
     */
    static int32 GetReferencerCount(const FSoftObjectPath& ObjectPath);

    /**
     * Collapse audit issues to a simple count for UI display
     */
    static int32 CollapseIssues(const TArray<FAuditIssue>& Issues);
};