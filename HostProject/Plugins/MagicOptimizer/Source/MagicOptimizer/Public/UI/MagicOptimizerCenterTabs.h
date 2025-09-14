#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Slate.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/AppStyle.h"
#include "UI/MagicOptimizerMainPanel.h"

// Forward declarations
class SMagicOptimizerAuditView;
class SMagicOptimizerBudgetsView;
class SMagicOptimizerRecommendView;
class SMagicOptimizerApplyView;
class SMagicOptimizerVerifyView;
class SMagicOptimizerReportsView;

/**
 * Center tabs widget for MagicOptimizer that contains:
 * - Tab buttons (Audit, Budgets, Recommend, Apply, Verify, Reports)
 * - Tab content switcher
 */
class MAGICOPTIMIZER_API SMagicOptimizerCenterTabs : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerCenterTabs) {}
    
    SLATE_ATTRIBUTE(int32, ActiveTabIndex)
    SLATE_ATTRIBUTE(TArray<FUITextureRow>, TextureData)
    SLATE_ATTRIBUTE(TArray<FString>, SelectedTextureIds)
    SLATE_ATTRIBUTE(FString, SearchText)
    
    SLATE_EVENT(FOnTabChanged, OnTabChanged)
    SLATE_EVENT(FOnSearchTextChanged, OnSearchTextChanged)
    SLATE_EVENT(FOnTextureSelectionChanged, OnTextureSelectionChanged)
    
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // Public interface
    void RefreshData(const TArray<FUITextureRow>& NewTextureData);

private:
    // Attributes
    TAttribute<int32> ActiveTabIndex;
    TAttribute<TArray<FUITextureRow>> TextureData;
    TAttribute<TArray<FString>> SelectedTextureIds;
    TAttribute<FString> SearchText;

    // Events
    FOnTabChanged OnTabChanged;
    FOnSearchTextChanged OnSearchTextChanged;
    FOnTextureSelectionChanged OnTextureSelectionChanged;

    // Child widgets
    TSharedPtr<SMagicOptimizerAuditView> AuditViewWidget;
    TSharedPtr<SMagicOptimizerBudgetsView> BudgetsViewWidget;
    TSharedPtr<SMagicOptimizerRecommendView> RecommendViewWidget;
    TSharedPtr<SMagicOptimizerApplyView> ApplyViewWidget;
    TSharedPtr<SMagicOptimizerVerifyView> VerifyViewWidget;
    TSharedPtr<SMagicOptimizerReportsView> ReportsViewWidget;

    // Widget creation
    TSharedRef<SWidget> MakeTabButtons();
    TSharedRef<SWidget> MakeTabContent();
    TSharedRef<SWidget> MakeTabButton(const FString& TabName, int32 TabIndex, bool bIsLast);

    // Event handlers
    void OnTabButtonClicked(int32 TabIndex);
};
