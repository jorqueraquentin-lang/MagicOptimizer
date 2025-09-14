#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Slate.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSeparator.h"
#include "Styling/AppStyle.h"
#include "UI/MagicOptimizerMainPanel.h"

/**
 * Right shelf widget for MagicOptimizer that contains:
 * - Quick fixes
 * - Preview plan button
 * - Bridges to other tools
 */
class MAGICOPTIMIZER_API SMagicOptimizerRightShelf : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerRightShelf) {}
    
    SLATE_ATTRIBUTE(TArray<FString>, SelectedTextureIds)
    
    SLATE_EVENT(FOnPreview, OnPreview)
    
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // Public interface
    void RefreshSelection(const TArray<FString>& NewSelection);

private:
    // Attributes
    TAttribute<TArray<FString>> SelectedTextureIds;

    // Events
    FOnPreview OnPreview;

    // Widget creation
    TSharedRef<SWidget> MakeQuickFixesSection();
    TSharedRef<SWidget> MakeBridgesSection();
    TSharedRef<SWidget> MakePreviewButton();
};
