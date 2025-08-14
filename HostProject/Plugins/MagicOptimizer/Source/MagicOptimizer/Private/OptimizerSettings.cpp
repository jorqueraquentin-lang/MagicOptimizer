// Copyright Epic Games, Inc. All Rights Reserved.

#include "OptimizerSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/ConfigCacheIni.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UOptimizerSettings::UOptimizerSettings()
{
	// Set default values
	CategoryMask = (uint8)EOptimizerCategory::Textures | (uint8)EOptimizerCategory::Meshes;
	TargetProfile = TEXT("PC_Balanced");
	bUseSelection = false;
	IncludePathsCsv = TEXT("");
	ExcludePathsCsv = TEXT("");
	bDryRun = true;
	MaxChanges = 100;
	bCreateBackups = true;
	bCloseEditor = false;
	OutputDirectory = TEXT("Saved/MagicOptimizer");
	bGenerateReports = true;
	PythonScriptPath = TEXT("Content/Python/magic_optimizer");
	bEnablePythonLogging = true;

	// Initialize with default presets
	CustomPresets.Empty();
}

UOptimizerSettings* UOptimizerSettings::Get()
{
	return GetMutableDefault<UOptimizerSettings>();
}

void UOptimizerSettings::GetDefaultSettings()
{
	// Load from config file
	LoadSettings();
}

void UOptimizerSettings::SaveSettings()
{
	// Save to config file
	SaveConfig();
}

void UOptimizerSettings::LoadSettings()
{
	// Load from config file
	LoadConfig();
}

FOptimizerPreset* UOptimizerSettings::GetPreset(const FString& PresetName)
{
	for (int32 i = 0; i < CustomPresets.Num(); ++i)
	{
		if (CustomPresets[i].Name == PresetName)
		{
			return &CustomPresets[i];
		}
	}
	return nullptr;
}

void UOptimizerSettings::AddCustomPreset(const FOptimizerPreset& Preset)
{
	// Check if preset already exists
	FOptimizerPreset* ExistingPreset = GetPreset(Preset.Name);
	if (ExistingPreset)
	{
		*ExistingPreset = Preset;
	}
	else
	{
		CustomPresets.Add(Preset);
	}
	
	SaveSettings();
}

void UOptimizerSettings::RemoveCustomPreset(const FString& PresetName)
{
	for (int32 i = 0; i < CustomPresets.Num(); ++i)
	{
		if (CustomPresets[i].Name == PresetName)
		{
			CustomPresets.RemoveAt(i);
			break;
		}
	}
	
	SaveSettings();
}

TArray<FString> UOptimizerSettings::GetAllPresets() const
{
	TArray<FString> PresetNames;
	
	// Add built-in presets
	PresetNames.Add(TEXT("PC_Ultra"));
	PresetNames.Add(TEXT("PC_Balanced"));
	PresetNames.Add(TEXT("Console_Optimized"));
	PresetNames.Add(TEXT("Mobile_Low"));
	PresetNames.Add(TEXT("Mobile_Ultra_Lite"));
	PresetNames.Add(TEXT("VR"));
	PresetNames.Add(TEXT("Cinematic"));
	PresetNames.Add(TEXT("UI_Crisp"));
	PresetNames.Add(TEXT("Archviz_High_Fidelity"));
	
	// Add custom presets
	for (const FOptimizerPreset& Preset : CustomPresets)
	{
		PresetNames.Add(Preset.Name);
	}
	
	return PresetNames;
}

void UOptimizerSettings::SetToDefaults()
{
	// Reset to constructor defaults
	CategoryMask = (uint8)EOptimizerCategory::Textures | (uint8)EOptimizerCategory::Meshes;
	TargetProfile = TEXT("PC_Balanced");
	bUseSelection = false;
	IncludePathsCsv = TEXT("");
	ExcludePathsCsv = TEXT("");
	bDryRun = true;
	MaxChanges = 100;
	bCreateBackups = true;
	bCloseEditor = false;
	OutputDirectory = TEXT("Saved/MagicOptimizer");
	bGenerateReports = true;
	PythonScriptPath = TEXT("Content/Python/magic_optimizer");
	bEnablePythonLogging = true;
	CustomPresets.Empty();
}
