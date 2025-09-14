#include "UI/MagicOptimizerCommandPalette.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerCommandPalette"

void SMagicOptimizerCommandPalette::Construct(const FArguments& InArgs)
{
    // Bind attributes and events
    IsOpen = InArgs._IsOpen;
    OnClose = InArgs._OnClose;
    OnExecuteCommand = InArgs._OnExecuteCommand;

    // Initialize commands
    InitializeCommands();
    UpdateFilteredCommands();

    // Set up animations
    FadeAnimation = FCurveSequence();
    FadeAlpha = FadeAnimation.AddCurve(0.0f, 0.2f, ECurveEaseFunction::QuadInOut);

    ScaleAnimation = FCurveSequence();
    ScaleAlpha = ScaleAnimation.AddCurve(0.0f, 0.3f);

    ChildSlot
    [
        // Modal overlay - only visible when open
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("NoBorder"))
        .ColorAndOpacity_Lambda([this]() -> FLinearColor
        {
            if (!IsOpen.Get()) return FLinearColor::Transparent;
            float Alpha = FadeAlpha.GetLerp();
            return FLinearColor(0.0f, 0.0f, 0.0f, Alpha * 0.5f);
        })
        .Padding(FMargin(0))
        .Visibility_Lambda([this]()
        {
            return IsOpen.Get() ? EVisibility::Visible : EVisibility::Collapsed;
        })
        [
            // Center the command palette
            SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Top)
            .Padding(FMargin(0, 100, 0, 0)) // Top margin for positioning
            [
                SNew(SBox)
                .WidthOverride(600)
                .RenderTransform_Lambda([this]() -> TOptional<FSlateRenderTransform>
                {
                    if (!IsOpen.Get()) return TOptional<FSlateRenderTransform>();
                    
                    float Scale = FMath::Lerp(0.85f, 1.0f, ScaleAlpha.GetLerp());
                    float Alpha = FadeAlpha.GetLerp();
                    
                    FVector2D Translation(0, (1.0f - Alpha) * -20.0f); // Slide down slightly
                    return FSlateRenderTransform(FScale2D(Scale), Translation);
                })
                .RenderTransformPivot(FVector2D(0.5f, 0.0f))
                [
                    MakeCommandPalette()
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerCommandPalette::MakeCommandPalette()
{
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 0.95f))
    .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    .Padding(FMargin(0))
    [
        SNew(SVerticalBox)
        
        // Header and search
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(20, 16, 20, 8)
        [
            SNew(SVerticalBox)
            
            // Title and description
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 8)
            [
                SNew(SHorizontalBox)
                
                +SHorizontalBox::Slot()
                .FillWidth(1.0f)
                [
                    SNew(SVerticalBox)
                    +SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("CommandPaletteTitle", "Command Palette"))
                        .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 16))
                        .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
                    ]
                    +SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("CommandPaletteDesc", "Type to filter commands. Use Enter to run."))
                        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                        .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
                    ]
                ]
            ]
            
            // Search input
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                MakeSearchInput()
            ]
        ]
        
        // Command list
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(20, 0, 20, 16)
        [
            SNew(SBox)
            .MaxDesiredHeight(300)
            [
                MakeCommandsList()
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerCommandPalette::MakeSearchInput()
{
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f))
    .Padding(FMargin(12, 8, 12, 8))
    [
        SNew(SHorizontalBox)
        
        // Search icon
        +SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 8, 0)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("🔍")))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
        ]
        
        // Search input
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        [
            SAssignNew(SearchInputWidget, SEditableTextBox)
            .HintText(LOCTEXT("SearchCommandsHint", "Run Scan, Auto Fix, Export CSV, Jump to Textures…"))
            .OnTextChanged(this, &SMagicOptimizerCommandPalette::OnSearchTextChanged)
            .OnTextCommitted(this, &SMagicOptimizerCommandPalette::OnSearchTextCommitted)
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 14))
            .ForegroundColor(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            .BackgroundColor(FLinearColor::Transparent)
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerCommandPalette::MakeCommandsList()
{
    return SAssignNew(CommandListWidget, SListView<TSharedPtr<FCommandPaletteItem>>)
        .ListItemsSource(&FilteredCommands)
        .OnGenerateRow(this, &SMagicOptimizerCommandPalette::OnGenerateCommandRow)
        .OnSelectionChanged(this, &SMagicOptimizerCommandPalette::OnCommandSelectionChanged)
        .SelectionMode(ESelectionMode::Single)
        .ScrollbarVisibility(EVisibility::Collapsed)
        .ClearSelectionOnClick(false);
}

TSharedRef<ITableRow> SMagicOptimizerCommandPalette::OnGenerateCommandRow(TSharedPtr<FCommandPaletteItem> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FCommandPaletteItem>>, OwnerTable)
    [
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("NoBorder"))
        .ColorAndOpacity_Lambda([this, InItem]()
        {
            return SelectedCommand == InItem ? 
                FLinearColor(0.3f, 0.5f, 0.8f, 0.3f) : 
                FLinearColor::Transparent;
        })
        .Padding(FMargin(12, 8, 12, 8))
        [
            SNew(SHorizontalBox)
            
            // Icon
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 12, 0)
            [
                SNew(STextBlock)
                .Text(FText::FromString(InItem->Icon))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 16))
                .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
            ]
            
            // Label and description
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                
                +SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(InItem->Label))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 13))
                    .ColorAndOpacity_Lambda([this, InItem]()
                    {
                        return SelectedCommand == InItem ? 
                            FLinearColor(1.0f, 1.0f, 1.0f, 1.0f) : 
                            FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
                    })
                ]
                
                +SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(InItem->Description))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
                    .Visibility(InItem->Description.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
                ]
            ]
            
            // Category badge
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(SBorder)
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                .ColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 0.8f))
                .BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f, 0.8f))
                .Padding(FMargin(6, 2, 6, 2))
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(InItem->Category))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
            ]
        ]
    ];
}

void SMagicOptimizerCommandPalette::OnSearchTextChanged(const FText& Text)
{
    CurrentSearchText = Text.ToString();
    UpdateFilteredCommands();
    
    // Reset selection to first item
    SelectedIndex = 0;
    UpdateSelection();
}

void SMagicOptimizerCommandPalette::OnSearchTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
    if (CommitType == ETextCommit::OnEnter)
    {
        OnExecuteSelectedCommand();
    }
}

void SMagicOptimizerCommandPalette::OnCommandSelectionChanged(TSharedPtr<FCommandPaletteItem> SelectedItem, ESelectInfo::Type SelectInfo)
{
    SelectedCommand = SelectedItem;
    
    // Update selected index to match
    for (int32 i = 0; i < FilteredCommands.Num(); ++i)
    {
        if (FilteredCommands[i] == SelectedItem)
        {
            SelectedIndex = i;
            break;
        }
    }
}

FReply SMagicOptimizerCommandPalette::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();
    
    if (Key == EKeys::Escape)
    {
        OnClose.ExecuteIfBound();
        return FReply::Handled();
    }
    else if (Key == EKeys::Up)
    {
        SelectPreviousCommand();
        return FReply::Handled();
    }
    else if (Key == EKeys::Down)
    {
        SelectNextCommand();
        return FReply::Handled();
    }
    else if (Key == EKeys::Enter)
    {
        OnExecuteSelectedCommand();
        return FReply::Handled();
    }
    
    return FReply::Unhandled();
}

void SMagicOptimizerCommandPalette::OnExecuteSelectedCommand()
{
    if (SelectedCommand.IsValid())
    {
        ExecuteCommand(SelectedCommand->Id);
        OnClose.ExecuteIfBound(); // Close palette after execution
    }
}

void SMagicOptimizerCommandPalette::ExecuteCommand(const FString& CommandId)
{
    OnExecuteCommand.ExecuteIfBound(CommandId);
}

void SMagicOptimizerCommandPalette::UpdateFilteredCommands()
{
    FilteredCommands.Empty();
    
    if (CurrentSearchText.IsEmpty())
    {
        // Show all commands when no search text
        for (const FCommandPaletteItem& Command : AllCommands)
        {
            FilteredCommands.Add(MakeShareable(new FCommandPaletteItem(Command)));
        }
    }
    else
    {
        // Filter and sort by relevance
        TArray<TPair<float, FCommandPaletteItem>> ScoredCommands;
        
        for (const FCommandPaletteItem& Command : AllCommands)
        {
            if (DoesCommandMatchSearch(Command, CurrentSearchText))
            {
                float Score = CalculateCommandScore(Command, CurrentSearchText);
                ScoredCommands.Add(TPair<float, FCommandPaletteItem>(Score, Command));
            }
        }
        
        // Sort by score (higher is better)
        ScoredCommands.Sort([](const TPair<float, FCommandPaletteItem>& A, const TPair<float, FCommandPaletteItem>& B)
        {
            return A.Key > B.Key;
        });
        
        // Convert to filtered list
        for (const auto& ScoredCommand : ScoredCommands)
        {
            FilteredCommands.Add(MakeShareable(new FCommandPaletteItem(ScoredCommand.Value)));
        }
    }
    
    // Refresh the list view
    if (CommandListWidget.IsValid())
    {
        CommandListWidget->RequestListRefresh();
    }
}

bool SMagicOptimizerCommandPalette::DoesCommandMatchSearch(const FCommandPaletteItem& Command, const FString& SearchText) const
{
    FString LowerSearchText = SearchText.ToLower();
    return Command.Label.ToLower().Contains(LowerSearchText) ||
           Command.Description.ToLower().Contains(LowerSearchText) ||
           Command.Category.ToLower().Contains(LowerSearchText);
}

float SMagicOptimizerCommandPalette::CalculateCommandScore(const FCommandPaletteItem& Command, const FString& SearchText) const
{
    FString LowerSearchText = SearchText.ToLower();
    FString LowerLabel = Command.Label.ToLower();
    FString LowerDescription = Command.Description.ToLower();
    
    float Score = 0.0f;
    
    // Exact match in label gets highest score
    if (LowerLabel == LowerSearchText)
    {
        Score += 100.0f;
    }
    // Label starts with search text gets high score
    else if (LowerLabel.StartsWith(LowerSearchText))
    {
        Score += 80.0f;
    }
    // Label contains search text gets medium score
    else if (LowerLabel.Contains(LowerSearchText))
    {
        Score += 60.0f;
    }
    // Description contains search text gets lower score
    else if (LowerDescription.Contains(LowerSearchText))
    {
        Score += 40.0f;
    }
    
    // Bonus for shorter commands (more specific)
    Score += (100.0f - LowerLabel.Len()) * 0.1f;
    
    return Score;
}

void SMagicOptimizerCommandPalette::SelectPreviousCommand()
{
    if (FilteredCommands.Num() == 0) return;
    
    SelectedIndex = FMath::Max(0, SelectedIndex - 1);
    UpdateSelection();
}

void SMagicOptimizerCommandPalette::SelectNextCommand()
{
    if (FilteredCommands.Num() == 0) return;
    
    SelectedIndex = FMath::Min(FilteredCommands.Num() - 1, SelectedIndex + 1);
    UpdateSelection();
}

void SMagicOptimizerCommandPalette::UpdateSelection()
{
    if (FilteredCommands.IsValidIndex(SelectedIndex))
    {
        SelectedCommand = FilteredCommands[SelectedIndex];
        
        if (CommandListWidget.IsValid())
        {
            CommandListWidget->SetSelection(SelectedCommand, ESelectInfo::Direct);
            CommandListWidget->RequestScrollIntoView(SelectedCommand);
        }
    }
}

void SMagicOptimizerCommandPalette::InitializeCommands()
{
    AllCommands.Empty();
    
    // Actions
    AllCommands.Add(FCommandPaletteItem(TEXT("run_scan"), TEXT("Run Scan"), TEXT("Start texture audit scan"), TEXT("▶️"), TEXT("Actions")));
    AllCommands.Add(FCommandPaletteItem(TEXT("auto_fix"), TEXT("Auto Fix"), TEXT("Apply automatic optimizations"), TEXT("✨"), TEXT("Actions")));
    AllCommands.Add(FCommandPaletteItem(TEXT("preview"), TEXT("Preview Plan"), TEXT("Preview changes before applying"), TEXT("👁"), TEXT("Actions")));
    
    // Export
    AllCommands.Add(FCommandPaletteItem(TEXT("export_csv"), TEXT("Export CSV"), TEXT("Download texture data as CSV"), TEXT("📁"), TEXT("Export")));
    AllCommands.Add(FCommandPaletteItem(TEXT("export_json"), TEXT("Export JSON"), TEXT("Download structured JSON data"), TEXT("📁"), TEXT("Export")));
    AllCommands.Add(FCommandPaletteItem(TEXT("export_html"), TEXT("Export HTML"), TEXT("Download formatted HTML report"), TEXT("📁"), TEXT("Export")));
    AllCommands.Add(FCommandPaletteItem(TEXT("copy_commandlet"), TEXT("Copy Commandlet"), TEXT("Copy command line for automation"), TEXT("📋"), TEXT("Export")));
    
    // Navigation
    AllCommands.Add(FCommandPaletteItem(TEXT("goto_audit"), TEXT("Jump to Audit"), TEXT("Switch to audit view"), TEXT("🔍"), TEXT("Navigation")));
    AllCommands.Add(FCommandPaletteItem(TEXT("goto_budgets"), TEXT("Jump to Budgets"), TEXT("Switch to budgets view"), TEXT("📊"), TEXT("Navigation")));
    AllCommands.Add(FCommandPaletteItem(TEXT("goto_recommend"), TEXT("Jump to Recommend"), TEXT("Switch to recommendations view"), TEXT("💡"), TEXT("Navigation")));
    AllCommands.Add(FCommandPaletteItem(TEXT("goto_apply"), TEXT("Jump to Apply"), TEXT("Switch to apply view"), TEXT("⚡"), TEXT("Navigation")));
    AllCommands.Add(FCommandPaletteItem(TEXT("goto_verify"), TEXT("Jump to Verify"), TEXT("Switch to verification view"), TEXT("✅"), TEXT("Navigation")));
    AllCommands.Add(FCommandPaletteItem(TEXT("goto_reports"), TEXT("Jump to Reports"), TEXT("Switch to reports view"), TEXT("📈"), TEXT("Navigation")));
    
    // Configuration
    AllCommands.Add(FCommandPaletteItem(TEXT("toggle_rules"), TEXT("Toggle Rules"), TEXT("Toggle optimization rules"), TEXT("⚙️"), TEXT("Config")));
    AllCommands.Add(FCommandPaletteItem(TEXT("open_scope"), TEXT("Configure Scope"), TEXT("Set project scope and exclusions"), TEXT("🎯"), TEXT("Config")));
    AllCommands.Add(FCommandPaletteItem(TEXT("copy_share_link"), TEXT("Copy Share Link"), TEXT("Copy shareable report link"), TEXT("🔗"), TEXT("Share")));
}

void SMagicOptimizerCommandPalette::SetIsOpen(bool bInIsOpen)
{
    if (bInIsOpen)
    {
        // Start open animations
        FadeAnimation.Play(this->AsShared());
        ScaleAnimation.Play(this->AsShared());
        
        // Focus search input after a brief delay to ensure visibility
        if (SearchInputWidget.IsValid())
        {
            FocusSearchInput();
        }
    }
    else
    {
        // For closing, we could reverse the animation here if needed
        // For now, the visibility controls hiding
    }
}

void SMagicOptimizerCommandPalette::FocusSearchInput()
{
    if (SearchInputWidget.IsValid())
    {
        FSlateApplication::Get().SetKeyboardFocus(SearchInputWidget, EFocusCause::SetDirectly);
    }
}

#undef LOCTEXT_NAMESPACE
