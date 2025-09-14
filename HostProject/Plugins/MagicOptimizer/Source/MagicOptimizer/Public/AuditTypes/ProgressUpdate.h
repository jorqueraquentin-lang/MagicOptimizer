#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "ProgressUpdate.generated.h"

/**
 * Real-time progress update structure for asset audit operations.
 * 
 * This structure provides comprehensive progress tracking information
 * for long-running asset audit operations, allowing the UI to display
 * real-time updates and status information to the user.
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FProgressUpdate
{
    GENERATED_BODY()

    // ============================================================================
    // PROGRESS METRICS
    // ============================================================================

    /** Progress percentage from 0.0 to 1.0 */
    UPROPERTY(BlueprintReadOnly, Category = "Progress", meta = (DisplayName = "Progress Percentage"))
    float ProgressPercentage;

    /** Progress percentage as an integer from 0 to 100 */
    UPROPERTY(BlueprintReadOnly, Category = "Progress", meta = (DisplayName = "Progress (0-100)"))
    int32 ProgressPercent;

    /** Current status message describing what's happening */
    UPROPERTY(BlueprintReadOnly, Category = "Progress", meta = (DisplayName = "Status Message"))
    FString StatusMessage;

    /** Detailed status message with more information */
    UPROPERTY(BlueprintReadOnly, Category = "Progress", meta = (DisplayName = "Detailed Status"))
    FString DetailedStatus;

    // ============================================================================
    // ASSET COUNTERS
    // ============================================================================

    /** Number of assets processed so far */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Counters", meta = (DisplayName = "Assets Processed"))
    int32 AssetsProcessed;

    /** Total number of assets to process */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Counters", meta = (DisplayName = "Total Assets"))
    int32 TotalAssets;

    /** Number of assets remaining to process */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Counters", meta = (DisplayName = "Assets Remaining"))
    int32 AssetsRemaining;

    /** Number of assets with issues found */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Counters", meta = (DisplayName = "Assets with Issues"))
    int32 AssetsWithIssues;

    /** Number of assets successfully processed */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Counters", meta = (DisplayName = "Assets Successful"))
    int32 AssetsSuccessful;

    /** Number of assets that failed to process */
    UPROPERTY(BlueprintReadOnly, Category = "Asset Counters", meta = (DisplayName = "Assets Failed"))
    int32 AssetsFailed;

    // ============================================================================
    // CURRENT OPERATION
    // ============================================================================

    /** Path of the asset currently being processed */
    UPROPERTY(BlueprintReadOnly, Category = "Current Operation", meta = (DisplayName = "Current Asset Path"))
    FString CurrentAssetPath;

    /** Name of the asset currently being processed */
    UPROPERTY(BlueprintReadOnly, Category = "Current Operation", meta = (DisplayName = "Current Asset Name"))
    FString CurrentAssetName;

    /** Type of the asset currently being processed */
    UPROPERTY(BlueprintReadOnly, Category = "Current Operation", meta = (DisplayName = "Current Asset Type"))
    FString CurrentAssetType;

    /** Current phase of the operation (Discovery, Analysis, Processing, etc.) */
    UPROPERTY(BlueprintReadOnly, Category = "Current Operation", meta = (DisplayName = "Current Phase"))
    FString CurrentPhase;

    // ============================================================================
    // TIMING INFORMATION
    // ============================================================================

    /** When the operation started */
    UPROPERTY(BlueprintReadOnly, Category = "Timing", meta = (DisplayName = "Start Time"))
    FDateTime StartTime;

    /** When the operation is estimated to complete */
    UPROPERTY(BlueprintReadOnly, Category = "Timing", meta = (DisplayName = "Estimated End Time"))
    FDateTime EstimatedEndTime;

    /** Elapsed time since the operation started */
    UPROPERTY(BlueprintReadOnly, Category = "Timing", meta = (DisplayName = "Elapsed Time"))
    FString ElapsedTime;

    /** Estimated time remaining */
    UPROPERTY(BlueprintReadOnly, Category = "Timing", meta = (DisplayName = "Time Remaining"))
    FString TimeRemaining;

    /** Average time per asset in seconds */
    UPROPERTY(BlueprintReadOnly, Category = "Timing", meta = (DisplayName = "Average Time Per Asset"))
    float AverageTimePerAsset;

    // ============================================================================
    // STATUS FLAGS
    // ============================================================================

    /** Whether the operation is complete */
    UPROPERTY(BlueprintReadOnly, Category = "Status Flags", meta = (DisplayName = "Is Complete"))
    bool bIsComplete;

    /** Whether the operation has errors */
    UPROPERTY(BlueprintReadOnly, Category = "Status Flags", meta = (DisplayName = "Has Errors"))
    bool bHasErrors;

    /** Whether the operation was cancelled */
    UPROPERTY(BlueprintReadOnly, Category = "Status Flags", meta = (DisplayName = "Was Cancelled"))
    bool bWasCancelled;

    /** Whether the operation is currently paused */
    UPROPERTY(BlueprintReadOnly, Category = "Status Flags", meta = (DisplayName = "Is Paused"))
    bool bIsPaused;

    /** Whether the operation is currently running */
    UPROPERTY(BlueprintReadOnly, Category = "Status Flags", meta = (DisplayName = "Is Running"))
    bool bIsRunning;

    // ============================================================================
    // ERROR INFORMATION
    // ============================================================================

    /** List of error messages encountered during the operation */
    UPROPERTY(BlueprintReadOnly, Category = "Error Information", meta = (DisplayName = "Error Messages"))
    TArray<FString> ErrorMessages;

    /** Number of errors encountered */
    UPROPERTY(BlueprintReadOnly, Category = "Error Information", meta = (DisplayName = "Error Count"))
    int32 ErrorCount;

    /** Last error message */
    UPROPERTY(BlueprintReadOnly, Category = "Error Information", meta = (DisplayName = "Last Error"))
    FString LastError;

    // ============================================================================
    // CONSTRUCTOR
    // ============================================================================

    /** Default constructor */
    FProgressUpdate()
        : ProgressPercentage(0.0f)
        , ProgressPercent(0)
        , StatusMessage(TEXT("Initializing..."))
        , DetailedStatus(TEXT("Preparing asset audit operation..."))
        , AssetsProcessed(0)
        , TotalAssets(0)
        , AssetsRemaining(0)
        , AssetsWithIssues(0)
        , AssetsSuccessful(0)
        , AssetsFailed(0)
        , CurrentAssetPath(TEXT(""))
        , CurrentAssetName(TEXT(""))
        , CurrentAssetType(TEXT(""))
        , CurrentPhase(TEXT("Initialization"))
        , StartTime(FDateTime::Now())
        , EstimatedEndTime(FDateTime::Now())
        , ElapsedTime(TEXT("00:00:00"))
        , TimeRemaining(TEXT("Calculating..."))
        , AverageTimePerAsset(0.0f)
        , bIsComplete(false)
        , bHasErrors(false)
        , bWasCancelled(false)
        , bIsPaused(false)
        , bIsRunning(false)
        , ErrorCount(0)
        , LastError(TEXT(""))
    {
        // Initialize estimated end time to start time + 1 hour as default
        EstimatedEndTime = StartTime + FTimespan::FromHours(1);
    }

    // ============================================================================
    // UTILITY FUNCTIONS
    // ============================================================================

    /** Update progress percentage and recalculate derived values */
    void UpdateProgress(float NewPercentage, const FString& NewStatusMessage = TEXT(""))
    {
        ProgressPercentage = FMath::Clamp(NewPercentage, 0.0f, 1.0f);
        ProgressPercent = FMath::RoundToInt(ProgressPercentage * 100.0f);
        
        if (!NewStatusMessage.IsEmpty())
        {
            StatusMessage = NewStatusMessage;
        }
        
        // Update remaining assets
        AssetsRemaining = FMath::Max(0, TotalAssets - AssetsProcessed);
        
        // Update elapsed time
        UpdateElapsedTime();
        
        // Update estimated end time
        UpdateEstimatedEndTime();
    }

    /** Update asset counters */
    void UpdateAssetCounters(int32 Processed, int32 Total, int32 WithIssues = 0, int32 Successful = 0, int32 Failed = 0)
    {
        AssetsProcessed = Processed;
        TotalAssets = Total;
        AssetsWithIssues = WithIssues;
        AssetsSuccessful = Successful;
        AssetsFailed = Failed;
        
        // Update remaining assets
        AssetsRemaining = FMath::Max(0, TotalAssets - AssetsProcessed);
        
        // Update progress percentage
        if (TotalAssets > 0)
        {
            float NewPercentage = static_cast<float>(AssetsProcessed) / static_cast<float>(TotalAssets);
            UpdateProgress(NewPercentage);
        }
    }

    /** Set current asset being processed */
    void SetCurrentAsset(const FString& AssetPath, const FString& AssetName, const FString& AssetType)
    {
        CurrentAssetPath = AssetPath;
        CurrentAssetName = AssetName;
        CurrentAssetType = AssetType;
    }

    /** Set current phase */
    void SetCurrentPhase(const FString& Phase)
    {
        CurrentPhase = Phase;
    }

    /** Add an error message */
    void AddError(const FString& ErrorMessage)
    {
        ErrorMessages.Add(ErrorMessage);
        ErrorCount = ErrorMessages.Num();
        LastError = ErrorMessage;
        bHasErrors = true;
    }

    /** Mark operation as complete */
    void MarkComplete()
    {
        bIsComplete = true;
        bIsRunning = false;
        ProgressPercentage = 1.0f;
        ProgressPercent = 100;
        StatusMessage = TEXT("Operation completed successfully");
        UpdateElapsedTime();
    }

    /** Mark operation as cancelled */
    void MarkCancelled()
    {
        bWasCancelled = true;
        bIsRunning = false;
        bIsComplete = true;
        StatusMessage = TEXT("Operation cancelled by user");
        UpdateElapsedTime();
    }

    /** Get a summary string for display */
    FString GetSummaryString() const
    {
        if (bIsComplete)
        {
            if (bWasCancelled)
            {
                return FString::Printf(TEXT("Cancelled - %d/%d assets processed"), AssetsProcessed, TotalAssets);
            }
            else
            {
                return FString::Printf(TEXT("Complete - %d/%d assets processed"), AssetsProcessed, TotalAssets);
            }
        }
        else if (bIsRunning)
        {
            return FString::Printf(TEXT("Running - %d/%d assets processed (%d%%)"), 
                AssetsProcessed, TotalAssets, ProgressPercent);
        }
        else
        {
            return TEXT("Not started");
        }
    }

    /** Get progress bar percentage for UI */
    float GetProgressBarPercentage() const
    {
        return ProgressPercentage;
    }

    /** Get progress text for UI */
    FString GetProgressText() const
    {
        return FString::Printf(TEXT("%d%%"), ProgressPercent);
    }

    /** Get status text for UI */
    FString GetStatusText() const
    {
        if (bIsComplete)
        {
            return StatusMessage;
        }
        else if (bIsRunning)
        {
            return FString::Printf(TEXT("%s - %s"), *StatusMessage, *CurrentAssetName);
        }
        else
        {
            return StatusMessage;
        }
    }

private:
    /** Update elapsed time string */
    void UpdateElapsedTime()
    {
        FTimespan Elapsed = FDateTime::Now() - StartTime;
        int32 Hours = Elapsed.GetHours();
        int32 Minutes = Elapsed.GetMinutes();
        int32 Seconds = Elapsed.GetSeconds();
        ElapsedTime = FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds);
    }

    /** Update estimated end time based on current progress */
    void UpdateEstimatedEndTime()
    {
        if (AssetsProcessed > 0 && TotalAssets > 0)
        {
            // Calculate average time per asset
            FTimespan Elapsed = FDateTime::Now() - StartTime;
            AverageTimePerAsset = Elapsed.GetTotalSeconds() / static_cast<float>(AssetsProcessed);
            
            // Estimate remaining time
            float RemainingSeconds = AverageTimePerAsset * static_cast<float>(AssetsRemaining);
            EstimatedEndTime = FDateTime::Now() + FTimespan::FromSeconds(RemainingSeconds);
            
            // Update time remaining string
            int32 RemainingHours = static_cast<int32>(RemainingSeconds) / 3600;
            int32 RemainingMinutes = (static_cast<int32>(RemainingSeconds) % 3600) / 60;
            int32 RemainingSecs = static_cast<int32>(RemainingSeconds) % 60;
            TimeRemaining = FString::Printf(TEXT("%02d:%02d:%02d"), RemainingHours, RemainingMinutes, RemainingSecs);
        }
    }
};
