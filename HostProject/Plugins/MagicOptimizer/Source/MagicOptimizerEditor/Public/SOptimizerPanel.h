// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STabView.h"
#include "Widgets/Views/STabPanel.h"
#include "OptimizerSettings.h"
#include "PythonBridge.h"
#include "ViewModels/TextureModels.h"
#include "ViewModels/TextureTableViewModel.h"

// Forward declarations for new section widgets
class STextureAuditSection;
class STextureRecommendSection;

// Forward declarations for new tab widgets
class STexturesTab;
class SMeshesTab;
class SMaterialsTab;
class SOptimizationTab;

class SCheckBox;
class SButton;
class SEditableTextBox;
class SScrollBox;
class SNotificationList;
class SListViewBase;

// Tab identifiers for the multi-tab interface
enum class EOptimizerTab : uint8
{
	Textures,
	Meshes,
	Materials,
	Optimization
};

class SOptimizerPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SOptimizerPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SOptimizerPanel();

protected:
	// Settings reference
	UPROPERTY()
	UOptimizerSettings* OptimizerSettings;

	// Python bridge reference
	UPROPERTY()
	UPythonBridge* PythonBridge;

	// UI state
	TArray<TSharedPtr<FString>> TargetProfiles;
	FString CurrentProfile;

	// Step navigation for streamlined UX
	enum class EOptimizerStep : uint8 { Audit, Apply, Verify };
	EOptimizerStep CurrentStep = EOptimizerStep::Audit;

	// Tab management
	EOptimizerTab CurrentTab = EOptimizerTab::Textures;
	TSharedPtr<STabView<EOptimizerTab>> TabView;
	TSharedPtr<STabPanel<EOptimizerTab>> TabPanel;

	// Tab widgets
	TSharedPtr<STexturesTab> TexturesTab;
	TSharedPtr<SMeshesTab> MeshesTab;
	TSharedPtr<SMaterialsTab> MaterialsTab;
	TSharedPtr<SOptimizationTab> OptimizationTab;

	// Output log buffer
	FString LastStdOut;
	FString LastStdErr;
	FString LastResultMessage;
	int32 LastAssetsProcessed = 0;
	int32 LastAssetsModified = 0;

	// Section widgets (legacy - will be replaced by tab widgets)
	TSharedPtr<STextureAuditSection> TextureAuditSection;
	TSharedPtr<STextureRecommendSection> TextureRecommendSection;

	// ViewModel for texture table state
	TSharedPtr<FTextureTableViewModel> TextureTableViewModel;

	// Legacy data storage (will be removed after refactoring)
	TArray<FTextureAuditRowPtr> TextureRows;
	TArray<FTextureAuditRowPtr> AllTextureRows;
	TArray<FTextureRecRowPtr> TextureRecRows;
	TArray<FTextureRecRowPtr> AllTextureRecRows;

	// UI event handlers
	FReply OnAuditClicked();
	FReply OnRecommendClicked();
	FReply OnApplyClicked();
	FReply OnVerifyClicked();
	FReply OnSettingsClicked();

	// Tab management
	void OnTabChanged(EOptimizerTab NewTab);
	TSharedRef<SWidget> CreateTabContent(EOptimizerTab TabType);
	FText GetTabDisplayName(EOptimizerTab TabType) const;
	FSlateIcon GetTabIcon(EOptimizerTab TabType) const;

	// Step controls
	FReply OnStepAudit();
	FReply OnStepApply();
	FReply OnStepVerify();
	bool IsAuditStepEnabled() const;
	bool IsApplyStepEnabled() const;
	bool IsVerifyStepEnabled() const;
	EVisibility GetAuditVisibility() const;
	EVisibility GetApplyVisibility() const;
	EVisibility GetVerifyVisibility() const;

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

	// Results loading
	void LoadTextureAuditCsv();
	TSharedRef<class ITableRow> OnGenerateTextureRow(FTextureAuditRowPtr Item, const TSharedRef<class STableViewBase>& OwnerTable);
	void LoadTextureRecommendationsCsv();
	TSharedRef<class ITableRow> OnGenerateTextureRecRow(FTextureRecRowPtr Item, const TSharedRef<class STableViewBase>& OwnerTable);

	// Simple computed summaries
	FText GetAuditSummaryText() const;
	EVisibility GetAuditSummaryVisibility() const;

	// Legacy sort methods (will be removed after refactoring)
	void SortTextureRows();
	FReply OnSortByPath();
	FReply OnSortByWidth();
	FReply OnSortByHeight();
	FReply OnSortByFormat();
	EColumnSortMode::Type GetSortModeForColumn(ETextureSortColumn Column) const;
	void OnHeaderColumnSort(const EColumnSortPriority::Type SortPriority, const FName& ColumnId, const EColumnSortMode::Type NewSortMode);

	// Legacy filter state (will be removed after refactoring)
	FString TextureFilterText;
	int32 FilterMinWidth = 0;
	int32 FilterMinHeight = 0;

	// UI refresh
	void RefreshUI();

	// Internal helpers
	void InitializeUI();
	void RunOptimizationPhase(const FString& Phase);
	TArray<FString> GetSelectedCategories() const;
	FOptimizerRunParams BuildRunParams(const FString& Phase) const;
	void ShowNotification(const FString& Message, bool bIsSuccess = true);
	void LogMessage(const FString& Message, bool bIsError = false);
	void UpdateUI();

	/** Show first-time consent dialog if needed */
	void ShowFirstTimeConsentDialog();

	/** Check if first-time consent dialog has been shown */
	bool HasShownFirstTimeConsent() const;
};
