// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "ViewModels/TextureModels.h"

class UOptimizerSettings;
class UPythonBridge;
class SDashboard;
class STextureAuditSection;
class STextureRecommendSection;
class SMeshesTab;
class SMaterialsTab;
class FTextureTableViewModel;
class FAssetThumbnailPool;
struct FSlateBrush;
template<typename ItemType> class SListView;
class SHeaderRow;

/**
 * SMagicOptimizerDock
 * Root dockable panel implementing the new guided flow UI:
 * Header (presets/targets/scope/run), left rail (scope/filters),
 * main views (Audit/Recommend/Apply/Verify/Reports), right quick-fix shelf.
 */
class SMagicOptimizerDock : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMagicOptimizerDock) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SMagicOptimizerDock();

private:
	// Forward declarations for nested types used before definition
	struct FRunEntry;
	// Settings and services
	UOptimizerSettings* OptimizerSettings = nullptr;
	UPythonBridge* PythonBridge = nullptr;

	// View state
	enum class EMainView : uint8 { Audit, Recommend, Apply, Verify, Reports };
	EMainView CurrentView = EMainView::Audit;

	// Audit sub-view state
	enum class EAuditAssetType : uint8 { Textures, Meshes, Materials };
	EAuditAssetType CurrentAuditType = EAuditAssetType::Textures;

	// Presets and targets
	TArray<TSharedPtr<FString>> PresetOptions;
	TSharedPtr<FString> CurrentPreset;
	TSet<FName> ActiveTargets; // e.g., PS5, Quest, Switch

	// Scoped paths (simple text summary and config-backed include/exclude)
	FString ScopeSummary;
	TArray<FString> ScopedIncludePaths;
	TArray<FString> ScopedExcludePaths;

	// Widgets
	TSharedPtr<class SEditableTextBox> SearchBox;
	TSharedPtr<class SWidgetSwitcher> ViewSwitcher;
	TSharedPtr<SDashboard> AuditOverviewWidget;
	TSharedPtr<STextureAuditSection> AuditTexturesWidget;
	TSharedPtr<STextureRecommendSection> RecommendWidget;
	TSharedPtr<class SBorder> RightQuickFixShelf;
	TSharedPtr<class SVerticalBox> QuickFixChipsBox;
	// Recommend view list of human-readable proposed changes
	TArray<TSharedPtr<FString>> ProposedChangeItems;
	TSharedPtr<SListView<TSharedPtr<FString>>> ProposedListView;
	TSharedPtr<SHeaderRow> ProposedHeader;
	// Reports list
	TSharedPtr<SListView<TSharedPtr<FRunEntry>>> RunsListView;
	// Compare deltas
	TArray<TSharedPtr<FString>> CompareDeltaLines;
	TSharedPtr<SListView<TSharedPtr<FString>>> CompareDeltaListView;

	// ViewModel
	TSharedPtr<FTextureTableViewModel> TextureTableViewModel;

	// Data caches
	TArray<FTextureAuditRowPtr> SelectedTextureRows;

	// Apply progress state
	bool bApplyRunning = false;
	int32 ApplyProgressCurrent = 0;
	int32 ApplyProgressTotal = 0;
	FString ApplyCurrentPath;

	// Verify snapshot data
	struct FTextureSnapshotItem { FString ObjectPath; bool bSRGB = false; };
	TArray<FTextureSnapshotItem> LoadedSnapshot;

	// Reports view state
	struct FRunEntry { FString Name; FString Dir; FString Preset; FString Date; FString CsvPath; FString JsonPath; FString HtmlPath; };
	TArray<TSharedPtr<FRunEntry>> RunEntries;
	TSharedPtr<FRunEntry> SelectedRun;
	TSharedPtr<FRunEntry> CompareRunA;
	TSharedPtr<FRunEntry> CompareRunB;

	// UI builders
	TSharedRef<SWidget> BuildHeader();
	TSharedRef<SWidget> BuildLeftRail();
	TSharedRef<SWidget> BuildMainViews();
	TSharedRef<SWidget> BuildRightShelf();
	TSharedRef<SWidget> BuildReportsView();
	TSharedRef<SWidget> BuildAuditSwitcher();

	// Header handlers
	void OnPresetSelected(TSharedPtr<FString> InPreset, ESelectInfo::Type InSelectInfo);
	FText GetPresetLabel() const;
	FReply OnOpenScopePicker();
	FText GetScopeSummaryText() const;
	FReply OnRunScan();
	FReply OnPreviewPlan();
	FReply OnAutoFix();
	FText GetCapText() const;

	// Helpers
	void InitializeServices();
	void InitializePresets();
	void LoadAuditData();
	void LoadRecommendations();
	void UpdateSourceControlHint();
	void UpdateQuickFixShelf();
	void OnSearchTextChanged(const FText& NewText);
	void SwitchView(EMainView NewView);
	void SwitchAuditType(EAuditAssetType NewType);
	void RefreshRunsList();
	void OnRunSelectionChanged(TSharedPtr<FRunEntry> Item, ESelectInfo::Type SelectInfo);
	FReply OnOpenArtifact(const FString& Path);
	void ComputeRunDeltas();

	// Keyboard accessibility
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool SupportsKeyboardFocus() const override { return true; }

	// Audit empty state
	EVisibility GetAuditEmptyVisibility() const;
	FReply OnOpenScopeFromEmpty();

	// Task drawer and notifications
	FReply OnToggleTaskDrawer();
	void Notify(const FString& Msg, bool bSuccess, float ExpireSeconds = 3.f);
	void AppendTaskLine(const FString& Line);
	TArray<TSharedPtr<FString>> TaskLines;
	TSharedPtr<SListView<TSharedPtr<FString>>> TaskLinesView;
	TSharedPtr<class SWindow> TaskDrawerWindow;
	TSharedPtr<class SNotificationItem> ActiveNotification;
	void StartProgressNotification(const FString& Title);
	void UpdateProgressNotification(const FString& Label, int32 Processed, int32 Total);
	void CompleteProgressNotification(bool bSuccess, const FString& FinalText);
	int32 CurrentTaskProcessed = 0;
	int32 CurrentTaskTotal = 0;
	FString CurrentTaskLabel;

	// Audit tabs
	TSharedPtr<class SMeshesTab> MeshesTabWidget;
	TSharedPtr<class SMaterialsTab> MaterialsTabWidget;

	// Recommend/Plan data
	TArray<FString> ProposedChangeLines;
	void BuildProposedChangesFromSelection();
	void ExportProposedChangesCsv() const;
	FReply OnExportProposedCsv();
	FReply OnCopyProposedForJira();

	// Asset type toggles (left rail)
	ECheckBoxState IsCategoryChecked(uint8 MaskBit) const;
	void OnCategoryChanged(ECheckBoxState NewState, uint8 MaskBit);

	// Scope picker
	FReply OnOpenScopePickerModal();
	void ApplyScopeFromModal(const TArray<FString>& Includes, const TArray<FString>& Excludes, bool bUseSelection, bool bUseLevel);

	// Scope tree (left rail)
	TArray<TSharedPtr<FString>> ScopePaths;
	TSet<FString> CheckedScopePaths;
	TSharedPtr<SListView<TSharedPtr<FString>>> ScopePathsList;
	void RefreshScopePaths();
	void OnToggleScopePath(const FString& Path, bool bChecked);

	// Source control hint
	FString SourceControlBranch;
	int32 NumFilesCheckedOut = 0;

	// Run artifacts
	FString LastSnapshotPath;
	FString LastReportDir;

	// Apply/Verify helpers
	static FString ToObjectPath(const FString& CsvPath);
	bool SaveTextureSnapshot(const TArray<FString>& ObjectPaths, FString& OutSnapshotPath);
	void ApplySrgbOffBatch(const TArray<FString>& ObjectPaths);
	void LoadSnapshotFromPath(const FString& SnapshotPath);
	FReply OnRevertOne(const FString ObjectPath);

	// Slide-over panel
	void OpenSlideOverForTexture(FTextureAuditRowPtr Item);
	TSharedPtr<class SWindow> SlideOverWindow;
	TSharedPtr<FSlateBrush> SlideThumbBrush;
	TSharedPtr<FAssetThumbnailPool> ThumbnailPool;

	// Saved views (session-only)
	struct FSavedView { FString Text; int32 MinW = 0; int32 MinH = 0; FString Format; int32 SortCol = 0; bool bAsc = true; };
	TMap<FString, FSavedView> SavedViews;
	TArray<TSharedPtr<FString>> SavedViewNames;
	TSharedPtr<FString> CurrentSavedView;
	void SaveCurrentView();
	void LoadSavedView(const FString& Name);
};


