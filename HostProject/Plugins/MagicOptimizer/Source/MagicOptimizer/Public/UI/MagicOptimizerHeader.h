#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Slate.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Styling/AppStyle.h"
#include "UI/MagicOptimizerMainPanel.h"

/**
 * Header widget for MagicOptimizer that contains:
 * - Brand/Title
 * - Preset dropdown
 * - Target chips
 * - Scope button
 * - Change cap input
 * - Action buttons (Run Scan, Auto Fix, Preview, Tasks)
 * - Progress bar (when running)
 */
class MAGICOPTIMIZER_API SMagicOptimizerHeader : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerHeader) {}
    
    SLATE_ATTRIBUTE(FString, CurrentPreset)
    SLATE_ATTRIBUTE(TArray<FString>, SelectedTargets)
    SLATE_ATTRIBUTE(FString, ScopeSummary)
    SLATE_ATTRIBUTE(int32, ChangeCap)
    SLATE_ATTRIBUTE(FProgressData, ProgressData)
    
    SLATE_EVENT(FOnPresetChanged, OnPresetChanged)
    SLATE_EVENT(FOnTargetToggled, OnTargetToggled)
    SLATE_EVENT(FOnScopeChanged, OnScopeChanged)
    SLATE_EVENT(FOnChangeCapChanged, OnChangeCapChanged)
    SLATE_EVENT(FOnRunScan, OnRunScan)
    SLATE_EVENT(FOnAutoFix, OnAutoFix)
    SLATE_EVENT(FOnPreview, OnPreview)
    SLATE_EVENT(FOnTasks, OnTasks)
    
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    // Attributes
    TAttribute<FString> CurrentPreset;
    TAttribute<TArray<FString>> SelectedTargets;
    TAttribute<FString> ScopeSummary;
    TAttribute<int32> ChangeCap;
    TAttribute<FProgressData> ProgressData;

    // Events
    FOnPresetChanged OnPresetChanged;
    FOnTargetToggled OnTargetToggled;
    FOnScopeChanged OnScopeChanged;
    FOnChangeCapChanged OnChangeCapChanged;
    FOnRunScan OnRunScan;
    FOnAutoFix OnAutoFix;
    FOnPreview OnPreview;
    FOnTasks OnTasks;

    // Widget creation
    TSharedRef<SWidget> MakeBrandSection();
    TSharedRef<SWidget> MakePresetDropdown();
    TSharedRef<SWidget> MakeTargetChips();
    TSharedRef<SWidget> MakeTargetChip(const FString& ChipId, const FString& ChipLabel, const FString& Icon);
    TSharedRef<SWidget> MakeScopeButton();
    TSharedRef<SWidget> MakeChangeCapInput();
    TSharedRef<SWidget> MakeActionButtons();
    TSharedRef<SWidget> MakeProgressBar();

    // Event handlers
    void OnPresetSelectionChanged(const FString& NewPreset);
    void OnTargetChipClicked(const FString& TargetId);
    void OnScopeButtonClicked();
    void OnChangeCapTextChanged(const FText& NewText);
    void OnRunScanClicked();
    void OnAutoFixClicked();
    void OnPreviewClicked();
    void OnTasksClicked();

    // Data
    TArray<FTargetChip> TargetChips;
    TArray<FPresetOption> PresetOptions;
};

// Delegate for change cap changed
DECLARE_DELEGATE_OneParam(FOnChangeCapChanged, int32);
