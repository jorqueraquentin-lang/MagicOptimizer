// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ViewModels/TextureModels.h"

class UOptimizerSettings;

class FTextureTableViewModel
{
public:
	// Sort column enumeration
	enum class ESortColumn : uint8
	{
		Path,
		Width,
		Height,
		Format
	};

	// Constructor
	FTextureTableViewModel();

	// Data management
	void SetSourceData(const TArray<FTextureAuditRowPtr>& InSourceData);
	void RefreshData();
	
	// Filtering
	void SetTextFilter(const FString& InFilter);
	void SetMinWidthFilter(int32 InMinWidth);
	void SetMinHeightFilter(int32 InMinHeight);
	void SetFormatFilter(const FString& InFilter);
	void ClearAllFilters();
	
	// Sorting
	void SetSortColumn(ESortColumn InColumn, bool bInAscending);
	ESortColumn GetCurrentSortColumn() const { return CurrentSortColumn; }
	bool IsSortAscending() const { return bSortAscending; }
	
	// Data access
	const TArray<FTextureAuditRowPtr>& GetFilteredData() const { return FilteredData; }
	const TArray<FTextureAuditRowPtr>& GetSourceData() const { return SourceData; }
	int32 GetTotalCount() const { return SourceData.Num(); }
	int32 GetFilteredCount() const { return FilteredData.Num(); }
	
	// Filter state
	FString GetTextFilter() const { return TextFilter; }
	int32 GetMinWidthFilter() const { return MinWidthFilter; }
	int32 GetMinHeightFilter() const { return MinHeightFilter; }
	FString GetFormatFilter() const { return FormatFilter; }
	bool HasActiveFilters() const;

	// Settings integration
	void LoadSettingsFromConfig(UOptimizerSettings* Settings);
	void SaveSettingsToConfig(UOptimizerSettings* Settings);

private:
	// Source and filtered data
	TArray<FTextureAuditRowPtr> SourceData;
	TArray<FTextureAuditRowPtr> FilteredData;
	
	// Filter state
	FString TextFilter;
	int32 MinWidthFilter;
	int32 MinHeightFilter;
	FString FormatFilter;
	
	// Sort state
	ESortColumn CurrentSortColumn;
	bool bSortAscending;
	
	// Internal methods
	void ApplyFilters();
	void ApplySorting();
	bool RowPassesFilters(const FTextureAuditRowPtr& Row) const;
	int32 CompareRows(const FTextureAuditRowPtr& A, const FTextureAuditRowPtr& B) const;
};
