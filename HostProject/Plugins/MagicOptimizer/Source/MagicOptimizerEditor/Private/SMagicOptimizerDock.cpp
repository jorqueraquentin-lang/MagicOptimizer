// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#include "SMagicOptimizerDock.h"

#include "Widgets/Layout/SScrollBorder.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"

#include "OptimizerSettings.h"
#include "PythonBridge.h"
#include "ViewModels/TextureTableViewModel.h"

#include "SDashboard.h"
#include "STextureAuditSection.h"
#include "STextureRecommendSection.h"
#include "SMeshesTab.h"
#include "SMaterialsTab.h"

#include "Services/Csv/TextureCsvReader.h"
#include "Services/Csv/MeshCsvReader.h"
#include "Services/Csv/MaterialCsvReader.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SSpacer.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/INotificationWidget.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "InputCoreTypes.h"
#include "Input/Events.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Editor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Editor/EditorEngine.h"
#include "FileHelpers.h"
#include "UObject/Package.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Policies/CondensedJsonPrintPolicy.h"

void SMagicOptimizerDock::Construct(const FArguments& InArgs)
{
	InitializeServices();
	InitializePresets();

	// Build main layout: Header | Body split with left rail, main, right shelf
	ChildSlot
	[
		SNew(SBorder)
		.Padding(0)
		.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildHeader()
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SHorizontalBox)
				// Left rail
				+ SHorizontalBox::Slot().AutoWidth().Padding(8,8,4,8)
				[
					BuildLeftRail()
				]
				// Main views
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(4,8)
				[
					BuildMainViews()
				]
				// Right shelf
				+ SHorizontalBox::Slot().AutoWidth().Padding(4,8,8,8)
				[
					BuildRightShelf()
				]
			]
		]
	];

	// Initial data
	LoadAuditData();
	UpdateQuickFixShelf();
	RefreshRunsList();
	RefreshScopePaths();
}

SMagicOptimizerDock::~SMagicOptimizerDock()
{
	if (TextureTableViewModel.IsValid() && OptimizerSettings)
	{
		TextureTableViewModel->SaveSettingsToConfig(OptimizerSettings);
	}
}

void SMagicOptimizerDock::InitializeServices()
{
	OptimizerSettings = UOptimizerSettings::Get();
	if (OptimizerSettings)
	{
		OptimizerSettings->LoadSettings();
	}
	PythonBridge = NewObject<UPythonBridge>();
	if (PythonBridge)
	{
		PythonBridge->Initialize();
	}
	TextureTableViewModel = MakeShareable(new FTextureTableViewModel());
	if (OptimizerSettings)
	{
		TextureTableViewModel->LoadSettingsFromConfig(OptimizerSettings);
	}
	// Preload source control hint
	UpdateSourceControlHint();
	// Create thumbnail pool (size 32, not shared)
	ThumbnailPool = MakeShareable(new FAssetThumbnailPool(32));
}

void SMagicOptimizerDock::InitializePresets()
{
	PresetOptions.Empty();
	TArray<FString> AllPresets;
	if (OptimizerSettings)
	{
		AllPresets = OptimizerSettings->GetAllPresets();
	}
	if (AllPresets.Num() == 0)
	{
		AllPresets = { TEXT("Console Balanced"), TEXT("Mobile Low"), TEXT("PC Ultra") };
	}
	for (const FString& P : AllPresets)
	{
		PresetOptions.Add(MakeShared<FString>(P));
	}
	const FString DefaultPreset = OptimizerSettings ? OptimizerSettings->TargetProfile : TEXT("Console Balanced");
	CurrentPreset = MakeShareable(new FString(DefaultPreset.IsEmpty() ? TEXT("Console Balanced") : DefaultPreset));
}

TSharedRef<SWidget> SMagicOptimizerDock::BuildHeader()
{
	return SNew(SBorder)
		.Padding(12)
		.BorderImage(FAppStyle::GetBrush("Brushes.Header"))
	[
		SNew(SHorizontalBox)
		// Preset selector
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,8,0)
		[
			SNew(SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&PresetOptions)
			.OnSelectionChanged(this, &SMagicOptimizerDock::OnPresetSelected)
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
			{
				return SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")));
			})
			.Content()
			[
				SNew(STextBlock).Text(this, &SMagicOptimizerDock::GetPresetLabel).ToolTipText(FText::FromString(TEXT("Select a preset. Applies thresholds and rules.")))
			]
		]
		// Targets chips (simple text buttons for now)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,8,0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(2,0)
			[
				SNew(SButton).Text(FText::FromString(TEXT("PS5"))).OnClicked_Lambda([this]()
				{
					FName Name("PS5"); if (ActiveTargets.Contains(Name)) { ActiveTargets.Remove(Name);} else { ActiveTargets.Add(Name);} return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2,0)
			[
				SNew(SButton).Text(FText::FromString(TEXT("Quest"))).OnClicked_Lambda([this]()
				{
					FName Name("Quest"); if (ActiveTargets.Contains(Name)) { ActiveTargets.Remove(Name);} else { ActiveTargets.Add(Name);} return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2,0)
			[
				SNew(SButton).Text(FText::FromString(TEXT("Switch"))).OnClicked_Lambda([this]()
				{
					FName Name("Switch"); if (ActiveTargets.Contains(Name)) { ActiveTargets.Remove(Name);} else { ActiveTargets.Add(Name);} return FReply::Handled();
				})
			]
		]
		// Scope button and summary
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,8,0)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Scope")))
			.ToolTipText(FText::FromString(TEXT("Pick selection/level/folders and excludes.")))
			.OnClicked(this, &SMagicOptimizerDock::OnOpenScopePicker)
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,8,0)
		[
			SNew(STextBlock).Text(this, &SMagicOptimizerDock::GetScopeSummaryText)
		]
		// Run controls
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8,0)
		[
			SNew(SButton).Text(FText::FromString(TEXT("Run Scan"))).ToolTipText(FText::FromString(TEXT("Run an audit. No writes."))).OnClicked(this, &SMagicOptimizerDock::OnRunScan)
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4,0)
		[
			SNew(SButton).Text(FText::FromString(TEXT("Auto Fix"))).ToolTipText(FText::FromString(TEXT("Apply safe fixes. Confirmation required."))).OnClicked(this, &SMagicOptimizerDock::OnAutoFix)
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4,0)
		[
			SNew(SButton).Text(FText::FromString(TEXT("Preview Plan"))).ToolTipText(FText::FromString(TEXT("Build a plan without writing."))).OnClicked(this, &SMagicOptimizerDock::OnPreviewPlan)
		]
		// Cap pill
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8,0)
		[
			SNew(SBorder)
			.Padding(FMargin(6,2))
			.BorderImage(FAppStyle::GetBrush("Brushes.Header"))
			[
				SNew(STextBlock).Text(this, &SMagicOptimizerDock::GetCapText)
			]
		]
		// Source control hint placeholder
		+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Right).VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text_Lambda([this]()
			{
				return FText::FromString(SourceControlBranch.IsEmpty() ? TEXT("Source control: Not connected") : FString::Printf(TEXT("%s | %d checked out"), *SourceControlBranch, NumFilesCheckedOut));
			})
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8,0)
		[
			SNew(SButton).Text(FText::FromString(TEXT("Tasks"))).ToolTipText(FText::FromString(TEXT("Open Task Drawer"))).OnClicked(this, &SMagicOptimizerDock::OnToggleTaskDrawer)
		]
	];
}

TSharedRef<SWidget> SMagicOptimizerDock::BuildLeftRail()
{
	return SNew(SBorder)
		.Padding(12)
		.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("Scope & Filters")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0,8,0,0)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Text(FText::FromString(TEXT("Asset Types"))) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[ SNew(SCheckBox).OnCheckStateChanged_Lambda([this](ECheckBoxState S){ OnCategoryChanged(S, (uint8)EOptimizerCategory::Textures);} ).IsChecked_Lambda([this](){ return IsCategoryChecked((uint8)EOptimizerCategory::Textures); }).Content()[ SNew(STextBlock).Text(FText::FromString(TEXT("Textures"))) ] ]
				+ SVerticalBox::Slot().AutoHeight()[ SNew(SCheckBox).OnCheckStateChanged_Lambda([this](ECheckBoxState S){ OnCategoryChanged(S, (uint8)EOptimizerCategory::Meshes);} ).IsChecked_Lambda([this](){ return IsCategoryChecked((uint8)EOptimizerCategory::Meshes); }).Content()[ SNew(STextBlock).Text(FText::FromString(TEXT("Meshes"))) ] ]
				+ SVerticalBox::Slot().AutoHeight()[ SNew(SCheckBox).OnCheckStateChanged_Lambda([this](ECheckBoxState S){ OnCategoryChanged(S, (uint8)EOptimizerCategory::Materials);} ).IsChecked_Lambda([this](){ return IsCategoryChecked((uint8)EOptimizerCategory::Materials); }).Content()[ SNew(STextBlock).Text(FText::FromString(TEXT("Materials"))) ] ]
				+ SVerticalBox::Slot().AutoHeight()[ SNew(SCheckBox).OnCheckStateChanged_Lambda([this](ECheckBoxState S){ OnCategoryChanged(S, (uint8)EOptimizerCategory::Audio);} ).IsChecked_Lambda([this](){ return IsCategoryChecked((uint8)EOptimizerCategory::Audio); }).Content()[ SNew(STextBlock).Text(FText::FromString(TEXT("Audio"))) ] ]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0,8,0,0)
		[
			SAssignNew(SearchBox, SEditableTextBox)
			.MinDesiredWidth(200)
			.HintText(FText::FromString(TEXT("Search (e.g. type normal, group UI)")))
			.OnTextChanged(this, &SMagicOptimizerDock::OnSearchTextChanged)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0,8,0,0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ SNew(STextBlock).Text(FText::FromString(TEXT("Saved Views"))) ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8,0)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&SavedViewNames)
				.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Sel, ESelectInfo::Type){ CurrentSavedView = Sel; if (Sel.IsValid()) LoadSavedView(*Sel); })
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> N){ return SNew(STextBlock).Text(FText::FromString(N.IsValid()?*N:TEXT(""))); })
				.Content()[ SNew(STextBlock).Text_Lambda([this](){ return FText::FromString(CurrentSavedView.IsValid()?*CurrentSavedView:TEXT("Select")); }) ]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8,0)
			[
				SNew(SButton).Text(FText::FromString(TEXT("Save"))).OnClicked_Lambda([this](){ SaveCurrentView(); return FReply::Handled(); })
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0,8,0,0)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("Scope Folders")))
		]
		+ SVerticalBox::Slot().FillHeight(0.4f)
		[
			SAssignNew(ScopePathsList, SListView<TSharedPtr<FString>>)
			.ListItemsSource(&ScopePaths)
			.OnGenerateRow_Lambda([this](TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& Owner)
			{
				return SNew(STableRow<TSharedPtr<FString>>, Owner)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()[ SNew(SCheckBox).IsChecked_Lambda([this, Item](){ return Item.IsValid() && CheckedScopePaths.Contains(*Item) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }).OnCheckStateChanged_Lambda([this, Item](ECheckBoxState S){ if (Item.IsValid()) OnToggleScopePath(*Item, S == ECheckBoxState::Checked); }) ]
					+ SHorizontalBox::Slot().FillWidth(1.f)[ SNew(STextBlock).Text(FText::FromString(Item.IsValid()?*Item:TEXT(""))) ]
				];
			})
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0,8,0,0)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("Saved Views (WIP)")))
		]
	];
}

TSharedRef<SWidget> SMagicOptimizerDock::BuildMainViews()
{
	return SAssignNew(ViewSwitcher, SWidgetSwitcher)
		.WidgetIndex_Lambda([this]() { return static_cast<int32>(CurrentView); })
		+ SWidgetSwitcher::Slot() // Audit
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0,0,0,8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(2,0)[ SNew(SButton).Text(FText::FromString(TEXT("Textures"))).OnClicked_Lambda([this](){ SwitchAuditType(EAuditAssetType::Textures); return FReply::Handled(); }) ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(2,0)[ SNew(SButton).Text(FText::FromString(TEXT("Meshes"))).OnClicked_Lambda([this](){ SwitchAuditType(EAuditAssetType::Meshes); return FReply::Handled(); }) ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(2,0)[ SNew(SButton).Text(FText::FromString(TEXT("Materials"))).OnClicked_Lambda([this](){ SwitchAuditType(EAuditAssetType::Materials); return FReply::Handled(); }) ]
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(AuditOverviewWidget, SDashboard)
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBox)
					.Visibility_Lambda([this](){ return CurrentAuditType==EAuditAssetType::Textures?EVisibility::Visible:EVisibility::Collapsed; })
					[
						SAssignNew(AuditTexturesWidget, STextureAuditSection)
					]
				]
				+ SOverlay::Slot()
				[
					SNew(SBox)
					.Visibility_Lambda([this](){ return CurrentAuditType==EAuditAssetType::Meshes?EVisibility::Visible:EVisibility::Collapsed; })
					[
						SAssignNew(MeshesTabWidget, SMeshesTab)
					]
				]
				+ SOverlay::Slot()
				[
					SNew(SBox)
					.Visibility_Lambda([this](){ return CurrentAuditType==EAuditAssetType::Materials?EVisibility::Visible:EVisibility::Collapsed; })
					[
						SAssignNew(MaterialsTabWidget, SMaterialsTab)
					]
				]
			]
		]
			]
		]
		+ SWidgetSwitcher::Slot() // Recommend
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0,0,0,8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text_Lambda([this]()
					{
						int32 Count = ProposedChangeLines.Num();
						return FText::FromString(FString::Printf(TEXT("Proposed changes: %d"), Count));
					})
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SSpacer)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4,0)
				[
					SNew(SButton).Text(FText::FromString(TEXT("Export CSV"))).OnClicked(this, &SMagicOptimizerDock::OnExportProposedCsv)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4,0)
				[
					SNew(SButton).Text(FText::FromString(TEXT("Copy for Jira"))).OnClicked(this, &SMagicOptimizerDock::OnCopyProposedForJira)
				]
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SAssignNew(ProposedListView, SListView<TSharedPtr<FString>>)
				.ItemHeight(28)
				.ListItemsSource(&ProposedChangeItems)
				.OnGenerateRow_Lambda([](TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& Owner)
				{
					return SNew(STableRow<TSharedPtr<FString>>, Owner)
					[
						SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
					];
				})
			]
		]
		+ SWidgetSwitcher::Slot() // Apply
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock).Text_Lambda([this]()
				{
					return bApplyRunning ? FText::FromString(TEXT("Applying changes...")) : FText::FromString(TEXT("Ready to apply changes"));
				})
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,6)
			[
				SNew(SProgressBar).Percent_Lambda([this]() { return ApplyProgressTotal > 0 ? (float)ApplyProgressCurrent / (float)ApplyProgressTotal : 0.f; })
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(ApplyCurrentPath); })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,6)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,4,0)[ SNew(SButton).Text(FText::FromString(TEXT("Pause"))) ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,4,0)[ SNew(SButton).Text(FText::FromString(TEXT("Resume"))) ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,4,0)[ SNew(SButton).Text(FText::FromString(TEXT("Cancel"))) ]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SSpacer)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Open Logs")))
					.OnClicked_Lambda([]()
					{
						const FString Dir = FPaths::ProjectSavedDir() / TEXT("MagicOptimizer/Reports");
						FPlatformProcess::ExploreFolder(*Dir);
						return FReply::Handled();
					})
				]
			]
		]
		+ SWidgetSwitcher::Slot() // Verify
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("Compare snapshot vs current")))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,6)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Load Snapshot")))
					.OnClicked_Lambda([this]()
					{
						if (!LastSnapshotPath.IsEmpty()) { LoadSnapshotFromPath(LastSnapshotPath); }
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Revert All")))
					.OnClicked_Lambda([this]()
					{
						for (const FTextureSnapshotItem& Item : LoadedSnapshot)
						{
							OnRevertOne(Item.ObjectPath);
						}
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(LastSnapshotPath.IsEmpty() ? TEXT("No snapshot loaded") : FString::Printf(TEXT("Snapshot: %s"), *LastSnapshotPath)); })
				]
			]
		]
		+ SWidgetSwitcher::Slot() // Reports
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()[ SNew(SButton).Text(FText::FromString(TEXT("Refresh"))).OnClicked_Lambda([this]() { RefreshRunsList(); return FReply::Handled(); }) ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8,0)[ SNew(SButton).Text(FText::FromString(TEXT("Open Reports Folder"))).OnClicked_Lambda([this]() { if (!LastReportDir.IsEmpty()) { FPlatformProcess::ExploreFolder(*LastReportDir); } return FReply::Handled(); }) ]
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SAssignNew(RunsListView, SListView<TSharedPtr<FRunEntry>>)
				.ListItemsSource(&RunEntries)
				.OnSelectionChanged(this, &SMagicOptimizerDock::OnRunSelectionChanged)
				.OnGenerateRow_Lambda([this](TSharedPtr<FRunEntry> Item, const TSharedRef<STableViewBase>& Owner)
				{
					return SNew(STableRow<TSharedPtr<FRunEntry>>, Owner)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).Padding(2,0)[ SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? Item->Name : TEXT(""))) ]
						+ SHorizontalBox::Slot().AutoWidth().Padding(2,0)[ SNew(SButton).Text(FText::FromString(TEXT("CSV"))).OnClicked_Lambda([this, Item]() { return OnOpenArtifact(Item->CsvPath); }) ]
						+ SHorizontalBox::Slot().AutoWidth().Padding(2,0)[ SNew(SButton).Text(FText::FromString(TEXT("JSON"))).OnClicked_Lambda([this, Item]() { return OnOpenArtifact(Item->JsonPath); }) ]
						+ SHorizontalBox::Slot().AutoWidth().Padding(2,0)[ SNew(SButton).Text(FText::FromString(TEXT("HTML"))).OnClicked_Lambda([this, Item]() { return OnOpenArtifact(Item->HtmlPath); }) ]
					];
				})
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()[ SNew(STextBlock).Text(FText::FromString(TEXT("Compare: "))) ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4,0)[ SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(CompareRunA.IsValid() ? CompareRunA->Name : TEXT("A")); }) ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4,0)[ SNew(STextBlock).Text(FText::FromString(TEXT("vs"))) ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4,0)[ SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(CompareRunB.IsValid() ? CompareRunB->Name : TEXT("B")); }) ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8,0)[ SNew(SButton).Text(FText::FromString(TEXT("Compute Deltas"))).OnClicked_Lambda([this]() { ComputeRunDeltas(); return FReply::Handled(); }) ]
			]
			+ SVerticalBox::Slot().FillHeight(0.6f)
			[
				SAssignNew(CompareDeltaListView, SListView<TSharedPtr<FString>>)
				.ListItemsSource(&CompareDeltaLines)
				.OnGenerateRow_Lambda([](TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& Owner)
				{
					return SNew(STableRow<TSharedPtr<FString>>, Owner)[ SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? *Item : TEXT(""))) ];
				})
			]
		];
}

TSharedRef<SWidget> SMagicOptimizerDock::BuildRightShelf()
{
	return SAssignNew(RightQuickFixShelf, SBorder)
		.Padding(12)
		.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
	[
		SAssignNew(QuickFixChipsBox, SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("Quick Fixes")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0,8,0,0)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Help")))
			.OnClicked_Lambda([]()
			{
				FPlatformProcess::ExploreFolder(*FPaths::ProjectDir());
				return FReply::Handled();
			})
		]
	];
}

void SMagicOptimizerDock::OnPresetSelected(TSharedPtr<FString> InPreset, ESelectInfo::Type InSelectInfo)
{
	CurrentPreset = InPreset;
	if (OptimizerSettings && InPreset.IsValid())
	{
		OptimizerSettings->TargetProfile = *InPreset;
		OptimizerSettings->SaveSettings();
	}
}

FText SMagicOptimizerDock::GetPresetLabel() const
{
	return FText::FromString(CurrentPreset.IsValid() ? *CurrentPreset : TEXT("Preset"));
}

FReply SMagicOptimizerDock::OnOpenScopePicker()
{
	return OnOpenScopePickerModal();
}

FText SMagicOptimizerDock::GetScopeSummaryText() const
{
	return FText::FromString(ScopeSummary.IsEmpty() ? TEXT("Scope: Game") : ScopeSummary);
}

FReply SMagicOptimizerDock::OnRunScan()
{
	if (!PythonBridge || !PythonBridge->IsPythonAvailable())
	{
		Notify(TEXT("Python bridge unavailable"), false);
		return FReply::Handled();
	}
	FOptimizerRunParams Params;
	Params.Phase = TEXT("Audit");
	Params.Profile = CurrentPreset.IsValid() ? *CurrentPreset : TEXT("Console Balanced");
	Params.bDryRun = true;
	Params.MaxChanges = OptimizerSettings ? OptimizerSettings->MaxChanges : 500;
	Params.bUseSelection = OptimizerSettings ? OptimizerSettings->bUseSelection : false;
	Params.IncludePaths = OptimizerSettings ? OptimizerSettings->IncludePathsCsv : TEXT("");
	Params.ExcludePaths = OptimizerSettings ? OptimizerSettings->ExcludePathsCsv : TEXT("");
	Params.Categories = { TEXT("Textures") };

	AppendTaskLine(TEXT("Run Scan started"));
	StartProgressNotification(TEXT("MagicOptimizer: Running Scan"));
	auto Result = PythonBridge->RunOptimization(Params);
	if (Result.AssetsProcessed > 0)
	{
		AppendTaskLine(FString::Printf(TEXT("Scan processed %d assets (%d modified)"), Result.AssetsProcessed, Result.AssetsModified));
	}
	LoadAuditData();
	UpdateQuickFixShelf();
	SwitchView(EMainView::Audit);
	CompleteProgressNotification(Result.bSuccess, Result.bSuccess ? TEXT("Scan complete") : TEXT("Scan failed"));
	return FReply::Handled();
}

FReply SMagicOptimizerDock::OnPreviewPlan()
{
	BuildProposedChangesFromSelection();
	SwitchView(EMainView::Recommend);
	Notify(TEXT("Preview plan ready"), true);
	return FReply::Handled();
}

FReply SMagicOptimizerDock::OnAutoFix()
{
	// Confirm before applying
	const FText Title = FText::FromString(TEXT("Apply fixes?"));
	const FText Msg = FText::FromString(TEXT("This will write changes. A snapshot is saved for reverts."));
	EAppReturnType::Type Response = FSlateApplication::Get().DisplayYesNoDialog(Title, Msg);
	if (Response != EAppReturnType::Yes)
	{
		return FReply::Handled();
	}
	if (OptimizerSettings && TextureTableViewModel.IsValid())
	{
		const int32 Planned = TextureTableViewModel->GetFilteredCount();
		const int32 Cap = OptimizerSettings->MaxChanges;
		if (Planned > Cap)
		{
			EAppReturnType::Type OverResp = FSlateApplication::Get().DisplayYesNoDialog(FText::FromString(TEXT("Plan exceeds change cap. Continue?")), FText::FromString(FString::Printf(TEXT("Planned %d > Cap %d"), Planned, Cap)));
			if (OverResp != EAppReturnType::Yes) { return FReply::Handled(); }
		}
	}
	// Guarded action: still keep dry run by default
	FOptimizerRunParams Params;
	Params.Phase = TEXT("Apply");
	Params.Profile = CurrentPreset.IsValid() ? *CurrentPreset : TEXT("Console Balanced");
	Params.bDryRun = false;
	Params.MaxChanges = OptimizerSettings ? OptimizerSettings->MaxChanges : 500;
	Params.Categories = { TEXT("Textures") };
	if (PythonBridge)
	{
		AppendTaskLine(TEXT("Apply started"));
		StartProgressNotification(TEXT("MagicOptimizer: Applying"));
		auto Result = PythonBridge->RunOptimization(Params);
		if (Result.AssetsProcessed > 0)
		{
			AppendTaskLine(FString::Printf(TEXT("Apply processed %d assets (%d modified)"), Result.AssetsProcessed, Result.AssetsModified));
		}
		CompleteProgressNotification(Result.bSuccess, Result.bSuccess ? TEXT("Apply complete") : TEXT("Apply failed"));
	}
	// Prepare snapshot/report for Verify & Reports
	LastReportDir = FPaths::ProjectSavedDir() / TEXT("MagicOptimizer/Reports");
	IFileManager::Get().MakeDirectory(*LastReportDir, true);
	// Gather object paths from selection or all filtered
	TArray<FString> ObjectPaths;
	for (const FTextureAuditRowPtr& Row : SelectedTextureRows)
	{
		if (Row.IsValid()) { ObjectPaths.Add(ToObjectPath(Row->Path)); }
	}
	if (ObjectPaths.Num() == 0 && TextureTableViewModel.IsValid())
	{
		for (const FTextureAuditRowPtr& Row : TextureTableViewModel->GetFilteredData())
		{
			if (Row.IsValid()) { ObjectPaths.Add(ToObjectPath(Row->Path)); }
		}
	}
	// Save snapshot and apply batch
	FString SnapshotPath;
	if (SaveTextureSnapshot(ObjectPaths, SnapshotPath))
	{
		LastSnapshotPath = SnapshotPath;
	}
	ApplySrgbOffBatch(ObjectPaths);
	RefreshRunsList();
	return FReply::Handled();
}

void SMagicOptimizerDock::LoadAuditData()
{
	TArray<TSharedPtr<struct FTextureAuditRow>> TextureRows;
	TextureCsvReader::ReadAuditCsv(OptimizerSettings, TextureRows);
	if (TextureTableViewModel.IsValid())
	{
		TextureTableViewModel->SetSourceData(TextureRows);
	}
	if (AuditOverviewWidget.IsValid())
	{
		AuditOverviewWidget->SetViewModel(TextureTableViewModel);
	}
	if (AuditTexturesWidget.IsValid())
	{
		AuditTexturesWidget->SetViewModel(TextureTableViewModel);
		AuditTexturesWidget->SetTextureRows(TextureRows);
		AuditTexturesWidget->OnSelectionChanged.BindLambda([this](const TArray<FTextureAuditRowPtr>& Selected)
		{
			SelectedTextureRows = Selected;
			UpdateQuickFixShelf();
		});
		AuditTexturesWidget->OnRowActivated.BindLambda([this](FTextureAuditRowPtr Item)
		{
			OpenSlideOverForTexture(Item);
		});
	}

	// Load Meshes and Materials CSV (basic for now)
	TArray<FMeshAuditRowPtr> MeshRows; MeshCsvReader::ReadAuditCsv(OptimizerSettings, MeshRows);
	TArray<FMaterialAuditRowPtr> MaterialRows; MaterialCsvReader::ReadAuditCsv(OptimizerSettings, MaterialRows);

	if (MeshesTabWidget.IsValid())
	{
		MeshesTabWidget->SetMeshRows(MeshRows);
	}
	if (MaterialsTabWidget.IsValid())
	{
		MaterialsTabWidget->SetMaterialRows(MaterialRows);
	}

	// TODO: propagate MeshRows/MaterialRows to SMeshesTab/SMaterialsTab on construction via delegates or member pointers

	// Default tab to most impacted type (by row count for now)
	int32 TexturesCount = TextureRows.Num();
	int32 MeshesCount = MeshRows.Num();
	int32 MaterialsCount = MaterialRows.Num();
	if (MeshesCount > TexturesCount && MeshesCount >= MaterialsCount)
	{
		SwitchAuditType(EAuditAssetType::Meshes);
	}
	else if (MaterialsCount > TexturesCount && MaterialsCount >= MeshesCount)
	{
		SwitchAuditType(EAuditAssetType::Materials);
	}
}

void SMagicOptimizerDock::LoadRecommendations()
{
	if (!RecommendWidget.IsValid()) { return; }
	TArray<FTextureRecRowPtr> RecRows;
	TextureCsvReader::ReadRecommendationsCsv(OptimizerSettings, RecRows);
	RecommendWidget->SetRecommendationRows(RecRows);
}

void SMagicOptimizerDock::UpdateSourceControlHint()
{
	// Minimal source control fetch: branch and checked out count
	ISourceControlModule& SCM = ISourceControlModule::Get();
	if (!SCM.IsEnabled())
	{
		SourceControlBranch = TEXT("");
		NumFilesCheckedOut = 0;
		return;
	}
	ISourceControlProvider& Provider = SCM.GetProvider();
	if (Provider.IsEnabled())
	{
		SourceControlBranch = Provider.GetBranchName();
		// Placeholder until we add an async status query
		NumFilesCheckedOut = 0;
	}
}

void SMagicOptimizerDock::UpdateQuickFixShelf()
{
	if (!QuickFixChipsBox.IsValid()) { return; }
	QuickFixChipsBox->ClearChildren();
	// Show only when there is a selection
	if (SelectedTextureRows.Num() == 0)
	{
		QuickFixChipsBox->AddSlot().AutoHeight()[ SNew(STextBlock).Text(FText::FromString(TEXT("Select rows to see Quick Fixes"))) ];
		return;
	}

	const int32 Count = SelectedTextureRows.Num();
	QuickFixChipsBox->AddSlot().AutoHeight()
	[
		SNew(SButton)
		.Text(FText::FromString(FString::Printf(TEXT("Clear sRGB on masks (%d)"), Count)))
		.OnClicked_Lambda([this]() { return OnPreviewPlan(); })
	];
	QuickFixChipsBox->AddSlot().AutoHeight()
	[
		SNew(SButton)
		.Text(FText::FromString(FString::Printf(TEXT("Convert normals to BC5 (%d)"), Count)))
		.OnClicked_Lambda([this]() { return OnPreviewPlan(); })
	];
}

void SMagicOptimizerDock::OnSearchTextChanged(const FText& NewText)
{
	if (TextureTableViewModel.IsValid())
	{
		TextureTableViewModel->SetTextFilter(NewText.ToString());
	}
	if (AuditTexturesWidget.IsValid())
	{
		AuditTexturesWidget->RefreshDisplay();
	}
}

void SMagicOptimizerDock::SwitchView(EMainView NewView)
{
	CurrentView = NewView;
	if (ViewSwitcher.IsValid())
	{
		ViewSwitcher->Invalidate(EInvalidateWidget::Layout);
	}
}

void SMagicOptimizerDock::BuildProposedChangesFromSelection()
{
	ProposedChangeLines.Empty();
	// Placeholder: use ViewModel filtered set size
	const int32 Count = TextureTableViewModel.IsValid() ? TextureTableViewModel->GetFilteredCount() : 0;
	ProposedChangeLines.Add(FString::Printf(TEXT("Set sRGB off for %d mask textures"), Count));
	// Hook to RecommendWidget in future to show diff
	ProposedChangeItems.Empty();
	for (const FString& Line : ProposedChangeLines)
	{
		ProposedChangeItems.Add(MakeShared<FString>(Line));
	}
	if (ProposedListView.IsValid())
	{
		ProposedListView->RequestListRefresh();
	}
}

void SMagicOptimizerDock::ExportProposedChangesCsv() const
{
	// TODO: implement CSV export
}

FReply SMagicOptimizerDock::OnExportProposedCsv()
{
	if (ProposedChangeLines.Num() == 0) { return FReply::Handled(); }
	const FString OutDir = FPaths::ProjectSavedDir() / TEXT("MagicOptimizer/Reports");
	IFileManager::Get().MakeDirectory(*OutDir, true);
	const FString OutPath = OutDir / TEXT("proposed_changes.csv");
	FString Csv;
	Csv += TEXT("change\n");
	for (const FString& Line : ProposedChangeLines)
	{
		Csv += TEXT("\"") + Line.ReplaceCharWithEscapedChar() + TEXT("\"\n");
	}
	FFileHelper::SaveStringToFile(Csv, *OutPath);
	return FReply::Handled();
}

FReply SMagicOptimizerDock::OnCopyProposedForJira()
{
	FString Body;
	for (const FString& Line : ProposedChangeLines)
	{
		Body += TEXT("- ") + Line + TEXT("\n");
	}
	FPlatformApplicationMisc::ClipboardCopy(*Body);
	return FReply::Handled();
}

void SMagicOptimizerDock::RefreshRunsList()
{
	RunEntries.Empty();
	const FString BaseDir = FPaths::ProjectSavedDir() / TEXT("MagicOptimizer/Reports");
	TArray<FString> Dirs;
	IFileManager::Get().FindFilesRecursive(Dirs, *BaseDir, TEXT("*"), false, true);
	for (const FString& Dir : Dirs)
	{
		TSharedPtr<FRunEntry> Entry = MakeShared<FRunEntry>();
		Entry->Dir = Dir;
		Entry->Name = FPaths::GetCleanFilename(Dir);
		Entry->Preset = CurrentPreset.IsValid() ? *CurrentPreset : TEXT("");
		Entry->CsvPath = Dir / TEXT("audit.csv");
		Entry->JsonPath = Dir / TEXT("audit.json");
		Entry->HtmlPath = Dir / TEXT("audit.html");
		RunEntries.Add(Entry);
	}
	if (RunsListView.IsValid()) { RunsListView->RequestListRefresh(); }
}

FReply SMagicOptimizerDock::OnOpenScopePickerModal()
{
	TSharedPtr<SWindow> Picker = SNew(SWindow)
		.Title(FText::FromString(TEXT("Scope Picker")))
		.ClientSize(FVector2D(520, 320))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	static TSharedPtr<SEditableTextBox> IncludesBox;
	static TSharedPtr<SEditableTextBox> ExcludesBox;
	static bool bUseSelectionLocal = false;
	static bool bUseLevelLocal = false;

	if (OptimizerSettings)
	{
		bUseSelectionLocal = OptimizerSettings->bUseSelection;
	}

	Picker->SetContent(
		SNew(SBorder)
		.Padding(8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Text(FText::FromString(TEXT("Choose scope"))) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,6)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[ SNew(SCheckBox).IsChecked_Lambda([&](){ return bUseSelectionLocal ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }).OnCheckStateChanged_Lambda([&](ECheckBoxState S){ bUseSelectionLocal = (S == ECheckBoxState::Checked); }).Content()[ SNew(STextBlock).Text(FText::FromString(TEXT("Current Selection"))) ] ]
				+ SVerticalBox::Slot().AutoHeight()[ SNew(SCheckBox).IsChecked_Lambda([&](){ return bUseLevelLocal ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }).OnCheckStateChanged_Lambda([&](ECheckBoxState S){ bUseLevelLocal = (S == ECheckBoxState::Checked); }).Content()[ SNew(STextBlock).Text(FText::FromString(TEXT("Current Level"))) ] ]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,6)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("Include Folders (comma separated)")))
			]
			+ SVerticalBox::Slot().AutoHeight()[ SAssignNew(IncludesBox, SEditableTextBox).Text(FText::FromString(OptimizerSettings ? OptimizerSettings->IncludePathsCsv : TEXT(""))) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,6)[ SNew(STextBlock).Text(FText::FromString(TEXT("Excludes (comma separated)"))) ]
			+ SVerticalBox::Slot().AutoHeight()[ SAssignNew(ExcludesBox, SEditableTextBox).Text(FText::FromString(OptimizerSettings ? OptimizerSettings->ExcludePathsCsv : TEXT(""))) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SSpacer)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4,0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Cancel")))
					.OnClicked_Lambda([Picker]() { if (Picker.IsValid()) { FSlateApplication::Get().RequestDestroyWindow(Picker.ToSharedRef()); } return FReply::Handled(); })
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4,0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Apply")))
					.OnClicked_Lambda([this, Picker]()
					{
						TArray<FString> Inc, Exc;
						if (IncludesBox.IsValid()) { IncludesBox->GetText().ToString().ParseIntoArray(Inc, TEXT(",")); }
						if (ExcludesBox.IsValid()) { ExcludesBox->GetText().ToString().ParseIntoArray(Exc, TEXT(",")); }
						ApplyScopeFromModal(Inc, Exc, bUseSelectionLocal, bUseLevelLocal);
						if (Picker.IsValid()) { FSlateApplication::Get().RequestDestroyWindow(Picker.ToSharedRef()); }
						return FReply::Handled();
					})
				]
			]
		]
	);

	FSlateApplication::Get().AddWindow(Picker.ToSharedRef(), true);
	return FReply::Handled();
}

void SMagicOptimizerDock::ApplyScopeFromModal(const TArray<FString>& Includes, const TArray<FString>& Excludes, bool bUseSelection, bool bUseLevel)
{
	ScopedIncludePaths = Includes;
	ScopedExcludePaths = Excludes;
	if (OptimizerSettings)
	{
		OptimizerSettings->bUseSelection = bUseSelection;
		OptimizerSettings->IncludePathsCsv = FString::Join(ScopedIncludePaths, TEXT(","));
		OptimizerSettings->ExcludePathsCsv = FString::Join(ScopedExcludePaths, TEXT(","));
		OptimizerSettings->SaveSettings();
	}
	int32 NumExc = ScopedExcludePaths.Num();
	ScopeSummary = FString::Printf(TEXT("Scope %s%s%s"), bUseSelection ? TEXT("Selection ") : TEXT("Game "), bUseLevel ? TEXT("+ Level ") : TEXT(""), NumExc > 0 ? *FString::Printf(TEXT("(%d excludes)"), NumExc) : TEXT(""));
}

FString SMagicOptimizerDock::ToObjectPath(const FString& CsvPath)
{
	if (CsvPath.Contains(TEXT(".")))
	{
		return CsvPath;
	}
	FString PackagePath = CsvPath;
	FString AssetName;
	if (PackagePath.Split(TEXT("/"), &PackagePath, &AssetName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
	{
		return CsvPath + TEXT(".") + AssetName;
	}
	return CsvPath;
}

bool SMagicOptimizerDock::SaveTextureSnapshot(const TArray<FString>& ObjectPaths, FString& OutSnapshotPath)
{
	TArray<TSharedPtr<FJsonValue>> Items;
	for (const FString& ObjPath : ObjectPaths)
	{
		UTexture* Texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), nullptr, *ObjPath));
		if (!Texture) { continue; }
		TSharedPtr<FJsonObject> Item = MakeShared<FJsonObject>();
		Item->SetStringField(TEXT("ObjectPath"), ObjPath);
		Item->SetBoolField(TEXT("SRGB"), Texture->SRGB);
		Items.Add(MakeShared<FJsonValueObject>(Item));
	}
	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetArrayField(TEXT("textures"), Items);
	const FString OutDir = FPaths::ProjectSavedDir() / TEXT("MagicOptimizer/Reports");
	IFileManager::Get().MakeDirectory(*OutDir, true);
	OutSnapshotPath = OutDir / TEXT("last_snapshot.json");
	FString JsonString;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
	return FFileHelper::SaveStringToFile(JsonString, *OutSnapshotPath);
}

void SMagicOptimizerDock::ApplySrgbOffBatch(const TArray<FString>& ObjectPaths)
{
	bApplyRunning = true;
	ApplyProgressCurrent = 0;
	ApplyProgressTotal = ObjectPaths.Num();
	for (const FString& ObjPath : ObjectPaths)
	{
		ApplyCurrentPath = ObjPath;
		UTexture* Texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), nullptr, *ObjPath));
		if (!Texture) { ++ApplyProgressCurrent; continue; }
		UPackage* Package = Texture->GetOutermost();
		Texture->Modify();
		Texture->SRGB = false;
		Texture->MarkPackageDirty();
		TArray<UPackage*> PackagesToSave; PackagesToSave.Add(Package);
		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, /*bCheckDirty=*/false, /*bPromptToSave=*/false);
		++ApplyProgressCurrent;
		UpdateProgressNotification(TEXT("Applying"), ApplyProgressCurrent, ApplyProgressTotal);
	}
	bApplyRunning = false;
}

void SMagicOptimizerDock::LoadSnapshotFromPath(const FString& SnapshotPath)
{
	LoadedSnapshot.Empty();
	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *SnapshotPath)) { Notify(TEXT("Snapshot not found"), false); return; }
	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()) { Notify(TEXT("Invalid snapshot"), false); return; }
	const TArray<TSharedPtr<FJsonValue>>* Textures = nullptr;
	if (!Root->TryGetArrayField(TEXT("textures"), Textures) || !Textures) { Notify(TEXT("Snapshot empty"), false); return; }
	for (const TSharedPtr<FJsonValue>& Val : *Textures)
	{
		if (!Val.IsValid() || Val->Type != EJson::Object) { continue; }
		TSharedPtr<FJsonObject> Obj = Val->AsObject();
		FTextureSnapshotItem Item;
		Item.ObjectPath = Obj->GetStringField(TEXT("ObjectPath"));
		Item.bSRGB = Obj->GetBoolField(TEXT("SRGB"));
		LoadedSnapshot.Add(Item);
	}
	Notify(TEXT("Snapshot loaded"), true);
}

FReply SMagicOptimizerDock::OnRevertOne(const FString ObjectPath)
{
	const FTextureSnapshotItem* Found = LoadedSnapshot.FindByPredicate([&](const FTextureSnapshotItem& I){ return I.ObjectPath == ObjectPath; });
	if (!Found) { Notify(TEXT("Not in snapshot"), false); return FReply::Handled(); }
	UTexture* Texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), nullptr, *ObjectPath));
	if (!Texture) { Notify(TEXT("Texture not found"), false); return FReply::Handled(); }
	UPackage* Package = Texture->GetOutermost();
	Texture->Modify();
	Texture->SRGB = Found->bSRGB;
	Texture->MarkPackageDirty();
	TArray<UPackage*> PackagesToSave; PackagesToSave.Add(Package);
	FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
	Notify(TEXT("Reverted"), true);
	return FReply::Handled();
}

FText SMagicOptimizerDock::GetCapText() const
{
	const int32 Cap = OptimizerSettings ? OptimizerSettings->MaxChanges : 500;
	return FText::FromString(FString::Printf(TEXT("Cap %d changes"), Cap));
}

EVisibility SMagicOptimizerDock::GetAuditEmptyVisibility() const
{
	const bool bHasScope = OptimizerSettings && (!OptimizerSettings->IncludePathsCsv.IsEmpty() || OptimizerSettings->bUseSelection);
	const bool bHasRows = TextureTableViewModel.IsValid() && TextureTableViewModel->GetFilteredCount() > 0;
	return (!bHasScope || !bHasRows) ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SMagicOptimizerDock::OnOpenScopeFromEmpty()
{
	return OnOpenScopePickerModal();
}

FReply SMagicOptimizerDock::OnToggleTaskDrawer()
{
	if (TaskDrawerWindow.IsValid())
	{
		FSlateApplication::Get().RequestDestroyWindow(TaskDrawerWindow.ToSharedRef());
		TaskDrawerWindow.Reset();
		return FReply::Handled();
	}
	TaskDrawerWindow = SNew(SWindow)
		.Title(FText::FromString(TEXT("Tasks")))
		.ClientSize(FVector2D(480, 360))
		.SupportsMaximize(false)
		.SupportsMinimize(false);
	TaskDrawerWindow->SetContent(
		SNew(SBorder).Padding(8)
		[
			SAssignNew(TaskLinesView, SListView<TSharedPtr<FString>>)
			.ListItemsSource(&TaskLines)
			.OnGenerateRow_Lambda([](TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& Owner)
			{
				return SNew(STableRow<TSharedPtr<FString>>, Owner)[ SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? *Item : TEXT(""))) ];
			})
		]
	);
	FSlateApplication::Get().AddWindow(TaskDrawerWindow.ToSharedRef());
	return FReply::Handled();
}

void SMagicOptimizerDock::Notify(const FString& Msg, bool bSuccess, float ExpireSeconds)
{
	FNotificationInfo Info(FText::FromString(Msg));
	Info.bFireAndForget = true;
	Info.ExpireDuration = ExpireSeconds;
	TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info);
	if (Item.IsValid())
	{
		Item->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
	}
}

void SMagicOptimizerDock::AppendTaskLine(const FString& Line)
{
	TaskLines.Add(MakeShared<FString>(Line));
	if (TaskLinesView.IsValid()) TaskLinesView->RequestListRefresh();
}

void SMagicOptimizerDock::OpenSlideOverForTexture(FTextureAuditRowPtr Item)
{
	if (!Item.IsValid()) { return; }
	if (SlideOverWindow.IsValid())
	{
		FSlateApplication::Get().RequestDestroyWindow(SlideOverWindow.ToSharedRef());
	}
	SlideOverWindow = SNew(SWindow)
		.Title(FText::FromString(TEXT("Details")))
		.ClientSize(FVector2D(520, 640))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.SizingRule(ESizingRule::UserSized);
	FString DiffSummary = FString::Printf(TEXT("Path: %s\nFormat: %s\n"), *Item->Path, *Item->Format);
	// Build a thumbnail widget for the selected asset (best-effort)
	TSharedPtr<SWidget> ThumbWidget;
	{
		FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		const FString ObjPath = ToObjectPath(Item->Path);
		FAssetData AD = ARM.Get().GetAssetByObjectPath(*ObjPath);
		if (AD.IsValid() && ThumbnailPool.IsValid())
		{
			FAssetThumbnail Thumb(AD, 256, 256, ThumbnailPool);
			ThumbWidget = Thumb.MakeThumbnailWidget();
		}
	}
	SlideOverWindow->SetContent(
		SNew(SBorder)
		.Padding(8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Text(FText::FromString(Item->Path)).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,6)[ SNew(STextBlock).Text(FText::FromString(DiffSummary)) ]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SBorder)
				.Padding(4)
				[
					(ThumbWidget.IsValid() ? ThumbWidget.ToSharedRef() : SNew(STextBlock).Text(FText::FromString(TEXT("No thumbnail"))))
				]
			]
		]
	);
	FSlateApplication::Get().AddWindowAsNativeChild(SlideOverWindow.ToSharedRef(), FSlateApplication::Get().GetActiveTopLevelWindow().ToSharedRef());
}

void SMagicOptimizerDock::OnRunSelectionChanged(TSharedPtr<FRunEntry> Item, ESelectInfo::Type)
{
	if (!Item.IsValid()) { return; }
	if (!CompareRunA.IsValid()) { CompareRunA = Item; return; }
	if (!CompareRunB.IsValid()) { CompareRunB = Item; return; }
	CompareRunA = Item;
	CompareRunB.Reset();
}

FReply SMagicOptimizerDock::OnOpenArtifact(const FString& Path)
{
	if (Path.IsEmpty()) { return FReply::Handled(); }
	if (FPaths::FileExists(Path))
	{
		FPlatformProcess::ExploreFolder(*FPaths::GetPath(Path));
	}
	return FReply::Handled();
}

void SMagicOptimizerDock::ComputeRunDeltas()
{
	CompareDeltaLines.Empty();
	if (!CompareRunA.IsValid() || !CompareRunB.IsValid()) { if (CompareDeltaListView.IsValid()) CompareDeltaListView->RequestListRefresh(); return; }
	auto LoadJson = [](const FString& Path, TSharedPtr<FJsonObject>& Out)->bool
	{
		FString Json; if (!FFileHelper::LoadFileToString(Json, *Path)) return false;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
		return FJsonSerializer::Deserialize(Reader, Out) && Out.IsValid();
	};
	TSharedPtr<FJsonObject> A, B;
	if (!LoadJson(CompareRunA->JsonPath, A) || !LoadJson(CompareRunB->JsonPath, B)) { if (CompareDeltaListView.IsValid()) CompareDeltaListView->RequestListRefresh(); return; }
	// Example: compare totals + disk/memory if available
	auto GetInt = [](TSharedPtr<FJsonObject> Obj, const FString& Key)->int32 { return Obj->HasTypedField<EJson::Number>(Key) ? (int32)Obj->GetNumberField(Key) : 0; };
	auto GetNum = [](TSharedPtr<FJsonObject> Obj, const FString& Key)->double { return Obj->HasTypedField<EJson::Number>(Key) ? Obj->GetNumberField(Key) : 0.0; };
	int32 ATotal = GetInt(A, TEXT("total_issues"));
	int32 BTotal = GetInt(B, TEXT("total_issues"));
	CompareDeltaLines.Add(MakeShared<FString>(FString::Printf(TEXT("Total issues: %d -> %d (%+d)"), ATotal, BTotal, BTotal-ATotal)));
	// Try multiple key names for disk/memory
	double ADisk = GetNum(A, TEXT("disk_mb")); if (ADisk==0) ADisk = GetNum(A, TEXT("size_mb"));
	double BDisk = GetNum(B, TEXT("disk_mb")); if (BDisk==0) BDisk = GetNum(B, TEXT("size_mb"));
	double AMem  = GetNum(A, TEXT("memory_mb")); if (AMem==0) AMem = GetNum(A, TEXT("estimated_memory_mb"));
	double BMem  = GetNum(B, TEXT("memory_mb")); if (BMem==0) BMem = GetNum(B, TEXT("estimated_memory_mb"));
	if (ADisk>0 || BDisk>0) { CompareDeltaLines.Add(MakeShared<FString>(FString::Printf(TEXT("Disk MB: %.1f -> %.1f (%+.1f)"), ADisk, BDisk, BDisk-ADisk))); }
	if (AMem>0  || BMem>0)  { CompareDeltaLines.Add(MakeShared<FString>(FString::Printf(TEXT("Memory MB: %.1f -> %.1f (%+.1f)"), AMem, BMem, BMem-AMem))); }
	if (CompareDeltaListView.IsValid()) CompareDeltaListView->RequestListRefresh();
}

void SMagicOptimizerDock::RefreshScopePaths()
{
	ScopePaths.Empty();
	TArray<FString> Folders; IFileManager::Get().FindFilesRecursive(Folders, *FPaths::ProjectContentDir(), TEXT("*"), false, true);
	for (const FString& Dir : Folders)
	{
		// Keep Content/* level only for brevity
		FString Rel = Dir; Rel.RemoveFromStart(FPaths::ProjectContentDir());
		if (!Rel.Contains(TEXT("/"))) { ScopePaths.Add(MakeShared<FString>(FString::Printf(TEXT("/Game/%s"), *Rel))); }
	}
	if (ScopePathsList.IsValid()) ScopePathsList->RequestListRefresh();
}

void SMagicOptimizerDock::OnToggleScopePath(const FString& Path, bool bChecked)
{
	if (bChecked) { CheckedScopePaths.Add(Path); } else { CheckedScopePaths.Remove(Path); }
	if (OptimizerSettings)
	{
		TArray<FString> Inc = ScopedIncludePaths;
		for (const FString& P : CheckedScopePaths) { Inc.AddUnique(P); }
		OptimizerSettings->IncludePathsCsv = FString::Join(Inc, TEXT(","));
		OptimizerSettings->SaveSettings();
		ScopedIncludePaths = Inc;
	}
}

void SMagicOptimizerDock::SwitchAuditType(EAuditAssetType NewType)
{
	CurrentAuditType = NewType;
}


FReply SMagicOptimizerDock::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape)
	{
		if (SlideOverWindow.IsValid())
		{
			FSlateApplication::Get().RequestDestroyWindow(SlideOverWindow.ToSharedRef());
			SlideOverWindow.Reset();
			return FReply::Handled();
		}
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SMagicOptimizerDock::StartProgressNotification(const FString& Title)
{
	FNotificationInfo Info(FText::FromString(Title));
	Info.bFireAndForget = false;
	Info.bUseThrobber = false;
	ActiveNotification = FSlateNotificationManager::Get().AddNotification(Info);
}

void SMagicOptimizerDock::UpdateProgressNotification(const FString& Label, int32 Processed, int32 Total)
{
	if (ActiveNotification.IsValid())
	{
		ActiveNotification->SetText(FText::FromString(FString::Printf(TEXT("%s (%d/%d)"), *Label, Processed, Total)));
		ActiveNotification->SetCompletionState(SNotificationItem::CS_Pending);
	}
}

void SMagicOptimizerDock::CompleteProgressNotification(bool bSuccess, const FString& FinalText)
{
	if (ActiveNotification.IsValid())
	{
		ActiveNotification->SetText(FText::FromString(FinalText));
		ActiveNotification->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
		ActiveNotification->ExpireAndFadeout();
		ActiveNotification.Reset();
	}
}


