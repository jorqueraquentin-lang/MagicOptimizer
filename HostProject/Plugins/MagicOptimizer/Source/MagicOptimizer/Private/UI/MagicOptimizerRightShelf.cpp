#include "UI/MagicOptimizerRightShelf.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSeparator.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerRightShelf"

void SMagicOptimizerRightShelf::Construct(const FArguments& InArgs)
{
    // Bind attributes
    SelectedTextureIds = InArgs._SelectedTextureIds;

    // Bind events
    OnPreview = InArgs._OnPreview;

    ChildSlot
    [
        SNew(SBorder)
        .Padding(FMargin(16, 20, 16, 20))
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
        .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
        [
            SNew(SVerticalBox)
            
            // Quick fixes section
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                MakeQuickFixesSection()
            ]
            
            // Preview button
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 16, 0, 0)
            [
                MakePreviewButton()
            ]
            
            // Separator
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 16, 0, 0)
            [
                SNew(SSeparator)
            ]
            
            // Bridges section
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 16, 0, 0)
            [
                MakeBridgesSection()
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerRightShelf::MakeQuickFixesSection()
{
    return SNew(SVerticalBox)
    
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 8)
    [
        SNew(STextBlock)
        .Text(LOCTEXT("QuickFixesTitle", "Quick Fixes"))
        .Font(FAppStyle::GetFontStyle("NormalFontBold"))
        .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
    ]
    
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 8)
    [
        SNew(SButton)
        .Text(LOCTEXT("QuickFix1", "Set sRGB off for mask textures"))
        .OnClicked_Lambda([this]()
        {
            return FReply::Handled();
        })
        .ButtonStyle(FAppStyle::Get(), "SimpleButton")
        .ContentPadding(FMargin(12, 8))
        .ColorAndOpacity(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
        .HAlign(HAlign_Left)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    ]
    
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 8)
    [
        SNew(SButton)
        .Text(LOCTEXT("QuickFix2", "Convert normals to BC5"))
        .OnClicked_Lambda([this]()
        {
            return FReply::Handled();
        })
        .ButtonStyle(FAppStyle::Get(), "SimpleButton")
        .ContentPadding(FMargin(12, 8))
        .ColorAndOpacity(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
        .HAlign(HAlign_Left)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    ]
    
    +SVerticalBox::Slot()
    .AutoHeight()
    [
        SNew(SButton)
        .Text(LOCTEXT("QuickFix3", "Disable VT on non-VT assets"))
        .OnClicked_Lambda([this]()
        {
            return FReply::Handled();
        })
        .ButtonStyle(FAppStyle::Get(), "SimpleButton")
        .ContentPadding(FMargin(12, 8))
        .ColorAndOpacity(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
        .HAlign(HAlign_Left)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    ];
}

TSharedRef<SWidget> SMagicOptimizerRightShelf::MakePreviewButton()
{
    return SNew(SButton)
    .Text(LOCTEXT("PreviewPlan", "Preview Plan"))
    .OnClicked_Lambda([this]()
    {
        OnPreview.ExecuteIfBound();
        return FReply::Handled();
    })
    .ButtonStyle(FAppStyle::Get(), "PrimaryButton")
    .ContentPadding(FMargin(16, 12))
    .ColorAndOpacity(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f))
    .HAlign(HAlign_Center)
    .VAlign(VAlign_Center)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"));
}

TSharedRef<SWidget> SMagicOptimizerRightShelf::MakeBridgesSection()
{
    return SNew(SVerticalBox)
    
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 8)
    [
        SNew(STextBlock)
        .Text(LOCTEXT("BridgesTitle", "Bridges"))
        .Font(FAppStyle::GetFontStyle("NormalFontBold"))
        .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
    ]
    
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 4)
    [
        SNew(SButton)
        .Text(LOCTEXT("OpenAssetAudit", "Open in Asset Audit"))
        .OnClicked_Lambda([this]()
        {
            return FReply::Handled();
        })
        .ButtonStyle(FAppStyle::Get(), "SimpleButton")
        .ContentPadding(FMargin(8, 4))
        .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
        .HAlign(HAlign_Left)
    ]
    
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 4)
    [
        SNew(SButton)
        .Text(LOCTEXT("OpenSizeMap", "Open Size Map"))
        .OnClicked_Lambda([this]()
        {
            return FReply::Handled();
        })
        .ButtonStyle(FAppStyle::Get(), "SimpleButton")
        .ContentPadding(FMargin(8, 4))
        .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
        .HAlign(HAlign_Left)
    ]
    
    +SVerticalBox::Slot()
    .AutoHeight()
    [
        SNew(SButton)
        .Text(LOCTEXT("StartTrace", "Start 30s Insights Trace"))
        .OnClicked_Lambda([this]()
        {
            return FReply::Handled();
        })
        .ButtonStyle(FAppStyle::Get(), "SimpleButton")
        .ContentPadding(FMargin(8, 4))
        .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
        .HAlign(HAlign_Left)
    ];
}

void SMagicOptimizerRightShelf::RefreshSelection(const TArray<FString>& NewSelection)
{
    // Update UI based on new selection
}

#undef LOCTEXT_NAMESPACE
