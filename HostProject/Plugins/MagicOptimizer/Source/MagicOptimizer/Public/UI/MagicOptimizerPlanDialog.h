#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "UI/UIDataTypes.h"

// Forward declarations - these are now defined in UIDataTypes.h

class MAGICOPTIMIZER_API SMagicOptimizerPlanDialog : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerPlanDialog) {}
    SLATE_ATTRIBUTE(bool, IsOpen)
    SLATE_ATTRIBUTE(TArray<FPlanChangeItem>, ProposedChanges)
    SLATE_ATTRIBUTE(int32, ChangeCap)
    SLATE_EVENT(FOnClosePlanDialog, OnClose)
    SLATE_EVENT(FOnApplyPlan, OnApply)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetIsOpen(bool bInIsOpen);

private:
    // Attributes
    TSlateAttribute<bool> IsOpen;
    TSlateAttribute<TArray<FPlanChangeItem>> ProposedChanges;
    TSlateAttribute<int32> ChangeCap;
    
    // Events
    FOnClosePlanDialog OnClose;
    FOnApplyPlan OnApply;

    // Widget creation
    TSharedRef<SWidget> MakePlanDialogContent();
    TSharedRef<SWidget> MakeChangesList();
    TSharedRef<SWidget> OnGenerateChangeRow(TSharedPtr<FPlanChangeItem> Change);
    TSharedRef<SWidget> MakeSummarySection();
    TSharedRef<SWidget> MakeActionsSection();

    // Data management
    int32 GetTotalChanges() const;
    bool IsPlanOverCap() const;
    FString GetSamplePathsText(const TArray<FString>& SamplePaths) const;

    // Change list data
    TArray<TSharedPtr<FPlanChangeItem>> ChangeListData;
    TSharedPtr<SListView<TSharedPtr<FPlanChangeItem>>> ChangeListView;
};
