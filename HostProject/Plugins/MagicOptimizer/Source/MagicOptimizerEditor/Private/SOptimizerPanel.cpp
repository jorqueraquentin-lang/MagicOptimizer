// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

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
#include "Widgets/SBoxPanel.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#include "EditorStyleSet.h"
#include "Slate/SlateTextureResource.h"
#include "ISettingsModule.h"
#include "Misc/ConfigCacheIni.h"
#include "MagicOptimizerLogging.h"
#include "OptimizerLogging.h"
#include "OptimizerSettings.h"
#include "PythonBridge.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "Services/Csv/TextureCsvReader.h"
#include "ContentBrowserActions.h"
#include "STextureAuditSection.h"
#include "STextureRecommendSection.h"
#include "STexturesTab.h"
#include "SMeshesTab.h"
#include "SMaterialsTab.h"
#include "SOptimizationTab.h"

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

	// Preload latest CSV if present
	LoadTextureAuditCsv();
	
	// Initialize tab widgets
	TexturesTab = SNew(STexturesTab);
	MeshesTab = SNew(SMeshesTab);
	MaterialsTab = SNew(SMaterialsTab);
	OptimizationTab = SNew(SOptimizationTab);

	// Wire up the new section widgets
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

	// Restore persisted settings into UI
    if (OptimizerSettings)
    {
        // Ensure latest values from config are loaded
        OptimizerSettings->LoadSettings();

        if (!OptimizerSettings->TargetProfile.IsEmpty())
        {
            CurrentProfile = OptimizerSettings->TargetProfile;
        }
        // No dropdown: ignore persisted RunMode
    }

	ChildSlot
	[
		SNew(SBox)
		.Padding(16.0f)
		[
			SNew(SVerticalBox)
			
			// Header with Settings
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f)
			[
				SNew(SHorizontalBox)
				
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Magic Optimizer")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.ColorAndOpacity(FLinearColor(0.2f, 0.6f, 1.0f))
				]
				
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Settings")))
					.OnClicked(this, &SOptimizerPanel::OnSettingsClicked)
					.ButtonStyle(FAppStyle::Get(), "FlatButton")
				]
			]

			// Tab Interface
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(8.0f)
			[
				SAssignNew(TabView, STabView<EOptimizerTab>)
				.TabRole(ETabRole::MajorTab)
				.OnTabChanged(this, &SOptimizerPanel::OnTabChanged)
				
				+ STabView<EOptimizerTab>::Slot(EOptimizerTab::Textures)
				.TabRole(ETabRole::MajorTab)
				[
					SNew(STabPanel<EOptimizerTab>)
					.TabRole(ETabRole::MajorTab)
					[
						SAssignNew(TabPanel, STabPanel<EOptimizerTab>)
						.TabRole(ETabRole::MajorTab)
						+ STabPanel<EOptimizerTab>::Slot(EOptimizerTab::Textures)
						[
							TexturesTab.ToSharedRef()
						]
						+ STabPanel<EOptimizerTab>::Slot(EOptimizerTab::Meshes)
						[
							MeshesTab.ToSharedRef()
						]
						+ STabPanel<EOptimizerTab>::Slot(EOptimizerTab::Materials)
						[
							MaterialsTab.ToSharedRef()
						]
						+ STabPanel<EOptimizerTab>::Slot(EOptimizerTab::Optimization)
						[
							OptimizationTab.ToSharedRef()
						]
					]
				]
			]
		]
	];

				// Options Section
                + SGridPanel::Slot(0, 4)
				.ColumnSpan(2)
				.Padding(4.0f)
				[
					SNew(SExpandableArea)
					.AreaTitle(FText::FromString(TEXT("Options")))
					.BodyContent()
					[
						SNew(SGridPanel)
						.FillColumn(1, 1.0f)

						// Categories (basic)
						+ SGridPanel::Slot(0, 0)
						.Padding(4.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Categories:")))
						]
						+ SGridPanel::Slot(1, 0)
						.Padding(4.0f)
						[
							SNew(SGridPanel)
							+ SGridPanel::Slot(0, 0)
							.Padding(2.0f)
							[
								SNew(SCheckBox)
								.IsChecked(this, &SOptimizerPanel::IsTexturesChecked)
								.OnCheckStateChanged(this, &SOptimizerPanel::OnTexturesChanged)
								.Content()
								[
									SNew(STextBlock).Text(FText::FromString(TEXT("Textures")))
								]
							]
							+ SGridPanel::Slot(1, 0)
							.Padding(2.0f)
							[
								SNew(SCheckBox)
								.IsChecked(this, &SOptimizerPanel::IsMeshesChecked)
								.OnCheckStateChanged(this, &SOptimizerPanel::OnMeshesChanged)
								.Content()
								[
									SNew(STextBlock).Text(FText::FromString(TEXT("Meshes")))
								]
							]
							+ SGridPanel::Slot(2, 0)
							.Padding(2.0f)
							[
								SNew(SCheckBox)
								.IsChecked(this, &SOptimizerPanel::IsMaterialsChecked)
								.OnCheckStateChanged(this, &SOptimizerPanel::OnMaterialsChanged)
								.Content()
								[
									SNew(STextBlock).Text(FText::FromString(TEXT("Materials")))
								]
							]
						]

						// Use Selection
						+ SGridPanel::Slot(0, 1)
						.Padding(4.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(FText::FromString(TEXT("Use Selection:")))
						]
						+ SGridPanel::Slot(1, 1)
						.Padding(4.0f)
						[
							SNew(SCheckBox)
							.IsChecked(this, &SOptimizerPanel::IsUseSelectionChecked)
							.OnCheckStateChanged(this, &SOptimizerPanel::OnUseSelectionChanged)
						]

						// Include Paths
						+ SGridPanel::Slot(0, 2)
						.Padding(4.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Include Paths (CSV):")))
							.ToolTipText(FText::FromString(TEXT("Comma-separated Content-relative paths to include (e.g., /Game/Maps,/Game/Props)")))
						]
						+ SGridPanel::Slot(1, 2)
						.Padding(4.0f)
						[
							SNew(SEditableTextBox)
							.Text(FText::FromString(GetIncludePaths()))
							.OnTextChanged(this, &SOptimizerPanel::OnIncludePathsChanged)
						]

						// Exclude Paths
						+ SGridPanel::Slot(0, 3)
						.Padding(4.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Exclude Paths (CSV):")))
							.ToolTipText(FText::FromString(TEXT("Comma-separated Content-relative paths to exclude (e.g., /Game/ThirdParty,/Game/Legacy)")))
						]
						+ SGridPanel::Slot(1, 3)
						.Padding(4.0f)
						[
							SNew(SEditableTextBox)
							.Text(FText::FromString(GetExcludePaths()))
							.OnTextChanged(this, &SOptimizerPanel::OnExcludePathsChanged)
						]

						// Dry Run / Backups
						+ SGridPanel::Slot(0, 4)
						.Padding(4.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(FText::FromString(TEXT("Safety:")))
						]
						+ SGridPanel::Slot(1, 4)
						.Padding(4.0f)
						[
							SNew(SGridPanel)
							+ SGridPanel::Slot(0, 0)
							.Padding(2.0f)
							[
								SNew(SCheckBox)
								.IsChecked(this, &SOptimizerPanel::IsDryRunChecked)
								.OnCheckStateChanged(this, &SOptimizerPanel::OnDryRunChanged)
								.Content() [ SNew(STextBlock).Text(FText::FromString(TEXT("Dry Run"))) ]
							]
							+ SGridPanel::Slot(1, 0)
							.Padding(2.0f)
							[
								SNew(SCheckBox)
								.IsChecked(this, &SOptimizerPanel::IsCreateBackupsChecked)
								.OnCheckStateChanged(this, &SOptimizerPanel::OnCreateBackupsChanged)
								.Content() [ SNew(STextBlock).Text(FText::FromString(TEXT("Create Backups"))) ]
							]
						]

						// Max Changes
						+ SGridPanel::Slot(0, 5)
						.Padding(4.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(FText::FromString(TEXT("Max Changes:")))
						]
						+ SGridPanel::Slot(1, 5)
						.Padding(4.0f)
						[
							SNew(SEditableTextBox)
							.Text(FText::FromString(GetMaxChanges()))
							.OnTextChanged(this, &SOptimizerPanel::OnMaxChangesChanged)
						]

						// Output Directory
						+ SGridPanel::Slot(0, 6)
						.Padding(4.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(FText::FromString(TEXT("Output Directory:")))
						]
						+ SGridPanel::Slot(1, 6)
						.Padding(4.0f)
						[
							SNew(SEditableTextBox)
							.Text(FText::FromString(GetOutputDirectory()))
							.OnTextChanged(this, &SOptimizerPanel::OnOutputDirectoryChanged)
						]

						// Python
						+ SGridPanel::Slot(0, 7)
						.Padding(4.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(FText::FromString(TEXT("Python Script Path:")))
						]
						+ SGridPanel::Slot(1, 7)
						.Padding(4.0f)
						[
							SNew(SEditableTextBox)
							.Text(FText::FromString(GetPythonScriptPath()))
							.OnTextChanged(this, &SOptimizerPanel::OnPythonScriptPathChanged)
						]
						+ SGridPanel::Slot(1, 8)
						.Padding(4.0f)
						[
							SNew(SCheckBox)
							.IsChecked(this, &SOptimizerPanel::IsPythonLoggingChecked)
							.OnCheckStateChanged(this, &SOptimizerPanel::OnPythonLoggingChanged)
															.Content() [ SNew(STextBlock).Text(FText::FromString(TEXT("Enable Python Logging"))) ]
						]
					]
				]

				// Output Area
                + SGridPanel::Slot(0, 5)
				.ColumnSpan(2)
				.Padding(4.0f)
				[
					SNew(SExpandableArea)
					.AreaTitle(FText::FromString(TEXT("Python Output")))
					.InitiallyCollapsed(false)
					.BodyContent()
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return FText::FromString(LastResultMessage.IsEmpty() ? TEXT("(no output)") : LastResultMessage); })
							.AutoWrapText(true)
						]
						+ SScrollBox::Slot()
						[
							SNew(STextBlock)
							.ColorAndOpacity(FLinearColor::Red)
							.Text_Lambda([this]() { return FText::FromString(LastStdErr); })
							.AutoWrapText(true)
						]
						+ SScrollBox::Slot()
						[
							SNew(STextBlock)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
							.Text_Lambda([this]() {
								if (LastAssetsProcessed <= 0 && LastAssetsModified <= 0 && LastResultMessage.IsEmpty())
								{
									return FText::GetEmpty();
								}
								const FString Summary = FString::Printf(TEXT("%s\nAssets processed: %d\nAssets modified: %d"), *LastResultMessage, LastAssetsProcessed, LastAssetsModified);
								return FText::FromString(Summary);
							})
						]
						+ SScrollBox::Slot()
						[
							SNew(SExpandableArea)
							.AreaTitle(FText::FromString(TEXT("Raw JSON (debug)")))
							.InitiallyCollapsed(true)
							.BodyContent()
							[
								SNew(STextBlock)
								.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
								.Text_Lambda([this]() { return FText::FromString(LastStdOut); })
								.AutoWrapText(true)
							]
						]
					]
				]

				// Audit Results
				+ SGridPanel::Slot(0, 6)
				.ColumnSpan(2)
				.Padding(4.0f)
				[
					SAssignNew(TextureAuditSection, STextureAuditSection)
				]

				// Recommendations
				+ SGridPanel::Slot(0, 7)
				.ColumnSpan(2)
				.Padding(4.0f)
				[
					SAssignNew(TextureRecommendSection, STextureRecommendSection)
				]
			]
		]
	];

	// Wire up ViewModel to UI widgets after creation
	if (TextureAuditSection.IsValid() && TextureTableViewModel.IsValid())
	{
		TextureAuditSection->SetViewModel(TextureTableViewModel);
		
		// Wire up automatic settings saving
		TextureAuditSection->OnSettingsChanged.BindLambda([this]()
		{
			if (TextureTableViewModel.IsValid() && OptimizerSettings)
			{
				TextureTableViewModel->SaveSettingsToConfig(OptimizerSettings);
			}
		});
	}

	// Wire up the TexturesTab ViewModel
	if (TexturesTab.IsValid() && TextureTableViewModel.IsValid())
	{
		TexturesTab->SetViewModel(TextureTableViewModel);
	}
}

// Save ViewModel settings when panel is destroyed
SOptimizerPanel::~SOptimizerPanel()
{
	if (TextureTableViewModel.IsValid() && OptimizerSettings)
	{
		TextureTableViewModel->SaveSettingsToConfig(OptimizerSettings);
	}
}

// Initialize UI state
void SOptimizerPanel::InitializeUI()
{
	// Initialize target profiles
	TargetProfiles.Empty();
	TargetProfiles.Add(MakeShareable(new FString(TEXT("PC_Ultra"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("PC_Balanced"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("Console_Optimized"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("Mobile_Low"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("Mobile_Ultra_Lite"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("VR"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("Cinematic"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("UI_Crisp"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("Archviz_High_Fidelity"))));
	TargetProfiles.Add(MakeShareable(new FString(TEXT("Custom"))));
	
    // Set defaults
    CurrentProfile = TEXT("PC_Balanced");

	// Initialize ViewModel
	TextureTableViewModel = MakeShareable(new FTextureTableViewModel());
	
	// Load settings into ViewModel
	if (OptimizerSettings)
	{
		TextureTableViewModel->LoadSettingsFromConfig(OptimizerSettings);
	}
}

// Button event handlers
FReply SOptimizerPanel::OnAuditClicked()
{
    MagicOptimizerLog::AppendLine(TEXT("UI: Audit button clicked"));
	RunOptimizationPhase(TEXT("Audit"));
	return FReply::Handled();
}

FReply SOptimizerPanel::OnRecommendClicked()
{
    MagicOptimizerLog::AppendLine(TEXT("UI: Recommend button clicked"));
	RunOptimizationPhase(TEXT("Recommend"));
	return FReply::Handled();
}

FReply SOptimizerPanel::OnApplyClicked()
{
    MagicOptimizerLog::AppendLine(TEXT("UI: Apply button clicked"));
	RunOptimizationPhase(TEXT("Apply"));
	return FReply::Handled();
}

FReply SOptimizerPanel::OnVerifyClicked()
{
    MagicOptimizerLog::AppendLine(TEXT("UI: Verify button clicked"));
	RunOptimizationPhase(TEXT("Verify"));
	return FReply::Handled();
}

FReply SOptimizerPanel::OnSettingsClicked()
{
	// Open our settings page in Project Settings if available
    if (FModuleManager::Get().IsModuleLoaded("Settings"))
    {
        ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
        // Focus the exact registered settings page
        SettingsModule.ShowViewer(TEXT("Project"), TEXT("Plugins"), TEXT("MagicOptimizer"));
    }
	else
	{
		ShowNotification(TEXT("Settings module unavailable"), false);
	}
	return FReply::Handled();
}

// Profile selection
void SOptimizerPanel::OnProfileSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid())
	{
		CurrentProfile = *SelectedItem;
		ShowNotification(FString::Printf(TEXT("Profile selected: %s"), *CurrentProfile));

		// Persist to settings
		if (OptimizerSettings)
		{
			OptimizerSettings->TargetProfile = CurrentProfile;
			OptimizerSettings->SaveSettings();
		}
	}
}

FText SOptimizerPanel::GetCurrentProfileText() const
{
	return FText::FromString(CurrentProfile);
}

// Removed run mode selection handlers (using buttons only)

// Run optimization phase
void SOptimizerPanel::RunOptimizationPhase(const FString& Phase)
{
	if (!PythonBridge || !PythonBridge->IsPythonAvailable())
	{
		ShowNotification(TEXT("Python bridge not available"), false);
        MagicOptimizerLog::AppendLine(TEXT("Error: Python bridge not available"));
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
    MagicOptimizerLog::AppendLine(FString::Printf(TEXT("Run: Phase=%s Profile=%s"), *Phase, *CurrentProfile));
    FOptimizerResult Result = PythonBridge->RunOptimization(Params);
    LastStdOut = Result.StdOut;
    LastStdErr = Result.StdErr;
    LastAssetsProcessed = Result.AssetsProcessed;
    LastAssetsModified = Result.AssetsModified;
    LastResultMessage = Result.Message;
	
	if (Result.bSuccess)
	{
		ShowNotification(FString::Printf(TEXT("%s phase completed successfully"), *Phase));
        MagicOptimizerLog::AppendLine(FString::Printf(TEXT("Success: %s | %s | Processed=%d Modified=%d"), *Phase, *Result.Message, Result.AssetsProcessed, Result.AssetsModified));
		if (Phase == TEXT("Audit"))
		{
			LoadTextureAuditCsv();
			if (TextureListView.IsValid())
			{
				TextureListView->RequestListRefresh();
			}
		}
		else if (Phase == TEXT("Recommend"))
		{
			LoadTextureRecommendationsCsv();
			if (TextureRecListView.IsValid())
			{
				TextureRecListView->RequestListRefresh();
			}
		}
	}
	else
	{
		ShowNotification(FString::Printf(TEXT("%s phase failed: %s"), *Phase, *Result.Message), false);
        MagicOptimizerLog::AppendLine(FString::Printf(TEXT("Failure: %s | %s | Err=%s"), *Phase, *Result.Message, *Result.StdErr));
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

void SOptimizerPanel::LoadTextureAuditCsv()
{
	AllTextureRows.Empty();
	TArray<FTextureAuditRowPtr> Parsed;
	const bool bOk = TextureCsvReader::ReadAuditCsv(OptimizerSettings, Parsed);
	if (!bOk)
	{
		MagicOptimizerLog::AppendLine(TEXT("UI: Texture audit CSV not found or unreadable"));
		TextureRows.Empty();
		return;
	}
	AllTextureRows = MoveTemp(Parsed);
	MagicOptimizerLog::AppendLine(FString::Printf(TEXT("UI: Loaded %d texture rows"), AllTextureRows.Num()));
	
	// Update the new widgets and ViewModel
	if (TextureTableViewModel.IsValid())
	{
		TextureTableViewModel->SetSourceData(AllTextureRows);
	}
	
	if (TextureAuditSection.IsValid())
	{
		TextureAuditSection->SetTextureRows(AllTextureRows);
	}
	
	// Legacy: Apply current filters and sorting
	ApplyTextureFilterAndSort();
}

TSharedRef<ITableRow> SOptimizerPanel::OnGenerateTextureRow(FTextureAuditRowPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FTextureAuditRowPtr>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(0.6f)
		[
			SNew(STextBlock).Text(FText::FromString(Item->Path))
		]
		+ SHorizontalBox::Slot().FillWidth(0.15f).HAlign(HAlign_Right)
		[
			SNew(STextBlock).Text(FText::FromString(FString::FromInt(Item->Width)))
		]
		+ SHorizontalBox::Slot().FillWidth(0.15f).HAlign(HAlign_Right)
		[
			SNew(STextBlock).Text(FText::FromString(FString::FromInt(Item->Height)))
		]
		+ SHorizontalBox::Slot().FillWidth(0.1f)
		[
			SNew(STextBlock).Text(FText::FromString(Item->Format))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Copy")))
				.OnClicked_Lambda([this, Item]()
				{
					if (Item.IsValid())
					{
						ContentBrowserActions::CopyPathToClipboard(Item->Path);
						ShowNotification(FString::Printf(TEXT("Copied path to clipboard: %s"), *Item->Path));
					}
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Open")))
				.OnClicked_Lambda([this, Item]()
				{
					if (!Item.IsValid())
					{
						return FReply::Handled();
					}
					const bool bOk = ContentBrowserActions::SyncToAssetPath(Item->Path);
					ShowNotification(bOk ? TEXT("Opened in Content Browser") : FString::Printf(TEXT("Asset not found: %s"), *Item->Path), bOk);
					return FReply::Handled();
				})
			]
		]
	];
}

void SOptimizerPanel::LoadTextureRecommendationsCsv()
{
	AllTextureRecRows.Empty();
	TArray<FTextureRecRowPtr> Parsed;
	const bool bOk = TextureCsvReader::ReadRecommendationsCsv(OptimizerSettings, Parsed);
	if (!bOk)
	{
		MagicOptimizerLog::AppendLine(TEXT("UI: Texture recommendations CSV not found or unreadable"));
		TextureRecRows.Empty();
		return;
	}
	AllTextureRecRows = MoveTemp(Parsed);
	TextureRecRows = AllTextureRecRows;
	
	// Update the new widget
	if (TextureRecommendSection.IsValid())
	{
		TextureRecommendSection->SetRecommendationRows(AllTextureRecRows);
	}
	
	MagicOptimizerLog::AppendLine(FString::Printf(TEXT("UI: Loaded %d recommendation rows"), TextureRecRows.Num()));
}

TSharedRef<ITableRow> SOptimizerPanel::OnGenerateTextureRecRow(FTextureRecRowPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<FTextureRecRowPtr>, OwnerTable)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(0.35f)
        [
            SNew(STextBlock).Text(FText::FromString(Item->Path))
        ]
        + SHorizontalBox::Slot().FillWidth(0.08f).HAlign(HAlign_Right)
        [
            SNew(STextBlock).Text(FText::FromString(FString::FromInt(Item->Width)))
        ]
        + SHorizontalBox::Slot().FillWidth(0.08f).HAlign(HAlign_Right)
        [
            SNew(STextBlock).Text(FText::FromString(FString::FromInt(Item->Height)))
        ]
        + SHorizontalBox::Slot().FillWidth(0.12f)
        [
            SNew(STextBlock).Text(FText::FromString(Item->Format))
        ]
        + SHorizontalBox::Slot().FillWidth(0.17f)
        [
            SNew(STextBlock).Text(FText::FromString(Item->Issues))
        ]
        + SHorizontalBox::Slot().FillWidth(0.20f)
        [
            SNew(STextBlock).Text(FText::FromString(Item->Recommendations))
        ]
        + SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Copy")))
                .OnClicked_Lambda([this, Item]()
                {
                    if (Item.IsValid())
                    {
                        FPlatformApplicationMisc::ClipboardCopy(*Item->Path);
                        ShowNotification(FString::Printf(TEXT("Copied path to clipboard: %s"), *Item->Path));
                    }
                    return FReply::Handled();
                })
            ]
            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Open")))
                .OnClicked_Lambda([this, Item]()
                {
                    if (!Item.IsValid())
                    {
                        return FReply::Handled();
                    }

                    FString ObjectPath = Item->Path;
                    if (!ObjectPath.Contains(TEXT(".")))
                    {
                        FString PackageName;
                        FString AssetName;
                        if (ObjectPath.Split(TEXT("/"), &PackageName, &AssetName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
                        {
                            ObjectPath = ObjectPath + TEXT(".") + AssetName;
                        }
                    }

                    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
                    const FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*ObjectPath));
                    if (AssetData.IsValid())
                    {
                        FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
                        TArray<FAssetData> AssetsToSync;
                        AssetsToSync.Add(AssetData);
                        ContentBrowserModule.Get().SyncBrowserToAssets(AssetsToSync);
                        ShowNotification(TEXT("Opened in Content Browser"));
                    }
                    else
                    {
                        ShowNotification(FString::Printf(TEXT("Asset not found: %s"), *ObjectPath), false);
                    }
                    return FReply::Handled();
                })
            ]
        ]
    ];
}
void SOptimizerPanel::SortTextureRows()
{
	auto CmpPathAsc = [](const FTextureAuditRowPtr& A, const FTextureAuditRowPtr& B){ return A->Path < B->Path; };
	auto CmpPathDesc = [](const FTextureAuditRowPtr& A, const FTextureAuditRowPtr& B){ return A->Path > B->Path; };
	auto CmpWidthAsc = [](const FTextureAuditRowPtr& A, const FTextureAuditRowPtr& B){ return A->Width < B->Width; };
	auto CmpWidthDesc = [](const FTextureAuditRowPtr& A, const FTextureAuditRowPtr& B){ return A->Width > B->Width; };
	auto CmpHeightAsc = [](const FTextureAuditRowPtr& A, const FTextureAuditRowPtr& B){ return A->Height < B->Height; };
	auto CmpHeightDesc = [](const FTextureAuditRowPtr& A, const FTextureAuditRowPtr& B){ return A->Height > B->Height; };
	auto CmpFormatAsc = [](const FTextureAuditRowPtr& A, const FTextureAuditRowPtr& B){ return A->Format < B->Format; };
	auto CmpFormatDesc = [](const FTextureAuditRowPtr& A, const FTextureAuditRowPtr& B){ return A->Format > B->Format; };

	switch (CurrentSortColumn)
	{
		case ETextureSortColumn::Path: TextureRows.StableSort(bSortAscending ? CmpPathAsc : CmpPathDesc); break;
		case ETextureSortColumn::Width: TextureRows.StableSort(bSortAscending ? CmpWidthAsc : CmpWidthDesc); break;
		case ETextureSortColumn::Height: TextureRows.StableSort(bSortAscending ? CmpHeightAsc : CmpHeightDesc); break;
		case ETextureSortColumn::Format: TextureRows.StableSort(bSortAscending ? CmpFormatAsc : CmpFormatDesc); break;
	}
}

EColumnSortMode::Type SOptimizerPanel::GetSortModeForColumn(ETextureSortColumn Column) const
{
	if (CurrentSortColumn != Column) { return EColumnSortMode::None; }
	return bSortAscending ? EColumnSortMode::Ascending : EColumnSortMode::Descending;
}

void SOptimizerPanel::OnHeaderColumnSort(const EColumnSortPriority::Type /*SortPriority*/, const FName& ColumnId, const EColumnSortMode::Type NewSortMode)
{
	if (ColumnId == "Path") { CurrentSortColumn = ETextureSortColumn::Path; }
	else if (ColumnId == "Width") { CurrentSortColumn = ETextureSortColumn::Width; }
	else if (ColumnId == "Height") { CurrentSortColumn = ETextureSortColumn::Height; }
	else if (ColumnId == "Format") { CurrentSortColumn = ETextureSortColumn::Format; }
	else { return; }

	bSortAscending = (NewSortMode != EColumnSortMode::Descending);
	SortTextureRows();
	if (TextureListView.IsValid()) { TextureListView->RequestListRefresh(); }
}

FReply SOptimizerPanel::OnSortByPath()
{
	if (CurrentSortColumn == ETextureSortColumn::Path) { bSortAscending = !bSortAscending; } else { CurrentSortColumn = ETextureSortColumn::Path; bSortAscending = true; }
	SortTextureRows();
	if (TextureListView.IsValid()) { TextureListView->RequestListRefresh(); }
	return FReply::Handled();
}

FReply SOptimizerPanel::OnSortByWidth()
{
	if (CurrentSortColumn == ETextureSortColumn::Width) { bSortAscending = !bSortAscending; } else { CurrentSortColumn = ETextureSortColumn::Width; bSortAscending = true; }
	SortTextureRows();
	if (TextureListView.IsValid()) { TextureListView->RequestListRefresh(); }
	return FReply::Handled();
}

FReply SOptimizerPanel::OnSortByHeight()
{
	if (CurrentSortColumn == ETextureSortColumn::Height) { bSortAscending = !bSortAscending; } else { CurrentSortColumn = ETextureSortColumn::Height; bSortAscending = true; }
	SortTextureRows();
	if (TextureListView.IsValid()) { TextureListView->RequestListRefresh(); }
	return FReply::Handled();
}

FReply SOptimizerPanel::OnSortByFormat()
{
	if (CurrentSortColumn == ETextureSortColumn::Format) { bSortAscending = !bSortAscending; } else { CurrentSortColumn = ETextureSortColumn::Format; bSortAscending = true; }
	SortTextureRows();
	if (TextureListView.IsValid()) { TextureListView->RequestListRefresh(); }
	return FReply::Handled();
}

void SOptimizerPanel::ApplyTextureFilterAndSort()
{
	TextureRows.Reset();
	const FString Needle = TextureFilterText.TrimStartAndEnd().ToLower();
	for (const FTextureAuditRowPtr& Row : AllTextureRows)
	{
		if (!Row.IsValid()) { continue; }
		if (Row->Width < FilterMinWidth || Row->Height < FilterMinHeight) { continue; }
		bool bMatches = true;
		if (!Needle.IsEmpty())
		{
			const FString PathLower = Row->Path.ToLower();
			const FString FormatLower = Row->Format.ToLower();
			bMatches = PathLower.Contains(Needle) || FormatLower.Contains(Needle);
		}
		if (bMatches)
		{
			TextureRows.Add(Row);
		}
	}
	SortTextureRows();
	if (TextureListView.IsValid()) { TextureListView->RequestListRefresh(); }
}

void SOptimizerPanel::OnFilterTextChanged(const FText& NewText)
{
	TextureFilterText = NewText.ToString();
	ApplyTextureFilterAndSort();
}

void SOptimizerPanel::OnMinWidthChanged(const FText& NewText)
{
	FilterMinWidth = FCString::Atoi(*NewText.ToString());
	ApplyTextureFilterAndSort();
}

void SOptimizerPanel::OnMinHeightChanged(const FText& NewText)
{
	FilterMinHeight = FCString::Atoi(*NewText.ToString());
	ApplyTextureFilterAndSort();
}

FReply SOptimizerPanel::OnClearTextureFilters()
{
	TextureFilterText.Reset();
	FilterMinWidth = 0;
	FilterMinHeight = 0;
	ApplyTextureFilterAndSort();
	return FReply::Handled();
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
ECheckBoxState SOptimizerPanel::IsTexturesChecked() const
{
	if (!OptimizerSettings) return ECheckBoxState::Unchecked;
	const bool b = (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Textures) != 0;
	return b ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SOptimizerPanel::OnTexturesChanged(ECheckBoxState NewState)
{
	if (!OptimizerSettings) return;
	if (NewState == ECheckBoxState::Checked)
		OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Textures;
	else
		OptimizerSettings->CategoryMask &= ~(uint8)EOptimizerCategory::Textures;
	OptimizerSettings->SaveSettings();
}
ECheckBoxState SOptimizerPanel::IsMeshesChecked() const
{
	if (!OptimizerSettings) return ECheckBoxState::Unchecked;
	const bool b = (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Meshes) != 0;
	return b ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SOptimizerPanel::OnMeshesChanged(ECheckBoxState NewState)
{
	if (!OptimizerSettings) return;
	if (NewState == ECheckBoxState::Checked)
		OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Meshes;
	else
		OptimizerSettings->CategoryMask &= ~(uint8)EOptimizerCategory::Meshes;
	OptimizerSettings->SaveSettings();
}
ECheckBoxState SOptimizerPanel::IsMaterialsChecked() const
{
	if (!OptimizerSettings) return ECheckBoxState::Unchecked;
	const bool b = (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Materials) != 0;
	return b ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SOptimizerPanel::OnMaterialsChanged(ECheckBoxState NewState)
{
	if (!OptimizerSettings) return;
	if (NewState == ECheckBoxState::Checked)
		OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Materials;
	else
		OptimizerSettings->CategoryMask &= ~(uint8)EOptimizerCategory::Materials;
	OptimizerSettings->SaveSettings();
}
ECheckBoxState SOptimizerPanel::IsLevelsChecked() const
{
	if (!OptimizerSettings) return ECheckBoxState::Unchecked;
	const bool b = (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Levels) != 0;
	return b ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SOptimizerPanel::OnLevelsChanged(ECheckBoxState NewState)
{
	if (!OptimizerSettings) return;
	if (NewState == ECheckBoxState::Checked)
		OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Levels;
	else
		OptimizerSettings->CategoryMask &= ~(uint8)EOptimizerCategory::Levels;
	OptimizerSettings->SaveSettings();
}
ECheckBoxState SOptimizerPanel::IsRuntimeChecked() const
{
	if (!OptimizerSettings) return ECheckBoxState::Unchecked;
	const bool b = (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Runtime) != 0;
	return b ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SOptimizerPanel::OnRuntimeChanged(ECheckBoxState NewState)
{
	if (!OptimizerSettings) return;
	if (NewState == ECheckBoxState::Checked)
		OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Runtime;
	else
		OptimizerSettings->CategoryMask &= ~(uint8)EOptimizerCategory::Runtime;
	OptimizerSettings->SaveSettings();
}
ECheckBoxState SOptimizerPanel::IsAudioChecked() const
{
	if (!OptimizerSettings) return ECheckBoxState::Unchecked;
	const bool b = (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Audio) != 0;
	return b ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SOptimizerPanel::OnAudioChanged(ECheckBoxState NewState)
{
	if (!OptimizerSettings) return;
	if (NewState == ECheckBoxState::Checked)
		OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Audio;
	else
		OptimizerSettings->CategoryMask &= ~(uint8)EOptimizerCategory::Audio;
	OptimizerSettings->SaveSettings();
}

ECheckBoxState SOptimizerPanel::IsUseSelectionChecked() const
{
	if (!OptimizerSettings) return ECheckBoxState::Unchecked;
	return OptimizerSettings->bUseSelection ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SOptimizerPanel::OnUseSelectionChanged(ECheckBoxState NewState)
{
	if (!OptimizerSettings) return;
	OptimizerSettings->bUseSelection = (NewState == ECheckBoxState::Checked);
	OptimizerSettings->SaveSettings();
}
FString SOptimizerPanel::GetIncludePaths() const
{
	return OptimizerSettings ? OptimizerSettings->IncludePathsCsv : TEXT("");
}
void SOptimizerPanel::OnIncludePathsChanged(const FText& NewText)
{
	if (!OptimizerSettings) return;
	OptimizerSettings->IncludePathsCsv = NewText.ToString();
	OptimizerSettings->SaveSettings();
}
FString SOptimizerPanel::GetExcludePaths() const
{
	return OptimizerSettings ? OptimizerSettings->ExcludePathsCsv : TEXT("");
}
void SOptimizerPanel::OnExcludePathsChanged(const FText& NewText)
{
	if (!OptimizerSettings) return;
	OptimizerSettings->ExcludePathsCsv = NewText.ToString();
	OptimizerSettings->SaveSettings();
}

ECheckBoxState SOptimizerPanel::IsDryRunChecked() const
{
	if (!OptimizerSettings) return ECheckBoxState::Checked;
	return OptimizerSettings->bDryRun ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SOptimizerPanel::OnDryRunChanged(ECheckBoxState NewState)
{
	if (!OptimizerSettings) return;
	OptimizerSettings->bDryRun = (NewState == ECheckBoxState::Checked);
	OptimizerSettings->SaveSettings();
}
ECheckBoxState SOptimizerPanel::IsCreateBackupsChecked() const
{
	if (!OptimizerSettings) return ECheckBoxState::Checked;
	return OptimizerSettings->bCreateBackups ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SOptimizerPanel::OnCreateBackupsChanged(ECheckBoxState NewState)
{
	if (!OptimizerSettings) return;
	OptimizerSettings->bCreateBackups = (NewState == ECheckBoxState::Checked);
	OptimizerSettings->SaveSettings();
}
ECheckBoxState SOptimizerPanel::IsCloseEditorChecked() const
{
	if (!OptimizerSettings) return ECheckBoxState::Unchecked;
	return OptimizerSettings->bCloseEditor ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SOptimizerPanel::OnCloseEditorChanged(ECheckBoxState NewState)
{
	if (!OptimizerSettings) return;
	OptimizerSettings->bCloseEditor = (NewState == ECheckBoxState::Checked);
	OptimizerSettings->SaveSettings();
}

FString SOptimizerPanel::GetMaxChanges() const
{
	if (!OptimizerSettings) return TEXT("100");
	return FString::FromInt(OptimizerSettings->MaxChanges);
}
void SOptimizerPanel::OnMaxChangesChanged(const FText& NewText)
{
	if (!OptimizerSettings) return;
	int32 Parsed = OptimizerSettings->MaxChanges;
	const FString S = NewText.ToString();
	Parsed = FCString::Atoi(*S);
	Parsed = FMath::Clamp(Parsed, 0, 1000000);
	OptimizerSettings->MaxChanges = Parsed;
	OptimizerSettings->SaveSettings();
}
FString SOptimizerPanel::GetOutputDirectory() const
{
	return OptimizerSettings ? OptimizerSettings->OutputDirectory : TEXT("");
}
void SOptimizerPanel::OnOutputDirectoryChanged(const FText& NewText)
{
	if (!OptimizerSettings) return;
	OptimizerSettings->OutputDirectory = NewText.ToString();
	OptimizerSettings->SaveSettings();
}

FString SOptimizerPanel::GetPythonScriptPath() const
{
	return OptimizerSettings ? OptimizerSettings->PythonScriptPath : TEXT("");
}
void SOptimizerPanel::OnPythonScriptPathChanged(const FText& NewText)
{
	if (!OptimizerSettings) return;
	OptimizerSettings->PythonScriptPath = NewText.ToString();
	OptimizerSettings->SaveSettings();
}
ECheckBoxState SOptimizerPanel::IsPythonLoggingChecked() const
{
	if (!OptimizerSettings) return ECheckBoxState::Checked;
	return OptimizerSettings->bEnablePythonLogging ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SOptimizerPanel::OnPythonLoggingChanged(ECheckBoxState NewState)
{
	if (!OptimizerSettings) return;
	OptimizerSettings->bEnablePythonLogging = (NewState == ECheckBoxState::Checked);
	OptimizerSettings->SaveSettings();
}

void SOptimizerPanel::UpdateUI() {}
void SOptimizerPanel::RefreshUI() {}

// Tab management methods
void SOptimizerPanel::OnTabChanged(EOptimizerTab NewTab)
{
	CurrentTab = NewTab;
	
	// Handle tab-specific logic here
	switch (NewTab)
	{
	case EOptimizerTab::Textures:
		// Textures tab selected
		break;
	case EOptimizerTab::Meshes:
		// Meshes tab selected
		break;
	case EOptimizerTab::Materials:
		// Materials tab selected
		break;
	case EOptimizerTab::Optimization:
		// Optimization tab selected
		break;
	}
}

TSharedRef<SWidget> SOptimizerPanel::CreateTabContent(EOptimizerTab TabType)
{
	switch (TabType)
	{
	case EOptimizerTab::Textures:
		return TexturesTab.IsValid() ? TexturesTab.ToSharedRef() : SNew(STextBlock).Text(FText::FromString(TEXT("Textures Tab")));
	case EOptimizerTab::Meshes:
		return MeshesTab.IsValid() ? MeshesTab.ToSharedRef() : SNew(STextBlock).Text(FText::FromString(TEXT("Meshes Tab")));
	case EOptimizerTab::Materials:
		return MaterialsTab.IsValid() ? MaterialsTab.ToSharedRef() : SNew(STextBlock).Text(FText::FromString(TEXT("Materials Tab")));
	case EOptimizerTab::Optimization:
		return OptimizationTab.IsValid() ? OptimizationTab.ToSharedRef() : SNew(STextBlock).Text(FText::FromString(TEXT("Optimization Tab")));
	default:
		return SNew(STextBlock).Text(FText::FromString(TEXT("Unknown Tab")));
	}
}

FText SOptimizerPanel::GetTabDisplayName(EOptimizerTab TabType) const
{
	switch (TabType)
	{
	case EOptimizerTab::Textures:
		return FText::FromString(TEXT("Textures"));
	case EOptimizerTab::Meshes:
		return FText::FromString(TEXT("Meshes"));
	case EOptimizerTab::Materials:
		return FText::FromString(TEXT("Materials"));
	case EOptimizerTab::Optimization:
		return FText::FromString(TEXT("Optimization"));
	default:
		return FText::FromString(TEXT("Unknown"));
	}
}

FSlateIcon SOptimizerPanel::GetTabIcon(EOptimizerTab TabType) const
{
	// TODO: Add proper icons for each tab
	switch (TabType)
	{
	case EOptimizerTab::Textures:
		return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Texture2D");
	case EOptimizerTab::Meshes:
		return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.StaticMesh");
	case EOptimizerTab::Materials:
		return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Material");
	case EOptimizerTab::Optimization:
		return FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Settings");
	default:
		return FSlateIcon();
	}
}
