#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "Engine/TextureRenderTarget2D.h"
#include "AssetAuditors/IAssetAuditor.h"
#include "AuditTypes/AuditConfig.h"
#include "AuditTypes/AuditResult.h"

/**
 * Texture auditor implementation for analyzing texture assets
 * Provides comprehensive texture optimization analysis and recommendations
 */
class MAGICOPTIMIZER_API FTextureAuditor : public IAssetAuditor
{
public:
    FTextureAuditor();
    virtual ~FTextureAuditor() = default;

    // IAssetAuditor interface implementation
    virtual bool CanAuditAsset(const FAssetData& AssetData) const override;
    virtual FAuditResult AuditAsset(const FAssetData& AssetData) const override;
    virtual TFuture<FAuditResult> AuditAssetAsync(const FAssetData& AssetData) const override;
    virtual void SetAuditConfig(const FAuditConfig& Config) override;
    virtual FAuditConfig GetAuditConfig() const override;
    virtual float GetAuditProgress() const override;
    virtual FString GetAuditStatus() const override;
    virtual bool HandleError(const FString& ErrorMessage) override;
    virtual void Reset() override;
    virtual FString GetAuditorName() const override;
    virtual TArray<EAssetType> GetSupportedAssetTypes() const override;

private:
    FAuditConfig CurrentConfig;
    mutable float CurrentProgress;
    mutable FString CurrentStatus;
    mutable int32 ProcessedCount;
    mutable int32 TotalCount;

    /**
     * Analyze texture properties and generate recommendations
     * @param Texture Texture to analyze
     * @param AssetData Asset data for the texture
     * @return Audit result with analysis and recommendations
     */
    FAuditResult AnalyzeTexture(const UTexture2D* Texture, const FAssetData& AssetData) const;

    /**
     * Analyze texture resolution and generate size-related recommendations
     * @param Texture Texture to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeTextureResolution(const UTexture2D* Texture, FAuditResult& Result) const;

    /**
     * Analyze texture compression and generate compression recommendations
     * @param Texture Texture to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeTextureCompression(const UTexture2D* Texture, FAuditResult& Result) const;

    /**
     * Analyze texture memory usage and generate memory optimization recommendations
     * @param Texture Texture to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeTextureMemory(const UTexture2D* Texture, FAuditResult& Result) const;

    /**
     * Analyze texture mipmaps and generate mipmap recommendations
     * @param Texture Texture to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeTextureMipmaps(const UTexture2D* Texture, FAuditResult& Result) const;

    /**
     * Analyze texture format and generate format recommendations
     * @param Texture Texture to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeTextureFormat(const UTexture2D* Texture, FAuditResult& Result) const;

    /**
     * Generate platform-specific optimization recommendations
     * @param Texture Texture to analyze
     * @param Result Audit result to populate
     */
    void GeneratePlatformRecommendations(const UTexture2D* Texture, FAuditResult& Result) const;

    /**
     * Calculate estimated memory usage for texture
     * @param Texture Texture to analyze
     * @return Estimated memory usage in MB
     */
    float CalculateTextureMemoryUsage(const UTexture2D* Texture) const;

    /**
     * Calculate quality score for texture resolution
     * @param Width Texture width
     * @param Height Texture height
     * @return Quality score (0.0 to 1.0)
     */
    float CalculateQualityScore(int32 Width, int32 Height) const;

    /**
     * Check if texture resolution is optimal for platform
     * @param Width Texture width
     * @param Height Texture height
     * @param Platform Target platform string
     * @return True if resolution is optimal
     */
    bool IsOptimalResolution(int32 Width, int32 Height, const FString& Platform) const;

    /**
     * Get recommended compression format for platform
     * @param Texture Texture to analyze
     * @param Platform Target platform string
     * @return Recommended compression format
     */
    FString GetRecommendedCompressionFormat(const UTexture2D* Texture, const FString& Platform) const;

    /**
     * Update progress tracking
     * @param Processed Number of processed items
     * @param Total Total number of items
     */
    void UpdateProgress(int32 Processed, int32 Total) const;

    // Enhanced helper methods using real UE 5.6 APIs

    /**
     * Get maximum recommended resolution for current platform
     * @return Maximum recommended resolution in pixels
     */
    int32 GetMaxRecommendedResolution() const;

    /**
     * Check if texture should be compressed
     * @param Texture Texture to check
     * @return True if texture should be compressed
     */
    bool ShouldBeCompressed(const UTexture2D* Texture) const;

    /**
     * Check if texture is a normal map
     * @param Texture Texture to check
     * @return True if texture is a normal map
     */
    bool IsNormalMapTexture(const UTexture2D* Texture) const;

    /**
     * Check if texture is grayscale
     * @param Texture Texture to check
     * @return True if texture is grayscale
     */
    bool IsGrayscaleTexture(const UTexture2D* Texture) const;

    /**
     * Check if texture is HDR
     * @param Texture Texture to check
     * @return True if texture is HDR
     */
    bool IsHDRTexture(const UTexture2D* Texture) const;

    /**
     * Get bytes per pixel for pixel format
     * @param Format Pixel format
     * @return Bytes per pixel
     */
    float GetBytesPerPixel(EPixelFormat Format) const;

    /**
     * Get estimated bytes per pixel for compression settings
     * @param CompressionSettings Compression settings
     * @return Estimated bytes per pixel
     */
    float GetEstimatedBytesPerPixel(TextureCompressionSettings CompressionSettings) const;

    /**
     * Analyze texture streaming settings
     * @param Texture Texture to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeTextureStreaming(const UTexture2D* Texture, FAuditResult& Result) const;

    /**
     * Analyze texture usage patterns
     * @param Texture Texture to analyze
     * @param Result Audit result to populate
     */
    void AnalyzeTextureUsage(const UTexture2D* Texture, FAuditResult& Result) const;

    /**
     * Check if texture should be streamed
     * @param Texture Texture to check
     * @return True if texture should be streamed
     */
    bool ShouldBeStreamed(const UTexture2D* Texture) const;
};
