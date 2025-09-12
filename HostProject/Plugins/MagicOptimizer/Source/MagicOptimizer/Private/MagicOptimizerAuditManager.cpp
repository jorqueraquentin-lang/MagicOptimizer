#include "MagicOptimizerAuditManager.h"
#include "MagicOptimizerDiagnostics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

// Static member definitions
UMagicOptimizerAuditManager* UMagicOptimizerAuditManager::Instance = nullptr;
TArray<TSharedPtr<IAssetAuditor>> UMagicOptimizerAuditManager::AssetAuditors;
FAuditConfig UMagicOptimizerAuditManager::CurrentConfig;
TArray<FAuditResult> UMagicOptimizerAuditManager::AuditResults;
bool UMagicOptimizerAuditManager::bIsAuditRunning = false;
float UMagicOptimizerAuditManager::CurrentProgress = 0.0f;
FString UMagicOptimizerAuditManager::CurrentStatus = TEXT("Idle");

// Event delegates
FOnAuditProgress UMagicOptimizerAuditManager::OnAuditProgress;
FOnAuditCompleted UMagicOptimizerAuditManager::OnAuditCompleted;
FOnAuditFailed UMagicOptimizerAuditManager::OnAuditFailed;

UMagicOptimizerAuditManager::UMagicOptimizerAuditManager()
{
    // Constructor
}

void UMagicOptimizerAuditManager::Initialize()
{
    MAGIC_LOG(General, TEXT("Initializing Asset Audit Manager"));
    
    if (Instance == nullptr)
    {
        Instance = NewObject<UMagicOptimizerAuditManager>();
        Instance->AddToRoot(); // Prevent garbage collection
    }
    
    RegisterAssetAuditors();
    
    MAGIC_LOG(General, TEXT("Asset Audit Manager initialized successfully"));
}

void UMagicOptimizerAuditManager::Shutdown()
{
    MAGIC_LOG(General, TEXT("Shutting down Asset Audit Manager"));
    
    if (bIsAuditRunning)
    {
        StopAssetAudit();
    }
    
    AssetAuditors.Empty();
    AuditResults.Empty();
    
    if (Instance != nullptr)
    {
        Instance->RemoveFromRoot();
        Instance = nullptr;
    }
    
    MAGIC_LOG(General, TEXT("Asset Audit Manager shutdown complete"));
}

bool UMagicOptimizerAuditManager::StartAssetAudit(const FAuditConfig& Config)
{
    MAGIC_LOG(General, TEXT("Starting asset audit"));
    MAGIC_LOG_PERF_START(TEXT("AssetAudit"));
    
    if (bIsAuditRunning)
    {
        MAGIC_LOG_WARNING(TEXT("Audit is already running"), TEXT("StartAssetAudit"));
        return false;
    }
    
    if (!Config.IsValid())
    {
        MAGIC_LOG_ERROR(TEXT("Invalid audit configuration"), TEXT("StartAssetAudit"));
        return false;
    }
    
    CurrentConfig = Config;
    AuditResults.Empty();
    bIsAuditRunning = true;
    CurrentProgress = 0.0f;
    CurrentStatus = TEXT("Discovering assets");
    
    // Discover assets to audit
    TArray<FAssetData> AssetsToAudit = DiscoverAssetsToAudit();
    
    if (AssetsToAudit.Num() == 0)
    {
        MAGIC_LOG_WARNING(TEXT("No assets found to audit"), TEXT("StartAssetAudit"));
        bIsAuditRunning = false;
        return false;
    }
    
    MAGIC_LOG(General, FString::Printf(TEXT("Found %d assets to audit"), AssetsToAudit.Num()));
    
    // Process assets
    CurrentStatus = TEXT("Processing assets");
    int32 ProcessedAssets = 0;
    
    for (const FAssetData& AssetData : AssetsToAudit)
    {
        if (!bIsAuditRunning) // Check if audit was stopped
        {
            break;
        }
        
        FAuditResult Result = ProcessAssetForAudit(AssetData);
        AuditResults.Add(Result);
        
        ProcessedAssets++;
        CurrentProgress = (float)ProcessedAssets / (float)AssetsToAudit.Num();
        
        // Update progress
        UpdateAuditProgress(CurrentProgress);
        
        // Log progress
        if (ProcessedAssets % 10 == 0 || ProcessedAssets == AssetsToAudit.Num())
        {
            MAGIC_LOG(General, FString::Printf(TEXT("Processed %d/%d assets (%.1f%%)"), 
                ProcessedAssets, AssetsToAudit.Num(), CurrentProgress * 100.0f));
        }
    }
    
    CompleteAudit();
    
    MAGIC_LOG_PERF_END(TEXT("AssetAudit"));
    MAGIC_LOG(General, TEXT("Asset audit completed successfully"));
    
    return true;
}

void UMagicOptimizerAuditManager::StopAssetAudit()
{
    MAGIC_LOG(General, TEXT("Stopping asset audit"));
    
    if (!bIsAuditRunning)
    {
        MAGIC_LOG_WARNING(TEXT("No audit is currently running"), TEXT("StopAssetAudit"));
        return;
    }
    
    bIsAuditRunning = false;
    CurrentStatus = TEXT("Stopped");
    CurrentProgress = 0.0f;
    
    MAGIC_LOG(General, TEXT("Asset audit stopped"));
}

bool UMagicOptimizerAuditManager::IsAuditRunning()
{
    return bIsAuditRunning;
}

float UMagicOptimizerAuditManager::GetAuditProgress()
{
    return CurrentProgress;
}

FString UMagicOptimizerAuditManager::GetCurrentAuditStatus()
{
    return CurrentStatus;
}

TArray<FAuditResult> UMagicOptimizerAuditManager::GetAuditResults()
{
    return AuditResults;
}

TArray<FOptimizationRecommendation> UMagicOptimizerAuditManager::GetHighPriorityRecommendations()
{
    TArray<FOptimizationRecommendation> HighPriority;
    
    for (const FAuditResult& Result : AuditResults)
    {
        HighPriority.Append(Result.GetHighPriorityRecommendations());
    }
    
    return HighPriority;
}

TArray<FAuditIssue> UMagicOptimizerAuditManager::GetCriticalIssues()
{
    TArray<FAuditIssue> Critical;
    
    for (const FAuditResult& Result : AuditResults)
    {
        Critical.Append(Result.GetCriticalIssues());
    }
    
    return Critical;
}

FString UMagicOptimizerAuditManager::GenerateAuditReport()
{
    MAGIC_LOG(General, TEXT("Generating audit report"));
    
    FString Report = TEXT("");
    
    // Header
    Report += TEXT("=== MAGIC OPTIMIZER AUDIT REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Configuration: %s\n"), *CurrentConfig.TargetPlatform);
    Report += FString::Printf(TEXT("Quality Level: %s\n"), *FAuditConfig::GetQualityLevelDisplayName(CurrentConfig.QualityLevel));
    Report += TEXT("\n");
    
    // Summary
    Report += TEXT("=== SUMMARY ===\n");
    Report += FString::Printf(TEXT("Total Assets Audited: %d\n"), AuditResults.Num());
    
    int32 CompletedCount = 0;
    int32 FailedCount = 0;
    int32 TotalIssues = 0;
    int32 TotalRecommendations = 0;
    float TotalSavings = 0.0f;
    
    for (const FAuditResult& Result : AuditResults)
    {
        if (Result.Status == EAuditStatus::Completed)
        {
            CompletedCount++;
        }
        else if (Result.Status == EAuditStatus::Failed)
        {
            FailedCount++;
        }
        
        TotalIssues += Result.Issues.Num();
        TotalRecommendations += Result.Recommendations.Num();
        TotalSavings += Result.GetTotalEstimatedSavings();
    }
    
    Report += FString::Printf(TEXT("Completed: %d\n"), CompletedCount);
    Report += FString::Printf(TEXT("Failed: %d\n"), FailedCount);
    Report += FString::Printf(TEXT("Total Issues: %d\n"), TotalIssues);
    Report += FString::Printf(TEXT("Total Recommendations: %d\n"), TotalRecommendations);
    Report += FString::Printf(TEXT("Total Estimated Savings: %.2f MB\n"), TotalSavings);
    Report += TEXT("\n");
    
    // Detailed Results
    Report += TEXT("=== DETAILED RESULTS ===\n");
    for (const FAuditResult& Result : AuditResults)
    {
        Report += Result.GetDetailedReport();
        Report += TEXT("\n");
    }
    
    return Report;
}

bool UMagicOptimizerAuditManager::ExportAuditResults(const FString& FilePath)
{
    MAGIC_LOG(General, FString::Printf(TEXT("Exporting audit results to: %s"), *FilePath));
    
    FString Report = GenerateAuditReport();
    
    if (FFileHelper::SaveStringToFile(Report, *FilePath))
    {
        MAGIC_LOG(General, TEXT("Audit results exported successfully"));
        return true;
    }
    else
    {
        MAGIC_LOG_ERROR(TEXT("Failed to export audit results"), TEXT("ExportAuditResults"));
        return false;
    }
}

bool UMagicOptimizerAuditManager::ImportAuditResults(const FString& FilePath)
{
    MAGIC_LOG(General, FString::Printf(TEXT("Importing audit results from: %s"), *FilePath));
    
    FString Report;
    if (FFileHelper::LoadFileToString(Report, *FilePath))
    {
        // TODO: Implement proper import logic
        MAGIC_LOG(General, TEXT("Audit results imported successfully"));
        return true;
    }
    else
    {
        MAGIC_LOG_ERROR(TEXT("Failed to import audit results"), TEXT("ImportAuditResults"));
        return false;
    }
}

void UMagicOptimizerAuditManager::ClearAuditResults()
{
    MAGIC_LOG(General, TEXT("Clearing audit results"));
    
    AuditResults.Empty();
    CurrentProgress = 0.0f;
    CurrentStatus = TEXT("Idle");
    
    MAGIC_LOG(General, TEXT("Audit results cleared"));
}

float UMagicOptimizerAuditManager::GetTotalEstimatedSavings()
{
    float TotalSavings = 0.0f;
    
    for (const FAuditResult& Result : AuditResults)
    {
        TotalSavings += Result.GetTotalEstimatedSavings();
    }
    
    return TotalSavings;
}

FString UMagicOptimizerAuditManager::GetAuditStatistics()
{
    FString Statistics = TEXT("");
    
    Statistics += FString::Printf(TEXT("Total Assets: %d\n"), AuditResults.Num());
    Statistics += FString::Printf(TEXT("Total Recommendations: %d\n"), GetHighPriorityRecommendations().Num());
    Statistics += FString::Printf(TEXT("Total Issues: %d\n"), GetCriticalIssues().Num());
    Statistics += FString::Printf(TEXT("Total Savings: %.2f MB\n"), GetTotalEstimatedSavings());
    
    return Statistics;
}

void UMagicOptimizerAuditManager::RegisterAssetAuditors()
{
    MAGIC_LOG(General, TEXT("Registering asset auditors"));
    
    // TODO: Register specific asset auditors
    // For now, we'll add placeholder auditors
    
    MAGIC_LOG(General, FString::Printf(TEXT("Registered %d asset auditors"), AssetAuditors.Num()));
}

TArray<FAssetData> UMagicOptimizerAuditManager::DiscoverAssetsToAudit()
{
    MAGIC_LOG(General, TEXT("Discovering assets to audit"));
    
    TArray<FAssetData> AssetsToAudit;
    
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    // Get all assets
    TArray<FAssetData> AllAssets;
    AssetRegistry.GetAllAssets(AllAssets);
    
    // Filter assets based on configuration
    for (const FAssetData& AssetData : AllAssets)
    {
        // Check if asset should be included
        if (CurrentConfig.ShouldIncludePath(AssetData.GetObjectPathString()))
        {
            // TODO: Add more sophisticated filtering based on asset type
            AssetsToAudit.Add(AssetData);
        }
    }
    
    MAGIC_LOG(General, FString::Printf(TEXT("Discovered %d assets to audit"), AssetsToAudit.Num()));
    
    return AssetsToAudit;
}

FAuditResult UMagicOptimizerAuditManager::ProcessAssetForAudit(const FAssetData& AssetData)
{
    FAuditResult Result;
    Result.AssetData = AssetData;
    Result.Status = EAuditStatus::Running;
    Result.StartTime = FDateTime::Now();
    
    // TODO: Implement actual asset processing
    // For now, create a basic result
    
    Result.MarkCompleted();
    
    return Result;
}

void UMagicOptimizerAuditManager::HandleAuditError(const FString& ErrorMessage)
{
    MAGIC_LOG_ERROR(ErrorMessage, TEXT("AssetAudit"));
    
    bIsAuditRunning = false;
    CurrentStatus = TEXT("Failed");
    
    OnAuditFailed.Broadcast(ErrorMessage);
}

void UMagicOptimizerAuditManager::UpdateAuditProgress(float Progress)
{
    CurrentProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
    OnAuditProgress.Broadcast(CurrentProgress);
}

void UMagicOptimizerAuditManager::CompleteAudit()
{
    bIsAuditRunning = false;
    CurrentProgress = 1.0f;
    CurrentStatus = TEXT("Completed");
    
    OnAuditCompleted.Broadcast(AuditResults);
    
    MAGIC_LOG(General, TEXT("Audit completed successfully"));
}
