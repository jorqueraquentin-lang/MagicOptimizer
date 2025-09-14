#include "UI/MagicOptimizerTextureAuditWidget.h"
#include "MagicOptimizerAuditManager.h"
#include "MagicOptimizerDiagnostics.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/ListView.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/SpinBox.h"
#include "Components/Slider.h"
#include "Components/ScrollBox.h"
#include "Components/CircularThrobber.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/GridPanel.h"
// #include "Components/Splitter.h" // Not available in UE 5.6
#include "Components/WidgetSwitcher.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Async/Async.h"

UMagicOptimizerTextureAuditWidget::UMagicOptimizerTextureAuditWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Initialize default values
    bIsAuditing = false;
    bHasResults = false;
    SelectedItem = nullptr;
    
    FilteredIssueType = EAuditIssueType::Info;
    FilteredSeverity = 0.0f;
    MemoryThreshold = 10.0f;
    SelectedPlatform = TEXT("PC");
    SelectedQualityLevel = TEXT("High");
    
    TotalAssets = 0;
    IssuesFound = 0;
    TotalMemoryUsage = 0.0f;
    EstimatedMemorySaved = 0.0f;
    ProcessingTime = 0.0f;
}

void UMagicOptimizerTextureAuditWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Setup UI components and event handlers
    SetupUI();
    SetupEventHandlers();
    PopulateComboBoxes();
    UpdateButtonStates();
    
    MAGIC_LOG(General, TEXT("Texture Audit Widget constructed"));
}

void UMagicOptimizerTextureAuditWidget::NativeDestruct()
{
    // Clean up timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ProgressUpdateTimer);
        World->GetTimerManager().ClearTimer(StatisticsUpdateTimer);
    }
    
    Super::NativeDestruct();
    
    MAGIC_LOG(General, TEXT("Texture Audit Widget destroyed"));
}

void UMagicOptimizerTextureAuditWidget::InitializeWidget()
{
    // AuditManager uses static methods, no need to get instance
    MAGIC_LOG(General, TEXT("Texture Audit Widget initialized"));
    
    // Setup UI components
    SetupUI();
    
    // Initialize state
    bIsAuditing = false;
    FilteredIssueType = EAuditIssueType::Info;
    FilteredSeverity = 0; // All severities
    SelectedQualityLevel = TEXT("High");
    SelectedPlatform = TEXT("WindowsEditor");
    
    // Update UI state
    UpdateButtonStates();
    UpdateStatusIndicator(EAuditStatus::Pending);
    
    MAGIC_LOG(General, TEXT("Texture Audit Widget setup completed"));
}

void UMagicOptimizerTextureAuditWidget::SetupUI()
{
    // Set initial values for UI components
    if (ProgressBar)
    {
        ProgressBar->SetPercent(0.0f);
    }
    
    if (ProgressText)
    {
        ProgressText->SetText(FText::FromString(TEXT("Ready to start audit")));
    }
    
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(TEXT("Ready")));
    }
    
    if (MaxAssetsSpinBox)
    {
        MaxAssetsSpinBox->SetValue(100);
    }
    
    if (MemoryThresholdSlider)
    {
        MemoryThresholdSlider->SetValue(MemoryThreshold);
    }
    
    if (MemoryThresholdText)
    {
        MemoryThresholdText->SetText(FText::FromString(FString::Printf(TEXT("%.1f MB"), MemoryThreshold)));
    }
    
    // Set default checkboxes
    if (IncludeTexturesCheckBox)
    {
        IncludeTexturesCheckBox->SetCheckedState(ECheckBoxState::Checked);
    }
    
    if (IncludeMaterialsCheckBox)
    {
        IncludeMaterialsCheckBox->SetCheckedState(ECheckBoxState::Checked);
    }
    
    if (IncludeMeshesCheckBox)
    {
        IncludeMeshesCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
    }
    
    if (IncludeBlueprintsCheckBox)
    {
        IncludeBlueprintsCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
    }
    
    // Set default include paths
    if (IncludePathsTextBox)
    {
        IncludePathsTextBox->SetText(FText::FromString(TEXT("/Game/")));
    }
    
    // Initialize statistics display
    UpdateStatistics();
    
    // Hide loading throbber initially
    if (LoadingThrobber)
    {
        LoadingThrobber->SetVisibility(ESlateVisibility::Hidden);
    }
    
    // Set initial status
    UpdateStatusIndicator(EAuditStatus::Pending);
}

void UMagicOptimizerTextureAuditWidget::SetupEventHandlers()
{
    // Button event handlers
    if (StartAuditButton)
    {
        StartAuditButton->OnClicked.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnStartAuditClicked);
    }
    
    if (StopAuditButton)
    {
        StopAuditButton->OnClicked.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnStopAuditClicked);
    }
    
    if (ExportButton)
    {
        ExportButton->OnClicked.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnExportClicked);
    }
    
    if (ApplyFixesButton)
    {
        ApplyFixesButton->OnClicked.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnApplyFixesClicked);
    }
    
    if (RefreshButton)
    {
        RefreshButton->OnClicked.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnRefreshClicked);
    }
    
    if (ClearFiltersButton)
    {
        ClearFiltersButton->OnClicked.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnClearFiltersClicked);
    }
    
    if (FixAllButton)
    {
        FixAllButton->OnClicked.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnFixAllClicked);
    }
    
    if (FixSelectedButton)
    {
        FixSelectedButton->OnClicked.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnFixSelectedClicked);
    }
    
    if (PreviewButton)
    {
        PreviewButton->OnClicked.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnPreviewClicked);
    }
    
    if (UndoButton)
    {
        UndoButton->OnClicked.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnUndoClicked);
    }
    
    if (RedoButton)
    {
        RedoButton->OnClicked.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnRedoClicked);
    }
    
    // ComboBox event handlers
    if (PlatformComboBox)
    {
        PlatformComboBox->OnSelectionChanged.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnPlatformChanged);
    }
    
    if (QualityLevelComboBox)
    {
        QualityLevelComboBox->OnSelectionChanged.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnQualityLevelChanged);
    }
    
    if (IssueTypeFilterComboBox)
    {
        IssueTypeFilterComboBox->OnSelectionChanged.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnIssueTypeFilterChanged);
    }
    
    if (SeverityFilterComboBox)
    {
        SeverityFilterComboBox->OnSelectionChanged.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnSeverityFilterChanged);
    }
    
    // Slider event handlers
    if (MemoryThresholdSlider)
    {
        MemoryThresholdSlider->OnValueChanged.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnMemoryThresholdChanged);
    }
    
    if (MaxAssetsSpinBox)
    {
        MaxAssetsSpinBox->OnValueChanged.AddDynamic(this, &UMagicOptimizerTextureAuditWidget::OnMaxAssetsChanged);
    }
    
    // ListView event handlers
    // Note: ListView selection events are handled differently in UE 5.6
    // We'll implement selection handling through Blueprint events or custom logic
}

void UMagicOptimizerTextureAuditWidget::PopulateComboBoxes()
{
    // Platform options
    if (PlatformComboBox)
    {
        PlatformComboBox->ClearOptions();
        PlatformComboBox->AddOption(TEXT("PC"));
        PlatformComboBox->AddOption(TEXT("Console"));
        PlatformComboBox->AddOption(TEXT("Mobile"));
        PlatformComboBox->AddOption(TEXT("Android"));
        PlatformComboBox->AddOption(TEXT("iOS"));
        PlatformComboBox->SetSelectedOption(TEXT("PC"));
    }
    
    // Quality level options
    if (QualityLevelComboBox)
    {
        QualityLevelComboBox->ClearOptions();
        QualityLevelComboBox->AddOption(TEXT("Low"));
        QualityLevelComboBox->AddOption(TEXT("Medium"));
        QualityLevelComboBox->AddOption(TEXT("High"));
        QualityLevelComboBox->AddOption(TEXT("Ultra"));
        QualityLevelComboBox->SetSelectedOption(TEXT("High"));
    }
    
    // Issue type filter options
    if (IssueTypeFilterComboBox)
    {
        IssueTypeFilterComboBox->ClearOptions();
        IssueTypeFilterComboBox->AddOption(TEXT("All Issues"));
        IssueTypeFilterComboBox->AddOption(TEXT("Resolution Problems"));
        IssueTypeFilterComboBox->AddOption(TEXT("Compression Issues"));
        IssueTypeFilterComboBox->AddOption(TEXT("Memory Usage"));
        IssueTypeFilterComboBox->AddOption(TEXT("Streaming Problems"));
        IssueTypeFilterComboBox->AddOption(TEXT("Format Issues"));
        IssueTypeFilterComboBox->SetSelectedOption(TEXT("All Issues"));
    }
    
    // Severity filter options
    if (SeverityFilterComboBox)
    {
        SeverityFilterComboBox->ClearOptions();
        SeverityFilterComboBox->AddOption(TEXT("All Severities"));
        SeverityFilterComboBox->AddOption(TEXT("Critical (0.8-1.0)"));
        SeverityFilterComboBox->AddOption(TEXT("High (0.6-0.8)"));
        SeverityFilterComboBox->AddOption(TEXT("Medium (0.4-0.6)"));
        SeverityFilterComboBox->AddOption(TEXT("Low (0.0-0.4)"));
        SeverityFilterComboBox->SetSelectedOption(TEXT("All Severities"));
    }
}

void UMagicOptimizerTextureAuditWidget::StartAudit()
{
    // AuditManager is static, always available
    
    if (bIsAuditing)
    {
        ShowStatusMessage(TEXT("Audit already in progress"), EAuditStatus::Running);
        return;
    }
    
    // Set up audit configuration
    FAuditConfig Config;
    Config.TargetPlatform = SelectedPlatform;
    
    // Convert string to enum
    if (SelectedQualityLevel == TEXT("Low"))
        Config.QualityLevel = EQualityLevel::Low;
    else if (SelectedQualityLevel == TEXT("Medium"))
        Config.QualityLevel = EQualityLevel::Medium;
    else if (SelectedQualityLevel == TEXT("High"))
        Config.QualityLevel = EQualityLevel::High;
    else if (SelectedQualityLevel == TEXT("Ultra"))
        Config.QualityLevel = EQualityLevel::Ultra;
    else
        Config.QualityLevel = EQualityLevel::High;
    
    // Set include paths
    FString IncludePaths = IncludePathsTextBox->GetText().ToString();
    if (!IncludePaths.IsEmpty())
    {
        Config.IncludePaths.Add(IncludePaths);
    }
    
    // Set asset types to audit
    if (IncludeTexturesCheckBox && IncludeTexturesCheckBox->IsChecked())
    {
        Config.IncludeAssetTypes.Add(EAssetType::Texture);
    }
    if (IncludeMaterialsCheckBox && IncludeMaterialsCheckBox->IsChecked())
    {
        Config.IncludeAssetTypes.Add(EAssetType::Material);
    }
    if (IncludeMeshesCheckBox && IncludeMeshesCheckBox->IsChecked())
    {
        Config.IncludeAssetTypes.Add(EAssetType::Mesh);
    }
    if (IncludeBlueprintsCheckBox && IncludeBlueprintsCheckBox->IsChecked())
    {
        Config.IncludeAssetTypes.Add(EAssetType::Blueprint);
    }
    
    // Start the audit
    bIsAuditing = true;
    UpdateButtonStates();
    UpdateStatusIndicator(EAuditStatus::Running);
    
    ShowStatusMessage(TEXT("Starting asset audit..."), EAuditStatus::Running);
    
    // Start progress update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(ProgressUpdateTimer, [this]()
        {
            // Update progress from audit manager (static methods)
            float Progress = UMagicOptimizerAuditManager::GetAuditProgress();
            FString StatusText = UMagicOptimizerAuditManager::GetCurrentAuditStatus();
            UpdateProgressDisplay(Progress, StatusText);
        }, 0.1f, true);
    }
    
    // Start the audit using the audit manager
    bool bStarted = UMagicOptimizerAuditManager::StartAssetAudit(Config);
    if (!bStarted)
    {
        bIsAuditing = false;
        UpdateButtonStates();
        UpdateStatusIndicator(EAuditStatus::Failed);
        ShowStatusMessage(TEXT("Failed to start audit"), EAuditStatus::Failed);
    }
    
    MAGIC_LOG(General, TEXT("Texture audit started"));
}

void UMagicOptimizerTextureAuditWidget::StopAudit()
{
    if (!bIsAuditing)
    {
        ShowStatusMessage(TEXT("No audit in progress"), EAuditStatus::Pending);
        return;
    }
    
    // Stop the audit
    bIsAuditing = false;
    UpdateButtonStates();
    UpdateStatusIndicator(EAuditStatus::Cancelled);
    
    // Clear progress timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ProgressUpdateTimer);
    }
    
    ShowStatusMessage(TEXT("Audit stopped by user"), EAuditStatus::Cancelled);
    
    MAGIC_LOG(General, TEXT("Texture audit stopped"));
}

void UMagicOptimizerTextureAuditWidget::ExportResults()
{
    if (!bHasResults)
    {
        ShowStatusMessage(TEXT("No results to export"), EAuditStatus::Pending);
        return;
    }
    
    // TODO: Implement export functionality
    ShowStatusMessage(TEXT("Export functionality not yet implemented"), EAuditStatus::Pending);
    
    MAGIC_LOG(General, TEXT("Export results requested"));
}

void UMagicOptimizerTextureAuditWidget::ApplyFixes()
{
    if (!bHasResults)
    {
        ShowStatusMessage(TEXT("No results to apply fixes to"), EAuditStatus::Pending);
        return;
    }
    
    // TODO: Implement fix application
    ShowStatusMessage(TEXT("Fix application not yet implemented"), EAuditStatus::Pending);
    
    MAGIC_LOG(General, TEXT("Apply fixes requested"));
}

void UMagicOptimizerTextureAuditWidget::RefreshResults()
{
    if (bHasResults)
    {
        UpdateResultsDisplay(CurrentResults);
        UpdateStatistics();
        ShowStatusMessage(TEXT("Results refreshed"), EAuditStatus::Completed);
    }
    else
    {
        ShowStatusMessage(TEXT("No results to refresh"), EAuditStatus::Pending);
    }
    
    MAGIC_LOG(General, TEXT("Results refreshed"));
}

void UMagicOptimizerTextureAuditWidget::FilterResults()
{
    if (!bHasResults)
    {
        return;
    }
    
    ApplyFilters();
    ShowStatusMessage(FString::Printf(TEXT("Filtered to %d assets"), FilteredResults.Num()), EAuditStatus::Completed);
    
    MAGIC_LOG(General, TEXT("Results filtered"));
}

void UMagicOptimizerTextureAuditWidget::ClearFilters()
{
    FilteredIssueType = EAuditIssueType::Info;
    FilteredSeverity = 0.0f;
    
    // Reset combo boxes
    if (IssueTypeFilterComboBox)
    {
        IssueTypeFilterComboBox->SetSelectedOption(TEXT("All Issues"));
    }
    if (SeverityFilterComboBox)
    {
        SeverityFilterComboBox->SetSelectedOption(TEXT("All Severities"));
    }
    
    // Refresh results
    FilterResults();
    ShowStatusMessage(TEXT("Filters cleared"), EAuditStatus::Completed);
    
    MAGIC_LOG(General, TEXT("Filters cleared"));
}

void UMagicOptimizerTextureAuditWidget::SelectAllAssets()
{
    // TODO: Implement select all functionality
    ShowStatusMessage(TEXT("Select all not yet implemented"), EAuditStatus::Pending);
}

void UMagicOptimizerTextureAuditWidget::DeselectAllAssets()
{
    // TODO: Implement deselect all functionality
    ShowStatusMessage(TEXT("Deselect all not yet implemented"), EAuditStatus::Pending);
}

void UMagicOptimizerTextureAuditWidget::UpdateProgressDisplay(float Progress, const FString& InStatusText)
{
    if (ProgressBar)
    {
        ProgressBar->SetPercent(Progress);
    }
    
    if (ProgressText)
    {
        ProgressText->SetText(FText::FromString(InStatusText));
    }
    
    // Show/hide loading throbber
    if (LoadingThrobber)
    {
        LoadingThrobber->SetVisibility(bIsAuditing ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

void UMagicOptimizerTextureAuditWidget::UpdateResultsDisplay(const TArray<FAuditResult>& Results)
{
    // Convert results to UI items
    FilteredResults.Empty();
    
    for (const FAuditResult& Result : Results)
    {
        UTextureAuditIssueItem* Item = NewObject<UTextureAuditIssueItem>();
        Item->AssetName = Result.AssetData.AssetName.ToString();
        Item->AssetPath = Result.AssetData.GetObjectPathString();
        Item->IssueCount = Result.Issues.Num();
        Item->MemoryUsageMB = FCString::Atof(*Result.Context.FindRef(TEXT("MemoryUsageMB"), TEXT("0.0")));
        Item->Status = Result.Status;
        Item->Issues = Result.Issues;
        Item->Recommendations = Result.Recommendations;
        
        // Determine severity color
        if (Result.Issues.Num() > 0)
        {
            float MaxSeverity = 0.0f;
            for (const FAuditIssue& Issue : Result.Issues)
            {
                MaxSeverity = FMath::Max(MaxSeverity, Issue.Severity);
            }
            
            if (MaxSeverity >= 0.8f)
            {
                Item->SeverityColor = FLinearColor::Red;
            }
            else if (MaxSeverity >= 0.6f)
            {
                Item->SeverityColor = FLinearColor(1.0f, 0.5f, 0.0f); // Orange
            }
            else if (MaxSeverity >= 0.4f)
            {
                Item->SeverityColor = FLinearColor::Yellow;
            }
            else
            {
                Item->SeverityColor = FLinearColor::Blue;
            }
        }
        else
        {
            Item->SeverityColor = FLinearColor::Green;
        }
        
        Item->StatusText = GetStatusText(Result.Status);
        Item->AssetType = Result.AssetData.AssetClassPath.GetAssetName().ToString();
        
        FilteredResults.Add(Item);
    }
    
    // Apply filters
    ApplyFilters();
    
    // Update list view
    if (ResultsListView)
    {
        ResultsListView->SetListItems(FilteredResults);
    }
    
    bHasResults = true;
    UpdateStatistics();
}

void UMagicOptimizerTextureAuditWidget::UpdateStatistics()
{
    // Calculate statistics
    TotalAssets = CurrentResults.Num();
    IssuesFound = 0;
    TotalMemoryUsage = 0.0f;
    EstimatedMemorySaved = 0.0f;
    
    for (const FAuditResult& Result : CurrentResults)
    {
        IssuesFound += Result.Issues.Num();
        TotalMemoryUsage += FCString::Atof(*Result.Context.FindRef(TEXT("MemoryUsageMB"), TEXT("0.0")));
        
        for (const FOptimizationRecommendation& Rec : Result.Recommendations)
        {
            EstimatedMemorySaved += Rec.EstimatedMemorySavingsMB;
        }
    }
    
    // Update UI
    if (TotalAssetsText)
    {
        TotalAssetsText->SetText(FText::FromString(FString::Printf(TEXT("%d"), TotalAssets)));
    }
    
    if (IssuesFoundText)
    {
        IssuesFoundText->SetText(FText::FromString(FString::Printf(TEXT("%d"), IssuesFound)));
    }
    
    if (MemorySavedText)
    {
        MemorySavedText->SetText(FText::FromString(FormatMemorySize(EstimatedMemorySaved)));
    }
    
    if (ProcessingTimeText)
    {
        ProcessingTimeText->SetText(FText::FromString(FormatTime(ProcessingTime)));
    }
}

void UMagicOptimizerTextureAuditWidget::UpdateIssueDetails(UTextureAuditIssueItem* InSelectedItem)
{
    if (!InSelectedItem)
    {
        return;
    }
    
    if (SelectedAssetNameText)
    {
        SelectedAssetNameText->SetText(FText::FromString(InSelectedItem->AssetName));
    }
    
    if (SelectedAssetPathText)
    {
        SelectedAssetPathText->SetText(FText::FromString(InSelectedItem->AssetPath));
    }
    
    if (SelectedAssetMemoryText)
    {
        SelectedAssetMemoryText->SetText(FText::FromString(FormatMemorySize(InSelectedItem->MemoryUsageMB)));
    }
    
    if (SelectedAssetIssuesText)
    {
        FString IssuesText = TEXT("");
        for (int32 i = 0; i < InSelectedItem->Issues.Num(); ++i)
        {
            if (i > 0) IssuesText += TEXT("\n");
            IssuesText += FString::Printf(TEXT("• %s (Severity: %.1f)"), *InSelectedItem->Issues[i].Title, InSelectedItem->Issues[i].Severity);
        }
        SelectedAssetIssuesText->SetText(FText::FromString(IssuesText));
    }
    
    if (SelectedAssetRecommendationsText)
    {
        FString RecommendationsText = TEXT("");
        for (int32 i = 0; i < InSelectedItem->Recommendations.Num(); ++i)
        {
            if (i > 0) RecommendationsText += TEXT("\n");
            RecommendationsText += FString::Printf(TEXT("• %s (Savings: %.1f MB)"), *InSelectedItem->Recommendations[i].Title, InSelectedItem->Recommendations[i].EstimatedMemorySavingsMB);
        }
        SelectedAssetRecommendationsText->SetText(FText::FromString(RecommendationsText));
    }
    
    this->SelectedItem = InSelectedItem;
}

void UMagicOptimizerTextureAuditWidget::ApplyFilters()
{
    TArray<UTextureAuditIssueItem*> FilteredItems;
    
    for (UTextureAuditIssueItem* Item : FilteredResults)
    {
        bool bPassesFilter = true;
        
        // Filter by issue type
        if (FilteredIssueType != EAuditIssueType::Info)
        {
            bool bHasMatchingIssue = false;
            for (const FAuditIssue& Issue : Item->Issues)
            {
                if (Issue.Type == FilteredIssueType)
                {
                    bHasMatchingIssue = true;
                    break;
                }
            }
            if (!bHasMatchingIssue)
            {
                bPassesFilter = false;
            }
        }
        
        // Filter by severity
        if (FilteredSeverity > 0.0f)
        {
            bool bHasMatchingSeverity = false;
            for (const FAuditIssue& Issue : Item->Issues)
            {
                if (Issue.Severity >= FilteredSeverity)
                {
                    bHasMatchingSeverity = true;
                    break;
                }
            }
            if (!bHasMatchingSeverity)
            {
                bPassesFilter = false;
            }
        }
        
        // Filter by memory threshold
        if (Item->MemoryUsageMB < MemoryThreshold)
        {
            bPassesFilter = false;
        }
        
        if (bPassesFilter)
        {
            FilteredItems.Add(Item);
        }
    }
    
    // Update list view with filtered items
    if (ResultsListView)
    {
        ResultsListView->SetListItems(FilteredItems);
    }
}

void UMagicOptimizerTextureAuditWidget::UpdateButtonStates()
{
    if (StartAuditButton)
    {
        StartAuditButton->SetIsEnabled(!bIsAuditing);
    }
    
    if (StopAuditButton)
    {
        StopAuditButton->SetIsEnabled(bIsAuditing);
    }
    
    if (ExportButton)
    {
        ExportButton->SetIsEnabled(bHasResults);
    }
    
    if (ApplyFixesButton)
    {
        ApplyFixesButton->SetIsEnabled(bHasResults);
    }
    
    if (FixAllButton)
    {
        FixAllButton->SetIsEnabled(SelectedItem != nullptr);
    }
    
    if (FixSelectedButton)
    {
        FixSelectedButton->SetIsEnabled(SelectedItem != nullptr);
    }
}

void UMagicOptimizerTextureAuditWidget::ShowStatusMessage(const FString& Message, EAuditStatus Status)
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(Message));
    }
    
    UpdateStatusIndicator(Status);
    
    MAGIC_LOG(General, FString::Printf(TEXT("Status: %s"), *Message));
}

void UMagicOptimizerTextureAuditWidget::UpdateStatusIndicator(EAuditStatus Status)
{
    if (StatusBorder)
    {
        StatusBorder->SetBrushColor(GetStatusColor(Status));
    }
    
    if (StatusIcon)
    {
        // TODO: Set appropriate icon based on status
    }
}

FLinearColor UMagicOptimizerTextureAuditWidget::GetStatusColor(EAuditStatus Status)
{
    switch (Status)
    {
        case EAuditStatus::Pending:
            return FLinearColor::Blue;
        case EAuditStatus::Running:
            return FLinearColor::Yellow;
        case EAuditStatus::Completed:
            return FLinearColor::Green;
        case EAuditStatus::Failed:
            return FLinearColor::Red;
        case EAuditStatus::Cancelled:
            return FLinearColor(0.5f, 0.5f, 0.5f); // Gray
        default:
            return FLinearColor::White;
    }
}

FString UMagicOptimizerTextureAuditWidget::GetStatusText(EAuditStatus Status)
{
    switch (Status)
    {
        case EAuditStatus::Pending:
            return TEXT("Pending");
        case EAuditStatus::Running:
            return TEXT("Running");
        case EAuditStatus::Completed:
            return TEXT("Completed");
        case EAuditStatus::Failed:
            return TEXT("Failed");
        case EAuditStatus::Cancelled:
            return TEXT("Cancelled");
        default:
            return TEXT("Unknown");
    }
}

FString UMagicOptimizerTextureAuditWidget::FormatMemorySize(float SizeInMB)
{
    if (SizeInMB >= 1024.0f)
    {
        return FString::Printf(TEXT("%.1f GB"), SizeInMB / 1024.0f);
    }
    else
    {
        return FString::Printf(TEXT("%.1f MB"), SizeInMB);
    }
}

FString UMagicOptimizerTextureAuditWidget::FormatTime(float TimeInSeconds)
{
    if (TimeInSeconds >= 60.0f)
    {
        int32 Minutes = FMath::FloorToInt(TimeInSeconds / 60.0f);
        float Seconds = FMath::Fmod(TimeInSeconds, 60.0f);
        return FString::Printf(TEXT("%dm %.1fs"), Minutes, Seconds);
    }
    else
    {
        return FString::Printf(TEXT("%.1fs"), TimeInSeconds);
    }
}

// Event Handlers
void UMagicOptimizerTextureAuditWidget::OnStartAuditClicked()
{
    StartAudit();
}

void UMagicOptimizerTextureAuditWidget::OnStopAuditClicked()
{
    StopAudit();
}

void UMagicOptimizerTextureAuditWidget::OnExportClicked()
{
    ExportResults();
}

void UMagicOptimizerTextureAuditWidget::OnApplyFixesClicked()
{
    ApplyFixes();
}

void UMagicOptimizerTextureAuditWidget::OnRefreshClicked()
{
    RefreshResults();
}

void UMagicOptimizerTextureAuditWidget::OnClearFiltersClicked()
{
    ClearFilters();
}

void UMagicOptimizerTextureAuditWidget::OnPlatformChanged(FString InSelectedItem, ESelectInfo::Type SelectionType)
{
    SelectedPlatform = InSelectedItem;
    MAGIC_LOG(General, FString::Printf(TEXT("Platform changed to: %s"), *SelectedPlatform));
}

void UMagicOptimizerTextureAuditWidget::OnQualityLevelChanged(FString InSelectedItem, ESelectInfo::Type SelectionType)
{
    SelectedQualityLevel = InSelectedItem;
    MAGIC_LOG(General, FString::Printf(TEXT("Quality level changed to: %s"), *SelectedQualityLevel));
}

void UMagicOptimizerTextureAuditWidget::OnIssueTypeFilterChanged(FString InSelectedItem, ESelectInfo::Type SelectionType)
{
    // Map string to enum
    if (InSelectedItem == TEXT("All Issues"))
    {
        FilteredIssueType = EAuditIssueType::Info;
    }
    else if (InSelectedItem == TEXT("Resolution Problems"))
    {
        FilteredIssueType = EAuditIssueType::Warning;
    }
    else if (InSelectedItem == TEXT("Compression Issues"))
    {
        FilteredIssueType = EAuditIssueType::Warning;
    }
    else if (InSelectedItem == TEXT("Memory Usage"))
    {
        FilteredIssueType = EAuditIssueType::Error;
    }
    else if (InSelectedItem == TEXT("Streaming Problems"))
    {
        FilteredIssueType = EAuditIssueType::Warning;
    }
    else if (InSelectedItem == TEXT("Format Issues"))
    {
        FilteredIssueType = EAuditIssueType::Error;
    }
    
    FilterResults();
}

void UMagicOptimizerTextureAuditWidget::OnSeverityFilterChanged(FString InSelectedItem, ESelectInfo::Type SelectionType)
{
    // Map string to severity value
    if (InSelectedItem == TEXT("All Severities"))
    {
        FilteredSeverity = 0.0f;
    }
    else if (InSelectedItem == TEXT("Critical (0.8-1.0)"))
    {
        FilteredSeverity = 0.8f;
    }
    else if (InSelectedItem == TEXT("High (0.6-0.8)"))
    {
        FilteredSeverity = 0.6f;
    }
    else if (InSelectedItem == TEXT("Medium (0.4-0.6)"))
    {
        FilteredSeverity = 0.4f;
    }
    else if (InSelectedItem == TEXT("Low (0.0-0.4)"))
    {
        FilteredSeverity = 0.0f;
    }
    
    FilterResults();
}

void UMagicOptimizerTextureAuditWidget::OnMemoryThresholdChanged(float Value)
{
    MemoryThreshold = Value;
    
    if (MemoryThresholdText)
    {
        MemoryThresholdText->SetText(FText::FromString(FString::Printf(TEXT("%.1f MB"), MemoryThreshold)));
    }
    
    FilterResults();
}

void UMagicOptimizerTextureAuditWidget::OnMaxAssetsChanged(float Value)
{
    MAGIC_LOG(General, FString::Printf(TEXT("Max assets changed to: %.0f"), Value));
}

void UMagicOptimizerTextureAuditWidget::OnResultsListSelectionChanged(UObject* InSelectedItem)
{
    UTextureAuditIssueItem* Item = Cast<UTextureAuditIssueItem>(InSelectedItem);
    UpdateIssueDetails(Item);
}

void UMagicOptimizerTextureAuditWidget::OnFixAllClicked()
{
    // TODO: Implement fix all functionality
    ShowStatusMessage(TEXT("Fix all not yet implemented"), EAuditStatus::Pending);
}

void UMagicOptimizerTextureAuditWidget::OnFixSelectedClicked()
{
    // TODO: Implement fix selected functionality
    ShowStatusMessage(TEXT("Fix selected not yet implemented"), EAuditStatus::Pending);
}

void UMagicOptimizerTextureAuditWidget::OnPreviewClicked()
{
    // TODO: Implement preview functionality
    ShowStatusMessage(TEXT("Preview not yet implemented"), EAuditStatus::Pending);
}

void UMagicOptimizerTextureAuditWidget::OnUndoClicked()
{
    // TODO: Implement undo functionality
    ShowStatusMessage(TEXT("Undo not yet implemented"), EAuditStatus::Pending);
}

void UMagicOptimizerTextureAuditWidget::OnRedoClicked()
{
    // TODO: Implement redo functionality
    ShowStatusMessage(TEXT("Redo not yet implemented"), EAuditStatus::Pending);
}
