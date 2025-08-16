// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#include "STexturesTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"
#include "OptimizerSettings.h"
#include "PythonBridge.h"
#include "STextureAuditSection.h"
#include "STextureRecommendSection.h"
#include "ContentBrowserActions.h"

void STexturesTab::Construct(const FArguments& InArgs)
{
	// Initialize UI
	InitializeUI();
	
	// Load settings
	LoadSettings();

	ChildSlot
	[
		SNew(SBox)
		.Padding(16.0f)
		[
			SNew(SScrollBox)
			
			+ SScrollBox::Slot()
			[
				SNew(SGridPanel)
				.FillColumn(1, 1.0f)
				
				// Header
				+ SGridPanel::Slot(0, 0)
				.ColumnSpan(2)
				.Padding(8.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Texture Optimization")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
					.ColorAndOpacity(FLinearColor(0.2f, 0.6f, 1.0f))
				]

				// Profile Selection
				+ SGridPanel::Slot(0, 1)
				.Padding(8.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Target Profile:")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]
				
				+ SGridPanel::Slot(1, 1)
				.Padding(8.0f)
				[
					SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&TargetProfiles)
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
					{
						return SNew(STextBlock).Text(FText::FromString(*Item));
					})
					.OnSelectionChanged(this, &STexturesTab::OnProfileSelected)
					.Content()
					[
						SNew(STextBlock)
						.Text(this, &STexturesTab::GetCurrentProfileText)
					]
				]

				// Action Buttons
				+ SGridPanel::Slot(0, 2)
				.ColumnSpan(2)
				.Padding(8.0f)
				[
					SNew(SGridPanel)
					.FillColumn(0, 1.0f)
					.FillColumn(1, 1.0f)
					.FillColumn(2, 1.0f)
					.FillColumn(3, 1.0f)
					
					+ SGridPanel::Slot(0, 0)
					.Padding(4.0f)
					[
						SAssignNew(AuditButton, SButton)
						.Text(FText::FromString(TEXT("Audit")))
						.OnClicked(this, &STexturesTab::OnAuditClicked)
						.ButtonStyle(FAppStyle::Get(), "PrimaryButton")
					]
					
					+ SGridPanel::Slot(1, 0)
					.Padding(4.0f)
					[
						SAssignNew(RecommendButton, SButton)
						.Text(FText::FromString(TEXT("Recommend")))
						.OnClicked(this, &STexturesTab::OnRecommendClicked)
						.ButtonStyle(FAppStyle::Get(), "Button")
					]
					
					+ SGridPanel::Slot(2, 0)
					.Padding(4.0f)
					[
						SAssignNew(ApplyButton, SButton)
						.Text(FText::FromString(TEXT("Apply")))
						.OnClicked(this, &STexturesTab::OnApplyClicked)
						.ButtonStyle(FAppStyle::Get(), "Button")
					]
					
					+ SGridPanel::Slot(3, 0)
					.Padding(4.0f)
					[
						SAssignNew(VerifyButton, SButton)
						.Text(FText::FromString(TEXT("Verify")))
						.OnClicked(this, &STexturesTab::OnVerifyClicked)
						.ButtonStyle(FAppStyle::Get(), "Button")
					]
				]

				// Settings Section
				+ SGridPanel::Slot(0, 3)
				.ColumnSpan(2)
				.Padding(8.0f)
				[
					SNew(SExpandableArea)
					.AreaTitle(FText::FromString(TEXT("Settings")))
					.InitiallyCollapsed(true)
					.BodyContent()
					[
						SNew(SGridPanel)
						.FillColumn(1, 1.0f)

						// Safety Settings
						+ SGridPanel::Slot(0, 0)
						.Padding(8.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Safety:")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						]
						+ SGridPanel::Slot(1, 0)
						.Padding(8.0f)
						[
							SNew(SGridPanel)
							+ SGridPanel::Slot(0, 0)
							.Padding(4.0f)
							[
								SAssignNew(DryRunCheckBox, SCheckBox)
								.IsChecked(this, &STexturesTab::IsDryRunChecked)
								.OnCheckStateChanged(this, &STexturesTab::OnDryRunChanged)
								.Content() [ SNew(STextBlock).Text(FText::FromString(TEXT("Dry Run"))) ]
							]
							+ SGridPanel::Slot(1, 0)
							.Padding(4.0f)
							[
								SAssignNew(CreateBackupsCheckBox, SCheckBox)
								.IsChecked(this, &STexturesTab::IsCreateBackupsChecked)
								.OnCheckStateChanged(this, &STexturesTab::OnCreateBackupsChanged)
								.Content() [ SNew(STextBlock).Text(FText::FromString(TEXT("Create Backups"))) ]
							]
						]

						// Max Changes
						+ SGridPanel::Slot(0, 1)
						.Padding(8.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Max Changes:")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						]
						+ SGridPanel::Slot(1, 1)
						.Padding(8.0f)
						[
							SAssignNew(MaxChangesTextBox, SEditableTextBox)
							.Text(this, &STexturesTab::GetMaxChanges)
							.OnTextChanged(this, &STexturesTab::OnMaxChangesChanged)
							.ToolTipText(FText::FromString(TEXT("Maximum number of textures to modify in one operation")))
						]

						// Include Paths
						+ SGridPanel::Slot(0, 2)
						.Padding(8.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Include Paths:")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						]
						+ SGridPanel::Slot(1, 2)
						.Padding(8.0f)
						[
							SAssignNew(IncludePathsTextBox, SEditableTextBox)
							.Text(this, &STexturesTab::GetIncludePaths)
							.OnTextChanged(this, &STexturesTab::OnIncludePathsChanged)
							.ToolTipText(FText::FromString(TEXT("Comma-separated Content-relative paths to include (e.g., /Game/Maps,/Game/Props)")))
						]

						// Exclude Paths
						+ SGridPanel::Slot(0, 3)
						.Padding(8.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Exclude Paths:")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						]
						+ SGridPanel::Slot(1, 3)
						.Padding(8.0f)
						[
							SAssignNew(ExcludePathsTextBox, SEditableTextBox)
							.Text(this, &STexturesTab::GetExcludePaths)
							.OnTextChanged(this, &STexturesTab::OnExcludePathsChanged)
							.ToolTipText(FText::FromString(TEXT("Comma-separated Content-relative paths to exclude (e.g., /Game/ThirdParty,/Game/Legacy)")))
						]

						// Output Directory
						+ SGridPanel::Slot(0, 4)
						.Padding(8.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Output Directory:")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						]
						+ SGridPanel::Slot(1, 4)
						.Padding(8.0f)
						[
							SAssignNew(OutputDirectoryTextBox, SEditableTextBox)
							.Text(this, &STexturesTab::GetOutputDirectory)
							.OnTextChanged(this, &STexturesTab::OnOutputDirectoryChanged)
							.ToolTipText(FText::FromString(TEXT("Directory where optimization reports and backups will be saved")))
						]
					]
				]

				// Texture Audit Section
				+ SGridPanel::Slot(0, 4)
				.ColumnSpan(2)
				.Padding(8.0f)
				[
					SAssignNew(TextureAuditSection, STextureAuditSection)
				]

				// Texture Recommendations Section
				+ SGridPanel::Slot(0, 5)
				.ColumnSpan(2)
				.Padding(8.0f)
				[
					SAssignNew(TextureRecommendSection, STextureRecommendSection)
				]
			]
		]
	];

	// Wire up the section widgets
	if (TextureAuditSection.IsValid())
	{
		TextureAuditSection->OnCopyPath.BindLambda([this](const FString& Path)
		{
			ContentBrowserActions::CopyPathToClipboard(Path);
			ShowNotification(FString::Printf(TEXT("Copied path to clipboard: %s"), *Path));
		});
		
		TextureAuditSection->OnOpenInContentBrowser.BindLambda([this](const FString& Path)
		{
			const bool bOk = ContentBrowserActions::SyncToAssetPath(Path);
			ShowNotification(bOk ? TEXT("Opened in Content Browser") : FString::Printf(TEXT("Asset not found: %s"), *Path), bOk);
		});
	}
	
	if (TextureRecommendSection.IsValid())
	{
		TextureRecommendSection->OnCopyPath.BindLambda([this](const FString& Path)
		{
			ContentBrowserActions::CopyPathToClipboard(Path);
			ShowNotification(FString::Printf(TEXT("Copied path to clipboard: %s"), *Path));
		});
		
		TextureRecommendSection->OnOpenInContentBrowser.BindLambda([this](const FString& Path)
		{
			const bool bOk = ContentBrowserActions::SyncToAssetPath(Path);
			ShowNotification(bOk ? TEXT("Opened in Content Browser") : FString::Printf(TEXT("Asset not found: %s"), *Path), bOk);
		});
	}
}

STexturesTab::~STexturesTab()
{
	SaveSettings();
}

void STexturesTab::SetViewModel(TSharedPtr<FTextureTableViewModel> InViewModel)
{
	TextureTableViewModel = InViewModel;
	
	// Wire up ViewModel to UI widgets
	if (TextureAuditSection.IsValid() && TextureTableViewModel.IsValid())
	{
		TextureAuditSection->SetViewModel(TextureTableViewModel);
	}
}

// Initialize UI state
void STexturesTab::InitializeUI()
{
	// Initialize target profiles
	TargetProfiles.Empty();
	TargetProfiles.Add(MakeShareable(new FString(TEXT("PC_Ultra"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("PC_Balanced"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("PC_Low"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("Mobile_High"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("Mobile_Low"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("Console_PS5"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("Console_Xbox"))));
	
	// Set default profile
	CurrentProfile = TEXT("PC_Balanced");
}

// Load settings from config
void STexturesTab::LoadSettings()
{
	UOptimizerSettings* Settings = UOptimizerSettings::Get();
	if (Settings)
	{
		Settings->LoadSettings();
		
		if (!Settings->TargetProfile.IsEmpty())
		{
			CurrentProfile = Settings->TargetProfile;
		}
	}
}

// Save settings to config
void STexturesTab::SaveSettings()
{
	UOptimizerSettings* Settings = UOptimizerSettings::Get();
	if (Settings)
	{
		Settings->TargetProfile = CurrentProfile;
		Settings->SaveSettings();
	}
}

// Event handlers
FReply STexturesTab::OnAuditClicked()
{
	ShowNotification(TEXT("Audit functionality - Coming soon!"));
	return FReply::Handled();
}

FReply STexturesTab::OnRecommendClicked()
{
	ShowNotification(TEXT("Recommend functionality - Coming soon!"));
	return FReply::Handled();
}

FReply STexturesTab::OnApplyClicked()
{
	ShowNotification(TEXT("Apply functionality - Coming soon!"));
	return FReply::Handled();
}

FReply STexturesTab::OnVerifyClicked()
{
	ShowNotification(TEXT("Verify functionality - Coming soon!"));
	return FReply::Handled();
}

void STexturesTab::OnProfileSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection.IsValid())
	{
		CurrentProfile = *NewSelection;
		SaveSettings();
	}
}

FText STexturesTab::GetCurrentProfileText() const
{
	return FText::FromString(CurrentProfile);
}

// Settings handlers
void STexturesTab::OnDryRunChanged(ECheckBoxState NewState)
{
	// TODO: Implement dry run setting
}

ECheckBoxState STexturesTab::IsDryRunChecked() const
{
	return ECheckBoxState::Checked; // TODO: Get from settings
}

void STexturesTab::OnCreateBackupsChanged(ECheckBoxState NewState)
{
	// TODO: Implement backup setting
}

ECheckBoxState STexturesTab::IsCreateBackupsChecked() const
{
	return ECheckBoxState::Checked; // TODO: Get from settings
}

void STexturesTab::OnMaxChangesChanged(const FText& NewText)
{
	// TODO: Implement max changes setting
}

FText STexturesTab::GetMaxChanges() const
{
	return FText::FromString(TEXT("100")); // TODO: Get from settings
}

void STexturesTab::OnIncludePathsChanged(const FText& NewText)
{
	// TODO: Implement include paths setting
}

FText STexturesTab::GetIncludePaths() const
{
	return FText::FromString(TEXT("")); // TODO: Get from settings
}

void STexturesTab::OnExcludePathsChanged(const FText& NewText)
{
	// TODO: Implement exclude paths setting
}

FText STexturesTab::GetExcludePaths() const
{
	return FText::FromString(TEXT("")); // TODO: Get from settings
}

void STexturesTab::OnOutputDirectoryChanged(const FText& NewText)
{
	// TODO: Implement output directory setting
}

FText STexturesTab::GetOutputDirectory() const
{
	return FText::FromString(TEXT("")); // TODO: Get from settings
}

void STexturesTab::ShowNotification(const FString& Message, bool bIsSuccess)
{
	// TODO: Implement notification system
	UE_LOG(LogTemp, Log, TEXT("STexturesTab: %s"), *Message);
}
