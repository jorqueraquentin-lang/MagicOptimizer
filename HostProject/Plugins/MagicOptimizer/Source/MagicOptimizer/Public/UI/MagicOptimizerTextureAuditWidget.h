#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/ListView.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/SpinBox.h"
// #include "Components/TabWidget.h" // Not available in UE 5.6
#include "Components/ScrollBox.h"
#include "Components/PanelWidget.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/GridPanel.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/Overlay.h"
// #include "Components/StackBox.h" // Not available in UE 5.6
// #include "Components/WrapBox.h" // Not available in UE 5.6
#include "Components/ScrollBox.h"
#include "Components/ScrollBar.h"
#include "Components/Slider.h"
#include "Components/ProgressBar.h"
#include "Components/CircularThrobber.h"
#include "Components/Throbber.h"
// #include "Components/AnimatedThrobber.h" // Not available in UE 5.6
// #include "Components/WidgetAnimation.h" // Not available in UE 5.6
#include "MagicOptimizerAuditManager.h"
#include "AssetAuditors/TextureAuditor.h"
#include "AuditTypes/AuditResult.h"
#include "MagicOptimizerTextureAuditWidget.generated.h"

/**
 * Texture Audit Issue Item for List View
 */
UCLASS()
class MAGICOPTIMIZER_API UTextureAuditIssueItem : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly)
    FString AssetName;

    UPROPERTY(BlueprintReadOnly)
    FString AssetPath;

    UPROPERTY(BlueprintReadOnly)
    int32 IssueCount;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly)
    EAuditStatus Status;

    UPROPERTY(BlueprintReadOnly)
    TArray<FAuditIssue> Issues;

    UPROPERTY(BlueprintReadOnly)
    TArray<FOptimizationRecommendation> Recommendations;

    UPROPERTY(BlueprintReadOnly)
    FLinearColor SeverityColor;

    UPROPERTY(BlueprintReadOnly)
    FString StatusText;

    UPROPERTY(BlueprintReadOnly)
    FString AssetType;
};

/**
 * Main Texture Audit Widget
 * Professional UI for texture auditing with real-time analysis
 */
UCLASS(BlueprintType, Blueprintable)
class MAGICOPTIMIZER_API UMagicOptimizerTextureAuditWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UMagicOptimizerTextureAuditWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

public:
    /**
     * Initialize the widget with audit manager
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Texture Audit")
    void InitializeWidget();

    /**
     * Start texture audit process
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Texture Audit")
    void StartAudit();

    /**
     * Stop current audit process
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Texture Audit")
    void StopAudit();

    /**
     * Export audit results to file
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Texture Audit")
    void ExportResults();

    /**
     * Apply fixes to selected assets
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Texture Audit")
    void ApplyFixes();

    /**
     * Refresh the results display
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Texture Audit")
    void RefreshResults();

    /**
     * Filter results based on criteria
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Texture Audit")
    void FilterResults();

    /**
     * Clear all filters
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Texture Audit")
    void ClearFilters();

    /**
     * Select all assets
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Texture Audit")
    void SelectAllAssets();

    /**
     * Deselect all assets
     */
    UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|Texture Audit")
    void DeselectAllAssets();

protected:
    /**
     * Update progress display
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Magic Optimizer|Texture Audit")
    void OnProgressUpdated(float Progress, const FString& InStatusText);

    /**
     * Update results display
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Magic Optimizer|Texture Audit")
    void OnResultsUpdated(const TArray<UTextureAuditIssueItem*>& Results);

    /**
     * Show issue details
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Magic Optimizer|Texture Audit")
    void OnIssueSelected(UTextureAuditIssueItem* InSelectedItem);

    /**
     * Show status message
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Magic Optimizer|Texture Audit")
    void OnStatusMessage(const FString& Message, EAuditStatus Status);

    /**
     * Audit completed callback
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Magic Optimizer|Texture Audit")
    void OnAuditCompleted(bool bSuccess, const FString& Message);

public:
    // UI Component Bindings
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* StartAuditButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* StopAuditButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* ExportButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* ApplyFixesButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* RefreshButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* ClearFiltersButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UProgressBar* ProgressBar;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* ProgressText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* StatusText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UListView* ResultsListView;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UComboBoxString* PlatformComboBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UComboBoxString* QualityLevelComboBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UComboBoxString* IssueTypeFilterComboBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UComboBoxString* SeverityFilterComboBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UCheckBox* IncludeTexturesCheckBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UCheckBox* IncludeMaterialsCheckBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UCheckBox* IncludeMeshesCheckBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UCheckBox* IncludeBlueprintsCheckBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UEditableTextBox* IncludePathsTextBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class USpinBox* MaxAssetsSpinBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class USlider* MemoryThresholdSlider;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* MemoryThresholdText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* TotalAssetsText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* IssuesFoundText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* MemorySavedText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* ProcessingTimeText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UScrollBox* IssueDetailsScrollBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* SelectedAssetNameText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* SelectedAssetPathText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* SelectedAssetMemoryText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* SelectedAssetIssuesText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* SelectedAssetRecommendationsText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* FixAllButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* FixSelectedButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* PreviewButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* UndoButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* RedoButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UCircularThrobber* LoadingThrobber;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UBorder* StatusBorder;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UImage* StatusIcon;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UCanvasPanel* MainCanvas;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UGridPanel* MainGridPanel;

    // UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    // class USplitter* MainSplitter; // Not available in UE 5.6

    // UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    // class UWidgetSwitcher* ContentSwitcher; // Not available in UE 5.6

protected:
    // Event Handlers
    UFUNCTION()
    void OnStartAuditClicked();

    UFUNCTION()
    void OnStopAuditClicked();

    UFUNCTION()
    void OnExportClicked();

    UFUNCTION()
    void OnApplyFixesClicked();

    UFUNCTION()
    void OnRefreshClicked();

    UFUNCTION()
    void OnClearFiltersClicked();

    UFUNCTION()
    void OnPlatformChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnQualityLevelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnIssueTypeFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnSeverityFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnMemoryThresholdChanged(float Value);

    UFUNCTION()
    void OnMaxAssetsChanged(float Value);

    UFUNCTION()
    void OnResultsListSelectionChanged(UObject* SelectedItem);

    UFUNCTION()
    void OnFixAllClicked();

    UFUNCTION()
    void OnFixSelectedClicked();

    UFUNCTION()
    void OnPreviewClicked();

    UFUNCTION()
    void OnUndoClicked();

    UFUNCTION()
    void OnRedoClicked();

private:
    // Internal Methods
    void SetupUI();
    void SetupEventHandlers();
    void PopulateComboBoxes();
    void UpdateProgressDisplay(float Progress, const FString& StatusText);
    void UpdateResultsDisplay(const TArray<FAuditResult>& Results);
    void UpdateStatistics();
    void UpdateIssueDetails(UTextureAuditIssueItem* SelectedItem);
    void ApplyFilters();
    void SortResults();
    void UpdateButtonStates();
    void ShowStatusMessage(const FString& Message, EAuditStatus Status);
    void UpdateStatusIndicator(EAuditStatus Status);
    FLinearColor GetStatusColor(EAuditStatus Status);
    FString GetStatusText(EAuditStatus Status);
    FString FormatMemorySize(float SizeInMB);
    FString FormatTime(float TimeInSeconds);

    // Audit Manager Integration
    // AuditManager uses static methods, no instance needed

    // Current State
    UPROPERTY()
    TArray<FAuditResult> CurrentResults;

    UPROPERTY()
    TArray<UTextureAuditIssueItem*> FilteredResults;

    UPROPERTY()
    UTextureAuditIssueItem* SelectedItem;

    UPROPERTY()
    bool bIsAuditing;

    UPROPERTY()
    bool bHasResults;

    // Filter State
    UPROPERTY()
    EAuditIssueType FilteredIssueType;

    UPROPERTY()
    float FilteredSeverity;

    UPROPERTY()
    float MemoryThreshold;

    UPROPERTY()
    FString SelectedPlatform;

    UPROPERTY()
    FString SelectedQualityLevel;

    // Statistics
    UPROPERTY()
    int32 TotalAssets;

    UPROPERTY()
    int32 IssuesFound;

    UPROPERTY()
    float TotalMemoryUsage;

    UPROPERTY()
    float EstimatedMemorySaved;

    UPROPERTY()
    float ProcessingTime;

    // Timer Handles
    FTimerHandle ProgressUpdateTimer;
    FTimerHandle StatisticsUpdateTimer;
};
