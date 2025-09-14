#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "UI/MagicOptimizerMainPanel.h"

/**
 * Task status enumeration
 */
UENUM(BlueprintType)
enum class ETaskStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    Running     UMETA(DisplayName = "Running"),
    Done        UMETA(DisplayName = "Done"),
    Failed      UMETA(DisplayName = "Failed"),
    Cancelled   UMETA(DisplayName = "Cancelled")
};

/**
 * Apply view widget for MagicOptimizer that contains:
 * - Task queue with real-time progress tracking
 * - Status visualization with progress bars
 * - Commit message generation
 */
class MAGICOPTIMIZER_API SMagicOptimizerApplyView : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerApplyView) {}
    
    SLATE_ATTRIBUTE(TArray<FTaskData>, Tasks)
    SLATE_EVENT(FSimpleDelegate, OnPause)
    
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void RefreshTasks(const TArray<FTaskData>& NewTasks);

private:
    // Attributes
    TAttribute<TArray<FTaskData>> Tasks;
    FSimpleDelegate OnPause;

    // Data
    TArray<TSharedPtr<FTaskData>> TaskList;
    FString CommitMessage;

    // Widget creation
    TSharedRef<SWidget> MakeQueueHeader();
    TSharedRef<SWidget> MakeTasksList();
    TSharedRef<ITableRow> OnGenerateTaskRow(TSharedPtr<FTaskData> InItem, const TSharedRef<STableViewBase>& OwnerTable);
    TSharedRef<SWidget> MakeTaskStatusBadge(ETaskStatus Status);
    TSharedRef<SWidget> MakeCommitMessageSection();

    // Data processing
    void UpdateTaskList();
    FString GenerateCommitMessage() const;
    FLinearColor GetStatusColor(ETaskStatus Status) const;
    FString GetStatusText(ETaskStatus Status) const;
    
    // Convert FTaskData status string to enum
    ETaskStatus GetTaskStatus(const FString& StatusString) const;
};
