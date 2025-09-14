#include "UI/MagicOptimizerSlideOver.h"
#include "UI/UIDataTypes.h"
#include "Slate.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/AppStyle.h"
#include "Animation/CurveHandle.h"
#include "Animation/CurveSequence.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerSlideOver"

void SMagicOptimizerSlideOver::Construct(const FArguments& InArgs)
{
    // Bind attributes and events
    IsOpen.Assign(*this, InArgs._IsOpen);
    SelectedTexture.Assign(*this, InArgs._SelectedTexture);
    TargetPlatforms.Assign(*this, InArgs._TargetPlatforms);
    OnClose = InArgs._OnClose;
    OnApplyToAsset = InArgs._OnApplyToAsset;

    // Initialize compression options
    CompressionOptions.Empty();
    CompressionOptions.Add(MakeShared<FString>(TEXT("Default")));
    CompressionOptions.Add(MakeShared<FString>(TEXT("Masks")));
    CompressionOptions.Add(MakeShared<FString>(TEXT("UserInterface")));
    CompressionOptions.Add(MakeShared<FString>(TEXT("Normal")));
    CompressionOptions.Add(MakeShared<FString>(TEXT("HDR")));
    CompressionOptions.Add(MakeShared<FString>(TEXT("BC5")));
    CompressionOptions.Add(MakeShared<FString>(TEXT("BC7")));

    ChildSlot
    [
        // Slide-over panel - slides in from right
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("NoBorder"))
        .Visibility_Lambda([this]()
        {
            return (IsOpen.Get() && SelectedTexture.Get().Id.Len() > 0) ? 
                EVisibility::Visible : EVisibility::Collapsed;
        })
        .HAlign(HAlign_Right)
        .VAlign(VAlign_Fill)
        [
            SNew(SBox)
            .WidthOverride(460)
            [
                MakeSlideOverContent()
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerSlideOver::MakeSlideOverContent()
{
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 0.95f))
    .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
    .Padding(FMargin(0))
    [
        SNew(SVerticalBox)
        
        // Header
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            MakeHeader()
        ]
        
        // Scrollable content
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SNew(SScrollBox)
            +SScrollBox::Slot()
            .Padding(12, 0, 12, 12)
            [
                SNew(SVerticalBox)
                
                // Before/After comparison
                +SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 0, 0, 12)
                [
                    MakeBeforeAfterSection()
                ]
                
                // Controls
                +SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 0, 0, 12)
                [
                    MakeControlsSection()
                ]
                
                // Cooked formats
                +SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 0, 0, 12)
                [
                    MakeCookedFormatsSection()
                ]
                
                // Estimates
                +SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 0, 0, 12)
                [
                    MakeEstimatesSection()
                ]
                
                // Actions
                +SVerticalBox::Slot()
                .AutoHeight()
                [
                    MakeActionsSection()
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerSlideOver::MakeHeader()
{
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
    .Padding(FMargin(12, 8, 8, 8))
    [
        SNew(SHorizontalBox)
        
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text_Lambda([this]()
            {
                const FUITextureRow& Texture = SelectedTexture.Get();
                return FText::FromString(Texture.Path);
            })
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 13))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            .WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
        ]
        
        +SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [
            SNew(SButton)
            .Text(LOCTEXT("CloseButton", "✕"))
            .ButtonStyle(FAppStyle::Get(), "SimpleButton")
            .ContentPadding(FMargin(8, 4, 8, 4))
            .OnClicked_Lambda([this]()
            {
                OnClose.ExecuteIfBound();
                return FReply::Handled();
            })
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerSlideOver::MakeBeforeAfterSection()
{
    return SNew(SHorizontalBox)
    
    // Before section
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .Padding(0, 0, 6, 0)
    [
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
        .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
        .Padding(FMargin(8))
        [
            SNew(SVerticalBox)
            
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 4)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("BeforeLabel", "Before"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
            
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SVerticalBox)
                +SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text_Lambda([this]()
                    {
                        const FUITextureRow& Texture = SelectedTexture.Get();
                        return FText::FromString(FString::Printf(TEXT("Compression %s"), *Texture.Compression));
                    })
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text_Lambda([this]()
                    {
                        const FUITextureRow& Texture = SelectedTexture.Get();
                        return FText::FromString(FString::Printf(TEXT("sRGB %s"), Texture.bSRGB ? TEXT("On") : TEXT("Off")));
                    })
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text_Lambda([this]()
                    {
                        const FUITextureRow& Texture = SelectedTexture.Get();
                        return FText::FromString(FString::Printf(TEXT("Mips %s"), Texture.bMips ? TEXT("Yes") : TEXT("No")));
                    })
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
            ]
        ]
    ]
    
    // After section (What-if)
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .Padding(6, 0, 0, 0)
    [
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
        .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
        .Padding(FMargin(8))
        [
            SNew(SVerticalBox)
            
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 4)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("AfterLabel", "After (What-if)"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
            
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SVerticalBox)
                +SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text_Lambda([this]()
                    {
                        return FText::FromString(FString::Printf(TEXT("Compression %s"), *CurrentSettings.Compression));
                    })
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text_Lambda([this]()
                    {
                        return FText::FromString(FString::Printf(TEXT("MaxTextureSize %d"), CurrentSettings.MaxTextureSize));
                    })
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text_Lambda([this]()
                    {
                        return FText::FromString(FString::Printf(TEXT("LODBias %d"), CurrentSettings.LODBias));
                    })
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerSlideOver::MakeControlsSection()
{
    return SNew(SVerticalBox)
    
    // Separator
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 12)
    [
        SNew(SSeparator)
        .Orientation(Orient_Horizontal)
        .ColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
    ]
    
    // Controls grid
    +SVerticalBox::Slot()
    .AutoHeight()
    [
        SNew(SHorizontalBox)
        
        // MaxTextureSize
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .Padding(0, 0, 8, 0)
        [
            SNew(SVerticalBox)
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 4)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("MaxTextureSizeLabel", "MaxTextureSize"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
                .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                SAssignNew(MaxTextureSizeTextBox, SEditableTextBox)
                .Text_Lambda([this]()
                {
                    return FText::AsNumber(CurrentSettings.MaxTextureSize);
                })
                .OnTextChanged(this, &SMagicOptimizerSlideOver::OnMaxTextureSizeChanged)
                .Font(FAppStyle::GetFontStyle("NormalFont"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
            ]
        ]
        
        // LODBias
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .Padding(8, 0, 8, 0)
        [
            SNew(SVerticalBox)
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 4)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("LODBiasLabel", "LODBias"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
                .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                SAssignNew(LODBiasTextBox, SEditableTextBox)
                .Text_Lambda([this]()
                {
                    return FText::AsNumber(CurrentSettings.LODBias);
                })
                .OnTextChanged(this, &SMagicOptimizerSlideOver::OnLODBiasChanged)
                .Font(FAppStyle::GetFontStyle("NormalFont"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
            ]
        ]
        
        // Compression
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .Padding(8, 0, 0, 0)
        [
            SNew(SVerticalBox)
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 4)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("CompressionLabel", "Compression"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
                .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                SAssignNew(CompressionComboBox, SComboBox<TSharedPtr<FString>>)
                .OptionsSource(&CompressionOptions)
                .OnGenerateWidget_Lambda([](TSharedPtr<FString> Option)
                {
                    return SNew(STextBlock).Text(FText::FromString(*Option));
                })
                .OnSelectionChanged(this, &SMagicOptimizerSlideOver::OnCompressionChanged)
                [
                    SNew(STextBlock)
                    .Text_Lambda([this]()
                    {
                        return FText::FromString(CurrentSettings.Compression);
                    })
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerSlideOver::MakeCookedFormatsSection()
{
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
    .Padding(FMargin(8))
    [
        SNew(SVerticalBox)
        
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 4)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("CookedFormatsTitle", "Cooked formats by target"))
            .Font(FAppStyle::GetFontStyle("NormalFontBold"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(STextBlock)
            .Text(LOCTEXT("CookedFormatsPlaceholder", "Cooked formats will be displayed here"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerSlideOver::MakeEstimatesSection()
{
    return SNew(STextBlock)
    .Text_Lambda([this]()
    {
        return FText::FromString(GetEstimatedSavings());
    })
    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
    .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f));
}

TSharedRef<SWidget> SMagicOptimizerSlideOver::MakeActionsSection()
{
    return SNew(SHorizontalBox)
    
    +SHorizontalBox::Slot()
    .AutoWidth()
    .Padding(0, 0, 8, 0)
    [
        SNew(SButton)
        .Text(LOCTEXT("ApplyToAssetButton", "🪄 Apply to this asset"))
        .ButtonStyle(FAppStyle::Get(), "PrimaryButton")
        .ContentPadding(FMargin(12, 6, 12, 6))
        .OnClicked_Lambda([this]()
        {
            OnApplyToThisAsset();
            return FReply::Handled();
        })
    ]
    
    +SHorizontalBox::Slot()
    .AutoWidth()
    [
        SNew(SButton)
        .Text(LOCTEXT("CloseButton", "Close"))
        .ButtonStyle(FAppStyle::Get(), "SimpleButton")
        .ContentPadding(FMargin(12, 6, 12, 6))
        .OnClicked_Lambda([this]()
        {
            OnClose.ExecuteIfBound();
            return FReply::Handled();
        })
    ];
}

// Event handlers
void SMagicOptimizerSlideOver::OnMaxTextureSizeChanged(const FText& NewText)
{
    int32 NewValue = FCString::Atoi(*NewText.ToString());
    CurrentSettings.MaxTextureSize = FMath::Clamp(NewValue, 64, 8192);
}

void SMagicOptimizerSlideOver::OnLODBiasChanged(const FText& NewText)
{
    int32 NewValue = FCString::Atoi(*NewText.ToString());
    CurrentSettings.LODBias = FMath::Clamp(NewValue, -3, 5);
}

void SMagicOptimizerSlideOver::OnCompressionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
    if (NewSelection.IsValid())
    {
        CurrentSettings.Compression = *NewSelection;
    }
}

void SMagicOptimizerSlideOver::OnApplyToThisAsset()
{
    const FUITextureRow& Texture = SelectedTexture.Get();
    if (Texture.Id.Len() > 0)
    {
        OnApplyToAsset.ExecuteIfBound(Texture.Id, CurrentSettings);
    }
}

// Data management
FString SMagicOptimizerSlideOver::GetCookedFormat(const FString& Compression, const FString& Target) const
{
    bool bIsMobile = (Target == TEXT("quest") || Target == TEXT("switch"));
    
    if (Compression == TEXT("Normal"))
    {
        return bIsMobile ? TEXT("ETC2/ASTC") : TEXT("BC5");
    }
    if (Compression == TEXT("HDR"))
    {
        return bIsMobile ? TEXT("ASTC HDR") : TEXT("BC6H");
    }
    if (Compression == TEXT("UserInterface"))
    {
        return bIsMobile ? TEXT("ETC2 RGBA") : TEXT("BC7");
    }
    if (Compression == TEXT("Masks"))
    {
        return bIsMobile ? TEXT("ETC2 RG") : TEXT("BC4/BC5");
    }
    
    return bIsMobile ? TEXT("ETC2") : TEXT("BC7");
}

TArray<FCookedFormat> SMagicOptimizerSlideOver::GetCookedFormats() const
{
    TArray<FCookedFormat> CookedFormats;
    TArray<FString> Platforms = TargetPlatforms.Get();
    
    // Default platforms if none provided
    if (Platforms.Num() == 0)
    {
        Platforms = {TEXT("PS5"), TEXT("XBOX"), TEXT("PC")};
    }
    
    for (const FString& Platform : Platforms)
    {
        FString CookedFormat = GetCookedFormat(CurrentSettings.Compression, Platform.ToLower());
        CookedFormats.Add(FCookedFormat(Platform.ToUpper(), CookedFormat));
    }
    
    return CookedFormats;
}

FString SMagicOptimizerSlideOver::GetEstimatedSavings() const
{
    float DiskSavings = (CurrentSettings.MaxTextureSize / 1024.0f) * 12.0f;
    float VRAMSavings = CurrentSettings.LODBias * 50.0f;
    
    return FString::Printf(TEXT("Estimates: Disk −%.0f MB • VRAM −%.0f MB (mock)"), 
        DiskSavings, VRAMSavings);
}

void SMagicOptimizerSlideOver::SetIsOpen(bool bInIsOpen)
{
    // Implementation can be left empty as visibility is handled by the lambda
}

void SMagicOptimizerSlideOver::SetSelectedTexture(const FUITextureRow& InTexture)
{
    // Initialize what-if settings based on selected texture
    if (InTexture.Size == TEXT("4k"))
    {
        CurrentSettings.MaxTextureSize = 4096;
    }
    else if (InTexture.Size == TEXT("2k"))
    {
        CurrentSettings.MaxTextureSize = 2048;
    }
    else
    {
        CurrentSettings.MaxTextureSize = 1024;
    }
    
    CurrentSettings.LODBias = InTexture.LOD;
    CurrentSettings.Compression = InTexture.Compression;

    // Update combo box selection
    if (CompressionComboBox.IsValid())
    {
        for (int32 i = 0; i < CompressionOptions.Num(); ++i)
        {
            if (*CompressionOptions[i] == CurrentSettings.Compression)
            {
                CompressionComboBox->SetSelectedItem(CompressionOptions[i]);
                break;
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE
