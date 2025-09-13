#include "AssetAuditors/MaterialAuditor.h"
#include "MagicOptimizerDiagnostics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Async/Async.h"

FMaterialAuditor::FMaterialAuditor()
{
    CurrentProgress = 0.0f;
    CurrentStatus = TEXT("Ready");
    ProcessedCount = 0;
    TotalCount = 0;
}

bool FMaterialAuditor::CanAuditAsset(const FAssetData& AssetData) const
{
    // Check if asset is a material type
    return AssetData.AssetClassPath.GetAssetName() == UMaterial::StaticClass()->GetFName() ||
           AssetData.AssetClassPath.GetAssetName() == UMaterialInstance::StaticClass()->GetFName() ||
           AssetData.AssetClassPath.ToString().Contains(TEXT("Material"));
}

FAuditResult FMaterialAuditor::AuditAsset(const FAssetData& AssetData) const
{
    FAuditResult Result;
    Result.AssetData = AssetData;
    Result.Status = EAuditStatus::Running;
    Result.StartTime = FDateTime::Now();

    try
    {
        CurrentStatus = FString::Printf(TEXT("Loading material: %s"), *AssetData.AssetName.ToString());
        
        // Load the material asset
        UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>(AssetData.GetAsset());
        if (!MaterialInterface)
        {
            FAuditIssue Issue;
            Issue.ID = TEXT("MATERIAL_LOAD_FAILED");
            Issue.Type = EAuditIssueType::Error;
            Issue.Title = TEXT("Material Loading Failed");
            Issue.Description = TEXT("Unable to load material asset for analysis");
            Issue.Severity = 0.7f;
            Result.AddIssue(Issue);
            Result.MarkFailed(TEXT("Unable to load material asset"));
            return Result;
        }

        // Perform basic material analysis
        AnalyzeMaterialComplexity(MaterialInterface, Result);
        AnalyzeMaterialTextures(MaterialInterface, Result);
        
        Result.MarkCompleted();
        
        MAGIC_LOG(General, FString::Printf(TEXT("Material audit completed: %s"), *AssetData.AssetName.ToString()));
    }
    catch (const std::exception& e)
    {
        FString ErrorMsg = FString::Printf(TEXT("Exception during material audit: %s"), *FString(e.what()));
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

TFuture<FAuditResult> FMaterialAuditor::AuditAssetAsync(const FAssetData& AssetData) const
{
    return Async(EAsyncExecution::TaskGraph, [this, AssetData]()
    {
        return AuditAsset(AssetData);
    });
}

void FMaterialAuditor::SetAuditConfig(const FAuditConfig& Config)
{
    CurrentConfig = Config;
}

FAuditConfig FMaterialAuditor::GetAuditConfig() const
{
    return CurrentConfig;
}

float FMaterialAuditor::GetAuditProgress() const
{
    return CurrentProgress;
}

FString FMaterialAuditor::GetAuditStatus() const
{
    return CurrentStatus;
}

bool FMaterialAuditor::HandleError(const FString& ErrorMessage)
{
    MAGIC_LOG_ERROR(ErrorMessage, TEXT("MaterialAuditor"));
    CurrentStatus = FString::Printf(TEXT("Error: %s"), *ErrorMessage);
    return true;
}

void FMaterialAuditor::Reset()
{
    CurrentProgress = 0.0f;
    CurrentStatus = TEXT("Ready");
    ProcessedCount = 0;
    TotalCount = 0;
}

FString FMaterialAuditor::GetAuditorName() const
{
    return TEXT("Material Auditor");
}

TArray<EAssetType> FMaterialAuditor::GetSupportedAssetTypes() const
{
    return { EAssetType::Material };
}

void FMaterialAuditor::AnalyzeMaterialComplexity(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const
{
    if (!MaterialInterface)
        return;

    // Get material complexity information
    float ShaderComplexity = CalculateShaderComplexity(MaterialInterface);
    
    // Update performance metrics
    Result.PerformanceMetrics.QualityScore = ShaderComplexity / 100.0f;
    
    // Check for high shader complexity
    if (ShaderComplexity > 80.0f)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("HIGH_SHADER_COMPLEXITY");
        Issue.Type = EAuditIssueType::Error;
        Issue.Title = TEXT("High Shader Complexity");
        Issue.Description = FString::Printf(TEXT("Material has high shader complexity: %.1f"), ShaderComplexity);
        Issue.Severity = 0.8f;
        Result.AddIssue(Issue);
    }
    else if (ShaderComplexity > 60.0f)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("MODERATE_SHADER_COMPLEXITY");
        Issue.Type = EAuditIssueType::Warning;
        Issue.Title = TEXT("Moderate Shader Complexity");
        Issue.Description = FString::Printf(TEXT("Material has moderate shader complexity: %.1f"), ShaderComplexity);
        Issue.Severity = 0.6f;
        Result.AddIssue(Issue);
    }
}

void FMaterialAuditor::AnalyzeMaterialTextures(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const
{
    if (!MaterialInterface)
        return;

    // Get texture usage information
    TArray<UTexture*> UsedTextures;
    MaterialInterface->GetUsedTextures(UsedTextures, EMaterialQualityLevel::Num, true, ERHIFeatureLevel::Num, true);
    
    int32 TextureCount = UsedTextures.Num();
    
    // Update performance metrics
    Result.PerformanceMetrics.MemoryUsageMB = TextureCount * 10.0f; // Rough estimate
    
    // Check for excessive texture usage
    if (TextureCount > 8)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("HIGH_TEXTURE_COUNT");
        Issue.Type = EAuditIssueType::Error;
        Issue.Title = TEXT("High Texture Count");
        Issue.Description = FString::Printf(TEXT("Material uses %d textures"), TextureCount);
        Issue.Severity = 0.8f;
        Result.AddIssue(Issue);
    }
    else if (TextureCount > 6)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("MODERATE_TEXTURE_COUNT");
        Issue.Type = EAuditIssueType::Warning;
        Issue.Title = TEXT("Moderate Texture Count");
        Issue.Description = FString::Printf(TEXT("Material uses %d textures"), TextureCount);
        Issue.Severity = 0.6f;
        Result.AddIssue(Issue);
    }

    // Check for missing textures
    if (TextureCount == 0)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("NO_TEXTURES");
        Issue.Type = EAuditIssueType::Info;
        Issue.Title = TEXT("No Textures");
        Issue.Description = TEXT("Material uses no textures");
        Issue.Severity = 0.3f;
        Result.AddIssue(Issue);
    }
}

void FMaterialAuditor::AnalyzeMaterialShaders(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const
{
    if (!MaterialInterface)
        return;

    // Basic shader analysis - simplified for now
    FAuditIssue Issue;
    Issue.ID = TEXT("MATERIAL_SHADER_ANALYSIS");
    Issue.Type = EAuditIssueType::Info;
    Issue.Title = TEXT("Material Shader Analysis");
    Issue.Description = TEXT("Shader analysis completed");
    Issue.Severity = 0.4f;
    Result.AddIssue(Issue);
}

void FMaterialAuditor::AnalyzeMaterialParameters(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const
{
    if (!MaterialInterface)
        return;

    // Check for material instance parameters
    if (const UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(MaterialInterface))
    {
        // Check for excessive scalar parameters
        TArray<FMaterialParameterInfo> ScalarParameterInfos;
        TArray<FGuid> ScalarParameterIds;
        MaterialInstance->GetAllScalarParameterInfo(ScalarParameterInfos, ScalarParameterIds);
        
        if (ScalarParameterIds.Num() > 20)
        {
            FAuditIssue Issue;
            Issue.ID = TEXT("HIGH_SCALAR_PARAMETER_COUNT");
            Issue.Type = EAuditIssueType::Warning;
            Issue.Title = TEXT("High Scalar Parameter Count");
            Issue.Description = FString::Printf(TEXT("Material instance has %d scalar parameters"), ScalarParameterIds.Num());
            Issue.Severity = 0.6f;
            Result.AddIssue(Issue);
        }
    }
}

void FMaterialAuditor::AnalyzeMaterialPerformance(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const
{
    if (!MaterialInterface)
        return;

    // Basic performance analysis - simplified for now
    FAuditIssue Issue;
    Issue.ID = TEXT("MATERIAL_PERFORMANCE_ANALYSIS");
    Issue.Type = EAuditIssueType::Info;
    Issue.Title = TEXT("Material Performance Analysis");
    Issue.Description = TEXT("Performance analysis completed");
    Issue.Severity = 0.4f;
    Result.AddIssue(Issue);
}

void FMaterialAuditor::GeneratePlatformRecommendations(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const
{
    if (!MaterialInterface)
        return;

    float ShaderComplexity = CalculateShaderComplexity(MaterialInterface);

    // Mobile platform recommendations
    if (CurrentConfig.TargetPlatform.Contains(TEXT("Mobile")) || CurrentConfig.TargetPlatform.Contains(TEXT("Android")) || CurrentConfig.TargetPlatform.Contains(TEXT("iOS")))
    {
        if (ShaderComplexity > 40.0f)
        {
            FOptimizationRecommendation Rec;
            Rec.ID = TEXT("MOBILE_OPTIMIZATION");
            Rec.Title = TEXT("Mobile Optimization");
            Rec.Description = TEXT("Simplify material for mobile platforms");
            Rec.Priority = EOptimizationPriority::High;
            Rec.Type = EAuditOptimizationType::Performance;
            Rec.EstimatedSavingsMB = 50.0f;
            Result.AddRecommendation(Rec);
        }
    }
}

float FMaterialAuditor::CalculateShaderComplexity(const UMaterialInterface* MaterialInterface) const
{
    if (!MaterialInterface)
        return 0.0f;

    // This is a simplified complexity calculation
    float Complexity = 0.0f;
    
    // Base complexity
    Complexity += 10.0f;
    
    // Add complexity for textures
    TArray<UTexture*> UsedTextures;
    MaterialInterface->GetUsedTextures(UsedTextures, EMaterialQualityLevel::Num, true, ERHIFeatureLevel::Num, true);
    Complexity += UsedTextures.Num() * 5.0f;
    
    // Add complexity for material instances
    if (const UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(MaterialInterface))
    {
        TArray<FMaterialParameterInfo> ScalarParameterInfos;
        TArray<FGuid> ScalarParameterIds;
        MaterialInstance->GetAllScalarParameterInfo(ScalarParameterInfos, ScalarParameterIds);
        Complexity += ScalarParameterIds.Num() * 2.0f;
    }
    
    // Add complexity for special features - simplified for now
    Complexity += 10.0f;
    
    return FMath::Clamp(Complexity, 0.0f, 100.0f);
}

bool FMaterialAuditor::IsOptimalForPlatform(const UMaterialInterface* MaterialInterface, const FString& Platform) const
{
    if (!MaterialInterface)
        return false;

    float ShaderComplexity = CalculateShaderComplexity(MaterialInterface);
    
    if (Platform.Contains(TEXT("Mobile")))
    {
        return ShaderComplexity <= 40.0f;
    }
    else if (Platform.Contains(TEXT("Console")))
    {
        return ShaderComplexity <= 70.0f;
    }
    else if (Platform.Contains(TEXT("PC")))
    {
        return ShaderComplexity <= 80.0f;
    }
    return ShaderComplexity <= 60.0f;
}

FString FMaterialAuditor::GetRecommendedShaderModel(const UMaterialInterface* MaterialInterface, const FString& Platform) const
{
    if (!MaterialInterface)
        return TEXT("Unknown");

    if (Platform.Contains(TEXT("Mobile")))
    {
        return TEXT("SM4 (Mobile)");
    }
    else if (Platform.Contains(TEXT("Console")))
    {
        return TEXT("SM5 (Console)");
    }
    else if (Platform.Contains(TEXT("PC")))
    {
        return TEXT("SM5 (PC)");
    }
    return TEXT("SM5 (Default)");
}

void FMaterialAuditor::UpdateProgress(int32 Processed, int32 Total) const
{
    ProcessedCount = Processed;
    TotalCount = Total;
    CurrentProgress = Total > 0 ? (float)Processed / (float)Total : 0.0f;
}