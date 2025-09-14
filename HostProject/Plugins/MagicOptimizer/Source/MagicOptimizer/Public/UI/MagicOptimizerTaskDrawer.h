#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "UI/MagicOptimizerMainPanel.h"
#include "Widgets/Views/SListView.h"

// Forward declarations
DECLARE_DELEGATE_OneParam(FOnCloseTaskDrawer, );

class MAGICOPTIMIZER_API SMagicOptimizerTaskDrawer : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerTaskDrawer) {}
    SLATE_ATTRIBUTE(bool, IsOpen)
    SLATE_ATTRIBUTE(TArray<FTaskData>, Tasks)
    SLATE_EVENT(FOnCloseTaskDrawer, OnClose)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetIsOpen(bool bInIsOpen);

private:
    // Attributes
    TSlateAttribute<bool> IsOpen;
    TSlateAttribute<TArray<FTaskData>> Tasks;
    
    // Events
    FOnCloseTaskDrawer OnClose;

    // Widget creation
    TSharedRef<SWidget> MakeTaskDrawerContent();
    TSharedRef<SWidget> MakeTasksList();
    TSharedRef<ITableRow> OnGenerateTaskRow(TSharedPtr<FTaskData> Task, const TSharedRef<STableViewBase>& OwnerTable);
    TSharedRef<SWidget> MakeTaskStatusBadge(const FString& Status);

    // Data management
    FLinearColor GetTaskStatusColor(const FString& Status) const;
    FString GetTaskDisplayStatus(const FString& Status) const;

    // Task list data
    TArray<TSharedPtr<FTaskData>> TaskListData;
    TSharedPtr<SListView<TSharedPtr<FTaskData>>> TaskListView;
};
