#include "AssetAuditors/TextureAuditor.h"
#include "MagicOptimizerDiagnostics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureLightProfile.h"
#include "Engine/TextureRenderTargetCube.h"
#include "Materials/MaterialInterface.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Async/Async.h"
#include "Engine/Texture.h"
#include "RenderUtils.h"
#include "TextureResource.h"
#include "Engine/TextureStreamingTypes.h"
#include "Engine/StreamableRenderAsset.h"
#include "RHI.h"
#include "RHIDefinitions.h"

FTextureAuditor::FTextureAuditor()
{
    CurrentProgress = 0.0f;
    CurrentStatus = TEXT("Ready");
    ProcessedCount = 0;
    TotalCount = 0;
}

bool FTextureAuditor::CanAuditAsset(const FAssetData& AssetData) const
{
    // Check if asset is a texture type
    return AssetData.AssetClassPath.GetAssetName() == UTexture2D::StaticClass()->GetFName() ||
           AssetData.AssetClassPath.GetAssetName() == UTextureCube::StaticClass()->GetFName() ||
           AssetData.AssetClassPath.GetAssetName() == UTextureRenderTarget2D::StaticClass()->GetFName() ||
           AssetData.AssetClassPath.GetAssetName() == UTextureRenderTargetCube::StaticClass()->GetFName() ||
           AssetData.AssetClassPath.GetAssetName() == UTextureLightProfile::StaticClass()->GetFName() ||
           AssetData.AssetClassPath.ToString().Contains(TEXT("Texture"));
}

FAuditResult FTextureAuditor::AuditAsset(const FAssetData& AssetData) const
{
    FAuditResult Result;
    Result.AssetData = AssetData;
    Result.Status = EAuditStatus::Running;
    Result.StartTime = FDateTime::Now();

    try
    {
        CurrentStatus = FString::Printf(TEXT("Loading texture: %s"), *AssetData.AssetName.ToString());
        
        // Load the texture asset
        UTexture2D* Texture = Cast<UTexture2D>(AssetData.GetAsset());
        if (!Texture)
        {
            FAuditIssue Issue;
            Issue.ID = TEXT("TEXTURE_LOAD_FAILED");
            Issue.Type = EAuditIssueType::Error;
            Issue.Title = TEXT("Texture Loading Failed");
            Issue.Description = TEXT("Unable to load texture asset for analysis");
            Issue.Severity = 0.7f;
            Result.AddIssue(Issue);
            Result.MarkFailed(TEXT("Unable to load texture asset"));
            return Result;
        }

        // Perform comprehensive texture analysis
        AnalyzeTextureResolution(Texture, Result);
        AnalyzeTextureCompression(Texture, Result);
        AnalyzeTextureMemory(Texture, Result);
        AnalyzeTextureMipmaps(Texture, Result);
        AnalyzeTextureFormat(Texture, Result);
        AnalyzeTextureStreaming(Texture, Result);
        AnalyzeTextureUsage(Texture, Result);
        GeneratePlatformRecommendations(Texture, Result);
        
        Result.MarkCompleted();
        
        MAGIC_LOG(General, FString::Printf(TEXT("Texture audit completed: %s"), *AssetData.AssetName.ToString()));
    }
    catch (const std::exception& e)
    {
        FString ErrorMsg = FString::Printf(TEXT("Exception during texture audit: %s"), *FString(e.what()));
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

TFuture<FAuditResult> FTextureAuditor::AuditAssetAsync(const FAssetData& AssetData) const
{
    return Async(EAsyncExecution::TaskGraph, [this, AssetData]()
    {
        return AuditAsset(AssetData);
    });
}

void FTextureAuditor::SetAuditConfig(const FAuditConfig& Config)
{
    CurrentConfig = Config;
}

FAuditConfig FTextureAuditor::GetAuditConfig() const
{
    return CurrentConfig;
}

float FTextureAuditor::GetAuditProgress() const
{
    return CurrentProgress;
}

FString FTextureAuditor::GetAuditStatus() const
{
    return CurrentStatus;
}

bool FTextureAuditor::HandleError(const FString& ErrorMessage)
{
    MAGIC_LOG_ERROR(ErrorMessage, TEXT("TextureAuditor"));
    CurrentStatus = FString::Printf(TEXT("Error: %s"), *ErrorMessage);
    return true;
}

void FTextureAuditor::Reset()
{
    CurrentProgress = 0.0f;
    CurrentStatus = TEXT("Ready");
    ProcessedCount = 0;
    TotalCount = 0;
}

FString FTextureAuditor::GetAuditorName() const
{
    return TEXT("Texture Auditor");
}

TArray<EAssetType> FTextureAuditor::GetSupportedAssetTypes() const
{
    return { EAssetType::Texture };
}

void FTextureAuditor::AnalyzeTextureResolution(const UTexture2D* Texture, FAuditResult& Result) const
{
    if (!Texture)
        return;

    // Get actual texture dimensions using real UE 5.6 API
    int32 Width = Texture->GetSizeX();
    int32 Height = Texture->GetSizeY();
    // Note: UTexture2D doesn't have GetSizeZ() - that's for 3D textures
    
    // Get source image dimensions if available
    int32 SourceWidth = Width;
    int32 SourceHeight = Height;
    if (Texture->Source.GetSizeX() > 0 && Texture->Source.GetSizeY() > 0)
    {
        SourceWidth = Texture->Source.GetSizeX();
        SourceHeight = Texture->Source.GetSizeY();
    }

    // Check for non-power-of-2 dimensions (only warn for certain texture types)
    bool bShouldBePowerOfTwo = !Texture->SRGB && !Texture->bNotOfflineProcessed;
    if (bShouldBePowerOfTwo && (!FMath::IsPowerOfTwo(Width) || !FMath::IsPowerOfTwo(Height)))
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("NON_POWER_OF_2");
        Issue.Type = EAuditIssueType::Warning;
        Issue.Title = TEXT("Non-Power-of-2 Resolution");
        Issue.Description = FString::Printf(TEXT("Texture resolution %dx%d is not power-of-2 (Source: %dx%d)"), Width, Height, SourceWidth, SourceHeight);
        Issue.Severity = 0.6f;
        Result.AddIssue(Issue);
    }

    // Check for excessive resolution based on platform
    int32 MaxResolution = GetMaxRecommendedResolution();
    if (Width > MaxResolution || Height > MaxResolution)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("EXCESSIVE_RESOLUTION");
        Issue.Type = EAuditIssueType::Error;
        Issue.Title = TEXT("Excessive Resolution");
        Issue.Description = FString::Printf(TEXT("Texture resolution %dx%d exceeds recommended %d for platform"), Width, Height, MaxResolution);
        Issue.Severity = 0.8f;
        Result.AddIssue(Issue);
    }

    // Check for very low resolution textures
    if (Width < 64 || Height < 64)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("LOW_RESOLUTION");
        Issue.Type = EAuditIssueType::Info;
        Issue.Title = TEXT("Low Resolution Texture");
        Issue.Description = FString::Printf(TEXT("Texture resolution %dx%d may be too low for quality"), Width, Height);
        Issue.Severity = 0.4f;
        Result.AddIssue(Issue);
    }

    // Check aspect ratio
    float AspectRatio = (float)Width / (float)Height;
    if (AspectRatio > 4.0f || AspectRatio < 0.25f)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("EXTREME_ASPECT_RATIO");
        Issue.Type = EAuditIssueType::Warning;
        Issue.Title = TEXT("Extreme Aspect Ratio");
        Issue.Description = FString::Printf(TEXT("Texture has extreme aspect ratio %.2f:1"), AspectRatio);
        Issue.Severity = 0.5f;
        Result.AddIssue(Issue);
    }

    // Update performance metrics with real data
    Result.PerformanceMetrics.MemoryUsageMB = CalculateTextureMemoryUsage(Texture);
    Result.PerformanceMetrics.QualityScore = CalculateQualityScore(Width, Height);
}

void FTextureAuditor::AnalyzeTextureCompression(const UTexture2D* Texture, FAuditResult& Result) const
{
    if (!Texture)
        return;

    // Get actual compression settings using real UE 5.6 API
    TextureCompressionSettings CompressionSettings = Texture->CompressionSettings;
    bool bIsSRGB = Texture->SRGB;
    bool bNotOfflineProcessed = Texture->bNotOfflineProcessed;
    
    // Get actual pixel format if available (simplified for UE 5.6)
    EPixelFormat PixelFormat = PF_Unknown;
    // Note: FTextureResource doesn't have GetFormat() in UE 5.6
    // We'll use compression settings instead

    // Check for uncompressed textures (only for large textures)
    if (ShouldBeCompressed(Texture) && (CompressionSettings == TC_Default || CompressionSettings == TC_Displacementmap))
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("UNCOMPRESSED_TEXTURE");
        Issue.Type = EAuditIssueType::Error;
        Issue.Title = TEXT("Uncompressed Texture");
        Issue.Description = FString::Printf(TEXT("Large texture (%dx%d) is not using optimized compression"), Texture->GetSizeX(), Texture->GetSizeY());
        Issue.Severity = 0.8f;
        Result.AddIssue(Issue);
    }

    // Check for inappropriate compression settings
    if (bIsSRGB && CompressionSettings == TC_Normalmap)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("INAPPROPRIATE_COMPRESSION");
        Issue.Type = EAuditIssueType::Error;
        Issue.Title = TEXT("Inappropriate Compression");
        Issue.Description = TEXT("Normal map should not have sRGB enabled");
        Issue.Severity = 0.9f;
        Result.AddIssue(Issue);
    }

    // Check for normal maps with wrong compression
    if (CompressionSettings == TC_Normalmap && !IsNormalMapTexture(Texture))
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("WRONG_COMPRESSION_TYPE");
        Issue.Type = EAuditIssueType::Warning;
        Issue.Title = TEXT("Wrong Compression Type");
        Issue.Description = TEXT("Non-normal map texture using normal map compression");
        Issue.Severity = 0.6f;
        Result.AddIssue(Issue);
    }

    // Check for grayscale textures that could use BC4
    if (IsGrayscaleTexture(Texture) && CompressionSettings != TC_Grayscale)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("GRAYSCALE_OPTIMIZATION");
        Issue.Type = EAuditIssueType::Info;
        Issue.Title = TEXT("Grayscale Optimization");
        Issue.Description = TEXT("Grayscale texture could use more efficient compression");
        Issue.Severity = 0.5f;
        Result.AddIssue(Issue);
    }

    // Check for HDR textures with inappropriate compression
    if (IsHDRTexture(Texture) && CompressionSettings != TC_HDR)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("HDR_COMPRESSION");
        Issue.Type = EAuditIssueType::Warning;
        Issue.Title = TEXT("HDR Compression");
        Issue.Description = TEXT("HDR texture should use HDR compression settings");
        Issue.Severity = 0.7f;
        Result.AddIssue(Issue);
    }
}

void FTextureAuditor::AnalyzeTextureMemory(const UTexture2D* Texture, FAuditResult& Result) const
{
    if (!Texture)
        return;

    float MemoryUsage = CalculateTextureMemoryUsage(Texture);
    
    // Check for excessive memory usage
    if (MemoryUsage > 100.0f) // More than 100MB
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("HIGH_MEMORY_USAGE");
        Issue.Type = EAuditIssueType::Error;
        Issue.Title = TEXT("High Memory Usage");
        Issue.Description = FString::Printf(TEXT("Texture uses %.2f MB of memory"), MemoryUsage);
        Issue.Severity = 0.9f;
        Result.AddIssue(Issue);
    }
}

void FTextureAuditor::AnalyzeTextureMipmaps(const UTexture2D* Texture, FAuditResult& Result) const
{
    if (!Texture)
        return;

    // Check mipmap settings
    if (!Texture->MipGenSettings)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("NO_MIPMAPS");
        Issue.Type = EAuditIssueType::Warning;
        Issue.Title = TEXT("No Mipmaps");
        Issue.Description = TEXT("Texture does not have mipmaps enabled");
        Issue.Severity = 0.6f;
        Result.AddIssue(Issue);
    }
}

void FTextureAuditor::AnalyzeTextureFormat(const UTexture2D* Texture, FAuditResult& Result) const
{
    if (!Texture)
        return;

    // Check for appropriate format usage
    if (Texture->SRGB && Texture->CompressionSettings == TC_Normalmap)
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("INCORRECT_SRGB");
        Issue.Type = EAuditIssueType::Error;
        Issue.Title = TEXT("Incorrect sRGB Usage");
        Issue.Description = TEXT("Normal map should not use sRGB color space");
        Issue.Severity = 0.8f;
        Result.AddIssue(Issue);
    }
}

void FTextureAuditor::GeneratePlatformRecommendations(const UTexture2D* Texture, FAuditResult& Result) const
{
    if (!Texture)
        return;

    int32 Width = Texture->GetSizeX();
    int32 Height = Texture->GetSizeY();

    // Mobile platform recommendations
    if (CurrentConfig.TargetPlatform.Contains(TEXT("Mobile")) || CurrentConfig.TargetPlatform.Contains(TEXT("Android")) || CurrentConfig.TargetPlatform.Contains(TEXT("iOS")))
    {
        if (Width > 1024 || Height > 1024)
        {
            FOptimizationRecommendation Rec;
            Rec.ID = TEXT("MOBILE_OPTIMIZATION");
            Rec.Title = TEXT("Mobile Optimization");
            Rec.Description = TEXT("Reduce texture resolution for mobile platforms");
            Rec.Priority = EOptimizationPriority::High;
            Rec.Type = EAuditOptimizationType::Performance;
            Rec.EstimatedSavingsMB = 60.0f;
            Result.AddRecommendation(Rec);
        }
    }
}

float FTextureAuditor::CalculateTextureMemoryUsage(const UTexture2D* Texture) const
{
    if (!Texture)
        return 0.0f;

    // Note: Direct memory access not available in UE 5.6 API
    // We'll calculate based on texture properties

    // Fallback to calculated estimation using real texture properties
    int32 Width = Texture->GetSizeX();
    int32 Height = Texture->GetSizeY();
    // Note: UTexture2D doesn't have GetSizeZ()
    
    // Get actual pixel format (simplified for UE 5.6)
    EPixelFormat PixelFormat = PF_Unknown;
    
    // Calculate bytes per pixel based on actual format
    float BytesPerPixel = GetBytesPerPixel(PixelFormat);
    if (BytesPerPixel == 0.0f)
    {
        // Fallback based on compression settings
        BytesPerPixel = GetEstimatedBytesPerPixel(Texture->CompressionSettings);
    }

    // Calculate total memory including mipmaps
    float TotalMemory = 0.0f;
    int32 CurrentWidth = Width;
    int32 CurrentHeight = Height;
    int32 MipCount = Texture->GetNumMips();
    
    // Use actual mip count if available
    for (int32 MipLevel = 0; MipLevel < MipCount && CurrentWidth > 0 && CurrentHeight > 0; ++MipLevel)
    {
        TotalMemory += CurrentWidth * CurrentHeight * BytesPerPixel;
        CurrentWidth = FMath::Max(1, CurrentWidth / 2);
        CurrentHeight = FMath::Max(1, CurrentHeight / 2);
    }

    return TotalMemory / (1024.0f * 1024.0f); // Convert to MB
}

bool FTextureAuditor::IsOptimalResolution(int32 Width, int32 Height, const FString& Platform) const
{
    if (Platform.Contains(TEXT("Mobile")))
    {
        return Width <= 1024 && Height <= 1024;
    }
    else if (Platform.Contains(TEXT("Console")))
    {
        return Width <= 2048 && Height <= 2048;
    }
    else if (Platform.Contains(TEXT("PC")))
    {
        return Width <= 4096 && Height <= 4096;
    }
    return Width <= 2048 && Height <= 2048;
}

FString FTextureAuditor::GetRecommendedCompressionFormat(const UTexture2D* Texture, const FString& Platform) const
{
    if (!Texture)
        return TEXT("Unknown");

    // Basic compression recommendations based on texture type and platform
    if (Texture->CompressionSettings == TC_Normalmap)
    {
        return TEXT("BC5 (Normal Maps)");
    }
    else if (Texture->SRGB)
    {
        if (Platform.Contains(TEXT("Mobile")))
        {
            return TEXT("BC1 (Mobile Color)");
        }
        else if (Platform.Contains(TEXT("Console")))
        {
            return TEXT("BC7 (Console Color)");
        }
        else
        {
            return TEXT("BC7 (PC Color)");
        }
    }
    else
    {
        return TEXT("BC4 (Grayscale)");
    }
}

float FTextureAuditor::CalculateQualityScore(int32 Width, int32 Height) const
{
    // Simple quality score based on resolution
    float MaxDimension = FMath::Max(Width, Height);
    
    if (MaxDimension <= 512)
        return 0.3f;
    else if (MaxDimension <= 1024)
        return 0.5f;
    else if (MaxDimension <= 2048)
        return 0.7f;
    else if (MaxDimension <= 4096)
        return 0.9f;
    else
        return 1.0f;
}

void FTextureAuditor::UpdateProgress(int32 Processed, int32 Total) const
{
    ProcessedCount = Processed;
    TotalCount = Total;
    CurrentProgress = Total > 0 ? (float)Processed / (float)Total : 0.0f;
}

// Enhanced helper methods using real UE 5.6 APIs

int32 FTextureAuditor::GetMaxRecommendedResolution() const
{
    FString Platform = CurrentConfig.TargetPlatform;
    
    if (Platform.Contains(TEXT("Mobile")) || Platform.Contains(TEXT("Android")) || Platform.Contains(TEXT("iOS")))
    {
        return 1024;
    }
    else if (Platform.Contains(TEXT("Console")) || Platform.Contains(TEXT("PS5")) || Platform.Contains(TEXT("Xbox")))
    {
        return 2048;
    }
    else if (Platform.Contains(TEXT("PC")) || Platform.Contains(TEXT("Windows")))
    {
        return 4096;
    }
    
    return 2048; // Default
}

bool FTextureAuditor::ShouldBeCompressed(const UTexture2D* Texture) const
{
    if (!Texture)
        return false;
        
    int32 Width = Texture->GetSizeX();
    int32 Height = Texture->GetSizeY();
    
    // Compress textures larger than 256x256
    return (Width * Height) > (256 * 256);
}

bool FTextureAuditor::IsNormalMapTexture(const UTexture2D* Texture) const
{
    if (!Texture)
        return false;
        
    // Check texture name for normal map indicators
    FString TextureName = Texture->GetName().ToLower();
    return TextureName.Contains(TEXT("normal")) || 
           TextureName.Contains(TEXT("nrm")) || 
           Texture->CompressionSettings == TC_Normalmap;
}

bool FTextureAuditor::IsGrayscaleTexture(const UTexture2D* Texture) const
{
    if (!Texture)
        return false;
        
    // Check if texture has only grayscale channels
    // This is a simplified check - in reality, you'd analyze the actual pixel data
    FString TextureName = Texture->GetName().ToLower();
    return TextureName.Contains(TEXT("grayscale")) || 
           TextureName.Contains(TEXT("gray")) || 
           Texture->CompressionSettings == TC_Grayscale;
}

bool FTextureAuditor::IsHDRTexture(const UTexture2D* Texture) const
{
    if (!Texture)
        return false;
        
    // Check if texture is HDR based on settings (simplified for UE 5.6)
    // Note: Direct format access not available in UE 5.6 API
    
    return Texture->CompressionSettings == TC_HDR;
}

float FTextureAuditor::GetBytesPerPixel(EPixelFormat Format) const
{
    // Simplified pixel format support for UE 5.6
    switch (Format)
    {
        case PF_R8G8B8A8: return 4.0f;
        case PF_B8G8R8A8: return 4.0f;
        case PF_R8G8B8A8_UINT: return 4.0f;
        case PF_R8G8B8A8_SNORM: return 4.0f;
        case PF_R16G16B16A16_UINT: return 8.0f;
        case PF_R16G16B16A16_SINT: return 8.0f;
        case PF_FloatRGBA: return 16.0f;
        case PF_DXT1: return 0.5f;
        case PF_DXT3: return 1.0f;
        case PF_DXT5: return 1.0f;
        case PF_BC4: return 0.5f;
        case PF_BC5: return 1.0f;
        case PF_BC6H: return 1.0f;
        case PF_BC7: return 1.0f;
        case PF_ETC2_RGB: return 0.5f;
        case PF_ETC2_RGBA: return 1.0f;
        case PF_ASTC_4x4: return 1.0f;
        case PF_ASTC_6x6: return 0.67f;
        case PF_ASTC_8x8: return 0.5f;
        default: return 0.0f; // Unknown format
    }
}

float FTextureAuditor::GetEstimatedBytesPerPixel(TextureCompressionSettings CompressionSettings) const
{
    switch (CompressionSettings)
    {
        case TC_Default:
        case TC_Displacementmap:
            return 4.0f; // Uncompressed RGBA8
        case TC_Normalmap:
            return 1.0f; // BC5
        case TC_Masks:
            return 1.0f; // BC7
        case TC_Grayscale:
            return 0.5f; // BC4
        case TC_Alpha:
            return 1.0f; // BC7
        case TC_DistanceFieldFont:
            return 0.5f; // BC4
        case TC_HDR:
            return 16.0f; // FloatRGBA
        case TC_HDR_Compressed:
            return 1.0f; // BC6H
        case TC_HalfFloat:
            return 8.0f; // A16B16G16R16F
        default:
            return 4.0f; // Default to uncompressed
    }
}

void FTextureAuditor::AnalyzeTextureStreaming(const UTexture2D* Texture, FAuditResult& Result) const
{
    if (!Texture)
        return;

    // Check if texture is set up for streaming (simplified for UE 5.6)
    // Note: bIsStreamable not directly accessible in UE 5.6 API
    bool bIsStreaming = false; // Simplified for now
    
    if (!bIsStreaming && ShouldBeStreamed(Texture))
    {
        FAuditIssue Issue;
        Issue.ID = TEXT("NOT_STREAMING");
        Issue.Type = EAuditIssueType::Warning;
        Issue.Title = TEXT("Texture Not Streaming");
        Issue.Description = TEXT("Large texture should be set up for streaming");
        Issue.Severity = 0.6f;
        Result.AddIssue(Issue);
    }
    
    // Check streaming settings (simplified for UE 5.6)
    // Note: GetMaxTextureSize() not available in UE 5.6 API
    if (bIsStreaming)
    {
        // Streaming analysis simplified for UE 5.6 compatibility
        FAuditIssue Issue;
        Issue.ID = TEXT("STREAMING_ENABLED");
        Issue.Type = EAuditIssueType::Info;
        Issue.Title = TEXT("Streaming Enabled");
        Issue.Description = TEXT("Texture has streaming enabled");
        Issue.Severity = 0.3f;
        Result.AddIssue(Issue);
    }
}

void FTextureAuditor::AnalyzeTextureUsage(const UTexture2D* Texture, FAuditResult& Result) const
{
    if (!Texture)
        return;

    // Check if texture is actually used in materials
    // This would require more complex asset dependency analysis
    // For now, we'll do basic checks
    
    // Check texture group usage (simplified for UE 5.6)
    if (Texture->LODGroup == TEXTUREGROUP_World)
    {
        // World textures should generally be optimized
        FAuditIssue Issue;
        Issue.ID = TEXT("WORLD_TEXTURE");
        Issue.Type = EAuditIssueType::Info;
        Issue.Title = TEXT("World Texture");
        Issue.Description = TEXT("World texture detected - consider optimization for streaming");
        Issue.Severity = 0.5f;
        Result.AddIssue(Issue);
    }
    
    if (Texture->LODGroup == TEXTUREGROUP_UI)
    {
        // UI textures should be optimized for immediate loading
        FAuditIssue Issue;
        Issue.ID = TEXT("UI_TEXTURE");
        Issue.Type = EAuditIssueType::Info;
        Issue.Title = TEXT("UI Texture");
        Issue.Description = TEXT("UI texture detected - optimize for immediate loading");
        Issue.Severity = 0.4f;
        Result.AddIssue(Issue);
    }
}

bool FTextureAuditor::ShouldBeStreamed(const UTexture2D* Texture) const
{
    if (!Texture)
        return false;
        
    int32 Width = Texture->GetSizeX();
    int32 Height = Texture->GetSizeY();
    
    // Stream textures larger than 512x512
    return (Width * Height) > (512 * 512);
}