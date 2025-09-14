#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "OptimizationRecommendation.generated.h"

/**
 * Enumeration for optimization recommendation priority levels
 */
UENUM(BlueprintType)
enum class EOptimizationPriority : uint8
{
    /** Low priority - minor optimization */
    Low UMETA(DisplayName = "Low"),
    
    /** Medium priority - moderate optimization */
    Medium UMETA(DisplayName = "Medium"),
    
    /** High priority - significant optimization */
    High UMETA(DisplayName = "High"),
    
    /** Critical priority - major optimization required */
    Critical UMETA(DisplayName = "Critical")
};

/**
 * Enumeration for optimization recommendation categories
 */
UENUM(BlueprintType)
enum class EOptimizationCategory : uint8
{
    /** Memory optimization */
    Memory UMETA(DisplayName = "Memory"),
    
    /** Performance optimization */
    Performance UMETA(DisplayName = "Performance"),
    
    /** Quality optimization */
    Quality UMETA(DisplayName = "Quality"),
    
    /** Size optimization */
    Size UMETA(DisplayName = "Size"),
    
    /** Compatibility optimization */
    Compatibility UMETA(DisplayName = "Compatibility"),
    
    /** Workflow optimization */
    Workflow UMETA(DisplayName = "Workflow")
};

/**
 * Comprehensive optimization recommendation structure for asset improvements.
 * 
 * This structure provides detailed recommendations for optimizing assets,
 * including priority levels, estimated benefits, and implementation details.
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FOptimizationRecommendation
{
    GENERATED_BODY()

    // ============================================================================
    // BASIC INFORMATION
    // ============================================================================

    /** Unique identifier for this recommendation */
    UPROPERTY(BlueprintReadOnly, Category = "Basic Info", meta = (DisplayName = "ID"))
    FString RecommendationID;

    /** Title of the recommendation */
    UPROPERTY(BlueprintReadOnly, Category = "Basic Info", meta = (DisplayName = "Title"))
    FString Title;

    /** Detailed description of the recommendation */
    UPROPERTY(BlueprintReadOnly, Category = "Basic Info", meta = (DisplayName = "Description"))
    FString Description;

    /** Category of the optimization */
    UPROPERTY(BlueprintReadOnly, Category = "Basic Info", meta = (DisplayName = "Category"))
    EOptimizationCategory Category;

    /** Priority level of the recommendation */
    UPROPERTY(BlueprintReadOnly, Category = "Basic Info", meta = (DisplayName = "Priority"))
    EOptimizationPriority Priority;

    // ============================================================================
    // IMPACT ESTIMATES
    // ============================================================================

    /** Estimated memory savings in megabytes */
    UPROPERTY(BlueprintReadOnly, Category = "Impact Estimates", meta = (DisplayName = "Memory Savings (MB)"))
    float EstimatedMemorySavingsMB;

    /** Estimated disk space savings in megabytes */
    UPROPERTY(BlueprintReadOnly, Category = "Impact Estimates", meta = (DisplayName = "Disk Savings (MB)"))
    float EstimatedDiskSavingsMB;

    /** Estimated performance gain percentage (0-100) */
    UPROPERTY(BlueprintReadOnly, Category = "Impact Estimates", meta = (DisplayName = "Performance Gain (%)"))
    float EstimatedPerformanceGain;

    /** Estimated quality impact percentage (-100 to +100, negative = quality loss) */
    UPROPERTY(BlueprintReadOnly, Category = "Impact Estimates", meta = (DisplayName = "Quality Impact (%)"))
    float EstimatedQualityImpact;

    /** Estimated time to implement in minutes */
    UPROPERTY(BlueprintReadOnly, Category = "Impact Estimates", meta = (DisplayName = "Implementation Time (min)"))
    int32 EstimatedImplementationTimeMinutes;

    // ============================================================================
    // IMPLEMENTATION DETAILS
    // ============================================================================

    /** Command or action to perform the optimization */
    UPROPERTY(BlueprintReadOnly, Category = "Implementation", meta = (DisplayName = "Action Command"))
    FString ActionCommand;

    /** Parameters for the action command */
    UPROPERTY(BlueprintReadOnly, Category = "Implementation", meta = (DisplayName = "Action Parameters"))
    TMap<FString, FString> ActionParameters;

    /** Whether this recommendation can be applied automatically */
    UPROPERTY(BlueprintReadOnly, Category = "Implementation", meta = (DisplayName = "Can Auto Apply"))
    bool bCanAutoApply;

    /** Whether this recommendation requires user confirmation */
    UPROPERTY(BlueprintReadOnly, Category = "Implementation", meta = (DisplayName = "Requires Confirmation"))
    bool bRequiresConfirmation;

    /** Whether this recommendation is reversible */
    UPROPERTY(BlueprintReadOnly, Category = "Implementation", meta = (DisplayName = "Is Reversible"))
    bool bIsReversible;

    /** Backup information for reverting changes */
    UPROPERTY(BlueprintReadOnly, Category = "Implementation", meta = (DisplayName = "Backup Info"))
    FString BackupInfo;

    // ============================================================================
    // ADDITIONAL INFORMATION
    // ============================================================================

    /** Detailed instructions for manual implementation */
    UPROPERTY(BlueprintReadOnly, Category = "Additional Info", meta = (DisplayName = "Instructions"))
    FString Instructions;

    /** Tips and best practices related to this recommendation */
    UPROPERTY(BlueprintReadOnly, Category = "Additional Info", meta = (DisplayName = "Tips"))
    FString Tips;

    /** Related documentation or help links */
    UPROPERTY(BlueprintReadOnly, Category = "Additional Info", meta = (DisplayName = "Documentation Links"))
    TArray<FString> DocumentationLinks;

    /** Tags for categorizing and filtering recommendations */
    UPROPERTY(BlueprintReadOnly, Category = "Additional Info", meta = (DisplayName = "Tags"))
    TArray<FString> Tags;

    /** When this recommendation was created */
    UPROPERTY(BlueprintReadOnly, Category = "Additional Info", meta = (DisplayName = "Created At"))
    FDateTime CreatedAt;

    /** When this recommendation was last updated */
    UPROPERTY(BlueprintReadOnly, Category = "Additional Info", meta = (DisplayName = "Updated At"))
    FDateTime UpdatedAt;

    // ============================================================================
    // CONSTRUCTOR
    // ============================================================================

    /** Default constructor */
    FOptimizationRecommendation()
        : RecommendationID(TEXT(""))
        , Title(TEXT("No Recommendation"))
        , Description(TEXT("No optimization recommendation available"))
        , Category(EOptimizationCategory::Performance)
        , Priority(EOptimizationPriority::Medium)
        , EstimatedMemorySavingsMB(0.0f)
        , EstimatedDiskSavingsMB(0.0f)
        , EstimatedPerformanceGain(0.0f)
        , EstimatedQualityImpact(0.0f)
        , EstimatedImplementationTimeMinutes(0)
        , ActionCommand(TEXT(""))
        , bCanAutoApply(false)
        , bRequiresConfirmation(true)
        , bIsReversible(true)
        , BackupInfo(TEXT(""))
        , Instructions(TEXT(""))
        , Tips(TEXT(""))
        , CreatedAt(FDateTime::Now())
        , UpdatedAt(FDateTime::Now())
    {
        // Generate a unique ID
        RecommendationID = FString::Printf(TEXT("REC_%s"), *FGuid::NewGuid().ToString());
    }

    // ============================================================================
    // UTILITY FUNCTIONS
    // ============================================================================

    /** Get the priority as a string */
    FString GetPriorityString() const
    {
        switch (Priority)
        {
            case EOptimizationPriority::Low:
                return TEXT("Low");
            case EOptimizationPriority::Medium:
                return TEXT("Medium");
            case EOptimizationPriority::High:
                return TEXT("High");
            case EOptimizationPriority::Critical:
                return TEXT("Critical");
            default:
                return TEXT("Unknown");
        }
    }

    /** Get the category as a string */
    FString GetCategoryString() const
    {
        switch (Category)
        {
            case EOptimizationCategory::Memory:
                return TEXT("Memory");
            case EOptimizationCategory::Performance:
                return TEXT("Performance");
            case EOptimizationCategory::Quality:
                return TEXT("Quality");
            case EOptimizationCategory::Size:
                return TEXT("Size");
            case EOptimizationCategory::Compatibility:
                return TEXT("Compatibility");
            case EOptimizationCategory::Workflow:
                return TEXT("Workflow");
            default:
                return TEXT("Unknown");
        }
    }

    /** Get the estimated total savings in MB */
    float GetTotalSavingsMB() const
    {
        return EstimatedMemorySavingsMB + EstimatedDiskSavingsMB;
    }

    /** Get a formatted savings string */
    FString GetSavingsString() const
    {
        float TotalSavings = GetTotalSavingsMB();
        if (TotalSavings > 1024.0f)
        {
            return FString::Printf(TEXT("%.1f GB"), TotalSavings / 1024.0f);
        }
        else
        {
            return FString::Printf(TEXT("%.1f MB"), TotalSavings);
        }
    }

    /** Get a formatted implementation time string */
    FString GetImplementationTimeString() const
    {
        if (EstimatedImplementationTimeMinutes < 60)
        {
            return FString::Printf(TEXT("%d min"), EstimatedImplementationTimeMinutes);
        }
        else
        {
            int32 Hours = EstimatedImplementationTimeMinutes / 60;
            int32 Minutes = EstimatedImplementationTimeMinutes % 60;
            if (Minutes > 0)
            {
                return FString::Printf(TEXT("%d h %d min"), Hours, Minutes);
            }
            else
            {
                return FString::Printf(TEXT("%d h"), Hours);
            }
        }
    }

    /** Get a summary string for display */
    FString GetSummaryString() const
    {
        return FString::Printf(TEXT("%s - %s (%s) - %s savings"),
            *Title,
            *GetPriorityString(),
            *GetCategoryString(),
            *GetSavingsString()
        );
    }

    /** Get a detailed description string */
    FString GetDetailedDescription() const
    {
        FString Detailed = Description;
        
        if (EstimatedMemorySavingsMB > 0.0f)
        {
            Detailed += FString::Printf(TEXT("\n\nMemory Savings: %.1f MB"), EstimatedMemorySavingsMB);
        }
        
        if (EstimatedDiskSavingsMB > 0.0f)
        {
            Detailed += FString::Printf(TEXT("\nDisk Savings: %.1f MB"), EstimatedDiskSavingsMB);
        }
        
        if (EstimatedPerformanceGain > 0.0f)
        {
            Detailed += FString::Printf(TEXT("\nPerformance Gain: %.1f%%"), EstimatedPerformanceGain);
        }
        
        if (EstimatedQualityImpact != 0.0f)
        {
            if (EstimatedQualityImpact > 0.0f)
            {
                Detailed += FString::Printf(TEXT("\nQuality Improvement: +%.1f%%"), EstimatedQualityImpact);
            }
            else
            {
                Detailed += FString::Printf(TEXT("\nQuality Impact: %.1f%%"), EstimatedQualityImpact);
            }
        }
        
        if (EstimatedImplementationTimeMinutes > 0)
        {
            Detailed += FString::Printf(TEXT("\nImplementation Time: %s"), *GetImplementationTimeString());
        }
        
        return Detailed;
    }

    /** Check if this recommendation is high priority */
    bool IsHighPriority() const
    {
        return Priority == EOptimizationPriority::High || Priority == EOptimizationPriority::Critical;
    }

    /** Check if this recommendation is critical */
    bool IsCritical() const
    {
        return Priority == EOptimizationPriority::Critical;
    }

    /** Get the priority color for UI display */
    FLinearColor GetPriorityColor() const
    {
        switch (Priority)
        {
            case EOptimizationPriority::Low:
                return FLinearColor::Green;
            case EOptimizationPriority::Medium:
                return FLinearColor::Yellow;
            case EOptimizationPriority::High:
                return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
            case EOptimizationPriority::Critical:
                return FLinearColor::Red;
            default:
                return FLinearColor::White;
        }
    }

    /** Add a tag to this recommendation */
    void AddTag(const FString& Tag)
    {
        if (!Tags.Contains(Tag))
        {
            Tags.Add(Tag);
        }
    }

    /** Remove a tag from this recommendation */
    void RemoveTag(const FString& Tag)
    {
        Tags.Remove(Tag);
    }

    /** Check if this recommendation has a specific tag */
    bool HasTag(const FString& Tag) const
    {
        return Tags.Contains(Tag);
    }

    /** Update the recommendation */
    void UpdateRecommendation(const FString& NewTitle, const FString& NewDescription)
    {
        Title = NewTitle;
        Description = NewDescription;
        UpdatedAt = FDateTime::Now();
    }
};


