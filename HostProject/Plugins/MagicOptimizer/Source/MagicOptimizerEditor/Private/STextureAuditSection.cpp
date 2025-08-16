// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#include "STextureAuditSection.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SVerticalBox.h"
#include "Widgets/Layout/SHorizontalBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "CoreStyle.h"
#include "HAL/PlatformApplicationMisc.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Services/Editor/ContentBrowserActions.h"
#include "ViewModels/TextureTableViewModel.h"

void STextureAuditSection::Construct(const FArguments& InArgs)
{
	ChildSlot
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
					.Text_Lambda([this]() { return FText::FromString(FilterText); })
					.OnTextChanged(this, &STextureAuditSection::OnFilterTextChanged)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,4,0)
				[
					SNew(SEditableTextBox)
					.MinDesiredWidth(70)
					.HintText(FText::FromString(TEXT("Min W")))
					.Text_Lambda([this]() { return FText::FromString(FString::FromInt(FilterMinWidth)); })
					.OnTextChanged(this, &STextureAuditSection::OnMinWidthChanged)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,4,0)
				[
					SNew(SEditableTextBox)
					.MinDesiredWidth(70)
					.HintText(FText::FromString(TEXT("Min H")))
					.Text_Lambda([this]() { return FText::FromString(FString::FromInt(FilterMinHeight)); })
					.OnTextChanged(this, &STextureAuditSection::OnMinHeightChanged)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,4,0)
				[
					SNew(SEditableTextBox)
					.MinDesiredWidth(80)
					.HintText(FText::FromString(TEXT("Format")))
					.Text_Lambda([this]() { return FText::FromString(FormatFilter); })
					.OnTextChanged(this, &STextureAuditSection::OnFormatFilterChanged)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Clear")))
					.OnClicked(this, &STextureAuditSection::OnClearFilters)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0,0,0,2)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { 
					if (ViewModel.IsValid())
					{
						return FText::FromString(FString::Printf(TEXT("Filtered %d / %d"), 
							ViewModel->GetFilteredCount(), ViewModel->GetTotalCount())); 
					}
					return FText::FromString(FString::Printf(TEXT("Filtered %d / %d"), 0, AllTextureRows.Num())); 
				})
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SAssignNew(TextureListView, SListView<FTextureAuditRowPtr>)
				.ItemHeight(20)
				.ListItemsSource_Lambda([this]() -> const TArray<FTextureAuditRowPtr>& { 
					return ViewModel.IsValid() ? ViewModel->GetFilteredData() : AllTextureRows; 
				})
				.OnGenerateRow(this, &STextureAuditSection::OnGenerateRow)
				.HeaderRow(
					SAssignNew(TextureHeaderRow, SHeaderRow)
					+ SHeaderRow::Column(FName(TEXT("Path"))).DefaultLabel(FText::FromString(TEXT("Path"))).HAlignCell(HAlign_Left).OnSort(this, &STextureAuditSection::OnHeaderColumnSort)
					+ SHeaderRow::Column(FName(TEXT("Width"))).DefaultLabel(FText::FromString(TEXT("Width"))).HAlignCell(HAlign_Right).OnSort(this, &STextureAuditSection::OnHeaderColumnSort)
					+ SHeaderRow::Column(FName(TEXT("Height"))).DefaultLabel(FText::FromString(TEXT("Height"))).HAlignCell(HAlign_Right).OnSort(this, &STextureAuditSection::OnHeaderColumnSort)
					+ SHeaderRow::Column(FName(TEXT("Format"))).DefaultLabel(FText::FromString(TEXT("Format"))).HAlignCell(HAlign_Left).OnSort(this, &STextureAuditSection::OnHeaderColumnSort)
					+ SHeaderRow::Column(FName(TEXT("Actions"))).DefaultLabel(FText::FromString(TEXT("Actions"))).HAlignCell(HAlign_Center)
				)
			]
		]
	];
}

void STextureAuditSection::SetTextureRows(const TArray<FTextureAuditRowPtr>& InRows)
{
	AllTextureRows = InRows;
	
	// Update ViewModel if available
	if (ViewModel.IsValid())
	{
		ViewModel->SetSourceData(InRows);
		UpdateUIFromViewModel();
	}
	else
	{
		// Fallback to local filtering if no ViewModel
		ApplyFiltersAndSort();
	}
}

void STextureAuditSection::SetViewModel(TSharedPtr<FTextureTableViewModel> InViewModel)
{
	ViewModel = InViewModel;
	
	// Update UI with ViewModel data if available
	if (ViewModel.IsValid() && AllTextureRows.Num() > 0)
	{
		ViewModel->SetSourceData(AllTextureRows);
		UpdateUIFromViewModel();
	}
}

void STextureAuditSection::UpdateUIFromViewModel()
{
	if (!ViewModel.IsValid())
	{
		return;
	}
	
	// Update UI state from ViewModel
	FilterText = ViewModel->GetTextFilter();
	FilterMinWidth = ViewModel->GetMinWidthFilter();
	FilterMinHeight = ViewModel->GetMinHeightFilter();
	FormatFilter = ViewModel->GetFormatFilter();
	
	// Update list view with filtered data
	if (TextureListView.IsValid())
	{
		TextureListView->RequestListRefresh();
	}
}

void STextureAuditSection::RefreshDisplay()
{
	ApplyFiltersAndSort();
	if (TextureListView.IsValid())
	{
		TextureListView->RequestListRefresh();
	}
}

void STextureAuditSection::SetFilterText(const FString& InFilterText)
{
	FilterText = InFilterText;
	
	if (ViewModel.IsValid())
	{
		ViewModel->SetTextFilter(InFilterText);
		UpdateUIFromViewModel();
		
		// Trigger settings save
		if (OnSettingsChanged.IsBound())
		{
			OnSettingsChanged.Execute();
		}
	}
	else
	{
		ApplyFiltersAndSort();
	}
}

void STextureAuditSection::SetMinWidth(int32 InMinWidth)
{
	FilterMinWidth = InMinWidth;
	
	if (ViewModel.IsValid())
	{
		ViewModel->SetMinWidthFilter(InMinWidth);
		UpdateUIFromViewModel();
		
		// Trigger settings save
		if (OnSettingsChanged.IsBound())
		{
			OnSettingsChanged.Execute();
		}
	}
	else
	{
		ApplyFiltersAndSort();
	}
}

void STextureAuditSection::SetMinHeight(int32 InMinHeight)
{
	FilterMinHeight = InMinHeight;
	
	if (ViewModel.IsValid())
	{
		ViewModel->SetMinHeightFilter(InMinHeight);
		UpdateUIFromViewModel();
		
		// Trigger settings save
		if (OnSettingsChanged.IsBound())
		{
			OnSettingsChanged.Execute();
		}
	}
	else
	{
		ApplyFiltersAndSort();
	}
}

void STextureAuditSection::SetFormatFilter(const FString& InFormatFilter)
{
	FormatFilter = InFormatFilter;
	
	if (ViewModel.IsValid())
	{
		ViewModel->SetFormatFilter(InFormatFilter);
		UpdateUIFromViewModel();
		
		// Trigger settings save
		if (OnSettingsChanged.IsBound())
		{
			OnSettingsChanged.Execute();
		}
	}
	else
	{
		ApplyFiltersAndSort();
	}
}

void STextureAuditSection::ClearFilters()
{
	FilterText.Empty();
	FilterMinWidth = 0;
	FilterMinHeight = 0;
	FormatFilter.Empty();
	
	if (ViewModel.IsValid())
	{
		ViewModel->ClearAllFilters();
		UpdateUIFromViewModel();
		
		// Trigger settings save
		if (OnSettingsChanged.IsBound())
		{
			OnSettingsChanged.Execute();
		}
	}
	else
	{
		ApplyFiltersAndSort();
	}
}

void STextureAuditSection::SetSortColumn(ETextureSortColumn InColumn, bool bInAscending)
{
	CurrentSortColumn = InColumn;
	bSortAscending = bInAscending;
	
	if (ViewModel.IsValid())
	{
		// Convert to ViewModel sort column
		FTextureTableViewModel::ESortColumn ViewModelColumn;
		switch (InColumn)
		{
		case ETextureSortColumn::Width:
			ViewModelColumn = FTextureTableViewModel::ESortColumn::Width;
			break;
		case ETextureSortColumn::Height:
			ViewModelColumn = FTextureTableViewModel::ESortColumn::Height;
			break;
		case ETextureSortColumn::Format:
			ViewModelColumn = FTextureTableViewModel::ESortColumn::Format;
			break;
		default:
			ViewModelColumn = FTextureTableViewModel::ESortColumn::Path;
			break;
		}
		
		ViewModel->SetSortColumn(ViewModelColumn, bInAscending);
		UpdateUIFromViewModel();
		
		// Trigger settings save
		if (OnSettingsChanged.IsBound())
		{
			OnSettingsChanged.Execute();
		}
	}
	else
	{
		ApplyFiltersAndSort();
	}
}

EColumnSortMode::Type STextureAuditSection::GetSortModeForColumn(ETextureSortColumn Column) const
{
	if (CurrentSortColumn == Column)
	{
		return bSortAscending ? EColumnSortMode::Ascending : EColumnSortMode::Descending;
	}
	return EColumnSortMode::None;
}

void STextureAuditSection::ApplyFiltersAndSort()
{
	// Apply filters
	FilteredTextureRows.Empty();
	
	for (const auto& Row : AllTextureRows)
	{
		bool bPassesFilter = true;
		
		// Text filter
		if (!FilterText.IsEmpty())
		{
			FString SearchText = FilterText.ToLower();
			FString PathLower = Row->Path.ToLower();
			FString FormatLower = Row->Format.ToLower();
			
			if (!PathLower.Contains(SearchText) && !FormatLower.Contains(SearchText))
			{
				bPassesFilter = false;
			}
		}
		
		// Width filter
		if (FilterMinWidth > 0 && Row->Width < FilterMinWidth)
		{
			bPassesFilter = false;
		}
		
		// Height filter
		if (FilterMinHeight > 0 && Row->Height < FilterMinHeight)
		{
			bPassesFilter = false;
		}
		
		// Format filter
		if (!FormatFilter.IsEmpty())
		{
			FString SearchFormat = FormatFilter.ToLower();
			FString RowFormat = Row->Format.ToLower();
			
			if (!RowFormat.Contains(SearchFormat))
			{
				bPassesFilter = false;
			}
		}
		
		if (bPassesFilter)
		{
			FilteredTextureRows.Add(Row);
		}
	}
	
	// Apply sorting
	SortRows();
	
	// Update list view
	if (TextureListView.IsValid())
	{
		TextureListView->RequestListRefresh();
	}
}

void STextureAuditSection::SortRows()
{
	FilteredTextureRows.Sort([this](const FTextureAuditRowPtr& A, const FTextureAuditRowPtr& B)
	{
		if (!A.IsValid() || !B.IsValid())
		{
			return false;
		}
		
		int32 CompareResult = 0;
		
		switch (CurrentSortColumn)
		{
		case ETextureSortColumn::Path:
			CompareResult = A->Path.Compare(B->Path);
			break;
		case ETextureSortColumn::Width:
			CompareResult = A->Width - B->Width;
			break;
		case ETextureSortColumn::Height:
			CompareResult = A->Height - B->Height;
			break;
		case ETextureSortColumn::Format:
			CompareResult = A->Format.Compare(B->Format);
			break;
		}
		
		return bSortAscending ? (CompareResult < 0) : (CompareResult > 0);
	});
}

TSharedRef<class ITableRow> STextureAuditSection::OnGenerateRow(FTextureAuditRowPtr Item, const TSharedRef<class STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FTextureAuditRowPtr>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(0.4f).Padding(2,0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->Path))
			.ToolTipText(FText::FromString(Item->Path))
		]
		+ SHorizontalBox::Slot().FillWidth(0.15f).Padding(2,0).HAlign(HAlign_Right)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::FromInt(Item->Width)))
		]
		+ SHorizontalBox::Slot().FillWidth(0.15f).Padding(2,0).HAlign(HAlign_Right)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::FromInt(Item->Height)))
		]
		+ SHorizontalBox::Slot().FillWidth(0.2f).Padding(2,0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->Format))
		]
		+ SHorizontalBox::Slot().FillWidth(0.1f).Padding(2,0).HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(1,0)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Copy")))
				.OnClicked_Lambda([this, Item]()
				{
					if (OnCopyPath.IsBound())
					{
						OnCopyPath.Execute(Item->Path);
					}
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(1,0)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Open")))
				.OnClicked_Lambda([this, Item]()
				{
					if (OnOpenInContentBrowser.IsBound())
					{
						OnOpenInContentBrowser.Execute(Item->Path);
					}
					return FReply::Handled();
				})
			]
		]
	];
}

FReply STextureAuditSection::OnHeaderColumnSort(const EColumnSortPriority::Type SortPriority, const FName& ColumnId, const EColumnSortMode::Type NewSortMode)
{
	if (ColumnId == TEXT("Path"))
	{
		SetSortColumn(ETextureSortColumn::Path, NewSortMode == EColumnSortMode::Ascending);
	}
	else if (ColumnId == TEXT("Width"))
	{
		SetSortColumn(ETextureSortColumn::Width, NewSortMode == EColumnSortMode::Ascending);
	}
	else if (ColumnId == TEXT("Height"))
	{
		SetSortColumn(ETextureSortColumn::Height, NewSortMode == EColumnSortMode::Ascending);
	}
	else if (ColumnId == TEXT("Format"))
	{
		SetSortColumn(ETextureSortColumn::Format, NewSortMode == EColumnSortMode::Ascending);
	}
	
	return FReply::Handled();
}

void STextureAuditSection::OnFilterTextChanged(const FText& NewText)
{
	FilterText = NewText.ToString();
	if (OnFilterChanged.IsBound())
	{
		OnFilterChanged.Execute(FilterText);
	}
	ApplyFiltersAndSort();
}

void STextureAuditSection::OnMinWidthChanged(const FText& NewText)
{
	FilterMinWidth = FCString::Atoi(*NewText.ToString());
	if (OnMinWidthChanged.IsBound())
	{
		OnMinWidthChanged.Execute(FilterMinWidth);
	}
	ApplyFiltersAndSort();
}

void STextureAuditSection::OnMinHeightChanged(const FText& NewText)
{
	FilterMinHeight = FCString::Atoi(*NewText.ToString());
	if (OnMinHeightChanged.IsBound())
	{
		OnMinHeightChanged.Execute(FilterMinHeight);
	}
	ApplyFiltersAndSort();
}

void STextureAuditSection::OnFormatFilterChanged(const FText& NewText)
{
	FormatFilter = NewText.ToString();
	if (OnFormatFilterChanged.IsBound())
	{
		OnFormatFilterChanged.Execute(FormatFilter);
	}
	ApplyFiltersAndSort();
}

FReply STextureAuditSection::OnClearFilters()
{
	ClearFilters();
	return FReply::Handled();
}
