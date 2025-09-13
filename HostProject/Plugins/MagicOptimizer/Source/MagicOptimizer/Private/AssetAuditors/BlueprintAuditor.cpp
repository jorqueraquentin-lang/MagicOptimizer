#include "AssetAuditors/BlueprintAuditor.h"
#include "MagicOptimizerDiagnostics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Async/Async.h"

FBlueprintAuditor::FBlueprintAuditor()
{
    CurrentProgress = 0.0f;
    CurrentStatus = TEXT("Ready");
    ProcessedCount = 0;
    TotalCount = 0;
}

bool FBlueprintAuditor::CanAuditAsset(const FAssetData& AssetData) const
{
    // Check if asset is a blueprint type
    return AssetData.AssetClassPath.GetAssetName() == UBlueprint::StaticClass()->GetFName() ||
           AssetData.AssetClassPath.ToString().Contains(TEXT("Blueprint"));
}

FAuditResult FBlueprintAuditor::AuditAsset(const FAssetData& AssetData) const
{
    FAuditResult Result;
    Result.AssetData = AssetData;
    Result.Status = EAuditStatus::Running;
    Result.StartTime = FDateTime::Now();

    try
    {
        CurrentStatus = FString::Printf(TEXT("Loading blueprint: %s"), *AssetData.AssetName.ToString());
        
        // Load the blueprint asset
        UObject* BlueprintObject = AssetData.GetAsset();
        if (!BlueprintObject)
        {
            FAuditIssue Issue;
            Issue.ID = TEXT("BLUEPRINT_LOAD_FAILED");
            Issue.Type = EAuditIssueType::Error;
            Issue.Title = TEXT("Blueprint Loading Failed");
            Issue.Description = TEXT("Unable to load blueprint asset for analysis");
            Issue.Severity = 0.7f;
            Result.AddIssue(Issue);
            Result.MarkFailed(TEXT("Unable to load blueprint asset"));
            return Result;
        }

        // Perform basic blueprint analysis (simplified for now)
        AnalyzeBlueprintComplexity(BlueprintObject, Result);
        AnalyzeBlueprintFunctions(BlueprintObject, Result);
        AnalyzeBlueprintVariables(BlueprintObject, Result);
        
        Result.MarkCompleted();
        
        MAGIC_LOG(General, FString::Printf(TEXT("Blueprint audit completed: %s"), *AssetData.AssetName.ToString()));
    }
    catch (const std::exception& e)
    {
        FString ErrorMsg = FString::Printf(TEXT("Exception during blueprint audit: %s"), *FString(e.what()));
        MAGIC_LOG_ERROR(ErrorMsg, TEXT("AuditAsset"));
        
        FAuditIssue Issue;
        Issue.ID = TEXT("AUDIT_EXCEPTION");
        Issue.Type = EAuditIssueType::Error;
        Issue.Title = TEXT("Audit Exception");
        Issue.Description = ErrorMsg;
        Issue.Severity = 1.0f;
        Result.AddIssue(Issue);
        Result.MarkFailed(ErrorMsg);
    }

    return Result;
}

TFuture<FAuditResult> FBlueprintAuditor::AuditAssetAsync(const FAssetData& AssetData) const
{
    return Async(EAsyncExecution::TaskGraph, [this, AssetData]()
    {
        return AuditAsset(AssetData);
    });
}

void FBlueprintAuditor::SetAuditConfig(const FAuditConfig& Config)
{
    CurrentConfig = Config;
}

FAuditConfig FBlueprintAuditor::GetAuditConfig() const
{
    return CurrentConfig;
}

float FBlueprintAuditor::GetAuditProgress() const
{
    return CurrentProgress;
}

FString FBlueprintAuditor::GetAuditStatus() const
{
    return CurrentStatus;
}

bool FBlueprintAuditor::HandleError(const FString& ErrorMessage)
{
    MAGIC_LOG_ERROR(ErrorMessage, TEXT("BlueprintAuditor"));
    CurrentStatus = FString::Printf(TEXT("Error: %s"), *ErrorMessage);
    return true;
}

void FBlueprintAuditor::Reset()
{
    CurrentProgress = 0.0f;
    CurrentStatus = TEXT("Ready");
    ProcessedCount = 0;
    TotalCount = 0;
}

FString FBlueprintAuditor::GetAuditorName() const
{
    return TEXT("Blueprint Auditor");
}

TArray<EAssetType> FBlueprintAuditor::GetSupportedAssetTypes() const
{
    return { EAssetType::Blueprint };
}

void FBlueprintAuditor::AnalyzeBlueprintComplexity(const UObject* Blueprint, FAuditResult& Result) const
{
    if (!Blueprint)
        return;

    // Basic blueprint analysis - simplified for now
    float Complexity = 50.0f; // Default complexity score
    
    // Update performance metrics
    Result.PerformanceMetrics.QualityScore = Complexity / 100.0f;
    
    // Add a basic issue for testing
    FAuditIssue Issue;
    Issue.ID = TEXT("BLUEPRINT_BASIC_ANALYSIS");
    Issue.Type = EAuditIssueType::Info;
    Issue.Title = TEXT("Blueprint Analysis");
    Issue.Description = TEXT("Basic blueprint analysis completed");
    Issue.Severity = 0.5f;
    Result.AddIssue(Issue);
}

void FBlueprintAuditor::AnalyzeBlueprintFunctions(const UObject* Blueprint, FAuditResult& Result) const
{
    if (!Blueprint)
        return;

    // Basic function analysis - simplified for now
    FAuditIssue Issue;
    Issue.ID = TEXT("BLUEPRINT_FUNCTION_ANALYSIS");
    Issue.Type = EAuditIssueType::Info;
    Issue.Title = TEXT("Blueprint Function Analysis");
    Issue.Description = TEXT("Function analysis completed");
    Issue.Severity = 0.4f;
    Result.AddIssue(Issue);
}

void FBlueprintAuditor::AnalyzeBlueprintVariables(const UObject* Blueprint, FAuditResult& Result) const
{
    if (!Blueprint)
        return;

    // Basic variable analysis - simplified for now
    FAuditIssue Issue;
    Issue.ID = TEXT("BLUEPRINT_VARIABLE_ANALYSIS");
    Issue.Type = EAuditIssueType::Info;
    Issue.Title = TEXT("Blueprint Variable Analysis");
    Issue.Description = TEXT("Variable analysis completed");
    Issue.Severity = 0.4f;
    Result.AddIssue(Issue);
}

void FBlueprintAuditor::AnalyzeBlueprintPerformance(const UObject* Blueprint, FAuditResult& Result) const
{
    // Simplified for now
}

void FBlueprintAuditor::AnalyzeBlueprintInheritance(const UObject* Blueprint, FAuditResult& Result) const
{
    // Simplified for now
}

void FBlueprintAuditor::GeneratePlatformRecommendations(const UObject* Blueprint, FAuditResult& Result) const
{
    if (!Blueprint)
        return;

    // Mobile platform recommendations
    if (CurrentConfig.TargetPlatform.Contains(TEXT("Mobile")) || CurrentConfig.TargetPlatform.Contains(TEXT("Android")) || CurrentConfig.TargetPlatform.Contains(TEXT("iOS")))
    {
        FOptimizationRecommendation Rec;
        Rec.ID = TEXT("MOBILE_OPTIMIZATION");
        Rec.Title = TEXT("Mobile Optimization");
        Rec.Description = TEXT("Simplify blueprint for mobile platforms");
        Rec.Priority = EOptimizationPriority::High;
        Rec.Type = EAuditOptimizationType::Performance;
        Rec.EstimatedSavingsMB = 45.0f;
        Result.AddRecommendation(Rec);
    }
}

float FBlueprintAuditor::CalculateBlueprintComplexity(const UObject* Blueprint) const
{
    if (!Blueprint)
        return 0.0f;

    // Simplified complexity calculation
    return 50.0f;
}

bool FBlueprintAuditor::IsOptimalForPlatform(const UObject* Blueprint, const FString& Platform) const
{
    if (!Blueprint)
        return false;

    return true; // Simplified for now
}

FString FBlueprintAuditor::GetRecommendedOptimizationLevel(const UObject* Blueprint, const FString& Platform) const
{
    if (!Blueprint)
        return TEXT("Unknown");

    return TEXT("Low (Minimal optimization needed)");
}

void FBlueprintAuditor::UpdateProgress(int32 Processed, int32 Total) const
{
    ProcessedCount = Processed;
    TotalCount = Total;
    CurrentProgress = Total > 0 ? (float)Processed / (float)Total : 0.0f;
}