// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#include "STextureRecommendSection.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SVerticalBox.h"
#include "Widgets/Layout/SHorizontalBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "CoreStyle.h"

void STextureRecommendSection::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SExpandableArea)
		.AreaTitle(FText::FromString(TEXT("Texture Recommendations")))
		.InitiallyCollapsed(false)
		.BodyContent()
		[
			SAssignNew(RecommendationListView, SListView<FTextureRecRowPtr>)
			.ItemHeight(20)
			.ListItemsSource(&RecommendationRows)
			.OnGenerateRow(this, &STextureRecommendSection::OnGenerateRow)
			.HeaderRow(
				SAssignNew(RecommendationHeaderRow, SHeaderRow)
				+ SHeaderRow::Column(FName(TEXT("Path"))).DefaultLabel(FText::FromString(TEXT("Path"))).HAlignCell(HAlign_Left)
				+ SHeaderRow::Column(FName(TEXT("Width"))).DefaultLabel(FText::FromString(TEXT("Width"))).HAlignCell(HAlign_Right)
				+ SHeaderRow::Column(FName(TEXT("Height"))).DefaultLabel(FText::FromString(TEXT("Height"))).HAlignCell(HAlign_Right)
				+ SHeaderRow::Column(FName(TEXT("Format"))).DefaultLabel(FText::FromString(TEXT("Format"))).HAlignCell(HAlign_Left)
				+ SHeaderRow::Column(FName(TEXT("Issues"))).DefaultLabel(FText::FromString(TEXT("Issues"))).HAlignCell(HAlign_Left)
				+ SHeaderRow::Column(FName(TEXT("Recommendations"))).DefaultLabel(FText::FromString(TEXT("Recommendations"))).HAlignCell(HAlign_Left)
				+ SHeaderRow::Column(FName(TEXT("Actions"))).DefaultLabel(FText::FromString(TEXT("Actions"))).HAlignCell(HAlign_Center)
			)
		]
	];
}

void STextureRecommendSection::SetRecommendationRows(const TArray<FTextureRecRowPtr>& InRows)
{
	RecommendationRows = InRows;
	RefreshDisplay();
}

void STextureRecommendSection::RefreshDisplay()
{
	if (RecommendationListView.IsValid())
	{
		RecommendationListView->RequestListRefresh();
	}
}

TSharedRef<class ITableRow> STextureRecommendSection::OnGenerateRow(FTextureRecRowPtr Item, const TSharedRef<class STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FTextureRecRowPtr>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(0.25f).Padding(2,0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->Path))
			.ToolTipText(FText::FromString(Item->Path))
		]
		+ SHorizontalBox::Slot().FillWidth(0.1f).Padding(2,0).HAlign(HAlign_Right)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::FromInt(Item->Width)))
		]
		+ SHorizontalBox::Slot().FillWidth(0.1f).Padding(2,0).HAlign(HAlign_Right)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::FromInt(Item->Height)))
		]
		+ SHorizontalBox::Slot().FillWidth(0.1f).Padding(2,0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->Format))
		]
		+ SHorizontalBox::Slot().FillWidth(0.2f).Padding(2,0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->Issues))
			.ToolTipText(FText::FromString(Item->Issues))
		]
		+ SHorizontalBox::Slot().FillWidth(0.15f).Padding(2,0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->Recommendations))
			.ToolTipText(FText::FromString(Item->Recommendations))
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
