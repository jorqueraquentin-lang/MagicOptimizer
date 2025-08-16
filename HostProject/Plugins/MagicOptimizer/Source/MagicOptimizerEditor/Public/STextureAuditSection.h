// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SHeaderRow.h"
#include "ViewModels/TextureModels.h"

class SListViewBase;
class SHeaderRow;
class SEditableTextBox;
class SButton;
class FTextureTableViewModel;

class STextureAuditSection : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STextureAuditSection) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// Data interface
	void SetTextureRows(const TArray<FTextureAuditRowPtr>& InRows);
	void RefreshDisplay();
	
	// ViewModel interface
	void SetViewModel(TSharedPtr<FTextureTableViewModel> InViewModel);

	// Filter interface (now delegates to ViewModel)
	void SetFilterText(const FString& InFilterText);
	void SetMinWidth(int32 InMinWidth);
	void SetMinHeight(int32 InMinHeight);
	void SetFormatFilter(const FString& InFormatFilter);
	void ClearFilters();

	// Sort interface (now delegates to ViewModel)
	void SetSortColumn(ETextureSortColumn InColumn, bool bInAscending);
	EColumnSortMode::Type GetSortModeForColumn(ETextureSortColumn Column) const;

	// Event delegates
	DECLARE_DELEGATE_OneParam(FOnFilterChanged, const FString&);
	DECLARE_DELEGATE_OneParam(FOnMinWidthChanged, int32);
	DECLARE_DELEGATE_OneParam(FOnMinHeightChanged, int32);
	DECLARE_DELEGATE_OneParam(FOnFormatFilterChanged, const FString&);
	DECLARE_DELEGATE_OneParam(FOnSortChanged, ETextureSortColumn);
	DECLARE_DELEGATE_OneParam(FOnRowAction, const FString&);
	DECLARE_DELEGATE(FOnSettingsChanged);

	FOnFilterChanged OnFilterChanged;
	FOnMinWidthChanged OnMinWidthChanged;
	FOnMinHeightChanged OnMinHeightChanged;
	FOnFormatFilterChanged OnFormatFilterChanged;
	FOnSortChanged OnSortChanged;
	FOnRowAction OnCopyPath;
	FOnRowAction OnOpenInContentBrowser;
	FOnSettingsChanged OnSettingsChanged;

protected:
	// ViewModel reference
	TSharedPtr<FTextureTableViewModel> ViewModel;
	
	// Internal data (now managed by ViewModel)
	TArray<FTextureAuditRowPtr> AllTextureRows;
	TArray<FTextureAuditRowPtr> FilteredTextureRows;
	
	// UI components
	TSharedPtr<SListView<FTextureAuditRowPtr>> TextureListView;
	TSharedPtr<SHeaderRow> TextureHeaderRow;
	
	// Filter state (now managed by ViewModel, kept for UI binding)
	FString FilterText;
	int32 FilterMinWidth = 0;
	int32 FilterMinHeight = 0;
	FString FormatFilter;
	
	// Sort state (now managed by ViewModel, kept for UI binding)
	enum class ETextureSortColumn { Path, Width, Height, Format };
	ETextureSortColumn CurrentSortColumn = ETextureSortColumn::Path;
	bool bSortAscending = true;

	// Internal methods
	void ApplyFiltersAndSort();
	void SortRows();
	TSharedRef<class ITableRow> OnGenerateRow(FTextureAuditRowPtr Item, const TSharedRef<class STableViewBase>& OwnerTable);
	FReply OnHeaderColumnSort(const EColumnSortPriority::Type SortPriority, const FName& ColumnId, const EColumnSortMode::Type NewSortMode);
	
	// Filter handlers
	void OnFilterTextChanged(const FText& NewText);
	void OnMinWidthChanged(const FText& NewText);
	void OnMinHeightChanged(const FText& NewText);
	void OnFormatFilterChanged(const FText& NewText);
	FReply OnClearFilters();
	
	// Helper methods
	void UpdateUIFromViewModel();
};
