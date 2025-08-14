// Copyright Epic Games, Inc. All Rights Reserved.

#include "SOptimizerPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SExpandableArea.h"

#include "EditorStyleSet.h"
#include "OptimizerSettings.h"
#include "PythonBridge.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformApplicationMisc.h"

void SOptimizerPanel::Construct(const FArguments& InArgs)
{
	// Initialize settings and Python bridge
	OptimizerSettings = UOptimizerSettings::Get();
	PythonBridge = NewObject<UPythonBridge>();
	
	if (PythonBridge)
	{
		PythonBridge->Initialize();
	}

	// Initialize UI state
	InitializeUI();

	ChildSlot
	[
		SNew(SBox)
		.Padding(8.0f)
		[
			SNew(SScrollBox)
			
			+ SScrollBox::Slot()
			[
				SNew(SGridPanel)
				.FillColumn(1, 1.0f)
				
				// Header
				+ SGridPanel::Slot(0, 0)
				.ColumnSpan(2)
				.Padding(4.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Magic Optimizer")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(FLinearColor(0.2f, 0.6f, 1.0f))
				]
				
				// Settings Button
				+ SGridPanel::Slot(0, 1)
				.ColumnSpan(2)
				.Padding(4.0f)
				.HAlign(HAlign_Right)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Settings")))
					.OnClicked(this, &SOptimizerPanel::OnSettingsClicked)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
				]
				
				// Profile Selection
				+ SGridPanel::Slot(0, 2)
				.Padding(4.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Target Profile:")))
				]
				
				+ SGridPanel::Slot(1, 2)
				.Padding(4.0f)
				[
					SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&TargetProfiles)
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
					{
						return SNew(STextBlock).Text(FText::FromString(*Item));
					})
					.OnSelectionChanged(this, &SOptimizerPanel::OnProfileSelected)
					.Content()
					[
						SNew(STextBlock)
						.Text(this, &SOptimizerPanel::GetCurrentProfileText)
					]
				]
				
				// Run Mode Selection
				+ SGridPanel::Slot(0, 3)
				.Padding(4.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Run Mode:")))
				]
				
				+ SGridPanel::Slot(1, 3)
				.Padding(4.0f)
				[
					SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&RunModes)
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
					{
						return SNew(STextBlock).Text(FText::FromString(*Item));
					})
					.OnSelectionChanged(this, &SOptimizerPanel::OnRunModeSelected)
					.Content()
					[
						SNew(STextBlock)
						.Text(this, &SOptimizerPanel::GetCurrentRunModeText)
					]
				]
				
				// Action Buttons
				+ SGridPanel::Slot(0, 4)
				.ColumnSpan(2)
				.Padding(4.0f)
				[
					SNew(SGridPanel)
					.FillColumn(0, 1.0f)
					.FillColumn(1, 1.0f)
					.FillColumn(2, 1.0f)
					.FillColumn(3, 1.0f)
					
					+ SGridPanel::Slot(0, 0)
					.Padding(2.0f)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Audit")))
						.OnClicked(this, &SOptimizerPanel::OnAuditClicked)
					]
					
					+ SGridPanel::Slot(1, 0)
					.Padding(2.0f)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Recommend")))
						.OnClicked(this, &SOptimizerPanel::OnRecommendClicked)
					]
					
					+ SGridPanel::Slot(2, 0)
					.Padding(2.0f)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Apply")))
						.OnClicked(this, &SOptimizerPanel::OnApplyClicked)
					]
					
					+ SGridPanel::Slot(3, 0)
					.Padding(2.0f)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Verify")))
						.OnClicked(this, &SOptimizerPanel::OnVerifyClicked)
					]
				]
			]
		]
	];
}

// Initialize UI state
void SOptimizerPanel::InitializeUI()
{
	// Initialize target profiles
	TargetProfiles = {
		MakeShareable(new FString(TEXT("PC_Ultra"))),
		MakeShareable(new FString(TEXT("PC_Balanced"))),
		MakeShareable(new FString(TEXT("Console_Optimized"))),
		MakeShareable(new FString(TEXT("Mobile_Low"))),
		MakeShareable(new FString(TEXT("Mobile_Ultra_Lite"))),
		MakeShareable(new FString(TEXT("VR"))),
		MakeShareable(new FString(TEXT("Cinematic"))),
		MakeShareable(new FString(TEXT("UI_Crisp"))),
		MakeShareable(new FString(TEXT("Archviz_High_Fidelity"))),
		MakeShareable(new FString(TEXT("Custom")))
	};
	
	// Initialize run modes
	RunModes = {
		MakeShareable(new FString(TEXT("Audit"))),
		MakeShareable(new FString(TEXT("Recommend"))),
		MakeShareable(new FString(TEXT("Apply"))),
		MakeShareable(new FString(TEXT("Verify")))
	};
	
	// Set defaults
	CurrentProfile = TEXT("PC_Balanced");
	CurrentRunMode = TEXT("Audit");
}

// Button event handlers
FReply SOptimizerPanel::OnAuditClicked()
{
	RunOptimizationPhase(TEXT("Audit"));
	return FReply::Handled();
}

FReply SOptimizerPanel::OnRecommendClicked()
{
	RunOptimizationPhase(TEXT("Recommend"));
	return FReply::Handled();
}

FReply SOptimizerPanel::OnApplyClicked()
{
	RunOptimizationPhase(TEXT("Apply"));
	return FReply::Handled();
}

FReply SOptimizerPanel::OnVerifyClicked()
{
	RunOptimizationPhase(TEXT("Verify"));
	return FReply::Handled();
}

FReply SOptimizerPanel::OnSettingsClicked()
{
	ShowNotification(TEXT("Settings clicked - functionality to be implemented"));
	return FReply::Handled();
}

// Profile selection
void SOptimizerPanel::OnProfileSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid())
	{
		CurrentProfile = *SelectedItem;
		ShowNotification(FString::Printf(TEXT("Profile selected: %s"), *CurrentProfile));
	}
}

FText SOptimizerPanel::GetCurrentProfileText() const
{
	return FText::FromString(CurrentProfile);
}

// Run mode selection
void SOptimizerPanel::OnRunModeSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid())
	{
		CurrentRunMode = *SelectedItem;
		ShowNotification(FString::Printf(TEXT("Run mode selected: %s"), *CurrentRunMode));
	}
}

FText SOptimizerPanel::GetCurrentRunModeText() const
{
	return FText::FromString(CurrentRunMode);
}

// Run optimization phase
void SOptimizerPanel::RunOptimizationPhase(const FString& Phase)
{
	if (!PythonBridge || !PythonBridge->IsPythonAvailable())
	{
		ShowNotification(TEXT("Python bridge not available"), false);
		return;
	}
	
	// Build run parameters
	FOptimizerRunParams Params;
	Params.Phase = Phase;
	Params.Profile = CurrentProfile;
	Params.bDryRun = true; // Default to dry run for safety
	Params.MaxChanges = 100;
	Params.bUseSelection = false;
	
	// Get selected categories
	Params.Categories = GetSelectedCategories();
	
	// Execute optimization
	FOptimizerResult Result = PythonBridge->RunOptimization(Params);
	
	if (Result.bSuccess)
	{
		ShowNotification(FString::Printf(TEXT("%s phase completed successfully"), *Phase));
	}
	else
	{
		ShowNotification(FString::Printf(TEXT("%s phase failed: %s"), *Phase, *Result.Message), false);
	}
}

// Get selected categories
TArray<FString> SOptimizerPanel::GetSelectedCategories() const
{
	TArray<FString> Categories;
	Categories.Add(TEXT("Textures"));
	Categories.Add(TEXT("Meshes"));
	Categories.Add(TEXT("Materials"));
	return Categories;
}

// Build run parameters
FOptimizerRunParams SOptimizerPanel::BuildRunParams(const FString& Phase) const
{
	FOptimizerRunParams Params;
	Params.Phase = Phase;
	Params.Profile = CurrentProfile;
	Params.bDryRun = true;
	Params.MaxChanges = 100;
	Params.bUseSelection = false;
	Params.Categories = GetSelectedCategories();
	return Params;
}

// Utility functions
void SOptimizerPanel::ShowNotification(const FString& Message, bool bIsSuccess)
{
	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: %s"), *Message);
	
	// In a real implementation, this would show a proper notification
	// For now, just log to console
}

void SOptimizerPanel::LogMessage(const FString& Message, bool bIsError)
{
	if (bIsError)
	{
		UE_LOG(LogTemp, Error, TEXT("MagicOptimizer: %s"), *Message);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: %s"), *Message);
	}
}

// Placeholder implementations for required methods
ECheckBoxState SOptimizerPanel::IsTexturesChecked() const { return ECheckBoxState::Checked; }
void SOptimizerPanel::OnTexturesChanged(ECheckBoxState NewState) {}
ECheckBoxState SOptimizerPanel::IsMeshesChecked() const { return ECheckBoxState::Checked; }
void SOptimizerPanel::OnMeshesChanged(ECheckBoxState NewState) {}
ECheckBoxState SOptimizerPanel::IsMaterialsChecked() const { return ECheckBoxState::Checked; }
void SOptimizerPanel::OnMaterialsChanged(ECheckBoxState NewState) {}
ECheckBoxState SOptimizerPanel::IsLevelsChecked() const { return ECheckBoxState::Unchecked; }
void SOptimizerPanel::OnLevelsChanged(ECheckBoxState NewState) {}
ECheckBoxState SOptimizerPanel::IsRuntimeChecked() const { return ECheckBoxState::Unchecked; }
void SOptimizerPanel::OnRuntimeChanged(ECheckBoxState NewState) {}
ECheckBoxState SOptimizerPanel::IsAudioChecked() const { return ECheckBoxState::Unchecked; }
void SOptimizerPanel::OnAudioChanged(ECheckBoxState NewState) {}

ECheckBoxState SOptimizerPanel::IsUseSelectionChecked() const { return ECheckBoxState::Unchecked; }
void SOptimizerPanel::OnUseSelectionChanged(ECheckBoxState NewState) {}
FString SOptimizerPanel::GetIncludePaths() const { return TEXT(""); }
void SOptimizerPanel::OnIncludePathsChanged(const FText& NewText) {}
FString SOptimizerPanel::GetExcludePaths() const { return TEXT(""); }
void SOptimizerPanel::OnExcludePathsChanged(const FText& NewText) {}

ECheckBoxState SOptimizerPanel::IsDryRunChecked() const { return ECheckBoxState::Checked; }
void SOptimizerPanel::OnDryRunChanged(ECheckBoxState NewState) {}
ECheckBoxState SOptimizerPanel::IsCreateBackupsChecked() const { return ECheckBoxState::Checked; }
void SOptimizerPanel::OnCreateBackupsChanged(ECheckBoxState NewState) {}
ECheckBoxState SOptimizerPanel::IsCloseEditorChecked() const { return ECheckBoxState::Unchecked; }
void SOptimizerPanel::OnCloseEditorChanged(ECheckBoxState NewState) {}

FString SOptimizerPanel::GetMaxChanges() const { return TEXT("100"); }
void SOptimizerPanel::OnMaxChangesChanged(const FText& NewText) {}
FString SOptimizerPanel::GetOutputDirectory() const { return TEXT(""); }
void SOptimizerPanel::OnOutputDirectoryChanged(const FText& NewText) {}

FString SOptimizerPanel::GetPythonScriptPath() const { return TEXT(""); }
void SOptimizerPanel::OnPythonScriptPathChanged(const FText& NewText) {}
ECheckBoxState SOptimizerPanel::IsPythonLoggingChecked() const { return ECheckBoxState::Checked; }
void SOptimizerPanel::OnPythonLoggingChanged(ECheckBoxState NewState) {}

void SOptimizerPanel::UpdateUI() {}
void SOptimizerPanel::RefreshUI() {}
