// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SHeaderRow.h"
#include "ViewModels/TextureModels.h"

class SListViewBase;
class SHeaderRow;

class STextureRecommendSection : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STextureRecommendSection) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// Data interface
	void SetRecommendationRows(const TArray<FTextureRecRowPtr>& InRows);
	void RefreshDisplay();

	// Event delegates
	DECLARE_DELEGATE_OneParam(FOnRowAction, const FString&);

	FOnRowAction OnCopyPath;
	FOnRowAction OnOpenInContentBrowser;

protected:
	// Internal data
	TArray<FTextureRecRowPtr> RecommendationRows;
	
	// UI components
	TSharedPtr<SListView<FTextureRecRowPtr>> RecommendationListView;
	TSharedPtr<SHeaderRow> RecommendationHeaderRow;

	// Internal methods
	TSharedRef<class ITableRow> OnGenerateRow(FTextureRecRowPtr Item, const TSharedRef<class STableViewBase>& OwnerTable);
};
