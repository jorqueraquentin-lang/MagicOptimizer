// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "OptimizerSettings.generated.h"

UENUM(BlueprintType)
enum class EOptimizerCategory : uint8
{
	None = 0x00,
	Textures = 0x01,
	Meshes = 0x02,
	Materials = 0x04,
	Levels = 0x08,
	Runtime = 0x10,
	Audio = 0x20
};

UENUM(BlueprintType)
enum class EOptimizerProfile : uint8
{
	PC_Ultra,
	PC_Balanced,
	Console_Optimized,
	Mobile_Low,
	Mobile_Ultra_Lite,
	VR,
	Cinematic,
	UI_Crisp,
	Archviz_High_Fidelity,
	Custom
};

UENUM(BlueprintType)
enum class EOptimizerRunMode : uint8
{
	Audit,
	Recommend,
	Apply,
	Verify
};

USTRUCT(BlueprintType)
struct FOptimizerPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	TMap<FString, FString> Settings;

	FOptimizerPreset()
	{
		Name = TEXT("Default");
		Description = TEXT("Default optimization preset");
	}
};

UCLASS(config = Editor, defaultconfig, meta = (DisplayName = "Magic Optimizer Settings"))
class MAGICOPTIMIZER_API UOptimizerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UOptimizerSettings();

	// Category mask for what types of assets to optimize
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Categories", meta = (Bitmask = "EOptimizerCategory"))
	uint8 CategoryMask;

	// Target profile for optimization
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FString TargetProfile;

	// Selected run mode (Audit/Recommend/Apply/Verify)
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FString RunMode;

	// Whether to use currently selected assets
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Scope")
	bool bUseSelection;

	// Include paths (comma separated)
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Scope")
	FString IncludePathsCsv;

	// Exclude paths (comma separated)
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Scope")
	FString ExcludePathsCsv;

	// Safety settings
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Safety")
	bool bDryRun;

	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Safety")
	int32 MaxChanges;

	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Safety")
	bool bCreateBackups;

	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Safety")
	bool bCloseEditor;

	// Output settings
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Output")
	FString OutputDirectory;

	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Output")
	bool bGenerateReports;

	// Python settings
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Python")
	FString PythonScriptPath;

	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Python")
	bool bEnablePythonLogging;

	// Auto-report settings
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Auto-Reporting", meta = (DisplayName = "Enable Auto-Reporting"))
	bool bEnableAutoReporting;

	UPROPERTY(config, BlueprintReadWrite, Category = "Auto-Reporting", meta = (DisplayName = "Cloudflare Worker URL"))
	FString AutoReportWorkerUrl;

	UPROPERTY(config, BlueprintReadWrite, Category = "Auto-Reporting", meta = (DisplayName = "Include Logs in Reports"))
	bool bIncludeLogsInReports;

	UPROPERTY(config, BlueprintReadWrite, Category = "Auto-Reporting", meta = (DisplayName = "Include Knowledge Data in Reports"))
	bool bIncludeKnowledgeInReports;

	UPROPERTY(config, BlueprintReadWrite, Category = "Auto-Reporting", meta = (DisplayName = "Report Errors"))
	bool bReportErrors;

	UPROPERTY(config, BlueprintReadWrite, Category = "Auto-Reporting", meta = (DisplayName = "Report Optimizations"))
	bool bReportOptimizations;

	UPROPERTY(config, BlueprintReadWrite, Category = "Auto-Reporting", meta = (DisplayName = "Report Sessions"))
	bool bReportSessions;

	UPROPERTY(config, BlueprintReadWrite, Category = "Auto-Reporting", meta = (DisplayName = "Anonymize Data"))
	bool bAnonymizeData;

	UPROPERTY(config, BlueprintReadWrite, Category = "Auto-Reporting", meta = (DisplayName = "Max Log Lines"))
	int32 MaxLogLines;

	// User consent for auto-reporting
	UPROPERTY(config, BlueprintReadWrite, Category = "Auto-Reporting", meta = (DisplayName = "User Consent"))
	bool bUserConsent;

	// Preset management
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Presets")
	TArray<FOptimizerPreset> CustomPresets;

	// Get singleton instance
	static UOptimizerSettings* Get();

	// Get default settings
	virtual void GetDefaultSettings();

	// Save settings
	virtual void SaveSettings();

	// Load settings
	virtual void LoadSettings();

	// Get preset by name
	FOptimizerPreset* GetPreset(const FString& PresetName);

	// Add custom preset
	void AddCustomPreset(const FOptimizerPreset& Preset);

	// Remove custom preset
	void RemoveCustomPreset(const FString& PresetName);

	// Get all available presets
	TArray<FString> GetAllPresets() const;

protected:
	// Default values
	virtual void SetToDefaults();
};
