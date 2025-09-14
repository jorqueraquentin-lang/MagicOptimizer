#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Slate.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"
#include "Engine/Engine.h"
#include "UI/UIDataTypes.h"
#include "UI/UIDataProvider.h"
#include "Model/AuditResult.h"
#include "MagicAuditTypes.h"

// Forward declarations
class SMagicOptimizerHeader;
class SMagicOptimizerLeftRail;
class SMagicOptimizerCenterTabs;
class SMagicOptimizerRightShelf;
class SMagicOptimizerAuditView;
class SMagicOptimizerBudgetsView;
class SMagicOptimizerRecommendView;
class SMagicOptimizerApplyView;
class SMagicOptimizerVerifyView;
class SMagicOptimizerReportsView;
class SMagicOptimizerCommandPalette;
class SMagicOptimizerScopeSheet;
class SMagicOptimizerPlanDialog;
class SMagicOptimizerTaskDrawer;
class SMagicOptimizerSlideOver;

// Data structures are now defined in UIDataTypes.h

/**
 * Main MagicOptimizer Panel - The primary UI widget that contains all the functionality
 * This matches the React design as closely as possible using pure C++ Slate
 */
class MAGICOPTIMIZER_API SMagicOptimizerMainPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerMainPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // SWidget interface
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

    // Public interface
    void RefreshData();
    void StartScan(bool bAutoFix = false, bool bPreview = false);
    void StopScan();
    void OpenCommandPalette();
    void ToggleDrawer();
    void OpenScopeSheet();
    void OpenPlanDialog();
    void OpenTaskDrawer();

private:
    // State management
    int32 ActiveTabIndex = 0;
    bool bDrawerOpen = false;
    bool bScopeSheetOpen = false;
    bool bPlanDialogOpen = false;
    bool bTaskDrawerOpen = false;
    bool bCommandPaletteOpen = false;

    // Current selections
    FString CurrentPreset = TEXT("Console Balanced");
    TArray<FString> SelectedTargets = {TEXT("ps5"), TEXT("xbox")};
    FString ScopeSummary = TEXT("/Game, 2 excludes");
    int32 ChangeCap = 800;
    FString SearchText = TEXT("");
    TArray<FString> SelectedTextureIds;

    // Data
    TArray<FTargetChip> TargetChips;
    TArray<FPresetOption> PresetOptions;
    TArray<FUITextureRow> TextureData;
    FHealthData HealthData;
    TArray<FTaskData> TaskData;
    FProgressData ProgressData;

    // Animations
    FCurveSequence DrawerAnim;
    FCurveHandle DrawerAlpha;
    FCurveSequence ProgressAnim;
    FCurveHandle ProgressAlpha;

    // Command system
    TSharedPtr<FUICommandList> Commands;

    // Child widgets
    TSharedPtr<SMagicOptimizerHeader> HeaderWidget;
    TSharedPtr<SMagicOptimizerLeftRail> LeftRailWidget;
    TSharedPtr<SMagicOptimizerCenterTabs> CenterTabsWidget;
    TSharedPtr<SMagicOptimizerRightShelf> RightShelfWidget;
    TSharedPtr<SMagicOptimizerCommandPalette> CommandPaletteWidget;
    TSharedPtr<SMagicOptimizerScopeSheet> ScopeSheetWidget;
    TSharedPtr<SMagicOptimizerPlanDialog> PlanDialogWidget;
    TSharedPtr<SMagicOptimizerTaskDrawer> TaskDrawerWidget;
    TSharedPtr<SMagicOptimizerSlideOver> SlideOverWidget;

    // Widget creation
    TSharedRef<SWidget> MakeMainContent();
    TSharedRef<SWidget> MakeDrawerOverlay();
    TSharedRef<SWidget> MakeModalOverlay();

    // Data initialization
    void InitializeData();
    void GenerateMockData();
    void ConvertAuditResultsToUIData(const TArray<FAuditResult>& Results);
    void ConvertUIDataToInternal(const TArray<FUITextureRow>& UITextureRows, const TArray<FUIHealthPoint>& UIHealthPoints, const TArray<FUIBeforeAfter>& UIBeforeAfter, const FUIBudgetGauges& UIGauges);
    FHealthData CalculateHealthFromAuditResults(const TArray<FAuditResult>& Results) const;

    // Event handlers
    void OnPresetChanged(const FString& NewPreset);
    void OnTargetToggled(const FString& TargetId);
    void OnScopeChanged(const FString& NewScope);
    void OnSearchTextChanged(const FText& NewText);
    void OnTextureSelectionChanged(const TArray<FString>& NewSelection);
    void OnTabChanged(int32 NewTabIndex);

    // Actions
    void OnRunScan();
    void OnAutoFix();
    void OnPreview();
    void OnTasks();
    void OnCloseDrawer();
    void OnCloseScopeSheet();
    void OnClosePlanDialog();
    void OnCloseTaskDrawer();
    void OnCloseCommandPalette();

    // Command execution
    void OnExecuteCommand(const FString& CommandId);
    
    // Texture settings application
    void OnApplyTextureSettings(const FString& TextureId, const struct FTextureSettings& Settings);

    // Command binding
    void BindCommands();

    // Animation callbacks
    void OnDrawerAnimationFinished();
    void OnProgressAnimationFinished();

    // Utility functions
    TArray<FUITextureRow> GetFilteredTextures() const;
    TArray<FUITextureRow> GetSelectedTextures() const;
    FHealthData CalculateHealthData() const;
    TArray<FTaskData> GetActiveTasks() const;

    // Delegate declarations
    DECLARE_DELEGATE_OneParam(FOnPresetChanged, const FString&);
    DECLARE_DELEGATE_OneParam(FOnTargetToggled, const FString&);
    DECLARE_DELEGATE_OneParam(FOnScopeChanged, const FString&);
    DECLARE_DELEGATE_OneParam(FOnSearchTextChanged, const FText&);
    DECLARE_DELEGATE_OneParam(FOnTextureSelectionChanged, const TArray<FString>&);
    DECLARE_DELEGATE_OneParam(FOnTabChanged, int32);
    DECLARE_DELEGATE(FOnRunScan);
    DECLARE_DELEGATE(FOnAutoFix);
    DECLARE_DELEGATE(FOnPreview);
    DECLARE_DELEGATE(FOnTasks);
    DECLARE_DELEGATE(FOnCloseDrawer);
    DECLARE_DELEGATE(FOnCloseScopeSheet);
    DECLARE_DELEGATE(FOnClosePlanDialog);
    DECLARE_DELEGATE(FOnCloseTaskDrawer);
    DECLARE_DELEGATE(FOnCloseCommandPalette);

    // Public delegates
    FOnPresetChanged OnPresetChangedDelegate;
    FOnTargetToggled OnTargetToggledDelegate;
    FOnScopeChanged OnScopeChangedDelegate;
    FOnSearchTextChanged OnSearchTextChangedDelegate;
    FOnTextureSelectionChanged OnTextureSelectionChangedDelegate;
    FOnTabChanged OnTabChangedDelegate;
    FOnRunScan OnRunScanDelegate;
    FOnAutoFix OnAutoFixDelegate;
    FOnPreview OnPreviewDelegate;
    FOnTasks OnTasksDelegate;
    FOnCloseDrawer OnCloseDrawerDelegate;
    FOnCloseScopeSheet OnCloseScopeSheetDelegate;
    FOnClosePlanDialog OnClosePlanDialogDelegate;
    FOnCloseTaskDrawer OnCloseTaskDrawerDelegate;
    FOnCloseCommandPalette OnCloseCommandPaletteDelegate;
};
