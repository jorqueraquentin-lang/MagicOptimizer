// TextureProcessor.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"

struct FTextureAnalysisResult
{
    FString AssetPath;
    int32 Width;
    int32 Height;
    FString Format;
    bool bHasMips;
    bool bIsSRGB;
    int64 SizeOnDisk;
    bool bSuccess;
    FString ErrorMessage;
};

class MAGICOPTIMIZER_API FTextureProcessor
{
public:
    // Analyze a single texture and return basic properties
    static bool AnalyzeTexture(const FString& AssetPath, FTextureAnalysisResult& OutResult);
    
    // Simple validation that texture exists and can be loaded
    static bool ValidateTextureAsset(const FString& AssetPath);
};
