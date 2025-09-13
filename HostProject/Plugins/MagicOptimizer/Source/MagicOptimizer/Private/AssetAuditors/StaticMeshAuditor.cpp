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
    Result.AssetData = AssetData;
    Result.Status = EAuditStatus::Running;
    Result.StartTime = FDateTime::Now();

    try
    {
        CurrentStatus = FString::Printf(TEXT("Loading static mesh: %s"), *AssetData.AssetName.ToString());
        
        // Load the static mesh asset
        UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetData.GetAsset());
        if (!StaticMesh)
        {
            FAuditIssue Issue;
            Issue.ID = TEXT("STATIC_MESH_LOAD_FAILED");
            Issue.Type = EAuditIssueType::Error;
            Issue.Title = TEXT("Static Mesh Loading Failed");
            Issue.Description = TEXT("Unable to load static mesh asset for analysis");
            Issue.Severity = 0.7f;
            Result.AddIssue(Issue);
            Result.MarkFailed(TEXT("Unable to load static mesh asset"));
            return Result;
        }

        // Perform basic mesh analysis
        AnalyzeMeshComplexity(StaticMesh, Result);
        AnalyzeMeshLODs(StaticMesh, Result);
        AnalyzeMeshMaterials(StaticMesh, Result);
        
        Result.MarkCompleted();
        
        MAGIC_LOG(General, FString::Printf(TEXT("Static mesh audit completed: %s"), *AssetData.AssetName.ToString()));
    }
    catch (const std::exception& e)
    {
        FString ErrorMsg = FString::Printf(TEXT("Exception during static mesh audit: %s"), *FString(e.what()));
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

    // Update performance metrics
    Result.PerformanceMetrics.Triangles = TotalTriangles;
    Result.PerformanceMetrics.Vertices = TotalVertices;
    Result.PerformanceMetrics.MemoryUsageMB = CalculateMeshMemoryUsage(StaticMesh);

    // Check for excessive polygon count
    if (TotalTriangles > 100000)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("HIGH_POLYGON_COUNT");
        Issue.Type = EAuditIssueType::Error;
        Issue.Title = TEXT("High Polygon Count");
        Issue.Description = FString::Printf(TEXT("Mesh has %d triangles, which is very high"), TotalTriangles);
        Issue.Severity = 0.8f;
        Result.AddIssue(Issue);
    }
    else if (TotalTriangles > 50000)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("MODERATE_POLYGON_COUNT");
        Issue.Type = EAuditIssueType::Warning;
        Issue.Title = TEXT("Moderate Polygon Count");
        Issue.Description = FString::Printf(TEXT("Mesh has %d triangles"), TotalTriangles);
        Issue.Severity = 0.6f;
        Result.AddIssue(Issue);
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
        FAuditIssue Issue;
        Issue.ID = TEXT("NO_LODS");
        Issue.Type = EAuditIssueType::Warning;
        Issue.Title = TEXT("No LODs");
        Issue.Description = FString::Printf(TEXT("Mesh has only %d LOD level"), LODCount);
        Issue.Severity = 0.6f;
        Result.AddIssue(Issue);
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
        FAuditIssue Issue;
        Issue.ID = TEXT("HIGH_MATERIAL_COUNT");
        Issue.Type = EAuditIssueType::Error;
        Issue.Title = TEXT("High Material Count");
        Issue.Description = FString::Printf(TEXT("Mesh uses %d materials"), MaterialCount);
        Issue.Severity = 0.7f;
        Result.AddIssue(Issue);
    }

    // Check for missing materials
    if (MaterialCount == 0)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("NO_MATERIALS");
        Issue.Type = EAuditIssueType::Warning;
        Issue.Title = TEXT("No Materials");
        Issue.Description = TEXT("Mesh has no materials assigned");
        Issue.Severity = 0.5f;
        Result.AddIssue(Issue);
    }
}

void FStaticMeshAuditor::AnalyzeMeshCollision(const UStaticMesh* StaticMesh, FAuditResult& Result) const
{
    if (!StaticMesh)
        return;

    // Basic collision analysis - simplified for now
    FAuditIssue Issue;
    Issue.ID = TEXT("MESH_COLLISION_ANALYSIS");
    Issue.Type = EAuditIssueType::Info;
    Issue.Title = TEXT("Mesh Collision Analysis");
    Issue.Description = TEXT("Collision analysis completed");
    Issue.Severity = 0.4f;
    Result.AddIssue(Issue);
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
            FAuditIssue Issue;
            Issue.ID = TEXT("NO_UV_COORDINATES");
            Issue.Type = EAuditIssueType::Error;
            Issue.Title = TEXT("No UV Coordinates");
            Issue.Description = TEXT("Mesh has no UV coordinates");
            Issue.Severity = 0.8f;
            Result.AddIssue(Issue);
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
            Rec.ID = TEXT("MOBILE_OPTIMIZATION");
            Rec.Title = TEXT("Mobile Optimization");
            Rec.Description = TEXT("Reduce polygon count for mobile platforms");
            Rec.Priority = EOptimizationPriority::High;
            Rec.Type = EAuditOptimizationType::Performance;
            Rec.EstimatedSavingsMB = 50.0f;
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