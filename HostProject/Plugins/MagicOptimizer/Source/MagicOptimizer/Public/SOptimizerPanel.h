// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "OptimizerSettings.h"
#include "PythonBridge.h"

class SCheckBox;
class SButton;
class SEditableTextBox;
class SScrollBox;
class SNotificationList;

class MAGICOPTIMIZER_API SOptimizerPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SOptimizerPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

protected:
	// Settings reference
	UPROPERTY()
	UOptimizerSettings* OptimizerSettings;

	// Python bridge reference
	UPROPERTY()
	UPythonBridge* PythonBridge;

	// UI state
	TArray<TSharedPtr<FString>> TargetProfiles;
	TArray<TSharedPtr<FString>> RunModes;
	FString CurrentProfile;
	FString CurrentRunMode;

	// UI event handlers
	FReply OnAuditClicked();
	FReply OnRecommendClicked();
	FReply OnApplyClicked();
	FReply OnVerifyClicked();
	FReply OnSettingsClicked();

	// Checkbox state handlers
	ECheckBoxState IsTexturesChecked() const;
	void OnTexturesChanged(ECheckBoxState NewState);
	ECheckBoxState IsMeshesChecked() const;
	void OnMeshesChanged(ECheckBoxState NewState);
	ECheckBoxState IsMaterialsChecked() const;
	void OnMaterialsChanged(ECheckBoxState NewState);
	ECheckBoxState IsLevelsChecked() const;
	void OnLevelsChanged(ECheckBoxState NewState);
	ECheckBoxState IsRuntimeChecked() const;
	void OnRuntimeChanged(ECheckBoxState NewState);
	ECheckBoxState IsAudioChecked() const;
	void OnAudioChanged(ECheckBoxState NewState);

	// Scope controls
	ECheckBoxState IsUseSelectionChecked() const;
	void OnUseSelectionChanged(ECheckBoxState NewState);
	FString GetIncludePaths() const;
	void OnIncludePathsChanged(const FText& NewText);
	FString GetExcludePaths() const;
	void OnExcludePathsChanged(const FText& NewText);

	// Safety controls
	ECheckBoxState IsDryRunChecked() const;
	void OnDryRunChanged(ECheckBoxState NewState);
	ECheckBoxState IsCreateBackupsChecked() const;
	void OnCreateBackupsChanged(ECheckBoxState NewState);
	ECheckBoxState IsCloseEditorChecked() const;
	void OnCloseEditorChanged(ECheckBoxState NewState);

	// Profile selection
	void OnProfileSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
	FText GetCurrentProfileText() const;

	// Run mode selection
	void OnRunModeSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
	FText GetCurrentRunModeText() const;

	// Max changes input
	FString GetMaxChanges() const;
	void OnMaxChangesChanged(const FText& NewText);

	// Output directory
	FString GetOutputDirectory() const;
	void OnOutputDirectoryChanged(const FText& NewText);

	// Python settings
	FString GetPythonScriptPath() const;
	void OnPythonScriptPathChanged(const FText& NewText);
	ECheckBoxState IsPythonLoggingChecked() const;
	void OnPythonLoggingChanged(ECheckBoxState NewState);

	// Utility functions
	void InitializeUI();
	void UpdateUI();
	void ShowNotification(const FString& Message, bool bIsSuccess = true);
	void LogMessage(const FString& Message, bool bIsError = false);

	// Run optimization
	void RunOptimizationPhase(const FString& Phase);
	TArray<FString> GetSelectedCategories() const;
	FOptimizerRunParams BuildRunParams(const FString& Phase) const;

	// UI refresh
	void RefreshUI();
};
