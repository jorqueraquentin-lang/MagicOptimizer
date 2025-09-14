#include "UI/MagicOptimizerTaskDrawer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerTaskDrawer"

void SMagicOptimizerTaskDrawer::Construct(const FArguments& InArgs)
{
    // Bind attributes and events
    IsOpen.Assign(*this, InArgs._IsOpen);
    Tasks.Assign(*this, InArgs._Tasks);
    OnClose = InArgs._OnClose;

    ChildSlot
    [
        // Modal dialog overlay - only visible when open
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("NoBorder"))
        .ColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f)) // Semi-transparent overlay
        .Padding(FMargin(0))
        .Visibility_Lambda([this]()
        {
            return IsOpen.Get() ? EVisibility::Visible : EVisibility::Collapsed;
        })
        [
            // Center the task drawer dialog
            SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            .Padding(FMargin(20))
            [
                SNew(SBox)
                .WidthOverride(500)
                .MaxDesiredHeight(600)
                [
                    MakeTaskDrawerContent()
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerTaskDrawer::MakeTaskDrawerContent()
{
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 0.95f))
    .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    .Padding(FMargin(0))
    [
        SNew(SVerticalBox)
        
        // Header
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(24, 20, 24, 16)
        [
            SNew(SVerticalBox)
            
            // Title
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 8)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("TaskDrawerTitle", "Tasks"))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 18))
                .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            ]
            
            // Description
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(STextBlock)
                .Text(LOCTEXT("TaskDrawerDesc", "Active and recent tasks"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 14))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
        ]
        
        // Task list content
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(24, 0, 24, 0)
        [
            MakeTasksList()
        ]
        
        // Footer with Close button
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(24, 16, 24, 20)
        [
            SNew(SHorizontalBox)
            
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            
            +SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                .Text(LOCTEXT("CloseButton", "Close"))
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .ContentPadding(FMargin(24, 12, 24, 12))
                .OnClicked_Lambda([this]()
                {
                    OnClose.ExecuteIfBound();
                    return FReply::Handled();
                })
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerTaskDrawer::MakeTasksList()
{
    // Update task list data from attribute
    TaskListData.Empty();
    TArray<FTaskData> CurrentTasks = Tasks.Get();
    
    for (const FTaskData& Task : CurrentTasks)
    {
        TaskListData.Add(MakeShared<FTaskData>(Task));
    }
    
    // Show "No tasks yet" message if empty
    if (TaskListData.Num() == 0)
    {
        return SNew(SBox)
        .Padding(FMargin(0, 20, 0, 20))
        [
            SNew(STextBlock)
            .Text(LOCTEXT("NoTasksMessage", "No tasks yet."))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            .Justification(ETextJustify::Center)
        ];
    }
    
    return SNew(SScrollBox)
    +SScrollBox::Slot()
    [
        SAssignNew(TaskListView, SListView<TSharedPtr<FTaskData>>)
        .ListItemsSource(&TaskListData)
        .OnGenerateRow(this, &SMagicOptimizerTaskDrawer::OnGenerateTaskRow)
        .ItemHeight(80)
        .SelectionMode(ESelectionMode::None)
    ];
}

TSharedRef<ITableRow> SMagicOptimizerTaskDrawer::OnGenerateTaskRow(TSharedPtr<FTaskData> Task, const TSharedRef<STableViewBase>& OwnerTable)
{
    if (!Task.IsValid())
    {
        return SNew(STableRow<TSharedPtr<FTaskData>>, OwnerTable);
    }
    
    return SNew(STableRow<TSharedPtr<FTaskData>>, OwnerTable)
    [
        SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
    .Padding(FMargin(12, 8, 12, 8))
    [
        SNew(SVerticalBox)
        
        // Task header with label and status badge
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 8)
        [
            SNew(SHorizontalBox)
            
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Task->Label))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 14))
                .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            ]
            
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                MakeTaskStatusBadge(Task->Status)
            ]
        ]
        
        // Progress bar
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SProgressBar)
            .Percent(Task->Progress / 100.0f)
            .BackgroundImage(FAppStyle::GetBrush("ProgressBar.Background"))
            .FillImage(FAppStyle::GetBrush("ProgressBar.Fill"))
            .MarqueeImage(FAppStyle::GetBrush("ProgressBar.Marquee"))
            .BorderPadding(FVector2D::ZeroVector)
            .Style(FAppStyle::Get(), "ProgressBar")
        ]
    ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerTaskDrawer::MakeTaskStatusBadge(const FString& Status)
{
    FLinearColor BadgeColor = GetTaskStatusColor(Status);
    FString DisplayStatus = GetTaskDisplayStatus(Status);
    
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(BadgeColor)
    .Padding(FMargin(8, 4, 8, 4))
    [
        SNew(STextBlock)
        .Text(FText::FromString(DisplayStatus))
        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
        .ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
    ];
}

// Data management
FLinearColor SMagicOptimizerTaskDrawer::GetTaskStatusColor(const FString& Status) const
{
    if (Status == TEXT("Done") || Status == TEXT("Completed"))
    {
        return FLinearColor(0.2f, 0.6f, 0.2f, 1.0f); // Green
    }
    else if (Status == TEXT("Running") || Status == TEXT("Active"))
    {
        return FLinearColor(0.2f, 0.4f, 0.8f, 1.0f); // Blue
    }
    else if (Status == TEXT("Failed") || Status == TEXT("Error"))
    {
        return FLinearColor(0.8f, 0.2f, 0.2f, 1.0f); // Red
    }
    else if (Status == TEXT("Paused"))
    {
        return FLinearColor(0.8f, 0.6f, 0.2f, 1.0f); // Orange
    }
    
    return FLinearColor(0.4f, 0.4f, 0.4f, 1.0f); // Gray (default)
}

FString SMagicOptimizerTaskDrawer::GetTaskDisplayStatus(const FString& Status) const
{
    if (Status == TEXT("Done"))
    {
        return TEXT("Done");
    }
    else if (Status == TEXT("Running"))
    {
        return TEXT("Running");
    }
    else if (Status == TEXT("Failed"))
    {
        return TEXT("Failed");
    }
    else if (Status == TEXT("Paused"))
    {
        return TEXT("Paused");
    }
    else if (Status == TEXT("Pending"))
    {
        return TEXT("Pending");
    }
    
    return Status; // Return as-is if not recognized
}

void SMagicOptimizerTaskDrawer::SetIsOpen(bool bInIsOpen)
{
    // Implementation can be left empty as visibility is handled by the lambda
}

#undef LOCTEXT_NAMESPACE
