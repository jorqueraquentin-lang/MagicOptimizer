#include "UI/MagicOptimizerCenterTabs.h"
#include "UI/MagicOptimizerAuditView.h"
#include "UI/MagicOptimizerBudgetsView.h"
#include "UI/MagicOptimizerRecommendView.h"
#include "UI/MagicOptimizerApplyView.h"
#include "UI/MagicOptimizerVerifyView.h"
#include "UI/MagicOptimizerReportsView.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerCenterTabs"

void SMagicOptimizerCenterTabs::Construct(const FArguments& InArgs)
{
    // Bind attributes
    ActiveTabIndex = InArgs._ActiveTabIndex;
    TextureData = InArgs._TextureData;
    SelectedTextureIds = InArgs._SelectedTextureIds;
    SearchText = InArgs._SearchText;

    // Bind events
    OnTabChanged = InArgs._OnTabChanged;
    OnSearchTextChanged = InArgs._OnSearchTextChanged;
    OnTextureSelectionChanged = InArgs._OnTextureSelectionChanged;

    // Create child widgets
    AuditViewWidget = SNew(SMagicOptimizerAuditView)
        .TextureData(TextureData)
        .SelectedTextureIds(SelectedTextureIds)
        .SearchText(SearchText)
        .OnSearchTextChanged(OnSearchTextChanged)
        .OnTextureSelectionChanged(OnTextureSelectionChanged);

    BudgetsViewWidget = SNew(SMagicOptimizerBudgetsView)
        .TextureData(TextureData);

    RecommendViewWidget = SNew(SMagicOptimizerRecommendView)
        .TextureData(TextureData)
        .SelectedTextureIds(SelectedTextureIds);

    ApplyViewWidget = SNew(SMagicOptimizerApplyView);

    VerifyViewWidget = SNew(SMagicOptimizerVerifyView);

    ReportsViewWidget = SNew(SMagicOptimizerReportsView);

    ChildSlot
    [
        SNew(SVerticalBox)
        
        // Tab buttons
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            MakeTabButtons()
        ]
        
        // Tab content
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(0, 8, 0, 0)
        [
            MakeTabContent()
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerCenterTabs::MakeTabButtons()
{
    return SNew(SBorder)
    .Padding(FMargin(0))
    .BorderImage(FAppStyle::GetBrush("NoBorder"))
    .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 0.6f)) // Semi-transparent for backdrop blur effect
    .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 0.8f))
    [
        SNew(SHorizontalBox)
        
        +SHorizontalBox::Slot()
        .AutoWidth()
        [
            MakeTabButton(TEXT("Audit"), 0, false)
        ]
        
        +SHorizontalBox::Slot()
        .AutoWidth()
        [
            MakeTabButton(TEXT("Budgets"), 1, false)
        ]
        
        +SHorizontalBox::Slot()
        .AutoWidth()
        [
            MakeTabButton(TEXT("Recommend"), 2, false)
        ]
        
        +SHorizontalBox::Slot()
        .AutoWidth()
        [
            MakeTabButton(TEXT("Apply"), 3, false)
        ]
        
        +SHorizontalBox::Slot()
        .AutoWidth()
        [
            MakeTabButton(TEXT("Verify"), 4, false)
        ]
        
        +SHorizontalBox::Slot()
        .AutoWidth()
        [
            MakeTabButton(TEXT("Reports"), 5, false)
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerCenterTabs::MakeTabButton(const FString& TabName, int32 TabIndex, bool bIsLast)
{
    return SNew(SButton)
    .Text(FText::FromString(TabName))
    .OnClicked_Lambda([this, TabIndex]()
    {
        OnTabButtonClicked(TabIndex);
        return FReply::Handled();
    })
    .ButtonStyle(FAppStyle::Get(), "SimpleButton")
    .ContentPadding(FMargin(20, 12, 20, 12))
    .ForegroundColor_Lambda([this, TabIndex]()
    {
        return ActiveTabIndex.Get() == TabIndex ? 
            FLinearColor(0.9f, 0.9f, 0.9f, 1.0f) : 
            FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
    })
    .HAlign(HAlign_Center)
    .VAlign(VAlign_Center)
    .ButtonColorAndOpacity_Lambda([this, TabIndex]()
    {
        return ActiveTabIndex.Get() == TabIndex ? 
            FLinearColor(0.15f, 0.15f, 0.15f, 0.8f) : 
            FLinearColor(0.08f, 0.08f, 0.08f, 0.6f);
    })
    // Static text styling - dynamic colors handled by ForegroundColor_Lambda above
    [
        SNew(STextBlock)
        .Text(FText::FromString(TEXT("Tab")))
        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 13))
        .Justification(ETextJustify::Center)
        // VAlign not needed for STextBlock in UE5.6
    ];
}

TSharedRef<SWidget> SMagicOptimizerCenterTabs::MakeTabContent()
{
    return SNew(SWidgetSwitcher)
    .WidgetIndex_Lambda([this]()
    {
        return ActiveTabIndex.Get();
    })
    
    +SWidgetSwitcher::Slot()
    [
        AuditViewWidget.ToSharedRef()
    ]
    
    +SWidgetSwitcher::Slot()
    [
        BudgetsViewWidget.ToSharedRef()
    ]
    
    +SWidgetSwitcher::Slot()
    [
        RecommendViewWidget.ToSharedRef()
    ]
    
    +SWidgetSwitcher::Slot()
    [
        ApplyViewWidget.ToSharedRef()
    ]
    
    +SWidgetSwitcher::Slot()
    [
        VerifyViewWidget.ToSharedRef()
    ]
    
    +SWidgetSwitcher::Slot()
    [
        ReportsViewWidget.ToSharedRef()
    ];
}

void SMagicOptimizerCenterTabs::OnTabButtonClicked(int32 TabIndex)
{
    OnTabChanged.ExecuteIfBound(TabIndex);
}

void SMagicOptimizerCenterTabs::RefreshData(const TArray<FUITextureRow>& NewTextureData)
{
    if (AuditViewWidget.IsValid())
    {
        AuditViewWidget->RefreshData(NewTextureData);
    }
    
    if (BudgetsViewWidget.IsValid())
    {
        BudgetsViewWidget->RefreshData(NewTextureData);
    }
    
    if (RecommendViewWidget.IsValid())
    {
        RecommendViewWidget->RefreshData(NewTextureData);
    }
}

#undef LOCTEXT_NAMESPACE
