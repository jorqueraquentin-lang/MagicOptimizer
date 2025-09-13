#include "MagicOptimizerBranding.h"
#include "Engine/Texture2D.h"
#include "Engine/Engine.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "MagicOptimizerDiagnostics.h"

UTexture2D* UMagicOptimizerBranding::GetMagicOptimizerLogo()
{
    return LoadBrandingTexture(TEXT("MagicOptimizer_logo"));
}

UTexture2D* UMagicOptimizerBranding::GetMagicOptimizerLogoCompressed()
{
    return LoadBrandingTexture(TEXT("MagicOptimizer_logo_compressed"));
}

UTexture2D* UMagicOptimizerBranding::GetPerseusXRLogo()
{
    return LoadBrandingTexture(TEXT("PerseusXR_Logo"));
}

FString UMagicOptimizerBranding::GetBrandedPluginName()
{
    return TEXT("Magic Optimizer by Perseus XR");
}

FString UMagicOptimizerBranding::GetCompanyName()
{
    return TEXT("Perseus XR PTY LTD");
}

FString UMagicOptimizerBranding::GetCompanyURL()
{
    return TEXT("https://perseusxr.com");
}

UTexture2D* UMagicOptimizerBranding::LoadBrandingTexture(const FString& TextureName)
{
    if (!GEngine)
    {
        MAGIC_LOG(Error, FString::Printf(TEXT("GEngine not available for loading branding texture: %s"), *TextureName));
        return nullptr;
    }

    // Construct the asset path
    FString AssetPath = FString::Printf(TEXT("/Game/MagicOptimizer/Branding/%s"), *TextureName);
    
    // Load the texture
    UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *AssetPath);
    
    if (!Texture)
    {
        MAGIC_LOG(Warning, FString::Printf(TEXT("Failed to load branding texture: %s"), *AssetPath));
        
        // Try alternative path format
        AssetPath = FString::Printf(TEXT("/MagicOptimizer/Content/Branding/%s"), *TextureName);
        Texture = LoadObject<UTexture2D>(nullptr, *AssetPath);
        
        if (!Texture)
        {
            MAGIC_LOG(Error, FString::Printf(TEXT("Failed to load branding texture from alternative path: %s"), *AssetPath));
        }
    }
    else
    {
        MAGIC_LOG(General, FString::Printf(TEXT("Successfully loaded branding texture: %s"), *TextureName));
    }
    
    return Texture;
}
