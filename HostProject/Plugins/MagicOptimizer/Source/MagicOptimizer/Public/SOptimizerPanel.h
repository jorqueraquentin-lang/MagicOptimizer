// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SHeaderRow.h"
#include "OptimizerSettings.h"
#include "PythonBridge.h"
#include "ViewModels/TextureModels.h"

class SCheckBox;
class SButton;
class SEditableTextBox;
class SScrollBox;
class SNotificationList;
class SListViewBase;

// Moved row models to ViewModels/TextureModels.h

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
	FString CurrentProfile;

	// Output log buffer
	FString LastStdOut;
	FString LastStdErr;
	FString LastResultMessage;
	int32 LastAssetsProcessed = 0;
	int32 LastAssetsModified = 0;

	// Audit results data
	TArray<FTextureAuditRowPtr> TextureRows;
	TArray<FTextureAuditRowPtr> AllTextureRows;
	TSharedPtr<class SListView<FTextureAuditRowPtr>> TextureListView;
	TSharedPtr<SHeaderRow> TextureHeaderRow;

	// Recommendations data
	TArray<FTextureRecRowPtr> TextureRecRows;
	TArray<FTextureRecRowPtr> AllTextureRecRows;
	TSharedPtr<class SListView<FTextureRecRowPtr>> TextureRecListView;
	TSharedPtr<SHeaderRow> TextureRecHeaderRow;

	// Sorting state for texture results
	enum class ETextureSortColumn { Path, Width, Height, Format };
	ETextureSortColumn CurrentSortColumn = ETextureSortColumn::Path;
	bool bSortAscending = true;

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

	// Results loading
	void LoadTextureAuditCsv();
	TSharedRef<class ITableRow> OnGenerateTextureRow(FTextureAuditRowPtr Item, const TSharedRef<class STableViewBase>& OwnerTable);
	void LoadTextureRecommendationsCsv();
	TSharedRef<class ITableRow> OnGenerateTextureRecRow(FTextureRecRowPtr Item, const TSharedRef<class STableViewBase>& OwnerTable);

	void SortTextureRows();
	FReply OnSortByPath();
	FReply OnSortByWidth();
	FReply OnSortByHeight();
	FReply OnSortByFormat();
	EColumnSortMode::Type GetSortModeForColumn(ETextureSortColumn Column) const;
	void OnHeaderColumnSort(const EColumnSortPriority::Type SortPriority, const FName& ColumnId, const EColumnSortMode::Type NewSortMode);

	// Filters for texture results
	FString TextureFilterText;
	int32 FilterMinWidth = 0;
	int32 FilterMinHeight = 0;
	void ApplyTextureFilterAndSort();
	void OnFilterTextChanged(const FText& NewText);
	void OnMinWidthChanged(const FText& NewText);
	void OnMinHeightChanged(const FText& NewText);
	FReply OnClearTextureFilters();

	// UI refresh
	void RefreshUI();
};
