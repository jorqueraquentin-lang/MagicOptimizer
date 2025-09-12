#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "PerformanceMetrics.h"
#include "OptimizationRecommendation.generated.h"

/**
 * Priority levels for optimization recommendations
 */
UENUM(BlueprintType)
enum class EOptimizationPriority : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

/**
 * Types of optimization recommendations
 */
UENUM(BlueprintType)
enum class EAuditOptimizationType : uint8
{
    Memory      UMETA(DisplayName = "Memory"),
    Performance UMETA(DisplayName = "Performance"),
    Quality     UMETA(DisplayName = "Quality"),
    Compatibility UMETA(DisplayName = "Compatibility"),
    Workflow    UMETA(DisplayName = "Workflow"),
    BestPractice UMETA(DisplayName = "Best Practice")
};

/**
 * Optimization recommendation for asset improvement
 * Provides specific suggestions for optimizing assets based on audit results
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FOptimizationRecommendation
{
    GENERATED_BODY()

    FOptimizationRecommendation()
    {
        Priority = EOptimizationPriority::Medium;
        Type = EAuditOptimizationType::Performance;
        Confidence = 0.5f;
        EstimatedSavingsMB = 0.0f;
        EstimatedPerformanceGain = 0.0f;
        EstimatedQualityImpact = 0.0f;
        bRequiresRestart = false;
        bRequiresRebuild = false;
        bIsReversible = true;
        bIsSafe = true;
        bIsTested = false;
    }

    /** Unique identifier for this recommendation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (DisplayName = "ID"))
    FString ID;

    /** Short title for the recommendation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (DisplayName = "Title"))
    FString Title;

    /** Detailed description of the recommendation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (DisplayName = "Description"))
    FString Description;

    /** Priority level of the recommendation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Priority", meta = (DisplayName = "Priority"))
    EOptimizationPriority Priority;

    /** Type of optimization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type", meta = (DisplayName = "Type"))
    EAuditOptimizationType Type;

    /** Confidence level (0.0 = low, 1.0 = high) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Confidence", meta = (DisplayName = "Confidence", ClampMin = "0.0", ClampMax = "1.0"))
    float Confidence;

    /** Estimated memory savings in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact", meta = (DisplayName = "Estimated Memory Savings (MB)"))
    float EstimatedSavingsMB;

    /** Estimated performance gain (0.0 = no gain, 1.0 = maximum gain) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact", meta = (DisplayName = "Estimated Performance Gain", ClampMin = "0.0", ClampMax = "1.0"))
    float EstimatedPerformanceGain;

    /** Estimated quality impact (-1.0 = quality loss, 0.0 = no change, 1.0 = quality gain) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact", meta = (DisplayName = "Estimated Quality Impact", ClampMin = "-1.0", ClampMax = "1.0"))
    float EstimatedQualityImpact;

    /** Specific action to take */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (DisplayName = "Action"))
    FString Action;

    /** Parameters for the action */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (DisplayName = "Parameters"))
    TMap<FString, FString> Parameters;

    /** Whether this recommendation requires a restart */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements", meta = (DisplayName = "Requires Restart"))
    bool bRequiresRestart;

    /** Whether this recommendation requires a rebuild */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements", meta = (DisplayName = "Requires Rebuild"))
    bool bRequiresRebuild;

    /** Whether this recommendation is reversible */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safety", meta = (DisplayName = "Is Reversible"))
    bool bIsReversible;

    /** Whether this recommendation is safe to apply */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safety", meta = (DisplayName = "Is Safe"))
    bool bIsSafe;

    /** Whether this recommendation has been tested */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing", meta = (DisplayName = "Is Tested"))
    bool bIsTested;

    /** Additional notes or warnings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notes", meta = (DisplayName = "Notes"))
    FString Notes;

    /** Related recommendations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relations", meta = (DisplayName = "Related Recommendations"))
    TArray<FString> RelatedRecommendations;

    /** Dependencies (recommendations that must be applied first) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relations", meta = (DisplayName = "Dependencies"))
    TArray<FString> Dependencies;

    /** Conflicts (recommendations that conflict with this one) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relations", meta = (DisplayName = "Conflicts"))
    TArray<FString> Conflicts;

    /**
     * Get priority as string
     * @return Priority string
     */
    FString GetPriorityString() const
    {
        switch (Priority)
        {
            case EOptimizationPriority::Low: return TEXT("Low");
            case EOptimizationPriority::Medium: return TEXT("Medium");
            case EOptimizationPriority::High: return TEXT("High");
            case EOptimizationPriority::Critical: return TEXT("Critical");
            default: return TEXT("Unknown");
        }
    }

    /**
     * Get type as string
     * @return Type string
     */
    FString GetTypeString() const
    {
        switch (Type)
        {
            case EAuditOptimizationType::Memory: return TEXT("Memory");
            case EAuditOptimizationType::Performance: return TEXT("Performance");
            case EAuditOptimizationType::Quality: return TEXT("Quality");
            case EAuditOptimizationType::Compatibility: return TEXT("Compatibility");
            case EAuditOptimizationType::Workflow: return TEXT("Workflow");
            case EAuditOptimizationType::BestPractice: return TEXT("Best Practice");
            default: return TEXT("Unknown");
        }
    }

    /**
     * Get confidence as string
     * @return Confidence string
     */
    FString GetConfidenceString() const
    {
        if (Confidence >= 0.9f) return TEXT("Very High");
        if (Confidence >= 0.7f) return TEXT("High");
        if (Confidence >= 0.5f) return TEXT("Medium");
        if (Confidence >= 0.3f) return TEXT("Low");
        return TEXT("Very Low");
    }

    /**
     * Get impact summary as string
     * @return Impact summary string
     */
    FString GetImpactSummary() const
    {
        FString Summary = TEXT("");
        
        if (EstimatedSavingsMB > 0.0f)
        {
            Summary += FString::Printf(TEXT("Memory: %.2fMB "), EstimatedSavingsMB);
        }
        
        if (EstimatedPerformanceGain > 0.0f)
        {
            Summary += FString::Printf(TEXT("Performance: +%.1f%% "), EstimatedPerformanceGain * 100.0f);
        }
        
        if (FMath::Abs(EstimatedQualityImpact) > 0.01f)
        {
            if (EstimatedQualityImpact > 0.0f)
            {
                Summary += FString::Printf(TEXT("Quality: +%.1f%% "), EstimatedQualityImpact * 100.0f);
            }
            else
            {
                Summary += FString::Printf(TEXT("Quality: %.1f%% "), EstimatedQualityImpact * 100.0f);
            }
        }
        
        return Summary.TrimEnd();
    }

    /**
     * Get safety summary as string
     * @return Safety summary string
     */
    FString GetSafetySummary() const
    {
        FString Summary = TEXT("");
        
        if (!bIsSafe)
        {
            Summary += TEXT("⚠️ Not Safe ");
        }
        
        if (bRequiresRestart)
        {
            Summary += TEXT("🔄 Requires Restart ");
        }
        
        if (bRequiresRebuild)
        {
            Summary += TEXT("🔨 Requires Rebuild ");
        }
        
        if (!bIsReversible)
        {
            Summary += TEXT("⚠️ Not Reversible ");
        }
        
        if (!bIsTested)
        {
            Summary += TEXT("🧪 Not Tested ");
        }
        
        return Summary.TrimEnd();
    }

    /**
     * Check if this recommendation can be applied
     * @return True if recommendation can be applied
     */
    bool CanBeApplied() const
    {
        return !ID.IsEmpty() && 
               !Title.IsEmpty() && 
               !Action.IsEmpty() && 
               bIsSafe;
    }

    /**
     * Check if this recommendation conflicts with another
     * @param Other Other recommendation to check
     * @return True if recommendations conflict
     */
    bool ConflictsWith(const FOptimizationRecommendation& Other) const
    {
        return Conflicts.Contains(Other.ID) || Other.Conflicts.Contains(ID);
    }

    /**
     * Check if this recommendation depends on another
     * @param Other Other recommendation to check
     * @return True if this recommendation depends on the other
     */
    bool DependsOn(const FOptimizationRecommendation& Other) const
    {
        return Dependencies.Contains(Other.ID);
    }

    /**
     * Get total impact score (combination of all impacts)
     * @return Total impact score
     */
    float GetTotalImpactScore() const
    {
        float MemoryScore = FMath::Clamp(EstimatedSavingsMB / 100.0f, 0.0f, 1.0f); // Normalize to 100MB
        float PerformanceScore = EstimatedPerformanceGain;
        float QualityScore = (EstimatedQualityImpact + 1.0f) * 0.5f; // Convert -1..1 to 0..1
        
        // Weighted average
        return (MemoryScore * 0.4f + PerformanceScore * 0.4f + QualityScore * 0.2f);
    }

    /**
     * Get recommendation summary as string
     * @return Recommendation summary string
     */
    FString GetSummary() const
    {
        return FString::Printf(TEXT("[%s] %s - %s (Confidence: %s, Impact: %s)"),
            *GetPriorityString(),
            *Title,
            *GetTypeString(),
            *GetConfidenceString(),
            *GetImpactSummary()
        );
    }

    /**
     * Create a copy of this recommendation with modified parameters
     * @param NewParameters New parameters to apply
     * @return New recommendation with modified parameters
     */
    FOptimizationRecommendation CreateCopyWithParameters(const TMap<FString, FString>& NewParameters) const
    {
        FOptimizationRecommendation Copy = *this;
        Copy.Parameters = NewParameters;
        return Copy;
    }
};
