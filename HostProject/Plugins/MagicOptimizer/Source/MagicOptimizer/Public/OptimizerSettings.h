#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "OptimizerSettings.generated.h"

/**
 * Settings for the Magic Optimizer plugin
 * Accessible via Project Settings → Plugins → Magic Optimizer
 * 
 * CRITICAL SETTINGS DESIGN PRINCIPLES:
 * 1. Use config = Editor (NOT defaultconfig) to prevent multiple config sections
 * 2. All settings must be in ONE config file location (plugin config directory)
 * 3. NEVER create project-level config files for plugin settings
 * 4. UDeveloperSettings automatically handles registration - NO manual calls needed
 * 5. This prevents duplicate settings windows and configuration conflicts
 * 
 * CONFIGURATION LOCATION:
 * - Plugin Config: HostProject/Plugins/MagicOptimizer/Config/DefaultEditor.ini
 * - DO NOT create: HostProject/Config/DefaultEditor.ini (causes duplicates)
 */
UCLASS(config = Editor, meta = (DisplayName = "Magic Optimizer Settings"))
class MAGICOPTIMIZER_API UOptimizerSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UOptimizerSettings();

    //~UDeveloperSettings interface
    virtual FName GetCategoryName() const override;
    virtual FText GetSectionText() const override;
    //~End of UDeveloperSettings interface

    // Basic optimization settings
    UPROPERTY(config, EditAnywhere, Category = "General", meta = (DisplayName = "Default Profile"))
    FString DefaultProfile;

    UPROPERTY(config, EditAnywhere, Category = "General", meta = (DisplayName = "Enable Auto-Save"))
    bool bEnableAutoSave;

    UPROPERTY(config, EditAnywhere, Category = "General", meta = (DisplayName = "Max Assets Per Batch"))
    int32 MaxAssetsPerBatch;

    // Asset type settings
    UPROPERTY(config, EditAnywhere, Category = "Asset Types", meta = (DisplayName = "Process Textures"))
    bool bProcessTextures;

    UPROPERTY(config, EditAnywhere, Category = "Asset Types", meta = (DisplayName = "Process Meshes"))
    bool bProcessMeshes;

    UPROPERTY(config, EditAnywhere, Category = "Asset Types", meta = (DisplayName = "Process Materials"))
    bool bProcessMaterials;

    // Safety settings
    UPROPERTY(config, EditAnywhere, Category = "Safety", meta = (DisplayName = "Create Backups"))
    bool bCreateBackups;

    UPROPERTY(config, EditAnywhere, Category = "Safety", meta = (DisplayName = "Dry Run by Default"))
    bool bDryRunByDefault;

    UPROPERTY(config, EditAnywhere, Category = "Safety", meta = (DisplayName = "Max Changes Per Run"))
    int32 MaxChangesPerRun;

    // Output settings
    UPROPERTY(config, EditAnywhere, Category = "Output", meta = (DisplayName = "Output Directory"))
    FString OutputDirectory;

    UPROPERTY(config, EditAnywhere, Category = "Output", meta = (DisplayName = "Generate Reports"))
    bool bGenerateReports;

    UPROPERTY(config, EditAnywhere, Category = "Output", meta = (DisplayName = "Include Timestamps"))
    bool bIncludeTimestamps;

public:
    // Helper functions
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer")
    void ResetToDefaults();

    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer")
    void SaveSettings();

    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer")
    void LoadSettings();
};
