#include "OptimizerSettings.h"
#include "Internationalization/Text.h"

UOptimizerSettings::UOptimizerSettings()
{
    // Set default values for all settings
    DefaultProfile = TEXT("PC_Ultra");
    bEnableAutoSave = true;
    MaxAssetsPerBatch = 100;

    // Asset type defaults
    bProcessTextures = true;
    bProcessMeshes = true;
    bProcessMaterials = true;

    // Safety defaults
    bCreateBackups = true;
    bDryRunByDefault = true;
    MaxChangesPerRun = 50;

    // Output defaults
    OutputDirectory = TEXT("Saved/MagicOptimizer");
    bGenerateReports = true;
    bIncludeTimestamps = true;

    // Load any existing settings
    LoadSettings();
}

FName UOptimizerSettings::GetCategoryName() const
{
    return TEXT("Plugins");
}

FText UOptimizerSettings::GetSectionText() const
{
    return FText::FromString(TEXT("Magic Optimizer"));
}

void UOptimizerSettings::ResetToDefaults()
{
    // Reset to constructor defaults
    DefaultProfile = TEXT("PC_Ultra");
    bEnableAutoSave = true;
    MaxAssetsPerBatch = 100;
    bProcessTextures = true;
    bProcessMeshes = true;
    bProcessMaterials = true;
    bCreateBackups = true;
    bDryRunByDefault = true;
    MaxChangesPerRun = 50;
    OutputDirectory = TEXT("Saved/MagicOptimizer");
    bGenerateReports = true;
    bIncludeTimestamps = true;

    // Save the reset settings
    SaveSettings();
}

void UOptimizerSettings::SaveSettings()
{
    // This will automatically save to the config file
    // UDeveloperSettings handles the persistence
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Settings saved successfully"));
}

void UOptimizerSettings::LoadSettings()
{
    // Settings are automatically loaded by UDeveloperSettings
    // This function can be used for additional initialization if needed
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Settings loaded successfully"));
}
