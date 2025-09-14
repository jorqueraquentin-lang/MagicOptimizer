#include "UI/MagicOptimizerHeader.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformWrapPanel.h"
#include "Widgets/Layout/SSeparator.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerHeader"

void SMagicOptimizerHeader::Construct(const FArguments& InArgs)
{
    // Bind attributes
    CurrentPreset = InArgs._CurrentPreset;
    SelectedTargets = InArgs._SelectedTargets;
    ScopeSummary = InArgs._ScopeSummary;
    ChangeCap = InArgs._ChangeCap;
    ProgressData = InArgs._ProgressData;

    // Bind events
    OnPresetChanged = InArgs._OnPresetChanged;
    OnTargetToggled = InArgs._OnTargetToggled;
    OnScopeChanged = InArgs._OnScopeChanged;
    OnChangeCapChanged = InArgs._OnChangeCapChanged;
    OnRunScan = InArgs._OnRunScan;
    OnAutoFix = InArgs._OnAutoFix;
    OnPreview = InArgs._OnPreview;
    OnTasks = InArgs._OnTasks;

    // Initialize data
    TargetChips = {
        FTargetChip(TEXT("ps5"), TEXT("PS5"), TEXT("Gamepad2")),
        FTargetChip(TEXT("xbox"), TEXT("Xbox"), TEXT("Gamepad2")),
        FTargetChip(TEXT("pc"), TEXT("PC"), TEXT("Cpu")),
        FTargetChip(TEXT("quest"), TEXT("Quest"), TEXT("Smartphone")),
        FTargetChip(TEXT("switch"), TEXT("Switch"), TEXT("Smartphone"))
    };

    PresetOptions = {
        FPresetOption(TEXT("pc_high"), TEXT("PC High"), TEXT("High quality settings for PC")),
        FPresetOption(TEXT("console_balanced"), TEXT("Console Balanced"), TEXT("Balanced settings for consoles")),
        FPresetOption(TEXT("mobile_low"), TEXT("Mobile Low"), TEXT("Optimized settings for mobile")),
        FPresetOption(TEXT("vr_crisp"), TEXT("VR Crisp UI"), TEXT("VR-optimized with crisp UI")),
        FPresetOption(TEXT("cinematic"), TEXT("Cinematic Preview"), TEXT("Cinematic quality preview"))
    };

    ChildSlot
    [
        SNew(SBorder)
        .Padding(FMargin(16, 12, 16, 12))
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .ColorAndOpacity(FLinearColor(0.05f, 0.05f, 0.05f, 1.0f))
        .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
        [
            SNew(SVerticalBox)
            
            // Main header row
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)
                
                // Brand
                +SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    MakeBrandSection()
                ]
                
                // Preset dropdown
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(16, 0, 0, 0)
                .VAlign(VAlign_Center)
                [
                    MakePresetDropdown()
                ]
                
                // Target chips
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(16, 0, 0, 0)
                .VAlign(VAlign_Center)
                [
                    MakeTargetChips()
                ]
                
                // Scope button
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(16, 0, 0, 0)
                .VAlign(VAlign_Center)
                [
                    MakeScopeButton()
                ]
                
                // Change cap input
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(16, 0, 0, 0)
                .VAlign(VAlign_Center)
                [
                    MakeChangeCapInput()
                ]
                
                // Spacer
                +SHorizontalBox::Slot()
                .FillWidth(1.0f)
                [
                    SNullWidget::NullWidget
                ]
                
                // Action buttons
                +SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    MakeActionButtons()
                ]
            ]
            
            // Progress bar (conditional)
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 8, 0, 0)
            [
                SNew(SBox)
                .Visibility_Lambda([this]()
                {
                    return ProgressData.Get().bRunning ? EVisibility::Visible : EVisibility::Collapsed;
                })
                [
                    MakeProgressBar()
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerHeader::MakeBrandSection()
{
    return SNew(SHorizontalBox)
    
    +SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    [
        SNew(STextBlock)
        .Text(LOCTEXT("Brand", "MagicOptimizer"))
        .Font(FAppStyle::GetFontStyle("BoldFont"))
        .ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
        .ShadowOffset(FVector2D(1.0f, 1.0f))
        .ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f))
    ];
}

TSharedRef<SWidget> SMagicOptimizerHeader::MakePresetDropdown()
{
    return SNew(SComboButton)
    .ButtonContent()
    [
        SNew(SHorizontalBox)
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text_Lambda([this]()
            {
                return FText::FromString(FString::Printf(TEXT("Preset: %s"), *CurrentPreset.Get()));
            })
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(4, 0, 0, 0)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("▼")))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
        ]
    ]
    .ButtonStyle(FAppStyle::Get(), "SimpleButton")
    .ContentPadding(FMargin(12, 6))
    .ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
    .MenuContent()
    [
        SNew(SVerticalBox)
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("PresetTitle", "Select Preset"))
            .Font(FAppStyle::GetFontStyle("BoldFont"))
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SSeparator)
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SVerticalBox)
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(4)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("PC High")))
                .OnClicked_Lambda([this]()
                {
                    OnPresetSelectionChanged(TEXT("PC High"));
                    return FReply::Handled();
                })
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .ContentPadding(FMargin(8, 4))
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(4)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Console Balanced")))
                .OnClicked_Lambda([this]()
                {
                    OnPresetSelectionChanged(TEXT("Console Balanced"));
                    return FReply::Handled();
                })
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .ContentPadding(FMargin(8, 4))
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(4)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Mobile Low")))
                .OnClicked_Lambda([this]()
                {
                    OnPresetSelectionChanged(TEXT("Mobile Low"));
                    return FReply::Handled();
                })
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .ContentPadding(FMargin(8, 4))
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(4)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("VR Crisp UI")))
                .OnClicked_Lambda([this]()
                {
                    OnPresetSelectionChanged(TEXT("VR Crisp UI"));
                    return FReply::Handled();
                })
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .ContentPadding(FMargin(8, 4))
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(4)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Cinematic Preview")))
                .OnClicked_Lambda([this]()
                {
                    OnPresetSelectionChanged(TEXT("Cinematic Preview"));
                    return FReply::Handled();
                })
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .ContentPadding(FMargin(8, 4))
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerHeader::MakeTargetChips()
{
    return SNew(SBorder)
    .Padding(FMargin(8, 6, 8, 6))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f))
    [
        SNew(SUniformWrapPanel)
        .SlotPadding(FMargin(4, 0, 0, 0))
        +SUniformWrapPanel::Slot()
        [
            MakeTargetChip(TEXT("ps5"), TEXT("PS5"), TEXT("🎮"))
        ]
        +SUniformWrapPanel::Slot()
        [
            MakeTargetChip(TEXT("xbox"), TEXT("Xbox"), TEXT("🎮"))
        ]
        +SUniformWrapPanel::Slot()
        [
            MakeTargetChip(TEXT("pc"), TEXT("PC"), TEXT("💻"))
        ]
        +SUniformWrapPanel::Slot()
        [
            MakeTargetChip(TEXT("quest"), TEXT("Quest"), TEXT("📱"))
        ]
        +SUniformWrapPanel::Slot()
        [
            MakeTargetChip(TEXT("switch"), TEXT("Switch"), TEXT("📱"))
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerHeader::MakeTargetChip(const FString& ChipId, const FString& ChipLabel, const FString& Icon)
{
    return SNew(SButton)
    .OnClicked_Lambda([this, ChipId]() { OnTargetChipClicked(ChipId); return FReply::Handled(); })
    .ButtonStyle(FAppStyle::Get(), "SimpleButton")
    .ContentPadding(FMargin(8, 6))
    .ButtonColorAndOpacity_Lambda([this, ChipId]()
    {
        return SelectedTargets.Get().Contains(ChipId) ? 
            FLinearColor(0.2f, 0.2f, 0.2f, 1.0f) : 
            FLinearColor(0.08f, 0.08f, 0.08f, 1.0f);
    })
    .HAlign(HAlign_Center)
    .VAlign(VAlign_Center)
    // BorderImage handled by ButtonColorAndOpacity above
    [
        SNew(SHorizontalBox)
        +SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 4, 0)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Icon))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
            .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(FText::FromString(ChipLabel))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerHeader::MakeScopeButton()
{
    return SNew(SButton)
    .Text_Lambda([this]()
    {
        return FText::FromString(FString::Printf(TEXT("Scope: %s"), *ScopeSummary.Get()));
    })
    .OnClicked_Lambda([this]() { OnScopeButtonClicked(); return FReply::Handled(); })
    .ButtonStyle(FAppStyle::Get(), "SimpleButton")
    .ContentPadding(FMargin(8, 4))
    .ButtonColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
}

TSharedRef<SWidget> SMagicOptimizerHeader::MakeChangeCapInput()
{
    return SNew(SBorder)
    .Padding(FMargin(8, 4, 8, 4))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
    [
        SNew(SHorizontalBox)
        +SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 8, 0)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("ChangeCapLabel", "Change Cap"))
            .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [
            SNew(SEditableTextBox)
            .Text_Lambda([this]()
            {
                return FText::AsNumber(ChangeCap.Get());
            })
            .OnTextChanged_Lambda([this](const FText& NewText)
            {
                OnChangeCapTextChanged(NewText);
            })
            .MinDesiredWidth(60.0f)
            .ForegroundColor(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerHeader::MakeActionButtons()
{
    return SNew(SHorizontalBox)
    
    +SHorizontalBox::Slot()
    .AutoWidth()
    .Padding(0, 0, 8, 0)
    [
        SNew(SButton)
        .OnClicked_Lambda([this]() { OnRunScanClicked(); return FReply::Handled(); })
        .ButtonStyle(FAppStyle::Get(), "PrimaryButton")
        .ContentPadding(FMargin(12, 8))
        .ButtonColorAndOpacity(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f))
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 4, 0)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("▶")))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor::White)
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("RunScan", "Run Scan"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor::White)
            ]
        ]
    ]
    
    +SHorizontalBox::Slot()
    .AutoWidth()
    .Padding(0, 0, 8, 0)
    [
        SNew(SButton)
        .OnClicked_Lambda([this]() { OnAutoFixClicked(); return FReply::Handled(); })
        .ButtonStyle(FAppStyle::Get(), "Button")
        .ContentPadding(FMargin(12, 8))
        .ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 4, 0)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("✨")))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("AutoFix", "Auto Fix"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            ]
        ]
    ]
    
    +SHorizontalBox::Slot()
    .AutoWidth()
    .Padding(0, 0, 8, 0)
    [
        SNew(SButton)
        .OnClicked_Lambda([this]() { OnPreviewClicked(); return FReply::Handled(); })
        .ButtonStyle(FAppStyle::Get(), "Button")
        .ContentPadding(FMargin(12, 8))
        .ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 4, 0)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("👁")))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("Preview", "Preview"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            ]
        ]
    ]
    
    +SHorizontalBox::Slot()
    .AutoWidth()
    [
        SNew(SButton)
        .OnClicked_Lambda([this]() { OnTasksClicked(); return FReply::Handled(); })
        .ButtonStyle(FAppStyle::Get(), "Button")
        .ContentPadding(FMargin(12, 8))
        .ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 4, 0)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("📋")))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("Tasks", "Tasks"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerHeader::MakeProgressBar()
{
    return SNew(SHorizontalBox)
    
    +SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    .Padding(0, 0, 12, 0)
    [
        SNew(STextBlock)
        .Text_Lambda([this]()
        {
            return FText::FromString(ProgressData.Get().Message);
        })
        .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
    ]
    
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .VAlign(VAlign_Center)
    [
        SNew(SProgressBar)
        .Percent_Lambda([this]()
        {
            return ProgressData.Get().Progress / 100.0f;
        })
        .FillColorAndOpacity(FLinearColor(0.2f, 0.7f, 0.2f, 1.0f))
    ]
    
    +SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    .Padding(12, 0, 0, 0)
    [
        SNew(STextBlock)
        .Text_Lambda([this]()
        {
            return FText::AsNumber(FMath::RoundToInt(ProgressData.Get().Progress));
        })
        .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
    ];
}

// Event handlers
void SMagicOptimizerHeader::OnPresetSelectionChanged(const FString& NewPreset)
{
    OnPresetChanged.ExecuteIfBound(NewPreset);
}

void SMagicOptimizerHeader::OnTargetChipClicked(const FString& TargetId)
{
    OnTargetToggled.ExecuteIfBound(TargetId);
}

void SMagicOptimizerHeader::OnScopeButtonClicked()
{
    OnScopeChanged.ExecuteIfBound(TEXT("Open Scope Sheet"));
}

void SMagicOptimizerHeader::OnChangeCapTextChanged(const FText& NewText)
{
    int32 NewCap = FCString::Atoi(*NewText.ToString());
    OnChangeCapChanged.ExecuteIfBound(NewCap);
}

void SMagicOptimizerHeader::OnRunScanClicked()
{
    OnRunScan.ExecuteIfBound();
}

void SMagicOptimizerHeader::OnAutoFixClicked()
{
    OnAutoFix.ExecuteIfBound();
}

void SMagicOptimizerHeader::OnPreviewClicked()
{
    OnPreview.ExecuteIfBound();
}

void SMagicOptimizerHeader::OnTasksClicked()
{
    OnTasks.ExecuteIfBound();
}

#undef LOCTEXT_NAMESPACE
