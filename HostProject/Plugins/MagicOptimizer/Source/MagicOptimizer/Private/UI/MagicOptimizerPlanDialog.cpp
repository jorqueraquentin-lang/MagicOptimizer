#include "UI/MagicOptimizerPlanDialog.h"
#include "UI/UIDataTypes.h"
#include "Slate.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerPlanDialog"

void SMagicOptimizerPlanDialog::Construct(const FArguments& InArgs)
{
    // Bind attributes and events
    IsOpen.Assign(*this, InArgs._IsOpen);
    ProposedChanges.Assign(*this, InArgs._ProposedChanges);
    ChangeCap.Assign(*this, InArgs._ChangeCap);
    OnClose = InArgs._OnClose;
    OnApply = InArgs._OnApply;

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
            // Center the plan dialog
            SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            .Padding(FMargin(20))
            [
                SNew(SBox)
                .WidthOverride(800)
                .MaxDesiredHeight(600)
                [
                    MakePlanDialogContent()
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerPlanDialog::MakePlanDialogContent()
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
                .Text(LOCTEXT("PlanDialogTitle", "Preview Plan"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 18))
                .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            ]
            
            // Description
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(STextBlock)
                .Text(LOCTEXT("PlanDialogDesc", "Review what would change before applying."))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 14))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
        ]
        
        // Changes list content
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(24, 0, 24, 0)
        [
            SNew(SVerticalBox)
            
            // Changes list
            +SVerticalBox::Slot()
            .FillHeight(1.0f)
            .Padding(0, 0, 0, 16)
            [
                MakeChangesList()
            ]
            
            // Separator
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 16)
            [
                SNew(SSeparator)
                .Orientation(Orient_Horizontal)
                .ColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
            ]
            
            // Summary section
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                MakeSummarySection()
            ]
        ]
        
        // Footer with actions
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(24, 16, 24, 20)
        [
            MakeActionsSection()
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerPlanDialog::MakeChangesList()
{
    // Update change list data from attribute
    ChangeListData.Empty();
    TArray<FPlanChangeItem> CurrentChanges = ProposedChanges.Get();
    
    for (const FPlanChangeItem& Change : CurrentChanges)
    {
        ChangeListData.Add(MakeShared<FPlanChangeItem>(Change));
    }
    
    // Show empty message if no changes
    if (ChangeListData.Num() == 0)
    {
        return SNew(SBox)
        .Padding(FMargin(0, 20, 0, 20))
        [
            SNew(STextBlock)
            .Text(LOCTEXT("NoChangesMessage", "No changes to preview."))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFont").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            .Justification(ETextJustify::Center)
        ];
    }
    
    return SNew(SScrollBox)
    +SScrollBox::Slot()
    [
        SAssignNew(ChangeListView, SListView<TSharedPtr<FPlanChangeItem>>)
        .ListItemsSource(&ChangeListData)
        .OnGenerateRow(this, &SMagicOptimizerPlanDialog::OnGenerateChangeRow)
        .ItemHeight(60)
        .SelectionMode(ESelectionMode::None)
    ];
}

TSharedRef<SWidget> SMagicOptimizerPlanDialog::OnGenerateChangeRow(TSharedPtr<FPlanChangeItem> Change)
{
    if (!Change.IsValid())
    {
        return SNullWidget::NullWidget;
    }
    
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
    .Padding(FMargin(12, 8, 12, 8))
    [
        SNew(SVerticalBox)
        
        // Change label
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 4)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Change->Label))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        // Sample paths
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(STextBlock)
            .Text(FText::FromString(FString::Printf(TEXT("Sample: %s"), *GetSamplePathsText(Change->SamplePaths))))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            .WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerPlanDialog::MakeSummarySection()
{
    return SNew(SHorizontalBox)
    
    // Total changes text
    +SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    .Padding(0, 0, 8, 0)
    [
        SNew(STextBlock)
        .Text(LOCTEXT("TotalChangesLabel", "Total changes"))
        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFont").FontObject, 14))
        .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
    ]
    
    // Total changes badge
    +SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    .Padding(0, 0, 16, 0)
    [
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .ColorAndOpacity_Lambda([this]() -> FLinearColor
        {
            return IsPlanOverCap() ? 
                FLinearColor(0.8f, 0.2f, 0.2f, 1.0f) : // Red if over cap
                FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);   // Gray if under cap
        })
        .Padding(FMargin(8, 4, 8, 4))
        [
            SNew(STextBlock)
            .Text_Lambda([this]()
            {
                return FText::AsNumber(GetTotalChanges());
            })
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
            .ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
        ]
    ]
    
    // Cap text
    +SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    .Padding(0, 0, 8, 0)
    [
        SNew(STextBlock)
        .Text(LOCTEXT("CapLabel", "Cap"))
        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFont").FontObject, 14))
        .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
    ]
    
    // Cap badge
    +SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    .Padding(0, 0, 16, 0)
    [
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .ColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
        .Padding(FMargin(8, 4, 8, 4))
        [
            SNew(STextBlock)
            .Text_Lambda([this]()
            {
                return FText::AsNumber(ChangeCap.Get());
            })
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
    ]
    
    // Warning text for over cap
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .VAlign(VAlign_Center)
    .Padding(16, 0, 0, 0)
    [
        SNew(STextBlock)
        .Text_Lambda([this]()
        {
            return IsPlanOverCap() ? 
                LOCTEXT("PlanOverCapWarning", "Plan exceeds cap. Confirmation required at apply.") : 
                FText::GetEmpty();
        })
        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
        .ColorAndOpacity(FLinearColor(0.8f, 0.2f, 0.2f, 1.0f))
        .Visibility_Lambda([this]()
        {
            return IsPlanOverCap() ? EVisibility::Visible : EVisibility::Collapsed;
        })
    ];
}

TSharedRef<SWidget> SMagicOptimizerPlanDialog::MakeActionsSection()
{
    return SNew(SHorizontalBox)
    
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    
    // Apply button
    +SHorizontalBox::Slot()
    .AutoWidth()
    .Padding(0, 0, 8, 0)
    [
        SNew(SButton)
        .Text(LOCTEXT("ApplyButton", "Apply"))
        .ButtonStyle(FAppStyle::Get(), "PrimaryButton")
        .ContentPadding(FMargin(24, 12, 24, 12))
        .ColorAndOpacity(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f))
        .OnClicked_Lambda([this]()
        {
            TArray<FProposedChange> Changes;
            // Convert plan changes to proposed changes for the delegate
            // Note: This would need proper conversion logic in a real implementation
            OnApply.ExecuteIfBound(Changes);
            OnClose.ExecuteIfBound();
            return FReply::Handled();
        })
    ]
    
    // Close button
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
    ];
}

// Data management
int32 SMagicOptimizerPlanDialog::GetTotalChanges() const
{
    int32 Total = 0;
    TArray<FPlanChangeItem> Changes = ProposedChanges.Get();
    
    for (const FPlanChangeItem& Change : Changes)
    {
        Total += Change.TotalCount;
    }
    
    return Total;
}

bool SMagicOptimizerPlanDialog::IsPlanOverCap() const
{
    return GetTotalChanges() > ChangeCap.Get();
}

FString SMagicOptimizerPlanDialog::GetSamplePathsText(const TArray<FString>& SamplePaths) const
{
    if (SamplePaths.Num() == 0)
    {
        return TEXT("(no samples)");
    }
    
    if (SamplePaths.Num() <= 3)
    {
        return FString::Join(SamplePaths, TEXT(", "));
    }
    
    // Take first 3 and add ellipsis
    TArray<FString> FirstThree = SamplePaths;
    FirstThree.SetNum(3);
    return FString::Join(FirstThree, TEXT(", ")) + TEXT(" …");
}

void SMagicOptimizerPlanDialog::SetIsOpen(bool bInIsOpen)
{
    // Implementation can be left empty as visibility is handled by the lambda
}

#undef LOCTEXT_NAMESPACE
