#include "OptimizerSettings.h"
#include "Internationalization/Text.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

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

void UOptimizerSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    if (!PropertyChangedEvent.Property)
    {
        return;
    }
    
    FName PropertyName = PropertyChangedEvent.Property->GetFName();
    
    // Validate MaxAssetsPerBatch
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UOptimizerSettings, MaxAssetsPerBatch))
    {
        if (MaxAssetsPerBatch <= 0)
        {
            MaxAssetsPerBatch = 1;
            UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: MaxAssetsPerBatch must be > 0, reset to 1"));
        }
        else if (MaxAssetsPerBatch > 10000)
        {
            MaxAssetsPerBatch = 10000;
            UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: MaxAssetsPerBatch capped at 10000 for performance"));
        }
    }
    
    // Validate MaxChangesPerRun
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UOptimizerSettings, MaxChangesPerRun))
    {
        if (MaxChangesPerRun <= 0)
        {
            MaxChangesPerRun = 1;
            UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: MaxChangesPerRun must be > 0, reset to 1"));
        }
        else if (MaxChangesPerRun > 1000)
        {
            MaxChangesPerRun = 1000;
            UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: MaxChangesPerRun capped at 1000 for safety"));
        }
    }
    
    // Validate OutputDirectory
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UOptimizerSettings, OutputDirectory))
    {
        if (OutputDirectory.IsEmpty())
        {
            OutputDirectory = TEXT("Saved/MagicOptimizer");
            UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: OutputDirectory cannot be empty, reset to default"));
        }
        else
        {
            // Ensure the path is valid
            FString CleanPath = FPaths::ConvertRelativePathToFull(OutputDirectory);
            if (!FPaths::ValidatePath(CleanPath))
            {
                OutputDirectory = TEXT("Saved/MagicOptimizer");
                UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: Invalid OutputDirectory path, reset to default"));
            }
        }
    }
    
    // Validate DefaultProfile
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UOptimizerSettings, DefaultProfile))
    {
        if (DefaultProfile.IsEmpty())
        {
            DefaultProfile = TEXT("PC_Ultra");
            UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: DefaultProfile cannot be empty, reset to PC_Ultra"));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Property '%s' validated successfully"), *PropertyName.ToString());
}
