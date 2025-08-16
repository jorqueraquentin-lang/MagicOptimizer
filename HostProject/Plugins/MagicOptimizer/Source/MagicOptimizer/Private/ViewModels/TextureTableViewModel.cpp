// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#include "ViewModels/TextureTableViewModel.h"
#include "OptimizerSettings.h"

FTextureTableViewModel::FTextureTableViewModel()
	: MinWidthFilter(0)
	, MinHeightFilter(0)
	, CurrentSortColumn(ESortColumn::Path)
	, bSortAscending(true)
{
}

void FTextureTableViewModel::SetSourceData(const TArray<FTextureAuditRowPtr>& InSourceData)
{
	SourceData = InSourceData;
	RefreshData();
}

void FTextureTableViewModel::RefreshData()
{
	ApplyFilters();
	ApplySorting();
}

void FTextureTableViewModel::SetTextFilter(const FString& InFilter)
{
	TextFilter = InFilter;
	RefreshData();
}

void FTextureTableViewModel::SetMinWidthFilter(int32 InMinWidth)
{
	MinWidthFilter = InMinWidth;
	RefreshData();
}

void FTextureTableViewModel::SetMinHeightFilter(int32 InMinHeight)
{
	MinHeightFilter = InMinHeight;
	RefreshData();
}

void FTextureTableViewModel::SetFormatFilter(const FString& InFilter)
{
	FormatFilter = InFilter;
	RefreshData();
}

void FTextureTableViewModel::ClearAllFilters()
{
	TextFilter.Empty();
	MinWidthFilter = 0;
	MinHeightFilter = 0;
	FormatFilter.Empty();
	RefreshData();
}

void FTextureTableViewModel::SetSortColumn(ESortColumn InColumn, bool bInAscending)
{
	CurrentSortColumn = InColumn;
	bSortAscending = bInAscending;
	ApplySorting();
}

bool FTextureTableViewModel::HasActiveFilters() const
{
	return !TextFilter.IsEmpty() || MinWidthFilter > 0 || MinHeightFilter > 0 || !FormatFilter.IsEmpty();
}

void FTextureTableViewModel::LoadSettingsFromConfig(UOptimizerSettings* Settings)
{
	if (Settings)
	{
		// Load filter settings
		TextFilter = Settings->TextureFilterText;
		MinWidthFilter = Settings->TextureFilterMinWidth;
		MinHeightFilter = Settings->TextureFilterMinHeight;
		FormatFilter = Settings->TextureFormatFilter;
		
		// Load sort settings
		if (Settings->TextureSortColumn == TEXT("Width"))
		{
			CurrentSortColumn = ESortColumn::Width;
		}
		else if (Settings->TextureSortColumn == TEXT("Height"))
		{
			CurrentSortColumn = ESortColumn::Height;
		}
		else if (Settings->TextureSortColumn == TEXT("Format"))
		{
			CurrentSortColumn = ESortColumn::Format;
		}
		else
		{
			CurrentSortColumn = ESortColumn::Path; // Default
		}
		
		bSortAscending = Settings->bTextureSortAscending;
		
		// Apply loaded settings to data
		RefreshData();
	}
}

void FTextureTableViewModel::SaveSettingsToConfig(UOptimizerSettings* Settings)
{
	if (Settings)
	{
		// Save filter settings
		Settings->TextureFilterText = TextFilter;
		Settings->TextureFilterMinWidth = MinWidthFilter;
		Settings->TextureFilterMinHeight = MinHeightFilter;
		Settings->TextureFormatFilter = FormatFilter;
		
		// Save sort settings
		switch (CurrentSortColumn)
		{
		case ESortColumn::Width:
			Settings->TextureSortColumn = TEXT("Width");
			break;
		case ESortColumn::Height:
			Settings->TextureSortColumn = TEXT("Height");
			break;
		case ESortColumn::Format:
			Settings->TextureSortColumn = TEXT("Format");
			break;
		default:
			Settings->TextureSortColumn = TEXT("Path");
			break;
		}
		
		Settings->bTextureSortAscending = bSortAscending;
		
		// Persist to disk
		Settings->SaveSettings();
	}
}

void FTextureTableViewModel::ApplyFilters()
{
	FilteredData.Empty();
	
	for (const auto& Row : SourceData)
	{
		if (RowPassesFilters(Row))
		{
			FilteredData.Add(Row);
		}
	}
}

void FTextureTableViewModel::ApplySorting()
{
	FilteredData.Sort([this](const FTextureAuditRowPtr& A, const FTextureAuditRowPtr& B)
	{
		int32 CompareResult = CompareRows(A, B);
		return bSortAscending ? (CompareResult < 0) : (CompareResult > 0);
	});
}

bool FTextureTableViewModel::RowPassesFilters(const FTextureAuditRowPtr& Row) const
{
	if (!Row.IsValid())
	{
		return false;
	}
	
	// Text filter
	if (!TextFilter.IsEmpty())
	{
		FString SearchText = TextFilter.ToLower();
		FString PathLower = Row->Path.ToLower();
		FString FormatLower = Row->Format.ToLower();
		
		if (!PathLower.Contains(SearchText) && !FormatLower.Contains(SearchText))
		{
			return false;
		}
	}
	
	// Width filter
	if (MinWidthFilter > 0 && Row->Width < MinWidthFilter)
	{
		return false;
	}
	
	// Height filter
	if (MinHeightFilter > 0 && Row->Height < MinHeightFilter)
	{
		return false;
	}
	
	// Format filter
	if (!FormatFilter.IsEmpty())
	{
		FString SearchFormat = FormatFilter.ToLower();
		FString RowFormat = Row->Format.ToLower();
		
		if (!RowFormat.Contains(SearchFormat))
		{
			return false;
		}
	}
	
	return true;
}

int32 FTextureTableViewModel::CompareRows(const FTextureAuditRowPtr& A, const FTextureAuditRowPtr& B) const
{
	if (!A.IsValid() || !B.IsValid())
	{
		return 0;
	}
	
	switch (CurrentSortColumn)
	{
	case ESortColumn::Path:
		return A->Path.Compare(B->Path);
		
	case ESortColumn::Width:
		return A->Width - B->Width;
		
	case ESortColumn::Height:
		return A->Height - B->Height;
		
	case ESortColumn::Format:
		return A->Format.Compare(B->Format);
		
	default:
		return 0;
	}
}
