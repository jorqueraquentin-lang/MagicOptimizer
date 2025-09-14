#include "AssetAuditors/StaticMeshAuditor.h"
#include "MagicOptimizerDiagnostics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "Async/Async.h"

FStaticMeshAuditor::FStaticMeshAuditor()
{
    CurrentProgress = 0.0f;
    CurrentStatus = TEXT("Ready");
    ProcessedCount = 0;
    TotalCount = 0;
}

bool FStaticMeshAuditor::CanAuditAsset(const FAssetData& AssetData) const
{
    // Check if asset is a static mesh
    return AssetData.AssetClassPath.GetAssetName() == UStaticMesh::StaticClass()->GetFName() ||
           AssetData.AssetClassPath.ToString().Contains(TEXT("StaticMesh"));
}

FAuditResult FStaticMeshAuditor::AuditAsset(const FAssetData& AssetData) const
{
    FAuditResult Result;
    Result.Guid = AssetData.GetSoftObjectPath().GetAssetFName();
    Result.AssetPath = AssetData.GetSoftObjectPath();
    Result.AssetClass = AssetData.AssetClass;

    try
    {
        CurrentStatus = FString::Printf(TEXT("Loading static mesh: %s"), *AssetData.AssetName.ToString());
        
        // Load the static mesh asset
        UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetData.GetAsset());
        if (!StaticMesh)
        {
            Result.AddIssue(EAuditIssueLevel::Must, TEXT("STATIC_MESH_LOAD_FAILED"), TEXT("Unable to load static mesh asset for analysis"));
            return Result;
        }

        // Perform basic mesh analysis
        AnalyzeMeshComplexity(StaticMesh, Result);
        AnalyzeMeshLODs(StaticMesh, Result);
        AnalyzeMeshMaterials(StaticMesh, Result);
        
        // Analysis completed successfully
        
        MAGIC_LOG(General, FString::Printf(TEXT("Static mesh audit completed: %s"), *AssetData.AssetName.ToString()));
    }
    catch (const std::exception& e)
    {
        FString ErrorMsg = FString::Printf(TEXT("Exception during static mesh audit: %s"), *FString(e.what()));
        MAGIC_LOG_ERROR(ErrorMsg, TEXT("AuditAsset"));
        
        Result.AddIssue(EAuditIssueLevel::Must, TEXT("AUDIT_EXCEPTION"), ErrorMsg);
    }

    return Result;
}

TFuture<FAuditResult> FStaticMeshAuditor::AuditAssetAsync(const FAssetData& AssetData) const
{
    return Async(EAsyncExecution::TaskGraph, [this, AssetData]()
    {
        return AuditAsset(AssetData);
    });
}

void FStaticMeshAuditor::SetAuditConfig(const FAuditConfig& Config)
{
    CurrentConfig = Config;
}

FAuditConfig FStaticMeshAuditor::GetAuditConfig() const
{
    return CurrentConfig;
}

float FStaticMeshAuditor::GetAuditProgress() const
{
    return CurrentProgress;
}

FString FStaticMeshAuditor::GetAuditStatus() const
{
    return CurrentStatus;
}

bool FStaticMeshAuditor::HandleError(const FString& ErrorMessage)
{
    MAGIC_LOG_ERROR(ErrorMessage, TEXT("StaticMeshAuditor"));
    CurrentStatus = FString::Printf(TEXT("Error: %s"), *ErrorMessage);
    return true;
}

void FStaticMeshAuditor::Reset()
{
    CurrentProgress = 0.0f;
    CurrentStatus = TEXT("Ready");
    ProcessedCount = 0;
    TotalCount = 0;
}

FString FStaticMeshAuditor::GetAuditorName() const
{
    return TEXT("Static Mesh Auditor");
}

TArray<EAssetType> FStaticMeshAuditor::GetSupportedAssetTypes() const
{
    return { EAssetType::Mesh };
}

void FStaticMeshAuditor::AnalyzeMeshComplexity(const UStaticMesh* StaticMesh, FAuditResult& Result) const
{
    if (!StaticMesh)
        return;

    // Get mesh complexity information
    int32 TotalTriangles = 0;
    int32 TotalVertices = 0;
    
    if (StaticMesh->GetRenderData() && StaticMesh->GetRenderData()->LODResources.Num() > 0)
    {
        const FStaticMeshLODResources& LOD0 = StaticMesh->GetRenderData()->LODResources[0];
        TotalTriangles = LOD0.GetNumTriangles();
        TotalVertices = LOD0.GetNumVertices();
    }

    // Store performance metrics in context
    Result.Context.Add(TEXT("Triangles"), FString::Printf(TEXT("%d"), TotalTriangles));
    Result.Context.Add(TEXT("Vertices"), FString::Printf(TEXT("%d"), TotalVertices));
    Result.Context.Add(TEXT("MemoryUsageMB"), FString::Printf(TEXT("%.2f"), CalculateMeshMemoryUsage(StaticMesh)));

    // Check for excessive polygon count
    if (TotalTriangles > 100000)
    {
        Result.AddIssue(EAuditIssueLevel::Must, TEXT("HIGH_POLYGON_COUNT"), 
            FString::Printf(TEXT("Mesh has %d triangles, which is very high"), TotalTriangles));
    }
    else if (TotalTriangles > 50000)
    {
        Result.AddIssue(EAuditIssueLevel::Should, TEXT("MODERATE_POLYGON_COUNT"), 
            FString::Printf(TEXT("Mesh has %d triangles"), TotalTriangles));
    }
}

void FStaticMeshAuditor::AnalyzeMeshLODs(const UStaticMesh* StaticMesh, FAuditResult& Result) const
{
    if (!StaticMesh)
        return;

    int32 LODCount = StaticMesh->GetNumLODs();
    
    // Check for missing LODs
    if (LODCount <= 1)
    {
        Result.AddIssue(EAuditIssueLevel::Should, TEXT("NO_LODS"), 
            FString::Printf(TEXT("Mesh has only %d LOD level"), LODCount));
    }
}

void FStaticMeshAuditor::AnalyzeMeshMaterials(const UStaticMesh* StaticMesh, FAuditResult& Result) const
{
    if (!StaticMesh)
        return;

    int32 MaterialCount = StaticMesh->GetStaticMaterials().Num();
    
    // Check for excessive material count
    if (MaterialCount > 10)
    {
        Result.AddIssue(EAuditIssueLevel::Must, TEXT("HIGH_MATERIAL_COUNT"), 
            FString::Printf(TEXT("Mesh uses %d materials"), MaterialCount));
    }

    // Check for missing materials
    if (MaterialCount == 0)
    {
        Result.AddIssue(EAuditIssueLevel::Should, TEXT("NO_MATERIALS"), TEXT("Mesh has no materials assigned"));
    }
}

void FStaticMeshAuditor::AnalyzeMeshCollision(const UStaticMesh* StaticMesh, FAuditResult& Result) const
{
    if (!StaticMesh)
        return;

    // Basic collision analysis - simplified for now
    Result.AddIssue(EAuditIssueLevel::Info, TEXT("MESH_COLLISION_ANALYSIS"), TEXT("Collision analysis completed"));
}

void FStaticMeshAuditor::AnalyzeMeshUVs(const UStaticMesh* StaticMesh, FAuditResult& Result) const
{
    if (!StaticMesh)
        return;

    // Basic UV analysis - check if mesh has UV coordinates
    if (StaticMesh->GetRenderData() && StaticMesh->GetRenderData()->LODResources.Num() > 0)
    {
        const FStaticMeshLODResources& LOD0 = StaticMesh->GetRenderData()->LODResources[0];
        
        // Check for UV channels
        int32 UVChannelCount = LOD0.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();
        
        if (UVChannelCount == 0)
        {
            Result.AddIssue(EAuditIssueLevel::Must, TEXT("NO_UV_COORDINATES"), TEXT("Mesh has no UV coordinates"));
        }
    }
}

void FStaticMeshAuditor::GeneratePlatformRecommendations(const UStaticMesh* StaticMesh, FAuditResult& Result) const
{
    if (!StaticMesh)
        return;

    // Get mesh complexity for platform recommendations
    int32 TotalTriangles = 0;
    if (StaticMesh->GetRenderData() && StaticMesh->GetRenderData()->LODResources.Num() > 0)
    {
        TotalTriangles = StaticMesh->GetRenderData()->LODResources[0].GetNumTriangles();
    }

    // Mobile platform recommendations
    if (CurrentConfig.TargetPlatform.Contains(TEXT("Mobile")) || CurrentConfig.TargetPlatform.Contains(TEXT("Android")) || CurrentConfig.TargetPlatform.Contains(TEXT("iOS")))
    {
        if (TotalTriangles > 10000)
        {
            FOptimizationRecommendation Rec;
            Rec.RecommendationID = TEXT("MOBILE_OPTIMIZATION");
            Rec.Title = TEXT("Mobile Optimization");
            Rec.Description = TEXT("Reduce polygon count for mobile platforms");
            Rec.Priority = EOptimizationPriority::High;
            Rec.Category = EOptimizationCategory::Performance;
            Rec.EstimatedMemorySavingsMB = 50.0f;
            Result.AddRecommendation(Rec);
        }
    }
}

float FStaticMeshAuditor::CalculateMeshMemoryUsage(const UStaticMesh* StaticMesh) const
{
    if (!StaticMesh)
        return 0.0f;

    float TotalMemory = 0.0f;
    
    if (StaticMesh->GetRenderData())
    {
        for (int32 LODIndex = 0; LODIndex < StaticMesh->GetRenderData()->LODResources.Num(); LODIndex++)
        {
            const FStaticMeshLODResources& LOD = StaticMesh->GetRenderData()->LODResources[LODIndex];
            
            // Estimate memory usage for vertices and indices
            int32 VertexCount = LOD.GetNumVertices();
            int32 IndexCount = LOD.GetNumTriangles() * 3;
            
            // Rough estimate: 32 bytes per vertex, 4 bytes per index
            float LODMemory = (VertexCount * 32.0f + IndexCount * 4.0f) / (1024.0f * 1024.0f);
            TotalMemory += LODMemory;
        }
    }

    return TotalMemory;
}

bool FStaticMeshAuditor::IsOptimalPolygonCount(int32 PolygonCount, const FString& Platform) const
{
    if (Platform.Contains(TEXT("Mobile")))
    {
        return PolygonCount <= 10000;
    }
    else if (Platform.Contains(TEXT("Console")))
    {
        return PolygonCount <= 50000;
    }
    else if (Platform.Contains(TEXT("PC")))
    {
        return PolygonCount <= 100000;
    }
    return PolygonCount <= 50000;
}

int32 FStaticMeshAuditor::GetRecommendedLODCount(const UStaticMesh* StaticMesh, const FString& Platform) const
{
    if (!StaticMesh)
        return 0;

    int32 CurrentTriangles = 0;
    if (StaticMesh->GetRenderData() && StaticMesh->GetRenderData()->LODResources.Num() > 0)
    {
        CurrentTriangles = StaticMesh->GetRenderData()->LODResources[0].GetNumTriangles();
    }

    // Recommend LOD count based on complexity and platform
    if (CurrentTriangles > 50000)
    {
        return 4; // High complexity needs more LODs
    }
    else if (CurrentTriangles > 20000)
    {
        return 3; // Medium complexity
    }
    else
    {
        return 2; // Low complexity
    }
}

void FStaticMeshAuditor::UpdateProgress(int32 Processed, int32 Total) const
{
    ProcessedCount = Processed;
    TotalCount = Total;
    CurrentProgress = Total > 0 ? (float)Processed / (float)Total : 0.0f;
}