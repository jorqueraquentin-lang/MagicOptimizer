#include "UI/MagicOptimizerScopeSheet.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerScopeSheet"

void SMagicOptimizerScopeSheet::Construct(const FArguments& InArgs)
{
    // Bind attributes and events
    IsOpen = InArgs._IsOpen;
    OnClose = InArgs._OnClose;
    OnApply = InArgs._OnApply;

    // Initialize data
    InitializeDefaultData();

    ChildSlot
    [
        // Modal overlay - only visible when open
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("NoBorder"))
        .ColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f)) // Semi-transparent overlay
        .Padding(FMargin(0))
        .Visibility_Lambda([this]()
        {
            return IsOpen.Get() ? EVisibility::Visible : EVisibility::Collapsed;
        })
        [
            // Center the scope sheet
            SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Top)
            .Padding(FMargin(0, 60, 0, 60)) // Top and bottom margins
            [
                SNew(SBox)
                .WidthOverride(1200)
                .MaxDesiredHeight(700)
                [
                    MakeScopeSheet()
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerScopeSheet::MakeScopeSheet()
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
            
            // Title and description
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 8)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("ScopeSheetTitle", "Scope & Profiles"))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 18)
                .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            ]
            
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 16)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("ScopeSheetDesc", "Choose scope and tweak TextureGroup budgets safely."))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 14)
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
            
            // Tab buttons
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                MakeTabButtons()
            ]
        ]
        
        // Tab content
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(24, 0, 24, 0)
        [
            SNew(SScrollBox)
            +SScrollBox::Slot()
            [
                MakeTabContent()
            ]
        ]
        
        // Footer with Apply button
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
                .Text(LOCTEXT("ApplyButton", "Apply"))
                .ButtonStyle(FAppStyle::Get(), "PrimaryButton")
                .ContentPadding(FMargin(24, 12, 24, 12))
                .ColorAndOpacity(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f))
                .OnClicked_Lambda([this]()
                {
                    OnApplyChanges();
                    return FReply::Handled();
                })
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerScopeSheet::MakeTabButtons()
{
    return SNew(SHorizontalBox)
    
    +SHorizontalBox::Slot()
    .AutoWidth()
    .Padding(0, 0, 12, 0)
    [
        MakeTabButton(TEXT("Include / Exclude"), 0)
    ]
    
    +SHorizontalBox::Slot()
    .AutoWidth()
    [
        MakeTabButton(TEXT("Profiles & Groups"), 1)
    ];
}

TSharedRef<SWidget> SMagicOptimizerScopeSheet::MakeTabButton(const FString& Label, int32 TabIndex)
{
    return SNew(SButton)
    .Text(FText::FromString(Label))
    .ButtonStyle(FAppStyle::Get(), "SimpleButton")
    .ContentPadding(FMargin(16, 10, 16, 10))
    .OnClicked_Lambda([this, TabIndex]()
    {
        OnTabChanged(TabIndex);
        return FReply::Handled();
    })
    .ColorAndOpacity_Lambda([this, TabIndex]()
    {
        return ActiveTab == TabIndex ? 
            FLinearColor(0.3f, 0.5f, 0.8f, 0.8f) : 
            FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
    })
    .BorderBackgroundColor_Lambda([this, TabIndex]()
    {
        return ActiveTab == TabIndex ? 
            FLinearColor(0.3f, 0.5f, 0.8f, 1.0f) : 
            FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
    })
    .TextStyle_Lambda([this, TabIndex]()
    {
        FTextBlockStyle Style = FAppStyle::GetWidgetStyle<FTextBlockStyle>("NormalText");
        Style.Font = FAppStyle::GetFontStyle("NormalFontBold");
        Style.Font.Size = 13;
        Style.ColorAndOpacity = ActiveTab == TabIndex ? 
            FLinearColor(1.0f, 1.0f, 1.0f, 1.0f) : 
            FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);
        return Style;
    });
}

TSharedRef<SWidget> SMagicOptimizerScopeSheet::MakeTabContent()
{
    if (ActiveTab == 0)
    {
        return MakeIncludeExcludeTab();
    }
    else
    {
        return MakeProfilesTab();
    }
}

TSharedRef<SWidget> SMagicOptimizerScopeSheet::MakeIncludeExcludeTab()
{
    return SNew(SHorizontalBox)
    
    // Left side - Include/Exclude
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .Padding(0, 0, 24, 0)
    [
        SNew(SVerticalBox)
        
        // Include section
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(0, 0, 0, 24)
        [
            MakeIncludeSection()
        ]
        
        // Exclude section
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            MakeExcludeSection()
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerScopeSheet::MakeIncludeSection()
{
    return SNew(SVerticalBox)
    
    // Title
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 16)
    [
        SNew(STextBlock)
        .Text(LOCTEXT("IncludeTitle", "Include"))
        .Font(FAppStyle::GetFontStyle("NormalFontBold"))
        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 16))
        .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
    ]
    
    // Folder checkboxes
    +SVerticalBox::Slot()
    .AutoHeight()
    [
        [this]() -> TSharedRef<SWidget>
        {
            TSharedRef<SVerticalBox> FolderBox = SNew(SVerticalBox);
            
            for (const FFolderSettings& Folder : FolderSettings)
            {
                FolderBox->AddSlot()
                .AutoHeight()
                .Padding(0, 0, 0, 8)
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(VAlign_Center)
                    .Padding(0, 0, 8, 0)
                    [
                        SNew(SCheckBox)
                        .IsChecked(Folder.bIncluded ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
                        .OnCheckStateChanged_Lambda([this, Folder](ECheckBoxState NewState)
                        {
                            OnFolderToggled(Folder.FolderName);
                        })
                    ]
                    +SHorizontalBox::Slot()
                    .FillWidth(1.0f)
                    .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(Folder.FolderName))
                        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 13)
                        .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                    ]
                ];
            }
            
            return FolderBox;
        }()
    ];
}

TSharedRef<SWidget> SMagicOptimizerScopeSheet::MakeExcludeSection()
{
    return SNew(SVerticalBox)
    
    // Title
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 16)
    [
        SNew(STextBlock)
        .Text(LOCTEXT("ExcludeTitle", "Excludes"))
        .Font(FAppStyle::GetFontStyle("NormalFontBold"))
        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 16))
        .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
    ]
    
    // Exclude paths
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 12)
    [
        [this]() -> TSharedRef<SWidget>
        {
            TSharedRef<SVerticalBox> ExcludeBox = SNew(SVerticalBox);
            
            for (int32 i = 0; i < ExcludePaths.Num(); ++i)
            {
                const int32 Index = i; // Capture for lambda
                ExcludeBox->AddSlot()
                .AutoHeight()
                .Padding(0, 0, 0, 8)
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    .FillWidth(1.0f)
                    .Padding(0, 0, 8, 0)
                    [
                        SNew(SEditableTextBox)
                        .Text(FText::FromString(ExcludePaths[Index]))
                        .OnTextChanged_Lambda([this, Index](const FText& NewText)
                        {
                            OnExcludePathChanged(Index, NewText);
                        })
                        .Font(FAppStyle::GetFontStyle("NormalFont"))
                        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 12)
                    ]
                    +SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SButton)
                        .Text(LOCTEXT("RemoveButton", "✕"))
                        .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                        .ContentPadding(FMargin(8, 4, 8, 4))
                        .ColorAndOpacity(FLinearColor(0.8f, 0.3f, 0.2f, 1.0f))
                        .OnClicked_Lambda([this, Index]()
                        {
                            OnRemoveExcludePath(Index);
                            return FReply::Handled();
                        })
                    ]
                ];
            }
            
            return ExcludeBox;
        }()
    ]
    
    // Add exclude button
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 16)
    [
        SNew(SButton)
        .Text(LOCTEXT("AddExcludeButton", "Add exclude"))
        .ButtonStyle(FAppStyle::Get(), "SimpleButton")
        .ContentPadding(FMargin(12, 6, 12, 6))
        .OnClicked_Lambda([this]()
        {
            OnAddExcludePath();
            return FReply::Handled();
        })
    ]
    
    // Apply only classified switch
    +SVerticalBox::Slot()
    .AutoHeight()
    [
        SNew(SHorizontalBox)
        +SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 8, 0)
        [
            SNew(SCheckBox)
            .IsChecked(bApplyOnlyClassified ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
            .OnCheckStateChanged(this, &SMagicOptimizerScopeSheet::OnApplyOnlyClassifiedChanged)
        ]
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("ApplyOnlyClassifiedLabel", "Apply Only on assets with clear classification"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 13)
            .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerScopeSheet::MakeProfilesTab()
{
    return SNew(SHorizontalBox)
    
    // Left side - Texture Groups
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .Padding(0, 0, 24, 0)
    [
        SNew(SVerticalBox)
        
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            [this]() -> TSharedRef<SWidget>
            {
                TSharedRef<SVerticalBox> GroupsBox = SNew(SVerticalBox);
                
                for (int32 i = 0; i < TextureGroups.Num(); ++i)
                {
                    GroupsBox->AddSlot()
                    .AutoHeight()
                    .Padding(0, 0, 0, 12)
                    [
                        MakeTextureGroupCard(i)
                    ];
                }
                
                return GroupsBox;
            }()
        ]
    ]
    
    // Right side - INI Preview
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    [
        MakeINIPreview()
    ];
}

TSharedRef<SWidget> SMagicOptimizerScopeSheet::MakeTextureGroupCard(int32 GroupIndex)
{
    if (!TextureGroups.IsValidIndex(GroupIndex))
    {
        return SNullWidget::NullWidget;
    }
    
    const FTextureGroupSettings& Group = TextureGroups[GroupIndex];
    
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
    .Padding(FMargin(16, 12, 16, 12))
    [
        SNew(SVerticalBox)
        
        // Group title
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 12)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Group.GroupName))
            .Font(FAppStyle::GetFontStyle("NormalFontBold"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 14)
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        // Settings
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            
            // MaxTextureSize
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 16, 0)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("MaxTextureSizeLabel", "MaxTextureSize"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 12)
                .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 24, 0)
            [
                SNew(SBox)
                .WidthOverride(80)
                [
                    SNew(SEditableTextBox)
                    .Text(FText::AsNumber(Group.MaxTextureSize))
                    .OnTextChanged_Lambda([this, GroupIndex](const FText& NewText)
                    {
                        OnTextureGroupChanged(GroupIndex, TEXT("MaxTextureSize"), NewText);
                    })
                    .Font(FAppStyle::GetFontStyle("NormalFont"))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 12)
                ]
            ]
            
            // LODBias
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 16, 0)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("LODBiasLabel", "LODBias"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 12)
                .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(SBox)
                .WidthOverride(60)
                [
                    SNew(SEditableTextBox)
                    .Text(FText::AsNumber(Group.LODBias))
                    .OnTextChanged_Lambda([this, GroupIndex](const FText& NewText)
                    {
                        OnTextureGroupChanged(GroupIndex, TEXT("LODBias"), NewText);
                    })
                    .Font(FAppStyle::GetFontStyle("NormalFont"))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 12)
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerScopeSheet::MakeINIPreview()
{
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
    .Padding(FMargin(16, 12, 16, 12))
    [
        SNew(SVerticalBox)
        
        // Title
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 12)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("INIPreviewTitle", "Preview .ini Diff"))
            .Font(FAppStyle::GetFontStyle("NormalFontBold"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 14)
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        // INI content
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .ColorAndOpacity(FLinearColor(0.05f, 0.05f, 0.05f, 1.0f))
            .BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
            .Padding(FMargin(12))
            [
                SNew(SMultiLineEditableText)
                .Text_Lambda([this]()
                {
                    return FText::FromString(GenerateINIContent());
                })
                .IsReadOnly(true)
                .Font(FAppStyle::GetFontStyle("MonoFont"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 11)
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                .BackgroundColor(FLinearColor::Transparent)
                .Justification(ETextJustify::Left)
            ]
        ]
    ];
}

// Event handlers
void SMagicOptimizerScopeSheet::OnTabChanged(int32 NewTabIndex)
{
    ActiveTab = NewTabIndex;
}

void SMagicOptimizerScopeSheet::OnFolderToggled(const FString& FolderName)
{
    for (FFolderSettings& Folder : FolderSettings)
    {
        if (Folder.FolderName == FolderName)
        {
            Folder.bIncluded = !Folder.bIncluded;
            break;
        }
    }
}

void SMagicOptimizerScopeSheet::OnExcludePathChanged(int32 Index, const FText& NewText)
{
    if (ExcludePaths.IsValidIndex(Index))
    {
        ExcludePaths[Index] = NewText.ToString();
    }
}

void SMagicOptimizerScopeSheet::OnAddExcludePath()
{
    ExcludePaths.Add(TEXT("/Game/Unused"));
}

void SMagicOptimizerScopeSheet::OnRemoveExcludePath(int32 Index)
{
    if (ExcludePaths.IsValidIndex(Index))
    {
        ExcludePaths.RemoveAt(Index);
    }
}

void SMagicOptimizerScopeSheet::OnApplyOnlyClassifiedChanged(ECheckBoxState NewState)
{
    bApplyOnlyClassified = (NewState == ECheckBoxState::Checked);
}

void SMagicOptimizerScopeSheet::OnTextureGroupChanged(int32 GroupIndex, const FString& Property, const FText& NewValue)
{
    if (!TextureGroups.IsValidIndex(GroupIndex))
    {
        return;
    }
    
    FTextureGroupSettings& Group = TextureGroups[GroupIndex];
    
    if (Property == TEXT("MaxTextureSize"))
    {
        Group.MaxTextureSize = FCString::Atoi(*NewValue.ToString());
        Group.MaxTextureSize = FMath::Clamp(Group.MaxTextureSize, 64, 8192);
    }
    else if (Property == TEXT("LODBias"))
    {
        Group.LODBias = FCString::Atoi(*NewValue.ToString());
        Group.LODBias = FMath::Clamp(Group.LODBias, -3, 5);
    }
}

void SMagicOptimizerScopeSheet::OnApplyChanges()
{
    FString ScopeSummary = GenerateScopeSummary();
    OnApply.ExecuteIfBound(ScopeSummary);
    OnClose.ExecuteIfBound();
}

// Data management
void SMagicOptimizerScopeSheet::InitializeDefaultData()
{
    // Initialize folder settings (matching React mock)
    FolderSettings.Empty();
    FolderSettings.Add(FFolderSettings(TEXT("game"), true));
    FolderSettings.Add(FFolderSettings(TEXT("maps"), true));
    FolderSettings.Add(FFolderSettings(TEXT("ui"), true));
    FolderSettings.Add(FFolderSettings(TEXT("materials"), true));
    FolderSettings.Add(FFolderSettings(TEXT("audio"), false));
    
    // Initialize exclude paths
    ExcludePaths.Empty();
    ExcludePaths.Add(TEXT("/Game/Developer"));
    
    // Initialize texture groups (matching React mock)
    TextureGroups.Empty();
    TextureGroups.Add(FTextureGroupSettings(TEXT("World"), 4096, 0));
    TextureGroups.Add(FTextureGroupSettings(TEXT("Characters"), 4096, 0));
    TextureGroups.Add(FTextureGroupSettings(TEXT("Props"), 2048, 0));
    TextureGroups.Add(FTextureGroupSettings(TEXT("UI"), 2048, 0));
    TextureGroups.Add(FTextureGroupSettings(TEXT("Effects"), 1024, 1));
}

FString SMagicOptimizerScopeSheet::GenerateINIContent() const
{
    FString INIContent = TEXT("[/Script/Engine.TextureLODSettings]\nTextureLODGroups=(\n");
    
    for (const FTextureGroupSettings& Group : TextureGroups)
    {
        INIContent += FString::Printf(TEXT("  (%s MaxTextureSize=%d, LODBias=%d),\n"), 
            *Group.GroupName, Group.MaxTextureSize, Group.LODBias);
    }
    
    INIContent += TEXT(")");
    return INIContent;
}

FString SMagicOptimizerScopeSheet::GenerateScopeSummary() const
{
    int32 IncludedCount = 0;
    for (const FFolderSettings& Folder : FolderSettings)
    {
        if (Folder.bIncluded)
        {
            IncludedCount++;
        }
    }
    
    return FString::Printf(TEXT("/Game, %d excludes"), ExcludePaths.Num());
}

void SMagicOptimizerScopeSheet::SetIsOpen(bool bInIsOpen)
{
    // Implementation can be left empty as visibility is handled by the lambda
}

#undef LOCTEXT_NAMESPACE
