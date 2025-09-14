#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Animation/CurveSequence.h"
#include "UI/MagicOptimizerMainPanel.h"
#include "MagicOptimizerCommandPalette.generated.h"

/**
 * Command palette item data
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FCommandPaletteItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Id;

    UPROPERTY(BlueprintReadOnly)
    FString Label;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    UPROPERTY(BlueprintReadOnly)
    FString Icon; // Unicode emoji or icon name

    UPROPERTY(BlueprintReadOnly)
    FString Category; // "Actions", "Navigation", "Export", etc.

    FCommandPaletteItem()
        : Id(TEXT(""))
        , Label(TEXT(""))
        , Description(TEXT(""))
        , Icon(TEXT(""))
        , Category(TEXT("Actions"))
    {}

    FCommandPaletteItem(const FString& InId, const FString& InLabel, const FString& InDescription, const FString& InIcon, const FString& InCategory = TEXT("Actions"))
        : Id(InId)
        , Label(InLabel)
        , Description(InDescription)
        , Icon(InIcon)
        , Category(InCategory)
    {}
};

DECLARE_DELEGATE_OneParam(FOnExecuteCommand, const FString&);

/**
 * Command palette widget for MagicOptimizer that provides:
 * - Ctrl+Shift+P keyboard shortcut activation
 * - Fuzzy search through available commands
 * - Quick access to all major functions
 * - Keyboard navigation with Enter to execute
 */
class MAGICOPTIMIZER_API SMagicOptimizerCommandPalette : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerCommandPalette) {}
    
    SLATE_ATTRIBUTE(bool, IsOpen)
    SLATE_EVENT(FSimpleDelegate, OnClose)
    SLATE_EVENT(FOnExecuteCommand, OnExecuteCommand)
    
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetIsOpen(bool bInIsOpen);
    void FocusSearchInput();

    // SWidget interface
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
    // Attributes
    TAttribute<bool> IsOpen;
    FSimpleDelegate OnClose;
    FOnExecuteCommand OnExecuteCommand;

    // Data
    TArray<FCommandPaletteItem> AllCommands;
    TArray<TSharedPtr<FCommandPaletteItem>> FilteredCommands;
    FString CurrentSearchText;
    TSharedPtr<FCommandPaletteItem> SelectedCommand;
    int32 SelectedIndex = 0;

    // UI references
    TSharedPtr<class SEditableTextBox> SearchInputWidget;
    TSharedPtr<SListView<TSharedPtr<FCommandPaletteItem>>> CommandListWidget;

    // Animations
    FCurveSequence FadeAnimation;
    FCurveHandle FadeAlpha;
    FCurveSequence ScaleAnimation;
    FCurveHandle ScaleAlpha;

    // Widget creation
    TSharedRef<SWidget> MakeCommandPalette();
    TSharedRef<SWidget> MakeSearchInput();
    TSharedRef<SWidget> MakeCommandsList();
    TSharedRef<ITableRow> OnGenerateCommandRow(TSharedPtr<FCommandPaletteItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);
    TSharedRef<SWidget> MakeQuickActions();

    // Event handlers
    void OnSearchTextChanged(const FText& Text);
    void OnSearchTextCommitted(const FText& Text, ETextCommit::Type CommitType);
    void OnCommandSelectionChanged(TSharedPtr<FCommandPaletteItem> SelectedItem, ESelectInfo::Type SelectInfo);
    void OnExecuteSelectedCommand();
    void ExecuteCommand(const FString& CommandId);

    // Command filtering
    void UpdateFilteredCommands();
    bool DoesCommandMatchSearch(const FCommandPaletteItem& Command, const FString& SearchText) const;
    float CalculateCommandScore(const FCommandPaletteItem& Command, const FString& SearchText) const;

    // Data initialization
    void InitializeCommands();

    // Navigation
    void SelectPreviousCommand();
    void SelectNextCommand();
    void UpdateSelection();
};
