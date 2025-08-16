// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

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
	RunMode = TEXT("Audit");
	bUseSelection = false;
	IncludePathsCsv = TEXT("");
	ExcludePathsCsv = TEXT("");
	bDryRun = true;
	MaxChanges = 100;
	bCreateBackups = true;
	bCloseEditor = false;
	OutputDirectory = TEXT("Saved/MagicOptimizer");
	bGenerateReports = true;
	PythonScriptPath = TEXT("");  // Empty to default to plugin-shipped Python
	bEnablePythonLogging = true;

	// Auto-report settings (enabled by default with user consent)
	bEnableAutoReporting = true;
	AutoReportWorkerUrl = TEXT("https://steep-frog-b764.summer-hall-9089.workers.dev");  // Pre-configured
	bIncludeLogsInReports = true;
	bIncludeKnowledgeInReports = true;
	bReportErrors = true;
	bReportOptimizations = true;
	bReportSessions = true;  // Enable session reporting for better insights
	bAnonymizeData = true;
	MaxLogLines = 2000;  // Increased for better diagnostics
	bUserConsent = true;  // Enabled by default with user consent

	// Initialize with default presets
	CustomPresets.Empty();

	// UI Settings - Filter and Sort Preferences (defaults)
	TextureFilterText = TEXT("");
	TextureFilterMinWidth = 0;
	TextureFilterMinHeight = 0;
	TextureFormatFilter = TEXT("");
	TextureSortColumn = TEXT("Path");
	bTextureSortAscending = true;
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
	// Persist to the per-project editor config to ensure values survive restarts
	SaveConfig(CPF_Config, *GEditorPerProjectIni);
	if (GConfig)
	{
		GConfig->Flush(false, GEditorPerProjectIni);
	}
}

void UOptimizerSettings::LoadSettings()
{
	// Load from the per-project editor config
	LoadConfig(GetClass(), *GEditorPerProjectIni);
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
	RunMode = TEXT("Audit");
	bUseSelection = false;
	IncludePathsCsv = TEXT("");
	ExcludePathsCsv = TEXT("");
	bDryRun = true;
	MaxChanges = 100;
	bCreateBackups = true;
	bCloseEditor = false;
	OutputDirectory = TEXT("Saved/MagicOptimizer");
	bGenerateReports = true;
	PythonScriptPath = TEXT("");  // Empty to default to plugin-shipped Python
	bEnablePythonLogging = true;

	// Auto-report settings (enabled by default with user consent)
	bEnableAutoReporting = true;
	AutoReportWorkerUrl = TEXT("https://steep-frog-b764.summer-hall-9089.workers.dev");  // Pre-configured
	bIncludeLogsInReports = true;
	bIncludeKnowledgeInReports = true;
	bReportErrors = true;
	bReportOptimizations = true;
	bReportSessions = true;  // Enable session reporting for better insights
	bAnonymizeData = true;
	MaxLogLines = 2000;  // Increased for better diagnostics
	bUserConsent = true;  // Enabled by default with user consent

	CustomPresets.Empty();

	// UI Settings - Filter and Sort Preferences (defaults)
	TextureFilterText = TEXT("");
	TextureFilterMinWidth = 0;
	TextureFilterMinHeight = 0;
	TextureFormatFilter = TEXT("");
	TextureSortColumn = TEXT("Path");
	bTextureSortAscending = true;
}
