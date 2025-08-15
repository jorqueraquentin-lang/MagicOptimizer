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
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#include "EditorStyleSet.h"
#include "ISettingsModule.h"
#include "Misc/ConfigCacheIni.h"
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
#include "Services/Editor/ContentBrowserActions.h"

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
                    .ButtonStyle(FAppStyle::Get(), "FlatButton")
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
				
                // Removed Run Mode dropdown (buttons below are the actions)
				
				// Action Buttons
                + SGridPanel::Slot(0, 3)
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
								.Content()[ SNew(STextBlock).Text(FText::FromString(TEXT("Dry Run"))) ]
							]
							+ SGridPanel::Slot(1, 0)
							.Padding(2.0f)
							[
								SNew(SCheckBox)
								.IsChecked(this, &SOptimizerPanel::IsCreateBackupsChecked)
								.OnCheckStateChanged(this, &SOptimizerPanel::OnCreateBackupsChanged)
								.Content()[ SNew(STextBlock).Text(FText::FromString(TEXT("Create Backups"))) ]
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
							.Content()[ SNew(STextBlock).Text(FText::FromString(TEXT("Enable Python Logging"))) ]
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
					SNew(SExpandableArea)
					.AreaTitle(FText::FromString(TEXT("Audit Results (Textures)")))
					.InitiallyCollapsed(false)
					.BodyContent()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0,0,0,4)
						[
							SNew(SHorizontalBox)
							// Filter row
							+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0,0,8,0)
							[
								SNew(SEditableTextBox)
								.HintText(FText::FromString(TEXT("Filter path or format...")))
								.Text_Lambda([this]() { return FText::FromString(TextureFilterText); })
								.OnTextChanged(this, &SOptimizerPanel::OnFilterTextChanged)
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,4,0)
							[
								SNew(SEditableTextBox)
								.MinDesiredWidth(70)
								.HintText(FText::FromString(TEXT("Min W")))
								.Text(FText::FromString(TEXT("")))
								.OnTextChanged(this, &SOptimizerPanel::OnMinWidthChanged)
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,4,0)
							[
								SNew(SEditableTextBox)
								.MinDesiredWidth(70)
								.HintText(FText::FromString(TEXT("Min H")))
								.Text(FText::FromString(TEXT("")))
								.OnTextChanged(this, &SOptimizerPanel::OnMinHeightChanged)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SButton)
								.Text(FText::FromString(TEXT("Clear")))
								.OnClicked(this, &SOptimizerPanel::OnClearTextureFilters)
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0,0,0,2)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return FText::FromString(FString::Printf(TEXT("Filtered %d / %d"), TextureRows.Num(), AllTextureRows.Num())); })
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						[
							SAssignNew(TextureListView, SListView<FTextureAuditRowPtr>)
							.ItemHeight(20)
							.ListItemsSource(&TextureRows)
							.OnGenerateRow(this, &SOptimizerPanel::OnGenerateTextureRow)
							.HeaderRow(
								SAssignNew(TextureHeaderRow, SHeaderRow)
								+ SHeaderRow::Column(FName(TEXT("Path"))).DefaultLabel(FText::FromString(TEXT("Path"))).HAlignCell(HAlign_Left).OnSort(this, &SOptimizerPanel::OnHeaderColumnSort)
								+ SHeaderRow::Column(FName(TEXT("Width"))).DefaultLabel(FText::FromString(TEXT("Width"))).HAlignCell(HAlign_Right).OnSort(this, &SOptimizerPanel::OnHeaderColumnSort)
								+ SHeaderRow::Column(FName(TEXT("Height"))).DefaultLabel(FText::FromString(TEXT("Height"))).HAlignCell(HAlign_Right).OnSort(this, &SOptimizerPanel::OnHeaderColumnSort)
								+ SHeaderRow::Column(FName(TEXT("Format"))).DefaultLabel(FText::FromString(TEXT("Format"))).HAlignCell(HAlign_Left).OnSort(this, &SOptimizerPanel::OnHeaderColumnSort)
								+ SHeaderRow::Column(FName(TEXT("Actions"))).DefaultLabel(FText::FromString(TEXT("Actions"))).HAlignCell(HAlign_Center)
							)
						]
					]
				]

				// Recommendations
				+ SGridPanel::Slot(0, 7)
				.ColumnSpan(2)
				.Padding(4.0f)
				[
					SNew(SExpandableArea)
					.AreaTitle(FText::FromString(TEXT("Texture Recommendations")))
					.InitiallyCollapsed(false)
					.BodyContent()
					[
						SAssignNew(TextureRecListView, SListView<FTextureRecRowPtr>)
						.ItemHeight(20)
						.ListItemsSource(&TextureRecRows)
						.OnGenerateRow(this, &SOptimizerPanel::OnGenerateTextureRecRow)
						.HeaderRow(
							SAssignNew(TextureRecHeaderRow, SHeaderRow)
							+ SHeaderRow::Column(FName(TEXT("Path"))).DefaultLabel(FText::FromString(TEXT("Path"))).HAlignCell(HAlign_Left)
							+ SHeaderRow::Column(FName(TEXT("Width"))).DefaultLabel(FText::FromString(TEXT("Width"))).HAlignCell(HAlign_Right)
							+ SHeaderRow::Column(FName(TEXT("Height"))).DefaultLabel(FText::FromString(TEXT("Height"))).HAlignCell(HAlign_Right)
							+ SHeaderRow::Column(FName(TEXT("Format"))).DefaultLabel(FText::FromString(TEXT("Format"))).HAlignCell(HAlign_Left)
							+ SHeaderRow::Column(FName(TEXT("Issues"))).DefaultLabel(FText::FromString(TEXT("Issues"))).HAlignCell(HAlign_Left)
							+ SHeaderRow::Column(FName(TEXT("Recommendations"))).DefaultLabel(FText::FromString(TEXT("Recommendations"))).HAlignCell(HAlign_Left)
							+ SHeaderRow::Column(FName(TEXT("Actions"))).DefaultLabel(FText::FromString(TEXT("Actions"))).HAlignCell(HAlign_Center)
						)
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
		if (TextureListView.IsValid()) { TextureListView->RequestListRefresh(); }
		return;
	}
	AllTextureRows = MoveTemp(Parsed);
	MagicOptimizerLog::AppendLine(FString::Printf(TEXT("UI: Loaded %d texture rows"), AllTextureRows.Num()));
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
		if (TextureRecListView.IsValid()) { TextureRecListView->RequestListRefresh(); }
		return;
	}
	AllTextureRecRows = MoveTemp(Parsed);
	TextureRecRows = AllTextureRecRows;
	if (TextureRecListView.IsValid()) { TextureRecListView->RequestListRefresh(); }
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
