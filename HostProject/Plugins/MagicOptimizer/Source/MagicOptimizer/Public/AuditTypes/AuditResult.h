#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "AssetRegistry/AssetData.h"
#include "AuditConfig.h"
#include "PerformanceMetrics.h"
#include "OptimizationRecommendation.h"
#include "AuditResult.generated.h"

/**
 * Types of audit issues
 */
UENUM(BlueprintType)
enum class EAuditIssueType : uint8
{
    Error       UMETA(DisplayName = "Error"),
    Warning     UMETA(DisplayName = "Warning"),
    Info        UMETA(DisplayName = "Info"),
    Suggestion  UMETA(DisplayName = "Suggestion")
};

/**
 * Audit issue found during asset analysis
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FAuditIssue
{
    GENERATED_BODY()

    FAuditIssue()
    {
        Type = EAuditIssueType::Info;
        Severity = 0.5f;
        bIsResolved = false;
    }

    /** Unique identifier for this issue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (DisplayName = "ID"))
    FString ID;

    /** Issue type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type", meta = (DisplayName = "Type"))
    EAuditIssueType Type;

    /** Short title for the issue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (DisplayName = "Title"))
    FString Title;

    /** Detailed description of the issue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (DisplayName = "Description"))
    FString Description;

    /** Severity level (0.0 = low, 1.0 = high) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Severity", meta = (DisplayName = "Severity", ClampMin = "0.0", ClampMax = "1.0"))
    float Severity;

    /** Whether this issue has been resolved */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (DisplayName = "Is Resolved"))
    bool bIsResolved;

    /** Additional context or data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context", meta = (DisplayName = "Context"))
    TMap<FString, FString> Context;

    /** Related recommendations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relations", meta = (DisplayName = "Related Recommendations"))
    TArray<FString> RelatedRecommendations;

    /**
     * Get type as string
     * @return Type string
     */
    FString GetTypeString() const
    {
        switch (Type)
        {
            case EAuditIssueType::Error: return TEXT("Error");
            case EAuditIssueType::Warning: return TEXT("Warning");
            case EAuditIssueType::Info: return TEXT("Info");
            case EAuditIssueType::Suggestion: return TEXT("Suggestion");
            default: return TEXT("Unknown");
        }
    }

    /**
     * Get severity as string
     * @return Severity string
     */
    FString GetSeverityString() const
    {
        if (Severity >= 0.9f) return TEXT("Critical");
        if (Severity >= 0.7f) return TEXT("High");
        if (Severity >= 0.5f) return TEXT("Medium");
        if (Severity >= 0.3f) return TEXT("Low");
        return TEXT("Very Low");
    }

    /**
     * Get issue summary as string
     * @return Issue summary string
     */
    FString GetSummary() const
    {
        return FString::Printf(TEXT("[%s] %s - %s (Severity: %s)"),
            *GetTypeString(),
            *Title,
            *Description,
            *GetSeverityString()
        );
    }
};

/**
 * Comprehensive audit result for a single asset
 * Contains all information gathered during asset analysis
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FAuditResult
{
    GENERATED_BODY()

    FAuditResult()
    {
        Status = EAuditStatus::Pending;
        ErrorCode = 0;
        bIsValid = false;
        bHasIssues = false;
        bHasRecommendations = false;
        bIsOptimized = false;
    }

    /** Asset data for this result */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset", meta = (DisplayName = "Asset Data"))
    FAssetData AssetData;

    /** Audit status */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (DisplayName = "Status"))
    EAuditStatus Status;

    /** Performance metrics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (DisplayName = "Performance Metrics"))
    FPerformanceMetrics PerformanceMetrics;

    /** Optimization recommendations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recommendations", meta = (DisplayName = "Recommendations"))
    TArray<FOptimizationRecommendation> Recommendations;

    /** Issues found during audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Issues", meta = (DisplayName = "Issues"))
    TArray<FAuditIssue> Issues;

    /** Start time of audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (DisplayName = "Start Time"))
    FDateTime StartTime;

    /** End time of audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (DisplayName = "End Time"))
    FDateTime EndTime;

    /** Error message if audit failed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Error", meta = (DisplayName = "Error Message"))
    FString ErrorMessage;

    /** Error code if audit failed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Error", meta = (DisplayName = "Error Code"))
    int32 ErrorCode;

    /** Whether this result is valid */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation", meta = (DisplayName = "Is Valid"))
    bool bIsValid;

    /** Whether this result has issues */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation", meta = (DisplayName = "Has Issues"))
    bool bHasIssues;

    /** Whether this result has recommendations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation", meta = (DisplayName = "Has Recommendations"))
    bool bHasRecommendations;

    /** Whether this asset is optimized */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation", meta = (DisplayName = "Is Optimized"))
    bool bIsOptimized;

    /** Additional metadata */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata", meta = (DisplayName = "Metadata"))
    TMap<FString, FString> Metadata;

    /**
     * Get audit duration in seconds
     * @return Duration in seconds
     */
    float GetDurationSeconds() const
    {
        if (StartTime.GetTicks() == 0 || EndTime.GetTicks() == 0)
        {
            return 0.0f;
        }
        
        FTimespan Duration = EndTime - StartTime;
        return Duration.GetTotalSeconds();
    }

    /**
     * Get audit duration in milliseconds
     * @return Duration in milliseconds
     */
    float GetDurationMilliseconds() const
    {
        return GetDurationSeconds() * 1000.0f;
    }

    /**
     * Get status as string
     * @return Status string
     */
    FString GetStatusString() const
    {
        switch (Status)
        {
            case EAuditStatus::Pending: return TEXT("Pending");
            case EAuditStatus::Running: return TEXT("Running");
            case EAuditStatus::Completed: return TEXT("Completed");
            case EAuditStatus::Failed: return TEXT("Failed");
            case EAuditStatus::Cancelled: return TEXT("Cancelled");
            default: return TEXT("Unknown");
        }
    }

    /**
     * Get asset name
     * @return Asset name
     */
    FString GetAssetName() const
    {
        return AssetData.AssetName.ToString();
    }

    /**
     * Get asset path
     * @return Asset path
     */
    FString GetAssetPath() const
    {
        return AssetData.GetObjectPathString();
    }

    /**
     * Get asset class name
     * @return Asset class name
     */
    FString GetAssetClassName() const
    {
        return AssetData.AssetClassPath.ToString();
    }

    /**
     * Get high priority recommendations
     * @return High priority recommendations
     */
    TArray<FOptimizationRecommendation> GetHighPriorityRecommendations() const
    {
        TArray<FOptimizationRecommendation> HighPriority;
        for (const FOptimizationRecommendation& Rec : Recommendations)
        {
            if (Rec.Priority == EOptimizationPriority::High || Rec.Priority == EOptimizationPriority::Critical)
            {
                HighPriority.Add(Rec);
            }
        }
        return HighPriority;
    }

    /**
     * Get critical issues
     * @return Critical issues
     */
    TArray<FAuditIssue> GetCriticalIssues() const
    {
        TArray<FAuditIssue> Critical;
        for (const FAuditIssue& Issue : Issues)
        {
            if (Issue.Severity >= 0.8f)
            {
                Critical.Add(Issue);
            }
        }
        return Critical;
    }

    /**
     * Get total estimated savings from all recommendations
     * @return Total estimated savings in MB
     */
    float GetTotalEstimatedSavings() const
    {
        float TotalSavings = 0.0f;
        for (const FOptimizationRecommendation& Rec : Recommendations)
        {
            TotalSavings += Rec.EstimatedMemorySavingsMB;
        }
        return TotalSavings;
    }

    /**
     * Get overall optimization score
     * @return Overall optimization score (0.0 = poor, 1.0 = excellent)
     */
    float GetOptimizationScore() const
    {
        if (Recommendations.Num() == 0)
        {
            return 1.0f; // No recommendations = already optimized
        }
        
        float TotalScore = 0.0f;
        for (const FOptimizationRecommendation& Rec : Recommendations)
        {
            // Calculate impact score based on priority and estimated savings
            float ImpactScore = 0.0f;
            switch (Rec.Priority)
            {
                case EOptimizationPriority::Low:
                    ImpactScore = 0.25f;
                    break;
                case EOptimizationPriority::Medium:
                    ImpactScore = 0.5f;
                    break;
                case EOptimizationPriority::High:
                    ImpactScore = 0.75f;
                    break;
                case EOptimizationPriority::Critical:
                    ImpactScore = 1.0f;
                    break;
            }
            
            // Factor in estimated savings
            float SavingsFactor = FMath::Clamp(Rec.EstimatedMemorySavingsMB / 100.0f, 0.0f, 1.0f);
            ImpactScore *= (0.5f + 0.5f * SavingsFactor);
            
            TotalScore += ImpactScore;
        }
        
        return FMath::Clamp(TotalScore / Recommendations.Num(), 0.0f, 1.0f);
    }

    /**
     * Get result summary as string
     * @return Result summary string
     */
    FString GetSummary() const
    {
        FString Summary = FString::Printf(TEXT("Asset: %s | Status: %s | Duration: %.2fms"),
            *GetAssetName(),
            *GetStatusString(),
            GetDurationMilliseconds()
        );
        
        if (bHasIssues)
        {
            Summary += FString::Printf(TEXT(" | Issues: %d"), Issues.Num());
        }
        
        if (bHasRecommendations)
        {
            Summary += FString::Printf(TEXT(" | Recommendations: %d"), Recommendations.Num());
        }
        
        if (PerformanceMetrics.MemoryUsageMB > 0.0f)
        {
            Summary += FString::Printf(TEXT(" | Memory: %.2fMB"), PerformanceMetrics.MemoryUsageMB);
        }
        
        return Summary;
    }

    /**
     * Get detailed report as string
     * @return Detailed report string
     */
    FString GetDetailedReport() const
    {
        FString Report = TEXT("");
        
        // Header
        Report += FString::Printf(TEXT("=== AUDIT RESULT FOR %s ===\n"), *GetAssetName());
        Report += FString::Printf(TEXT("Asset Path: %s\n"), *GetAssetPath());
        Report += FString::Printf(TEXT("Asset Class: %s\n"), *GetAssetClassName());
        Report += FString::Printf(TEXT("Status: %s\n"), *GetStatusString());
        Report += FString::Printf(TEXT("Duration: %.2fms\n"), GetDurationMilliseconds());
        Report += TEXT("\n");
        
        // Performance Metrics
        Report += TEXT("=== PERFORMANCE METRICS ===\n");
        Report += FString::Printf(TEXT("Memory Usage: %.2fMB\n"), PerformanceMetrics.MemoryUsageMB);
        Report += FString::Printf(TEXT("Draw Calls: %d\n"), PerformanceMetrics.DrawCalls);
        Report += FString::Printf(TEXT("Triangles: %d\n"), PerformanceMetrics.Triangles);
        Report += FString::Printf(TEXT("Vertices: %d\n"), PerformanceMetrics.Vertices);
        Report += FString::Printf(TEXT("Quality Score: %.2f\n"), PerformanceMetrics.QualityScore);
        Report += FString::Printf(TEXT("Performance Score: %.2f\n"), PerformanceMetrics.PerformanceScore);
        Report += TEXT("\n");
        
        // Issues
        if (Issues.Num() > 0)
        {
            Report += TEXT("=== ISSUES FOUND ===\n");
            for (const FAuditIssue& Issue : Issues)
            {
                Report += FString::Printf(TEXT("- %s\n"), *Issue.GetSummary());
            }
            Report += TEXT("\n");
        }
        
        // Recommendations
        if (Recommendations.Num() > 0)
        {
            Report += TEXT("=== OPTIMIZATION RECOMMENDATIONS ===\n");
            for (const FOptimizationRecommendation& Rec : Recommendations)
            {
                Report += FString::Printf(TEXT("- %s\n"), *Rec.GetSummaryString());
            }
            Report += TEXT("\n");
        }
        
        // Error Information
        if (Status == EAuditStatus::Failed && !ErrorMessage.IsEmpty())
        {
            Report += TEXT("=== ERROR INFORMATION ===\n");
            Report += FString::Printf(TEXT("Error Code: %d\n"), ErrorCode);
            Report += FString::Printf(TEXT("Error Message: %s\n"), *ErrorMessage);
            Report += TEXT("\n");
        }
        
        return Report;
    }

    /**
     * Add a recommendation
     * @param Recommendation Recommendation to add
     */
    void AddRecommendation(const FOptimizationRecommendation& Recommendation)
    {
        Recommendations.Add(Recommendation);
        bHasRecommendations = true;
    }

    /**
     * Add an issue
     * @param Issue Issue to add
     */
    void AddIssue(const FAuditIssue& Issue)
    {
        Issues.Add(Issue);
        bHasIssues = true;
    }

    /**
     * Set metadata value
     * @param Key Metadata key
     * @param Value Metadata value
     */
    void SetMetadata(const FString& Key, const FString& Value)
    {
        Metadata.Add(Key, Value);
    }

    /**
     * Get metadata value
     * @param Key Metadata key
     * @param DefaultValue Default value if key not found
     * @return Metadata value
     */
    FString GetMetadata(const FString& Key, const FString& DefaultValue = TEXT("")) const
    {
        if (const FString* Value = Metadata.Find(Key))
        {
            return *Value;
        }
        return DefaultValue;
    }

    /**
     * Mark audit as completed
     */
    void MarkCompleted()
    {
        Status = EAuditStatus::Completed;
        EndTime = FDateTime::Now();
        bIsValid = true;
    }

    /**
     * Mark audit as failed
     * @param ErrorMsg Error message
     * @param ErrorCodeValue Error code
     */
    void MarkFailed(const FString& ErrorMsg, int32 ErrorCodeValue = -1)
    {
        Status = EAuditStatus::Failed;
        EndTime = FDateTime::Now();
        ErrorMessage = ErrorMsg;
        ErrorCode = ErrorCodeValue;
        bIsValid = false;
    }

    /**
     * Reset result to initial state
     */
    void Reset()
    {
        Status = EAuditStatus::Pending;
        ErrorCode = 0;
        ErrorMessage.Empty();
        bIsValid = false;
        bHasIssues = false;
        bHasRecommendations = false;
        bIsOptimized = false;
        StartTime = FDateTime();
        EndTime = FDateTime();
        PerformanceMetrics.Reset();
        Recommendations.Empty();
        Issues.Empty();
        Metadata.Empty();
    }
};
