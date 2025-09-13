#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "MagicOptimizerBranding.generated.h"

/**
 * Magic Optimizer Branding Assets
 * Provides easy access to plugin logos and branding materials
 */
UCLASS()
class MAGICOPTIMIZER_API UMagicOptimizerBranding : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Get the main Magic Optimizer logo
     * @return The main plugin logo texture
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Branding")
    static UTexture2D* GetMagicOptimizerLogo();

    /**
     * Get the compressed Magic Optimizer logo for UI use
     * @return The compressed plugin logo texture
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Branding")
    static UTexture2D* GetMagicOptimizerLogoCompressed();

    /**
     * Get the Perseus XR company logo
     * @return The Perseus XR logo texture
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Branding")
    static UTexture2D* GetPerseusXRLogo();

    /**
     * Get the plugin name with proper branding
     * @return Formatted plugin name
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Branding")
    static FString GetBrandedPluginName();

    /**
     * Get the company name
     * @return Company name
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Branding")
    static FString GetCompanyName();

    /**
     * Get the company website URL
     * @return Company website URL
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Branding")
    static FString GetCompanyURL();

private:
    /**
     * Load a texture from the branding content folder
     * @param TextureName Name of the texture file
     * @return Loaded texture or nullptr if failed
     */
    static UTexture2D* LoadBrandingTexture(const FString& TextureName);
};
