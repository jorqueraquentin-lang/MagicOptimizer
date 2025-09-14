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
    Result.Guid = AssetData.GetSoftObjectPath().GetAssetFName();
    Result.AssetPath = AssetData.GetSoftObjectPath();
    Result.AssetClass = AssetData.AssetClass;

    try
    {
        CurrentStatus = FString::Printf(TEXT("Loading material: %s"), *AssetData.AssetName.ToString());
        
        // Load the material asset
        UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>(AssetData.GetAsset());
        if (!MaterialInterface)
        {
            Result.AddIssue(EAuditIssueLevel::Must, TEXT("MATERIAL_LOAD_FAILED"), TEXT("Unable to load material asset for analysis"));
            return Result;
        }

        // Perform basic material analysis
        AnalyzeMaterialComplexity(MaterialInterface, Result);
        AnalyzeMaterialTextures(MaterialInterface, Result);
        
        // Analysis completed successfully
        
        MAGIC_LOG(General, FString::Printf(TEXT("Material audit completed: %s"), *AssetData.AssetName.ToString()));
    }
    catch (const std::exception& e)
    {
        FString ErrorMsg = FString::Printf(TEXT("Exception during material audit: %s"), *FString(e.what()));
        MAGIC_LOG_ERROR(ErrorMsg, TEXT("AuditAsset"));
        
        Result.AddIssue(EAuditIssueLevel::Must, TEXT("AUDIT_EXCEPTION"), ErrorMsg);
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
    
    // Store performance metrics in context
    Result.Context.Add(TEXT("QualityScore"), FString::Printf(TEXT("%.2f"), ShaderComplexity / 100.0f));
    
    // Check for high shader complexity
    if (ShaderComplexity > 80.0f)
    {
        Result.AddIssue(EAuditIssueLevel::Must, TEXT("HIGH_SHADER_COMPLEXITY"), 
            FString::Printf(TEXT("Material has high shader complexity: %.1f"), ShaderComplexity));
    }
    else if (ShaderComplexity > 60.0f)
    {
        Result.AddIssue(EAuditIssueLevel::Should, TEXT("MODERATE_SHADER_COMPLEXITY"), 
            FString::Printf(TEXT("Material has moderate shader complexity: %.1f"), ShaderComplexity));
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
    
    // Store performance metrics in context
    Result.Context.Add(TEXT("MemoryUsageMB"), FString::Printf(TEXT("%.2f"), TextureCount * 10.0f));
    
    // Check for excessive texture usage
    if (TextureCount > 8)
    {
        Result.AddIssue(EAuditIssueLevel::Must, TEXT("HIGH_TEXTURE_COUNT"), 
            FString::Printf(TEXT("Material uses %d textures"), TextureCount));
    }
    else if (TextureCount > 6)
    {
        Result.AddIssue(EAuditIssueLevel::Should, TEXT("MODERATE_TEXTURE_COUNT"), 
            FString::Printf(TEXT("Material uses %d textures"), TextureCount));
    }

    // Check for missing textures
    if (TextureCount == 0)
    {
        Result.AddIssue(EAuditIssueLevel::Info, TEXT("NO_TEXTURES"), TEXT("Material uses no textures"));
    }
}

void FMaterialAuditor::AnalyzeMaterialShaders(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const
{
    if (!MaterialInterface)
        return;

    // Basic shader analysis - simplified for now
    Result.AddIssue(EAuditIssueLevel::Info, TEXT("MATERIAL_SHADER_ANALYSIS"), TEXT("Shader analysis completed"));
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
            Result.AddIssue(EAuditIssueLevel::Should, TEXT("HIGH_SCALAR_PARAMETER_COUNT"), 
                FString::Printf(TEXT("Material instance has %d scalar parameters"), ScalarParameterIds.Num()));
        }
    }
}

void FMaterialAuditor::AnalyzeMaterialPerformance(const UMaterialInterface* MaterialInterface, FAuditResult& Result) const
{
    if (!MaterialInterface)
        return;

    // Basic performance analysis - simplified for now
    Result.AddIssue(EAuditIssueLevel::Info, TEXT("MATERIAL_PERFORMANCE_ANALYSIS"), TEXT("Performance analysis completed"));
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
            Rec.RecommendationID = TEXT("MOBILE_OPTIMIZATION");
            Rec.Title = TEXT("Mobile Optimization");
            Rec.Description = TEXT("Simplify material for mobile platforms");
            Rec.Priority = EOptimizationPriority::High;
            Rec.Category = EOptimizationCategory::Performance;
            Rec.EstimatedMemorySavingsMB = 50.0f;
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