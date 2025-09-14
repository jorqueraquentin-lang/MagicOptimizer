#include "UI/MagicOptimizerApplyView.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Layout/SSeparator.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerApplyView"

void SMagicOptimizerApplyView::Construct(const FArguments& InArgs)
{
    // Bind attributes
    Tasks = InArgs._Tasks;
    OnPause = InArgs._OnPause;

    // Initialize data
    UpdateTaskList();
    CommitMessage = GenerateCommitMessage();

    ChildSlot
    [
        SNew(SBorder)
        .Padding(FMargin(20))
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
        .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
        [
            SNew(SVerticalBox)
            
            // Header
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 20)
            [
                MakeQueueHeader()
            ]
            
            // Tasks list
            +SVerticalBox::Slot()
            .FillHeight(1.0f)
            .Padding(0, 0, 0, 20)
            [
                MakeTasksList()
            ]
            
            // Commit message section
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                MakeCommitMessageSection()
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerApplyView::MakeQueueHeader()
{
    return SNew(STextBlock)
        .Text(LOCTEXT("QueueTitle", "Queue"))
        .Font(FAppStyle::GetFontStyle("NormalFontBold"))
        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 14))
        .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f));
}

TSharedRef<SWidget> SMagicOptimizerApplyView::MakeTasksList()
{
    if (TaskList.Num() == 0)
    {
        return SNew(SBorder)
        .Padding(FMargin(20))
        .BorderImage(FAppStyle::GetBrush("NoBorder"))
        .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
        [
            SNew(STextBlock)
            .Text(LOCTEXT("NoActiveTasks", "No active tasks."))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            .Justification(ETextJustify::Center)
        ];
    }

    return SNew(SListView<TSharedPtr<FTaskData>>)
        .ListItemsSource(&TaskList)
        .OnGenerateRow(this, &SMagicOptimizerApplyView::OnGenerateTaskRow)
        .SelectionMode(ESelectionMode::None)
        .ScrollbarVisibility(EVisibility::Collapsed);
}

TSharedRef<ITableRow> SMagicOptimizerApplyView::OnGenerateTaskRow(TSharedPtr<FTaskData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    const ETaskStatus Status = GetTaskStatus(InItem->Status);
    
    return SNew(STableRow<TSharedPtr<FTaskData>>, OwnerTable)
    [
        SNew(SBorder)
        .Padding(FMargin(16, 12, 16, 12))
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
        .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
        [
            SNew(SVerticalBox)
            
            // Task name and status
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 8)
            [
                SNew(SHorizontalBox)
                
                // Task name
                +SHorizontalBox::Slot()
                .FillWidth(1.0f)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(InItem->Label))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                
                // Status badge
                +SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    MakeTaskStatusBadge(Status)
                ]
            ]
            
            // Progress bar
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SProgressBar)
                .Percent(InItem->Progress / 100.0f)
                .FillColorAndOpacity(GetStatusColor(Status))
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerApplyView::MakeTaskStatusBadge(ETaskStatus Status)
{
    FLinearColor BadgeColor = GetStatusColor(Status);
    FString StatusText = GetStatusText(Status);
    
    // Adjust badge color opacity for background
    BadgeColor.A = 0.8f;
    
    return SNew(SBorder)
    .Padding(FMargin(8, 4, 8, 4))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(BadgeColor)
    .BorderBackgroundColor(BadgeColor)
    [
        SNew(STextBlock)
        .Text(FText::FromString(StatusText))
        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
        .Font(FAppStyle::GetFontStyle("NormalFontBold"))
        .ColorAndOpacity(FLinearColor::White)
    ];
}

TSharedRef<SWidget> SMagicOptimizerApplyView::MakeCommitMessageSection()
{
    return SNew(SVerticalBox)
    
    // Separator
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 12)
    [
        SNew(SSeparator)
        .ColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
    ]
    
    // Commit message
    +SVerticalBox::Slot()
    .AutoHeight()
    [
        SNew(SHorizontalBox)
        
        // Label
        +SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 12, 0)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("CommitMessageLabel", "When done, copy this commit message:"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
        ]
        
        // Message
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(FText::FromString(CommitMessage))
            .Font(FAppStyle::GetFontStyle("MonoFont"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
            .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
        ]
    ];
}

void SMagicOptimizerApplyView::UpdateTaskList()
{
    TaskList.Empty();
    const TArray<FTaskData>& CurrentTasks = Tasks.Get();
    
    for (const FTaskData& Task : CurrentTasks)
    {
        TaskList.Add(MakeShareable(new FTaskData(Task)));
    }
    
    CommitMessage = GenerateCommitMessage();
}

FString SMagicOptimizerApplyView::GenerateCommitMessage() const
{
    // Generate a commit message based on active tasks
    // In real implementation, this would be more sophisticated
    return TEXT("MagicOptimizer apply Console Balanced on selected textures");
}

FLinearColor SMagicOptimizerApplyView::GetStatusColor(ETaskStatus Status) const
{
    switch (Status)
    {
        case ETaskStatus::Running:
            return FLinearColor(0.2f, 0.6f, 0.8f, 1.0f); // Blue
        case ETaskStatus::Done:
            return FLinearColor(0.2f, 0.7f, 0.2f, 1.0f); // Green
        case ETaskStatus::Failed:
            return FLinearColor(0.8f, 0.2f, 0.2f, 1.0f); // Red
        case ETaskStatus::Cancelled:
            return FLinearColor(0.6f, 0.6f, 0.6f, 1.0f); // Gray
        case ETaskStatus::Pending:
        default:
            return FLinearColor(0.4f, 0.4f, 0.4f, 1.0f); // Dark gray
    }
}

FString SMagicOptimizerApplyView::GetStatusText(ETaskStatus Status) const
{
    switch (Status)
    {
        case ETaskStatus::Running:
            return TEXT("Running");
        case ETaskStatus::Done:
            return TEXT("Done");
        case ETaskStatus::Failed:
            return TEXT("Failed");
        case ETaskStatus::Cancelled:
            return TEXT("Cancelled");
        case ETaskStatus::Pending:
        default:
            return TEXT("Pending");
    }
}

ETaskStatus SMagicOptimizerApplyView::GetTaskStatus(const FString& StatusString) const
{
    if (StatusString == TEXT("Running"))
        return ETaskStatus::Running;
    else if (StatusString == TEXT("Done"))
        return ETaskStatus::Done;
    else if (StatusString == TEXT("Failed"))
        return ETaskStatus::Failed;
    else if (StatusString == TEXT("Cancelled"))
        return ETaskStatus::Cancelled;
    else
        return ETaskStatus::Pending;
}

void SMagicOptimizerApplyView::RefreshTasks(const TArray<FTaskData>& NewTasks)
{
    UpdateTaskList();
}

#undef LOCTEXT_NAMESPACE
