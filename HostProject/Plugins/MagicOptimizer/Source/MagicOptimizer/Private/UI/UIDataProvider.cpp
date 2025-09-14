#include "UI/UIDataProvider.h"
#include "Model/AuditResult.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Algo/Transform.h"
#include "Math/UnrealMathUtility.h"

void FUIDataProvider::FromAuditResults(
    const TArray<FAuditResult>& InResults,
    TArray<FUITextureRow>& OutRows,
    TArray<FUIHealthPoint>& OutHealth,
    TArray<FUIBeforeAfter>& OutBeforeAfter,
    FUIBudgetGauges& OutGauges)
{
    // Convert audit results to texture rows
    OutRows.Empty();
    for (const FAuditResult& Result : InResults)
    {
        if (Result.AssetClass == TEXT("Texture2D"))
        {
            OutRows.Add(ConvertAuditResultToTexture(Result));
        }
    }

    // Generate mock data for other UI elements (will be replaced with real data later)
    MockIfEmpty(OutRows, OutHealth, OutBeforeAfter, OutGauges);
}

void FUIDataProvider::MockIfEmpty(
    TArray<FUITextureRow>& OutRows,
    TArray<FUIHealthPoint>& OutHealth,
    TArray<FUIBeforeAfter>& OutBeforeAfter,
    FUIBudgetGauges& OutGauges)
{
    // Generate mock texture rows if none exist
    if (OutRows.Num() == 0)
    {
        OutRows.Add({
            TEXT("T_Example_01"),
            TEXT("/Game/Textures/T_Example_01"),
            TEXT("2K"),
            TEXT("World"),
            TEXT("Default"),
            true, false, true, 0, 5, 2
        });
        OutRows.Add({
            TEXT("T_Example_02"),
            TEXT("/Game/Textures/T_Example_02"),
            TEXT("1K"),
            TEXT("Effects"),
            TEXT("Masks"),
            false, true, false, 1, 3, 1
        });
    }

    // Generate mock health points
    if (OutHealth.Num() == 0)
    {
        OutHealth.Add({0, 85});
        OutHealth.Add({1, 78});
        OutHealth.Add({2, 82});
        OutHealth.Add({3, 88});
        OutHealth.Add({4, 91});
    }

    // Generate mock before/after data
    if (OutBeforeAfter.Num() == 0)
    {
        OutBeforeAfter.Add({TEXT("Disk Usage"), 1024.0, 768.0});
        OutBeforeAfter.Add({TEXT("GPU Memory"), 512.0, 384.0});
        OutBeforeAfter.Add({TEXT("Draw Calls"), 256.0, 192.0});
    }

    // Generate mock budget gauges
    if (OutGauges.PoolUsePct == 0)
    {
        OutGauges.PoolUsePct = 65;
        OutGauges.VTPct = 30;
        OutGauges.DiskPct = 45;
    }
}

FUITextureRow FUIDataProvider::ConvertAuditResultToTexture(const FAuditResult& Result)
{
    FUITextureRow Row;
    
    // Extract basic info
    Row.Id = Result.Guid.ToString(EGuidFormats::Short);
    Row.Path = Result.AssetPath.ToString();
    
    // Extract texture properties from context
    Row.Size = ToSizeTag(Result.MaxSize);
    Row.Group = Result.Context.FindRef(TEXT("TextureGroup"), TEXT("World"));
    Row.Compression = Result.Context.FindRef(TEXT("CompressionSettings"), TEXT("Default"));
    
    // Extract flags from Flags set
    Row.bSRGB = Result.Flags.Contains(TEXT("sRGBOn"));
    Row.bMips = !Result.Flags.Contains(TEXT("NoMips"));
    Row.bVT = Result.Flags.Contains(TEXT("VT"));
    
    // Extract other properties
    Row.LOD = Result.LODBias;
    Row.Referencers = GetReferencerCount(Result.AssetPath);
    Row.Issues = CollapseIssues(Result.Issues);
    
    return Row;
}

FString FUIDataProvider::ToSizeTag(int32 Size)
{
    if (Size <= 512) return TEXT("512");
    if (Size <= 1024) return TEXT("1K");
    if (Size <= 2048) return TEXT("2K");
    if (Size <= 4096) return TEXT("4K");
    return TEXT("8K+");
}

int32 FUIDataProvider::GetReferencerCount(const FSoftObjectPath& ObjectPath)
{
    int32 Count = 0;
    if (!ObjectPath.IsValid()) return 0;

    FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FName> OutReferencers;
    const FName PackageName = ObjectPath.GetLongPackageFName();
    
    if (PackageName != NAME_None)
    {
        ARM.Get().GetReferencers(
            PackageName,
            OutReferencers,
            EAssetRegistryDependencyType::Hard | EAssetRegistryDependencyType::Soft);
        Count = OutReferencers.Num();
    }
    
    return Count;
}

int32 FUIDataProvider::CollapseIssues(const TArray<FAuditIssue>& Issues)
{
    int32 Count = 0;
    for (const FAuditIssue& Issue : Issues)
    {
        if (Issue.Level == EAuditIssueLevel::Must) Count += 2;
        else if (Issue.Level == EAuditIssueLevel::Should) Count += 1;
        // Info issues don't count
    }
    return Count;
}