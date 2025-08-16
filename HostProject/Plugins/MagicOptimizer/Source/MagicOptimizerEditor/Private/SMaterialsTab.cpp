// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#include "SMaterialsTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorStyleSet.h"

void SMaterialsTab::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBox)
		.Padding(16.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Material Optimization - Coming Soon!")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
			.ColorAndOpacity(FLinearColor(0.2f, 0.6f, 1.0f))
		]
	];
}

SMaterialsTab::~SMaterialsTab()
{
	// TODO: Implement cleanup
}
