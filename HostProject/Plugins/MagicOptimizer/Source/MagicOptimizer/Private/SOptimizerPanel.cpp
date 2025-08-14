// Copyright Epic Games, Inc. All Rights Reserved.

#include "SOptimizerPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SVerticalBox.h"
#include "Widgets/Layout/SHorizontalBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Framework/Notifications/SNotificationList.h"
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
		SNew(SVerticalBox)

		// Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f)
		[
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Magic Optimizer")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
				.ColorAndOpacity(FLinearColor(0.2f, 0.6f, 1.0f))
			]
			
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Settings")))
				.OnClicked(this, &SOptimizerPanel::OnSettingsClicked)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton")
			]
		]

		// Main content
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(8.0f)
		[
			SNew(SScrollBox)
			
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)

				// Profile and Run Mode Section
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.0f)
				[
					SNew(SExpandableArea)
					.AreaTitle(FText::FromString(TEXT("Configuration")))
					.InitiallyCollapsed(false)
					.BodyContent()
					[
						SNew(SGridPanel)
						.FillColumn(1, 1.0f)
						
						+ SGridPanel::Slot(0, 0)
						.Padding(4.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Target Profile:")))
						]
						
						+ SGridPanel::Slot(1, 0)
						.Padding(4.0f)
						[
							SNew(SComboBox<TSharedPtr<FString>>)
							.OptionsSource(&TargetProfiles)
							.OnGenerateWidget(this, &SOptimizerPanel::OnGenerateProfileComboItem)
							.OnSelectionChanged(this, &SOptimizerPanel::OnProfileSelected)
							.Content()
							[
								SNew(STextBlock)
								.Text(this, &SOptimizerPanel::GetCurrentProfileText)
							]
						]
						
						+ SGridPanel::Slot(0, 1)
						.Padding(4.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Run Mode:")))
						]
						
						+ SGridPanel::Slot(1, 1)
						.Padding(4.0f)
						[
							SNew(SComboBox<TSharedPtr<FString>>)
							.OptionsSource(&RunModes)
							.OnGenerateWidget(this, &SOptimizerPanel::OnGenerateRunModeComboItem)
							.OnSelectionChanged(this, &SOptimizerPanel::OnRunModeSelected)
							.Content()
							[
								SNew(STextBlock)
								.Text(this, &SOptimizerPanel::GetCurrentRunModeText)
							]
						]
					]
				]

				// Categories Section
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.0f)
				[
					SNew(SExpandableArea)
					.AreaTitle(FText::FromString(TEXT("Asset Categories")))
					.InitiallyCollapsed(false)
					.BodyContent()
					[
						SNew(SUniformGridPanel)
						.SlotPadding(8.0f)
						
						+ SUniformGridPanel::Slot(0, 0)
						[
							SNew(SCheckBox)
							.IsChecked(this, &SOptimizerPanel::IsTexturesChecked)
							.OnCheckStateChanged(this, &SOptimizerPanel::OnTexturesChanged)
							.Content()
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Textures")))
							]
						]
						
						+ SUniformGridPanel::Slot(1, 0)
						[
							SNew(SCheckBox)
							.IsChecked(this, &SOptimizerPanel::IsMeshesChecked)
							.OnCheckStateChanged(this, &SOptimizerPanel::OnMeshesChanged)
							.Content()
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Meshes")))
							]
						]
						
						+ SUniformGridPanel::Slot(0, 1)
						[
							SNew(SCheckBox)
							.IsChecked(this, &SOptimizerPanel::IsMaterialsChecked)
							.OnCheckStateChanged(this, &SOptimizerPanel::OnMaterialsChanged)
							.Content()
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Materials")))
							]
						]
						
						+ SUniformGridPanel::Slot(1, 1)
						[
							SNew(SCheckBox)
							.IsChecked(this, &SOptimizerPanel::IsLevelsChecked)
							.OnCheckStateChanged(this, &SOptimizerPanel::OnLevelsChanged)
							.Content()
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Levels")))
							]
						]
						
						+ SUniformGridPanel::Slot(0, 2)
						[
							SNew(SCheckBox)
							.IsChecked(this, &SOptimizerPanel::IsRuntimeChecked)
							.OnCheckStateChanged(this, &SOptimizerPanel::OnRuntimeChanged)
							.Content()
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Runtime")))
							]
						]
						
						+ SUniformGridPanel::Slot(1, 2)
						[
							SNew(SCheckBox)
							.IsChecked(this, &SOptimizerPanel::IsAudioChecked)
							.OnCheckStateChanged(this, &SOptimizerPanel::OnAudioChanged)
							.Content()
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Audio")))
							]
						]
					]
				]

				// Scope Section
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.0f)
				[
					SNew(SExpandableArea)
					.AreaTitle(FText::FromString(TEXT("Scope")))
					.InitiallyCollapsed(false)
					.BodyContent()
					[
						SNew(SVerticalBox)
						
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(4.0f)
						[
							SNew(SCheckBox)
							.IsChecked(this, &SOptimizerPanel::IsUseSelectionChecked)
							.OnCheckStateChanged(this, &SOptimizerPanel::OnUseSelectionChanged)
							.Content()
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Use Selected Assets")))
							]
						]
						
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(4.0f)
						[
							SNew(SHorizontalBox)
							
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(4.0f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Include Paths:")))
							]
							
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(SEditableTextBox)
								.Text(this, &SOptimizerPanel::GetIncludePaths)
								.OnTextChanged(this, &SOptimizerPanel::OnIncludePathsChanged)
								.HintText(FText::FromString(TEXT("Comma separated paths (e.g., /Game/Textures, /Game/Meshes)")))
							]
						]
						
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(4.0f)
						[
							SNew(SHorizontalBox)
							
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(4.0f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Exclude Paths:")))
							]
							
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(SEditableTextBox)
								.Text(this, &SOptimizerPanel::GetExcludePaths)
								.OnTextChanged(this, &SOptimizerPanel::OnExcludePathsChanged)
								.HintText(FText::FromString(TEXT("Comma separated paths to exclude")))
							]
						]
					]
				]

				// Safety Section
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.0f)
				[
					SNew(SExpandableArea)
					.AreaTitle(FText::FromString(TEXT("Safety Settings")))
					.InitiallyCollapsed(false)
					.BodyContent()
					[
						SNew(SVerticalBox)
						
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(4.0f)
						[
							SNew(SCheckBox)
							.IsChecked(this, &SOptimizerPanel::IsDryRunChecked)
							.OnCheckStateChanged(this, &SOptimizerPanel::OnDryRunChanged)
							.Content()
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Dry Run (Preview changes only)")))
							]
						]
						
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(4.0f)
						[
							SNew(SCheckBox)
							.IsChecked(this, &SOptimizerPanel::IsCreateBackupsChecked)
							.OnCheckStateChanged(this, &SOptimizerPanel::OnCreateBackupsChanged)
							.Content()
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Create Backups")))
							]
						]
						
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(4.0f)
						[
							SNew(SHorizontalBox)
							
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(4.0f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Max Changes:")))
							]
							
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SEditableTextBox)
								.Text(this, &SOptimizerPanel::GetMaxChanges)
								.OnTextChanged(this, &SOptimizerPanel::OnMaxChangesChanged)
								.MinDesiredWidth(100.0f)
							]
						]
					]
				]

				// Output Section
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.0f)
				[
					SNew(SExpandableArea)
					.AreaTitle(FText::FromString(TEXT("Output Settings")))
					.InitiallyCollapsed(false)
					.BodyContent()
					[
						SNew(SHorizontalBox)
						
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4.0f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Output Directory:")))
						]
						
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(SEditableTextBox)
							.Text(this, &SOptimizerPanel::GetOutputDirectory)
							.OnTextChanged(this, &SOptimizerPanel::OnOutputDirectoryChanged)
							.HintText(FText::FromString(TEXT("Directory for output files")))
						]
					]
				]
			]
		]

		// Action Buttons
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f)
		[
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Run Audit")))
				.OnClicked(this, &SOptimizerPanel::OnAuditClicked)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
			]
			
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Recommend")))
				.OnClicked(this, &SOptimizerPanel::OnRecommendClicked)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Info")
			]
			
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Apply")))
				.OnClicked(this, &SOptimizerPanel::OnApplyClicked)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Warning")
			]
			
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Verify")))
				.OnClicked(this, &SOptimizerPanel::OnVerifyClicked)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
			]
			
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.Padding(4.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Ready to optimize")))
				.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f))
			]
		]
	];

	// Initial UI update
	UpdateUI();
}

void SOptimizerPanel::InitializeUI()
{
	// Initialize target profiles
	TargetProfiles.Empty();
	TargetProfiles.Add(TEXT("PC_Ultra"));
	TargetProfiles.Add(TEXT("PC_Balanced"));
	TargetProfiles.Add(TEXT("Console_Optimized"));
	TargetProfiles.Add(TEXT("Mobile_Low"));
	TargetProfiles.Add(TEXT("Mobile_Ultra_Lite"));
	TargetProfiles.Add(TEXT("VR"));
	TargetProfiles.Add(TEXT("Cinematic"));
	TargetProfiles.Add(TEXT("UI_Crisp"));
	TargetProfiles.Add(TEXT("Archviz_High_Fidelity"));

	// Initialize run modes
	RunModes.Empty();
	RunModes.Add(TEXT("Audit"));
	RunModes.Add(TEXT("Recommend"));
	RunModes.Add(TEXT("Apply"));
	RunModes.Add(TEXT("Verify"));

	// Set defaults
	CurrentProfile = TEXT("PC_Balanced");
	CurrentRunMode = TEXT("Audit");
}

void SOptimizerPanel::UpdateUI()
{
	// Update UI based on current settings
	if (OptimizerSettings)
	{
		CurrentProfile = OptimizerSettings->TargetProfile;
	}
}

// Event handlers implementation
FReply SOptimizerPanel::OnAuditClicked()
{
	RunOptimizationPhase(TEXT("audit"));
	return FReply::Handled();
}

FReply SOptimizerPanel::OnRecommendClicked()
{
	RunOptimizationPhase(TEXT("recommend"));
	return FReply::Handled();
}

FReply SOptimizerPanel::OnApplyClicked()
{
	RunOptimizationPhase(TEXT("apply"));
	return FReply::Handled();
}

FReply SOptimizerPanel::OnVerifyClicked()
{
	RunOptimizationPhase(TEXT("verify"));
	return FReply::Handled();
}

FReply SOptimizerPanel::OnSettingsClicked()
{
	// TODO: Open settings panel
	ShowNotification(TEXT("Settings panel not implemented yet"), false);
	return FReply::Handled();
}

// Checkbox state handlers
ECheckBoxState SOptimizerPanel::IsTexturesChecked() const
{
	return (OptimizerSettings && (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Textures)) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnTexturesChanged(ECheckBoxState NewState)
{
	if (OptimizerSettings)
	{
		if (NewState == ECheckBoxState::Checked)
		{
			OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Textures;
		}
		else
		{
			OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Textures);
		}
		OptimizerSettings->SaveSettings();
	}
}

ECheckBoxState SOptimizerPanel::IsMeshesChecked() const
{
	return (OptimizerSettings && (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Meshes)) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnMeshesChanged(ECheckBoxState NewState)
{
	if (OptimizerSettings)
	{
		if (NewState == ECheckBoxState::Checked)
		{
			OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Meshes;
		}
		else
		{
			OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Meshes);
		}
		OptimizerSettings->SaveSettings();
	}
}

ECheckBoxState SOptimizerPanel::IsMaterialsChecked() const
{
	return (OptimizerSettings && (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Materials)) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnMaterialsChanged(ECheckBoxState NewState)
{
	if (OptimizerSettings)
	{
		if (NewState == ECheckBoxState::Checked)
		{
			OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Materials;
		}
		else
		{
			OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Materials);
		}
		OptimizerSettings->SaveSettings();
	}
}

ECheckBoxState SOptimizerPanel::IsLevelsChecked() const
{
	return (OptimizerSettings && (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Levels)) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnLevelsChanged(ECheckBoxState NewState)
{
	if (OptimizerSettings)
	{
		if (NewState == ECheckBoxState::Checked)
		{
			OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Levels;
		}
		else
		{
			OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Levels);
		}
		OptimizerSettings->SaveSettings();
	}
}

ECheckBoxState SOptimizerPanel::IsRuntimeChecked() const
{
	return (OptimizerSettings && (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Runtime)) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnRuntimeChanged(ECheckBoxState NewState)
{
	if (OptimizerSettings)
	{
		if (NewState == ECheckBoxState::Checked)
		{
			OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Runtime;
		}
		else
		{
			OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Runtime);
		}
		OptimizerSettings->SaveSettings();
	}
}

ECheckBoxState SOptimizerPanel::IsAudioChecked() const
{
	return (OptimizerSettings && (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Audio)) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnAudioChanged(ECheckBoxState NewState)
{
	if (OptimizerSettings)
	{
		if (NewState == ECheckBoxState::Checked)
		{
			OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Audio;
		}
		else
		{
			OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Audio);
		}
		OptimizerSettings->SaveSettings();
	}
}

// Scope controls
ECheckBoxState SOptimizerPanel::IsUseSelectionChecked() const
{
	return (OptimizerSettings && OptimizerSettings->bUseSelection) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnUseSelectionChanged(ECheckBoxState NewState)
{
	if (OptimizerSettings)
	{
		OptimizerSettings->bUseSelection = (NewState == ECheckBoxState::Checked);
		OptimizerSettings->SaveSettings();
	}
}

FString SOptimizerPanel::GetIncludePaths() const
{
	return OptimizerSettings ? OptimizerSettings->IncludePathsCsv : TEXT("");
}

void SOptimizerPanel::OnIncludePathsChanged(const FText& NewText)
{
	if (OptimizerSettings)
	{
		OptimizerSettings->IncludePathsCsv = NewText.ToString();
		OptimizerSettings->SaveSettings();
	}
}

FString SOptimizerPanel::GetExcludePaths() const
{
	return OptimizerSettings ? OptimizerSettings->ExcludePathsCsv : TEXT("");
}

void SOptimizerPanel::OnExcludePathsChanged(const FText& NewText)
{
	if (OptimizerSettings)
	{
		OptimizerSettings->ExcludePathsCsv = NewText.ToString();
		OptimizerSettings->SaveSettings();
	}
}

// Safety controls
ECheckBoxState SOptimizerPanel::IsDryRunChecked() const
{
	return (OptimizerSettings && OptimizerSettings->bDryRun) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnDryRunChanged(ECheckBoxState NewState)
{
	if (OptimizerSettings)
	{
		OptimizerSettings->bDryRun = (NewState == ECheckBoxState::Checked);
		OptimizerSettings->SaveSettings();
	}
}

ECheckBoxState SOptimizerPanel::IsCreateBackupsChecked() const
{
	return (OptimizerSettings && OptimizerSettings->bCreateBackups) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnCreateBackupsChanged(ECheckBoxState NewState)
{
	if (OptimizerSettings)
	{
		OptimizerSettings->bCreateBackups = (NewState == ECheckBoxState::Checked);
		OptimizerSettings->SaveSettings();
	}
}

ECheckBoxState SOptimizerPanel::IsCloseEditorChecked() const
{
	return (OptimizerSettings && OptimizerSettings->bCloseEditor) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnCloseEditorChanged(ECheckBoxState NewState)
{
	if (OptimizerSettings)
	{
		OptimizerSettings->bCloseEditor = (NewState == ECheckBoxState::Checked);
		OptimizerSettings->SaveSettings();
	}
}

// Profile selection
TSharedRef<SWidget> SOptimizerPanel::OnGenerateProfileComboItem(FString Item) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(Item));
}

void SOptimizerPanel::OnProfileSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid() && OptimizerSettings)
	{
		CurrentProfile = *SelectedItem;
		OptimizerSettings->TargetProfile = CurrentProfile;
		OptimizerSettings->SaveSettings();
	}
}

FText SOptimizerPanel::GetCurrentProfileText() const
{
	return FText::FromString(CurrentProfile);
}

// Run mode selection
TSharedRef<SWidget> SOptimizerPanel::OnGenerateRunModeComboItem(FString Item) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(Item));
}

void SOptimizerPanel::OnRunModeSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid())
	{
		CurrentRunMode = *SelectedItem;
	}
}

FText SOptimizerPanel::GetCurrentRunModeText() const
{
	return FText::FromString(CurrentRunMode);
}

// Max changes input
FString SOptimizerPanel::GetMaxChanges() const
{
	return OptimizerSettings ? FString::FromInt(OptimizerSettings->MaxChanges) : TEXT("100");
}

void SOptimizerPanel::OnMaxChangesChanged(const FText& NewText)
{
	if (OptimizerSettings)
	{
		int32 MaxChanges = FCString::Atoi(*NewText.ToString());
		if (MaxChanges > 0)
		{
			OptimizerSettings->MaxChanges = MaxChanges;
			OptimizerSettings->SaveSettings();
		}
	}
}

// Output directory
FString SOptimizerPanel::GetOutputDirectory() const
{
	return OptimizerSettings ? OptimizerSettings->OutputDirectory : TEXT("Saved/MagicOptimizer");
}

void SOptimizerPanel::OnOutputDirectoryChanged(const FText& NewText)
{
	if (OptimizerSettings)
	{
		OptimizerSettings->OutputDirectory = NewText.ToString();
		OptimizerSettings->SaveSettings();
	}
}

// Python settings
FString SOptimizerPanel::GetPythonScriptPath() const
{
	return OptimizerSettings ? OptimizerSettings->PythonScriptPath : TEXT("Content/Python/magic_optimizer");
}

void SOptimizerPanel::OnPythonScriptPathChanged(const FText& NewText)
{
	if (OptimizerSettings)
	{
		OptimizerSettings->PythonScriptPath = NewText.ToString();
		OptimizerSettings->SaveSettings();
	}
}

ECheckBoxState SOptimizerPanel::IsPythonLoggingChecked() const
{
	return (OptimizerSettings && OptimizerSettings->bEnablePythonLogging) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnPythonLoggingChanged(ECheckBoxState NewState)
{
	if (OptimizerSettings)
	{
		OptimizerSettings->bEnablePythonLogging = (NewState == ECheckBoxState::Checked);
		OptimizerSettings->SaveSettings();
	}
}

// Utility functions
void SOptimizerPanel::ShowNotification(const FString& Message, bool bIsSuccess)
{
	FNotificationInfo Info(FText::FromString(Message));
	Info.bUseSuccessFailIcons = true;
	Info.ExpireDuration = 3.0f;
	
	if (bIsSuccess)
	{
		Info.Image = FEditorStyle::GetBrush("NotificationList.SuccessImage");
	}
	else
	{
		Info.Image = FEditorStyle::GetBrush("NotificationList.FailImage");
	}
	
	FSlateNotificationManager::Get().AddNotification(Info);
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

// Run optimization
void SOptimizerPanel::RunOptimizationPhase(const FString& Phase)
{
	if (!PythonBridge || !PythonBridge->IsPythonAvailable())
	{
		ShowNotification(TEXT("Python bridge not available"), false);
		return;
	}

	TArray<FString> Categories = GetSelectedCategories();
	if (Categories.Num() == 0)
	{
		ShowNotification(TEXT("No asset categories selected"), false);
		return;
	}

	// Show progress notification
	FString PhaseText = Phase.Mid(0, 1).ToUpper() + Phase.Mid(1);
	ShowNotification(FString::Printf(TEXT("Starting %s..."), *PhaseText), true);

	// Run optimization
	FOptimizerResult Result = PythonBridge->RunPhase(Phase, Categories);
	
	if (Result.bSuccess)
	{
		ShowNotification(FString::Printf(TEXT("%s completed successfully"), *PhaseText), true);
		LogMessage(FString::Printf(TEXT("%s completed: %s"), *PhaseText, *Result.Message));
	}
	else
	{
		ShowNotification(FString::Printf(TEXT("%s failed"), *PhaseText), false);
		LogMessage(FString::Printf(TEXT("%s failed: %s"), *PhaseText, *Result.Message), true);
	}
}

TArray<FString> SOptimizerPanel::GetSelectedCategories() const
{
	TArray<FString> Categories;
	
	if (OptimizerSettings)
	{
		if (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Textures)
		{
			Categories.Add(TEXT("Textures"));
		}
		if (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Meshes)
		{
			Categories.Add(TEXT("Meshes"));
		}
		if (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Materials)
		{
			Categories.Add(TEXT("Materials"));
		}
		if (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Levels)
		{
			Categories.Add(TEXT("Levels"));
		}
		if (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Runtime)
		{
			Categories.Add(TEXT("Runtime"));
		}
		if (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Audio)
		{
			Categories.Add(TEXT("Audio"));
		}
	}
	
	return Categories;
}

FOptimizerRunParams SOptimizerPanel::BuildRunParams(const FString& Phase) const
{
	FOptimizerRunParams Params;
	Params.Phase = Phase;
	Params.Categories = GetSelectedCategories();
	Params.Profile = CurrentProfile;
	Params.bDryRun = OptimizerSettings ? OptimizerSettings->bDryRun : true;
	Params.MaxChanges = OptimizerSettings ? OptimizerSettings->MaxChanges : 100;
	Params.IncludePaths = OptimizerSettings ? OptimizerSettings->IncludePathsCsv : TEXT("");
	Params.ExcludePaths = OptimizerSettings ? OptimizerSettings->ExcludePathsCsv : TEXT("");
	Params.bUseSelection = OptimizerSettings ? OptimizerSettings->bUseSelection : false;
	
	return Params;
}

void SOptimizerPanel::RefreshUI()
{
	UpdateUI();
}
