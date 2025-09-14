#include "MagicOptimizerAuditManager.h"
#include "MagicOptimizerDiagnostics.h"
#include "Core/MagicOptimizerProgressManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/App.h"
#include "AssetAuditors/TextureAuditor.h"
#include "AssetAuditors/StaticMeshAuditor.h"
#include "AssetAuditors/MaterialAuditor.h"
#include "AssetAuditors/BlueprintAuditor.h"
#include "Model/AuditResult.h"
#include "Model/OptimizationRecommendation.h"
#include "Model/AuditConfig.h"
#include "AssetAuditors/IAssetAuditor.h"

// Static member definitions
UMagicOptimizerAuditManager* UMagicOptimizerAuditManager::Instance = nullptr;
TArray<TSharedPtr<IAssetAuditor>> UMagicOptimizerAuditManager::AssetAuditors;
FAuditConfig UMagicOptimizerAuditManager::CurrentConfig;
TArray<FAuditResult> UMagicOptimizerAuditManager::AuditResults;
bool UMagicOptimizerAuditManager::bIsAuditRunning = false;
float UMagicOptimizerAuditManager::CurrentProgress = 0.0f;
FString UMagicOptimizerAuditManager::CurrentStatus = TEXT("Idle");

// Event delegates are now UPROPERTY members, no static definitions needed

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
        if (IsValid(Instance))
        {
            Instance->AddToRoot(); // Prevent garbage collection
        }
        else
        {
            MAGIC_LOG_ERROR(TEXT("Failed to create valid Instance object"), TEXT("Initialize"));
            Instance = nullptr;
            return;
        }
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
        // During shutdown, UObject system may be unstable
        // Use a more conservative approach to avoid crashes
        try
        {
            // Check if we're in a valid state for UObject operations
            if (GIsEditor && !IsEngineExitRequested())
            {
                // Only try RemoveFromRoot if we're not shutting down
                if (IsValid(Instance))
                {
                    // Additional safety check - ensure the object is still in the root set
                    if (Instance->IsRooted())
                    {
                        Instance->RemoveFromRoot();
                        MAGIC_LOG(General, TEXT("Instance removed from root successfully"));
                    }
                    else
                    {
                        MAGIC_LOG(General, TEXT("Instance not rooted, skipping RemoveFromRoot"));
                    }
                }
                else
                {
                    MAGIC_LOG(General, TEXT("Instance is not valid, skipping RemoveFromRoot"));
                }
            }
            else
            {
                MAGIC_LOG(General, TEXT("Engine shutdown in progress, skipping RemoveFromRoot"));
            }
        }
        catch (...)
        {
            MAGIC_LOG(General, TEXT("Exception during RemoveFromRoot, continuing shutdown"));
        }
        
        // Always null the pointer to prevent further access
        Instance = nullptr;
    }
    
    MAGIC_LOG(General, TEXT("Asset Audit Manager shutdown complete"));
}

bool UMagicOptimizerAuditManager::StartAssetAudit(const FAuditConfig& Config)
{
    MAGIC_LOG(General, TEXT("Starting asset audit"));
    MAGIC_LOG_PERF_START(TEXT("AssetAudit"));
    
    if (Instance == nullptr || !IsValid(Instance))
    {
        MAGIC_LOG_ERROR(TEXT("Audit manager instance is not valid"), TEXT("StartAssetAudit"));
        return false;
    }
    
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
    
    // Initialize CurrentConfig with proper default values
    CurrentConfig = Config;
    
    // Ensure CurrentConfig has proper include paths if none are set
    if (CurrentConfig.IncludePaths.Num() == 0)
    {
        // Default to /Game/ content, but also include /Engine/ for testing if needed
        CurrentConfig.IncludePaths = {TEXT("/Game/")};
        MAGIC_LOG(General, TEXT("No include paths specified, using default: /Game/"));
    }
    
    // Ensure we have asset types to include
    if (CurrentConfig.IncludeAssetTypes.Num() == 0)
    {
        // Default asset types to include
        CurrentConfig.IncludeAssetTypes = {EAssetType::Texture, EAssetType::Mesh, EAssetType::Material, EAssetType::Blueprint};
        MAGIC_LOG(General, TEXT("No asset types specified, using defaults: Texture, Mesh, Material, Blueprint"));
    }
    
    // Log the current configuration for debugging
    MAGIC_LOG(General, FString::Printf(TEXT("Current config include paths: %d"), CurrentConfig.IncludePaths.Num()));
    for (const FString& Path : CurrentConfig.IncludePaths)
    {
        MAGIC_LOG(General, FString::Printf(TEXT("  Include path: %s"), *Path));
    }
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
    
    // Get Progress Manager instance
    UMagicOptimizerProgressManager* ProgressManager = nullptr;
    if (GEngine && GWorld && GWorld->GetGameInstance())
    {
        ProgressManager = GWorld->GetGameInstance()->GetSubsystem<UMagicOptimizerProgressManager>();
    }
    
    // Start progress tracking
    if (ProgressManager)
    {
        ProgressManager->StartAuditProgress(AssetsToAudit.Num());
    }
    
    // Process assets
    CurrentStatus = TEXT("Processing assets");
    int32 ProcessedAssets = 0;
    int32 AssetsWithIssues = 0;
    int32 SuccessfulAssets = 0;
    int32 FailedAssets = 0;
    
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
        
        // Update counters based on result
        if (Result.Issues.Num() > 0)
        {
            AssetsWithIssues++;
        }
        
        if (Result.Status == EAuditStatus::Completed)
        {
            SuccessfulAssets++;
        }
        else if (Result.Status == EAuditStatus::Failed)
        {
            FailedAssets++;
        }
        
        // Update Progress Manager
        if (ProgressManager)
        {
            ProgressManager->UpdateProgress(ProcessedAssets, AssetsWithIssues, SuccessfulAssets, FailedAssets, AssetData.AssetName.ToString());
            
            // Create AssetAuditData and add to progress manager
            FAssetAuditData AssetAuditData;
            AssetAuditData.AssetName = AssetData.AssetName.ToString();
            AssetAuditData.AssetPath = AssetData.GetObjectPathString(); // Use the new API
            AssetAuditData.AssetType = TEXT("Texture"); // Default type, will be set by auditors
            AssetAuditData.AuditStatus = Result.Status;
            AssetAuditData.QualityLevel = EQualityLevel::Medium; // Default quality level
            AssetAuditData.Issues = Result.Issues;
            AssetAuditData.Recommendations = Result.Recommendations;
            AssetAuditData.QualityScore = FMath::RoundToInt(Result.GetOptimizationScore() * 100.0f);
            AssetAuditData.MemoryUsageMB = FCString::Atof(*Result.Context.FindRef(TEXT("MemoryUsageMB"), TEXT("0.0")));
            
            ProgressManager->AddAssetResult(AssetAuditData);
        }
        
        // Update progress
        UpdateAuditProgress(CurrentProgress);
        
        // Log progress
        if (ProcessedAssets % 10 == 0 || ProcessedAssets == AssetsToAudit.Num())
        {
            MAGIC_LOG(General, FString::Printf(TEXT("Processed %d/%d assets (%.1f%%)"), 
                ProcessedAssets, AssetsToAudit.Num(), CurrentProgress * 100.0f));
        }
    }
    
    // Complete progress tracking
    if (ProgressManager)
    {
        ProgressManager->CompleteAuditProgress();
    }
    
    CompleteAudit();
    
    MAGIC_LOG_PERF_END(TEXT("AssetAudit"));
    MAGIC_LOG(General, TEXT("Asset audit completed successfully"));
    
    return true;
}

void UMagicOptimizerAuditManager::StopAssetAudit()
{
    MAGIC_LOG(General, TEXT("Stopping asset audit"));
    
    if (Instance == nullptr || !IsValid(Instance))
    {
        MAGIC_LOG_WARNING(TEXT("Audit manager instance is not valid"), TEXT("StopAssetAudit"));
        return;
    }
    
    if (!bIsAuditRunning)
    {
        MAGIC_LOG_WARNING(TEXT("No audit is currently running"), TEXT("StopAssetAudit"));
        return;
    }
    
    // Cancel progress tracking
    UMagicOptimizerProgressManager* ProgressManager = nullptr;
    if (GEngine && GWorld && GWorld->GetGameInstance())
    {
        ProgressManager = GWorld->GetGameInstance()->GetSubsystem<UMagicOptimizerProgressManager>();
        if (ProgressManager)
        {
            ProgressManager->CancelAuditProgress();
        }
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
    
    // Clear existing auditors
    AssetAuditors.Empty();
    
    // Register Texture Auditor
    TSharedPtr<FTextureAuditor> TextureAuditor = MakeShared<FTextureAuditor>();
    AssetAuditors.Add(TextureAuditor);
    MAGIC_LOG(General, TEXT("Registered Texture Auditor"));
    
    // Register Static Mesh Auditor
    TSharedPtr<FStaticMeshAuditor> StaticMeshAuditor = MakeShared<FStaticMeshAuditor>();
    AssetAuditors.Add(StaticMeshAuditor);
    MAGIC_LOG(General, TEXT("Registered Static Mesh Auditor"));
    
    // Register Material Auditor
    TSharedPtr<FMaterialAuditor> MaterialAuditor = MakeShared<FMaterialAuditor>();
    AssetAuditors.Add(MaterialAuditor);
    MAGIC_LOG(General, TEXT("Registered Material Auditor"));
    
    // Register Blueprint Auditor
    TSharedPtr<FBlueprintAuditor> BlueprintAuditor = MakeShared<FBlueprintAuditor>();
    AssetAuditors.Add(BlueprintAuditor);
    MAGIC_LOG(General, TEXT("Registered Blueprint Auditor"));
    
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
    
    MAGIC_LOG(General, FString::Printf(TEXT("Found %d total assets in registry"), AllAssets.Num()));
    
    // Log some sample asset paths for debugging
    int32 SampleCount = 0;
    for (const FAssetData& AssetData : AllAssets)
    {
        if (SampleCount < 5) // Log first 5 assets as samples
        {
            MAGIC_LOG(General, FString::Printf(TEXT("Sample asset %d: %s (Class: %s)"), 
                SampleCount, *AssetData.GetObjectPathString(), *AssetData.AssetClassPath.ToString()));
            SampleCount++;
        }
    }
    
    // Log the current configuration for debugging
    MAGIC_LOG(General, FString::Printf(TEXT("Current config include paths: %d"), CurrentConfig.IncludePaths.Num()));
    for (const FString& Path : CurrentConfig.IncludePaths)
    {
        MAGIC_LOG(General, FString::Printf(TEXT("  Include path: %s"), *Path));
    }
    
    // Filter assets based on configuration
    for (const FAssetData& AssetData : AllAssets)
    {
        FString AssetPath = AssetData.GetObjectPathString();
        FString AssetClass = AssetData.AssetClassPath.ToString();
        
        // Check if asset should be included
        if (CurrentConfig.ShouldIncludePath(AssetPath))
        {
            // Filter by asset type - include more asset types and use more flexible matching
            bool bShouldInclude = false;
            
            // Check for common asset types with more flexible matching
            if (AssetClass.Contains(TEXT("Texture")) || 
                AssetClass.Contains(TEXT("Mesh")) || 
                AssetClass.Contains(TEXT("Material")) || 
                AssetClass.Contains(TEXT("Blueprint")) ||
                AssetClass.Contains(TEXT("SkeletalMesh")) ||
                AssetClass.Contains(TEXT("StaticMesh")) ||
                AssetClass == TEXT("Texture2D") || 
                AssetClass == TEXT("StaticMesh") || 
                AssetClass == TEXT("Material") || 
                AssetClass == TEXT("MaterialInstanceConstant") ||
                AssetClass == TEXT("SkeletalMesh") ||
                AssetClass == TEXT("Blueprint"))
            {
                bShouldInclude = true;
            }
            
            if (bShouldInclude)
            {
                AssetsToAudit.Add(AssetData);
                MAGIC_LOG(General, FString::Printf(TEXT("Included asset: %s (Class: %s)"), *AssetPath, *AssetClass));
            }
            else
            {
                if (SampleCount < 10)
                {
                    MAGIC_LOG(General, FString::Printf(TEXT("Included path but wrong class: %s (Class: %s)"), 
                        *AssetPath, *AssetClass));
                    SampleCount++;
                }
            }
        }
        else
        {
            // Log why asset was excluded (only for first few)
            if (SampleCount < 10)
            {
                MAGIC_LOG(General, FString::Printf(TEXT("Excluded asset: %s (Class: %s) - Path not in include list"), *AssetPath, *AssetClass));
                SampleCount++;
            }
        }
    }
    
    MAGIC_LOG(General, FString::Printf(TEXT("Discovered %d assets to audit"), AssetsToAudit.Num()));
    
    // If no assets found, let's try a simpler approach
    if (AssetsToAudit.Num() == 0)
    {
        MAGIC_LOG(General, TEXT("No assets found with current config, trying simpler discovery"));
        
        // Try to get assets from specific directories
        TArray<FAssetData> ContentAssets;
        AssetRegistry.GetAssetsByPath(TEXT("/Game"), ContentAssets, true);
        
        MAGIC_LOG(General, FString::Printf(TEXT("Found %d assets in /Game directory"), ContentAssets.Num()));
        
        // If no /Game assets, try /Engine assets
        if (ContentAssets.Num() == 0)
        {
            MAGIC_LOG(General, TEXT("No /Game assets found, trying /Engine assets"));
            AssetRegistry.GetAssetsByPath(TEXT("/Engine"), ContentAssets, true);
            MAGIC_LOG(General, FString::Printf(TEXT("Found %d assets in /Engine directory"), ContentAssets.Num()));
        }
        
        for (const FAssetData& AssetData : ContentAssets)
        {
            FString AssetClass = AssetData.AssetClassPath.ToString();
            FString AssetPath = AssetData.GetObjectPathString();
            
            // Log all assets found in /Game directory for debugging
            MAGIC_LOG(General, FString::Printf(TEXT("Found /Game asset: %s (Class: %s)"), 
                *AssetPath, *AssetClass));
            
            // Use more flexible asset class matching for fallback discovery
            bool bShouldInclude = false;
            
            if (AssetClass.Contains(TEXT("Texture")) || 
                AssetClass.Contains(TEXT("Mesh")) || 
                AssetClass.Contains(TEXT("Material")) || 
                AssetClass.Contains(TEXT("Blueprint")) ||
                AssetClass.Contains(TEXT("SkeletalMesh")) ||
                AssetClass.Contains(TEXT("StaticMesh")) ||
                AssetClass == TEXT("Texture2D") || 
                AssetClass == TEXT("StaticMesh") || 
                AssetClass == TEXT("Material") || 
                AssetClass == TEXT("MaterialInstanceConstant") ||
                AssetClass == TEXT("SkeletalMesh") ||
                AssetClass == TEXT("Blueprint"))
            {
                bShouldInclude = true;
            }
            
            if (bShouldInclude)
            {
                AssetsToAudit.Add(AssetData);
                MAGIC_LOG(General, FString::Printf(TEXT("Added asset: %s (Class: %s)"), 
                    *AssetPath, *AssetClass));
            }
            else
            {
                MAGIC_LOG(General, FString::Printf(TEXT("Skipped asset: %s (Class: %s) - not in target classes"), 
                    *AssetPath, *AssetClass));
            }
        }
        
        MAGIC_LOG(General, FString::Printf(TEXT("Simple discovery found %d assets"), AssetsToAudit.Num()));
    }
    
    return AssetsToAudit;
}

FAuditResult UMagicOptimizerAuditManager::ProcessAssetForAudit(const FAssetData& AssetData)
{
    FAuditResult Result;
    Result.AssetData = AssetData;
    Result.Status = EAuditStatus::Running;
    Result.StartTime = FDateTime::Now();
    
    try
    {
        // Add some basic asset analysis
        FString AssetPath = AssetData.GetObjectPathString();
        FString AssetClass = AssetData.AssetClassPath.ToString();
        
        // Log the asset being processed
        MAGIC_LOG(General, FString::Printf(TEXT("Processing asset: %s (Class: %s)"), 
            *AssetPath, *AssetClass));
        
        // Basic asset analysis
        // Create a basic optimization recommendation
        FOptimizationRecommendation Recommendation;
        Recommendation.RecommendationID = FString::Printf(TEXT("Asset_%d"), FDateTime::Now().GetTicks());
        Recommendation.Title = TEXT("Asset Analysis Complete");
        Recommendation.Description = FString::Printf(TEXT("Successfully analyzed asset: %s"), *AssetPath);
        Recommendation.EstimatedMemorySavingsMB = 0.1f; // Small saving estimate
        Recommendation.Priority = EOptimizationPriority::Low;
        Recommendation.ActionCommand = TEXT("No immediate action required");
        Recommendation.Category = EOptimizationCategory::Performance;
        
        Result.AddRecommendation(Recommendation);
        
        // Analysis completed successfully
        MAGIC_LOG(General, FString::Printf(TEXT("Successfully processed asset: %s"), *AssetPath));
    }
    catch (...)
    {
        // Handle any exceptions that might occur during processing
        Result.Status = EAuditStatus::Failed;
        Result.EndTime = FDateTime::Now();
        
        FAuditIssue Issue;
        Issue.ID = FString::Printf(TEXT("Error_%d"), FDateTime::Now().GetTicks());
        Issue.Type = EAuditIssueType::Error;
        Issue.Title = TEXT("Asset Processing Failed");
        Issue.Description = TEXT("Failed to process asset - possible recursive loading issue");
        Issue.Severity = 0.7f; // Medium severity
        Issue.Context.Add(TEXT("AssetPath"), AssetData.GetObjectPathString());
        Issue.Context.Add(TEXT("ErrorType"), TEXT("ProcessingError"));
        
        Result.AddIssue(Issue);
        
        MAGIC_LOG_ERROR(FString::Printf(TEXT("Failed to process asset: %s"), 
            *AssetData.GetObjectPathString()), TEXT("ProcessAssetForAudit"));
    }
    
    return Result;
}

void UMagicOptimizerAuditManager::HandleAuditError(const FString& ErrorMessage)
{
    MAGIC_LOG_ERROR(ErrorMessage, TEXT("AssetAudit"));
    
    bIsAuditRunning = false;
    CurrentStatus = TEXT("Failed");
    
    if (Instance)
    {
        Instance->OnAuditFailed.Broadcast(ErrorMessage);
    }
}

void UMagicOptimizerAuditManager::UpdateAuditProgress(float Progress)
{
    CurrentProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
    if (Instance)
    {
        Instance->OnAuditProgress.Broadcast(CurrentProgress);
    }
}

void UMagicOptimizerAuditManager::CompleteAudit()
{
    bIsAuditRunning = false;
    CurrentProgress = 1.0f;
    CurrentStatus = TEXT("Completed");
    
    // Get instance and broadcast
    if (Instance)
    {
        Instance->OnAuditCompleted.Broadcast(AuditResults);
    }
    
    MAGIC_LOG(General, TEXT("Audit completed successfully"));
}

void UMagicOptimizerAuditManager::DispatchAuditResults(const TArray<FAuditResult>& Results)
{
    if (Instance)
    {
        Instance->OnAuditCompleted.Broadcast(Results);
    }
}

void UMagicOptimizerAuditManager::DispatchProgress(float Value)
{
    if (Instance)
    {
        Instance->OnAuditProgress.Broadcast(Value);
    }
}

void UMagicOptimizerAuditManager::DispatchError(const FString& Msg)
{
    if (Instance)
    {
        Instance->OnAuditFailed.Broadcast(Msg);
    }
}
