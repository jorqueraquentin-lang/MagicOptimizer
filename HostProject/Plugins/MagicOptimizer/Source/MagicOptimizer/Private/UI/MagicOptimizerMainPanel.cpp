#include "UI/MagicOptimizerMainPanel.h"
#include "UI/MagicOptimizerHeader.h"
#include "UI/MagicOptimizerLeftRail.h"
#include "UI/MagicOptimizerCenterTabs.h"
#include "UI/MagicOptimizerRightShelf.h"
#include "UI/MagicOptimizerCommandPalette.h"
#include "UI/MagicOptimizerScopeSheet.h"
#include "UI/MagicOptimizerPlanDialog.h"
#include "UI/MagicOptimizerTaskDrawer.h"
#include "UI/MagicOptimizerSlideOver.h"
#include "MagicOptimizerAuditManager.h"
#include "Model/AuditConfig.h"
#include "Model/AuditResult.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/UICommandList.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SOverlay.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "HAL/PlatformApplicationMisc.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerMainPanel"

void SMagicOptimizerMainPanel::Construct(const FArguments& InArgs)
{
    // Initialize data
    InitializeData();

    // Set up animations
    DrawerAnim = FCurveSequence();
    DrawerAlpha = DrawerAnim.AddCurve(0.0f, 0.3f, ECurveEaseFunction::QuadInOut);
    
    ProgressAnim = FCurveSequence();
    ProgressAlpha = ProgressAnim.AddCurve(0.0f, 0.2f, ECurveEaseFunction::QuadInOut);

    // Bind commands
    BindCommands();

    // Create child widgets
    HeaderWidget = SNew(SMagicOptimizerHeader)
        .CurrentPreset(CurrentPreset)
        .SelectedTargets(SelectedTargets)
        .ScopeSummary(ScopeSummary)
        .ChangeCap(ChangeCap)
        .ProgressData(ProgressData)
        .OnPresetChanged(this, &SMagicOptimizerMainPanel::OnPresetChanged)
        .OnTargetToggled(this, &SMagicOptimizerMainPanel::OnTargetToggled)
        .OnScopeChanged(this, &SMagicOptimizerMainPanel::OnScopeChanged)
        .OnChangeCapChanged_Lambda([this](int32 NewCap) { ChangeCap = NewCap; })
        .OnRunScan(this, &SMagicOptimizerMainPanel::OnRunScan)
        .OnAutoFix(this, &SMagicOptimizerMainPanel::OnAutoFix)
        .OnPreview(this, &SMagicOptimizerMainPanel::OnPreview)
        .OnTasks(this, &SMagicOptimizerMainPanel::OnTasks);

    LeftRailWidget = SNew(SMagicOptimizerLeftRail)
        .OnScopeChanged(this, &SMagicOptimizerMainPanel::OnScopeChanged);

    CenterTabsWidget = SNew(SMagicOptimizerCenterTabs)
        .ActiveTabIndex(ActiveTabIndex)
        .TextureData(GetFilteredTextures())
        .SelectedTextureIds(SelectedTextureIds)
        .SearchText(SearchText)
        .OnTabChanged(this, &SMagicOptimizerMainPanel::OnTabChanged)
        .OnSearchTextChanged(this, &SMagicOptimizerMainPanel::OnSearchTextChanged)
        .OnTextureSelectionChanged(this, &SMagicOptimizerMainPanel::OnTextureSelectionChanged);
        // OnTextureRowClicked not available for CenterTabs widget

    RightShelfWidget = SNew(SMagicOptimizerRightShelf)
        .SelectedTextureIds(SelectedTextureIds)
        .OnPreview(this, &SMagicOptimizerMainPanel::OnPreview);

    // Create modal widgets
    CommandPaletteWidget = SNew(SMagicOptimizerCommandPalette)
        .IsOpen(bCommandPaletteOpen)
        .OnClose(this, &SMagicOptimizerMainPanel::OnCloseCommandPalette)
        .OnExecuteCommand(this, &SMagicOptimizerMainPanel::OnExecuteCommand);

    ScopeSheetWidget = SNew(SMagicOptimizerScopeSheet)
        .IsOpen(bScopeSheetOpen)
        .OnClose(this, &SMagicOptimizerMainPanel::OnCloseScopeSheet)
        .OnApply(this, &SMagicOptimizerMainPanel::OnScopeChanged);

    PlanDialogWidget = SNew(SMagicOptimizerPlanDialog)
        .IsOpen(bPlanDialogOpen)
        .OnClose(this, &SMagicOptimizerMainPanel::OnClosePlanDialog);

    TaskDrawerWidget = SNew(SMagicOptimizerTaskDrawer)
        .IsOpen(bTaskDrawerOpen)
        .Tasks(GetActiveTasks())
        .OnClose(this, &SMagicOptimizerMainPanel::OnCloseTaskDrawer);

    SlideOverWidget = SNew(SMagicOptimizerSlideOver)
        .IsOpen(bDrawerOpen)
        .SelectedTexture_Lambda([this]() -> FUITextureRow
        {
            // Return the first selected texture, or empty if none selected
            if (SelectedTextureIds.Num() > 0)
            {
                for (const FUITextureRow& Texture : TextureData)
                {
                    if (Texture.Id == SelectedTextureIds[0])
                    {
                        return Texture;
                    }
                }
            }
            return FUITextureRow{};
        })
        .TargetPlatforms_Lambda([this]() -> TArray<FString>
        {
            return TArray<FString>{TEXT("PS5"), TEXT("XBOX"), TEXT("PC"), TEXT("Quest"), TEXT("Switch")};
        })
        .OnClose(this, &SMagicOptimizerMainPanel::OnCloseDrawer)
        .OnApplyToAsset_Lambda([this](const FString& TextureId, const FTextureSettings& Settings)
        {
            OnApplyTextureSettings(TextureId, Settings);
        });

    ChildSlot
    [
        SNew(SBorder)
        .Padding(0)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .ColorAndOpacity(FLinearColor(0.02f, 0.02f, 0.02f, 1.0f))
        .BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f, 1.0f))
        [
            SNew(SOverlay)
            
            // Main content
            +SOverlay::Slot()
            [
                MakeMainContent()
            ]
            
            // Drawer overlay
            +SOverlay::Slot()
            [
                MakeDrawerOverlay()
            ]
            
            // Modal overlays
            +SOverlay::Slot()
            [
                MakeModalOverlay()
            ]
        ]
    ];
    
    // Set keyboard focus to this widget to handle global shortcuts
    FSlateApplication::Get().SetKeyboardFocus(AsShared(), EFocusCause::SetDirectly);
}

TSharedRef<SWidget> SMagicOptimizerMainPanel::MakeMainContent()
{
    return SNew(SVerticalBox)
    
    // Header
    +SVerticalBox::Slot()
    .AutoHeight()
    [
        HeaderWidget.ToSharedRef()
    ]
    
    // 3-column layout
    +SVerticalBox::Slot()
    .FillHeight(1.0f)
    .Padding(4.0f, 0.0f, 4.0f, 4.0f)
    [
        SNew(SSplitter)
        .Orientation(Orient_Horizontal)
        
        // Left rail
        +SSplitter::Slot()
        .Value(0.22f)
        .MinSize(200.0f)
        [
            LeftRailWidget.ToSharedRef()
        ]
        
        // Center tabs
        +SSplitter::Slot()
        .Value(0.58f)
        .MinSize(400.0f)
        [
            CenterTabsWidget.ToSharedRef()
        ]
        
        // Right shelf
        +SSplitter::Slot()
        .Value(0.20f)
        .MinSize(250.0f)
        [
            RightShelfWidget.ToSharedRef()
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerMainPanel::MakeDrawerOverlay()
{
    return SNew(SOverlay)
    
    // Slide-over drawer
    +SOverlay::Slot()
    .HAlign(HAlign_Right)
    .VAlign(VAlign_Fill)
    [
        SlideOverWidget.ToSharedRef()
    ];
}

TSharedRef<SWidget> SMagicOptimizerMainPanel::MakeModalOverlay()
{
    return SNew(SOverlay)
    
    // Command palette
    +SOverlay::Slot()
    .HAlign(HAlign_Center)
    .VAlign(VAlign_Center)
    [
        CommandPaletteWidget.ToSharedRef()
    ]
    
    // Scope sheet
    +SOverlay::Slot()
    .HAlign(HAlign_Center)
    .VAlign(VAlign_Center)
    [
        ScopeSheetWidget.ToSharedRef()
    ]
    
    // Plan dialog
    +SOverlay::Slot()
    .HAlign(HAlign_Center)
    .VAlign(VAlign_Center)
    [
        PlanDialogWidget.ToSharedRef()
    ]
    
    // Task drawer
    +SOverlay::Slot()
    .HAlign(HAlign_Center)
    .VAlign(VAlign_Center)
    [
        TaskDrawerWidget.ToSharedRef()
    ];
}

void SMagicOptimizerMainPanel::InitializeData()
{
    // Initialize target chips
    TargetChips = {
        FTargetChip(TEXT("ps5"), TEXT("PS5"), TEXT("Gamepad2")),
        FTargetChip(TEXT("xbox"), TEXT("Xbox"), TEXT("Gamepad2")),
        FTargetChip(TEXT("pc"), TEXT("PC"), TEXT("Cpu")),
        FTargetChip(TEXT("quest"), TEXT("Quest"), TEXT("Smartphone")),
        FTargetChip(TEXT("switch"), TEXT("Switch"), TEXT("Smartphone"))
    };

    // Initialize preset options
    PresetOptions = {
        FPresetOption(TEXT("pc_high"), TEXT("PC High"), TEXT("High quality settings for PC")),
        FPresetOption(TEXT("console_balanced"), TEXT("Console Balanced"), TEXT("Balanced settings for consoles")),
        FPresetOption(TEXT("mobile_low"), TEXT("Mobile Low"), TEXT("Optimized settings for mobile")),
        FPresetOption(TEXT("vr_crisp"), TEXT("VR Crisp UI"), TEXT("VR-optimized with crisp UI")),
        FPresetOption(TEXT("cinematic"), TEXT("Cinematic Preview"), TEXT("Cinematic quality preview"))
    };

    // Initialize audit manager
    UMagicOptimizerAuditManager::Initialize();
    
    // TODO: Bind to audit manager delegates when audit system is implemented
    // UMagicOptimizerAuditManager::OnAuditProgress.AddLambda([this](float Progress)
    // {
    //     ProgressData.Progress = Progress * 100.0f;
    //     ProgressData.bRunning = Progress < 1.0f;
    //     ProgressData.Message = FString::Printf(TEXT("Scanning... %d%%"), FMath::RoundToInt(Progress * 100.0f));
    // });
    
    // TODO: Bind audit completion delegate when audit system is implemented
    // UMagicOptimizerAuditManager::OnAuditCompleted.AddLambda([this](const TArray<FAuditResult>& Results)
    // {
    //     ProgressData.bRunning = false;
    //     ProgressData.Progress = 100.0f;
    //     ProgressData.Message = TEXT("Scan complete");
    //     
    //     // Convert audit results to UI data
    //     ConvertAuditResultsToUIData(Results);
    // });
    
    // TODO: Bind audit failed delegate when audit system is implemented
    // UMagicOptimizerAuditManager::OnAuditFailed.AddLambda([this](const FString& ErrorMessage)
    // {
    //     ProgressData.bRunning = false;
    //     ProgressData.Progress = 0.0f;
    //     ProgressData.Message = FString::Printf(TEXT("Scan failed: %s"), *ErrorMessage);
    //     
    //     UE_LOG(LogTemp, Error, TEXT("Audit failed: %s"), *ErrorMessage);
    // });

    // Try to load existing audit data, or generate mock data if none available
    TArray<FAuditResult> ExistingResults = UMagicOptimizerAuditManager::GetAuditResults();
    if (ExistingResults.Num() > 0)
    {
        // Use UIDataProvider to convert audit results to UI data
        TArray<FUITextureRow> UITextureRows;
        TArray<FUIHealthPoint> UIHealthPoints;
        TArray<FUIBeforeAfter> UIBeforeAfter;
        FUIBudgetGauges UIGauges;
        
        FUIDataProvider::FromAuditResults(ExistingResults, UITextureRows, UIHealthPoints, UIBeforeAfter, UIGauges);
        
        // Convert UI data to internal format
        ConvertUIDataToInternal(UITextureRows, UIHealthPoints, UIBeforeAfter, UIGauges);
    }
    else
    {
        // Generate mock data for development/testing
        GenerateMockData();
    }
}

void SMagicOptimizerMainPanel::GenerateMockData()
{
    // Generate mock texture data
    TextureData.Empty();
    for (int32 i = 1; i <= 80; ++i)
    {
        FUITextureRow Texture;
        Texture.Id = FString::Printf(TEXT("tex_%d"), i);
        Texture.Path = FString::Printf(TEXT("/Game/Props/T_%03d"), i);
        
        const TArray<FString> Sizes = {TEXT("512"), TEXT("1k"), TEXT("2k"), TEXT("4k")};
        const TArray<FString> Groups = {TEXT("World"), TEXT("UI"), TEXT("Effects"), TEXT("Characters"), TEXT("Props")};
        const TArray<FString> Compressions = {TEXT("Default"), TEXT("Masks"), TEXT("UserInterface"), TEXT("Normal"), TEXT("HDR")};
        
        Texture.Size = Sizes[FMath::RandRange(0, Sizes.Num() - 1)];
        Texture.Group = Groups[FMath::RandRange(0, Groups.Num() - 1)];
        Texture.Compression = Compressions[FMath::RandRange(0, Compressions.Num() - 1)];
        Texture.bSRGB = FMath::RandBool();
        Texture.bMips = FMath::RandRange(0, 10) > 3;
        Texture.bVT = FMath::RandRange(0, 10) > 6;
        Texture.LOD = FMath::RandRange(0, 2);
        Texture.Referencers = FMath::RandRange(0, 15);
        Texture.Issues = FMath::RandRange(0, 3);
        
        TextureData.Add(Texture);
    }

    // Generate health data
    HealthData = CalculateHealthData();

    // Generate task data
    TaskData.Empty();
    // Add some mock tasks if needed
}

void SMagicOptimizerMainPanel::ConvertUIDataToInternal(const TArray<FUITextureRow>& UITextureRows, const TArray<FUIHealthPoint>& UIHealthPoints, const TArray<FUIBeforeAfter>& UIBeforeAfter, const FUIBudgetGauges& UIGauges)
{
    // Convert UI texture rows to internal texture data
    TextureData.Empty();
    for (const FUITextureRow& Row : UITextureRows)
    {
        FUITextureRow Texture;
        Texture.Id = Row.Id;
        Texture.Path = Row.Path;
        Texture.Size = Row.Size;
        Texture.Group = Row.Group;
        Texture.Compression = Row.Compression;
        Texture.bSRGB = Row.bSRGB;
        Texture.bMips = Row.bMips;
        Texture.bVT = Row.bVT;
        Texture.LOD = Row.LOD;
        Texture.Referencers = Row.Referencers;
        Texture.Issues = Row.Issues;
        TextureData.Add(Texture);
    }
    
    // Convert health data
    HealthData.Score = UIHealthPoints.Num() > 0 ? UIHealthPoints.Last().Score : 50;
    HealthData.History.Empty();
    for (const FUIHealthPoint& Point : UIHealthPoints)
    {
        HealthData.History.Add(Point.Score);
    }
    
    // Generate mock health data for missing fields
    HealthData.TopOffenders = {TEXT("Masks with sRGB on"), TEXT("Normals not BC5"), TEXT("Non VT assets flagged VT")};
    HealthData.OffenderCounts = {45, 82, 16};
    HealthData.Summary = FString::Printf(TEXT("%d assets scanned"), TextureData.Num());
    
    // Generate trend data from health points
    HealthData.Trend.Empty();
    for (int32 i = 0; i < 7; ++i)
    {
        int32 Index = FMath::Clamp(i * UIHealthPoints.Num() / 7, 0, UIHealthPoints.Num() - 1);
        HealthData.Trend.Add(UIHealthPoints[Index].Score);
    }
    
    // Refresh UI components
    RefreshData();
    
    UE_LOG(LogTemp, Log, TEXT("Converted %d UI texture rows to internal data"), UITextureRows.Num());
}

void SMagicOptimizerMainPanel::ConvertAuditResultsToUIData(const TArray<FAuditResult>& Results)
{
    // Use UIDataProvider to convert audit results to UI data
    TArray<FUITextureRow> UITextureRows;
    TArray<FUIHealthPoint> UIHealthPoints;
    TArray<FUIBeforeAfter> UIBeforeAfter;
    FUIBudgetGauges UIGauges;
    
    FUIDataProvider::FromAuditResults(Results, UITextureRows, UIHealthPoints, UIBeforeAfter, UIGauges);
    
    // Convert UI data to internal format
    ConvertUIDataToInternal(UITextureRows, UIHealthPoints, UIBeforeAfter, UIGauges);
}

FHealthData SMagicOptimizerMainPanel::CalculateHealthFromAuditResults(const TArray<FAuditResult>& Results) const
{
    FHealthData Health;
    
    if (Results.Num() == 0)
    {
        // Default health when no results
        Health.Score = 50;
        Health.TopOffenders = {TEXT("No data available")};
        Health.Summary = TEXT("No audit data available");
        Health.Trend = {50, 50, 50, 50, 50, 50, 50};
        return Health;
    }
    
    // Calculate health score based on issues found
    int32 TotalIssues = 0;
    int32 CriticalIssues = 0;
    int32 TotalAssets = Results.Num();
    TArray<FString> TopOffendersList;
    
    for (const FAuditResult& Result : Results)
    {
        TotalIssues += Result.Issues.Num();
        
        for (const FAuditIssue& Issue : Result.Issues)
        {
            if (Issue.Severity >= 0.8f)
            {
                CriticalIssues++;
                
                // Add to top offenders if it's a critical issue
                FString OffenderName = FPaths::GetBaseFilename(Result.GetAssetPath());
                if (!TopOffendersList.Contains(OffenderName) && TopOffendersList.Num() < 5)
                {
                    TopOffendersList.Add(OffenderName);
                }
            }
        }
    }
    
    // Calculate score (higher is better, lower issues = higher score)
    float IssueRatio = TotalAssets > 0 ? (float)TotalIssues / (float)TotalAssets : 0.0f;
    Health.Score = FMath::Clamp(100 - FMath::RoundToInt(IssueRatio * 20.0f), 0, 100);
    
    // Adjust score based on critical issues
    if (CriticalIssues > 0)
    {
        Health.Score = FMath::Max(Health.Score - (CriticalIssues * 5), 20);
    }
    
    Health.TopOffenders = TopOffendersList.Num() > 0 ? TopOffendersList : TArray<FString>{TEXT("No critical issues")};
    
    Health.Summary = FString::Printf(TEXT("%d assets scanned, %d issues found"), 
        TotalAssets, TotalIssues);
    
    // Generate trend data (mock for now, could be stored historically)
    int32 BaseScore = Health.Score;
    Health.Trend = {
        FMath::Max(BaseScore - 10, 0),
        FMath::Max(BaseScore - 8, 0),
        FMath::Max(BaseScore - 5, 0),
        FMath::Max(BaseScore - 3, 0),
        FMath::Max(BaseScore - 1, 0),
        BaseScore,
        BaseScore
    };
    
    return Health;
}

FHealthData SMagicOptimizerMainPanel::CalculateHealthData() const
{
    FHealthData Health;
    Health.Score = 78;
    
    // Generate mock history data
    for (int32 i = 0; i < 20; ++i)
    {
        Health.History.Add(FMath::RandRange(60, 90));
    }
    
    Health.TopOffenders = {
        TEXT("Masks with sRGB on"),
        TEXT("Normals not BC5"),
        TEXT("Non VT assets flagged VT")
    };
    
    Health.OffenderCounts = {45, 82, 16};
    
    return Health;
}

void SMagicOptimizerMainPanel::RefreshData()
{
    // Refresh all child widgets with updated data
    if (CenterTabsWidget.IsValid())
    {
        // Force refresh of center tabs data
        CenterTabsWidget->SetAttribute(TEXT("TextureData"), TAttribute<TArray<FUITextureRow>>::Create(
            TAttribute<TArray<FUITextureRow>>::FGetter::CreateLambda([this]() -> TArray<FUITextureRow>
            {
                return GetFilteredTextures();
            })
        ));
    }
    
    if (LeftRailWidget.IsValid())
    {
        // Update left rail if it has data dependencies
    }
    
    if (RightShelfWidget.IsValid())
    {
        // Update right shelf with new selection data
    }
    
    UE_LOG(LogTemp, Log, TEXT("Refreshed UI data - %d textures, health score: %d"), 
        TextureData.Num(), HealthData.Score);
}

TArray<FTaskData> SMagicOptimizerMainPanel::GetActiveTasks() const
{
    return TaskData;
}

TArray<FUITextureRow> SMagicOptimizerMainPanel::GetFilteredTextures() const
{
    if (SearchText.IsEmpty())
    {
        return TextureData;
    }
    
    TArray<FUITextureRow> Filtered;
    const FString SearchLower = SearchText.ToLower();
    
    for (const FUITextureRow& Texture : TextureData)
    {
        if (Texture.Path.ToLower().Contains(SearchLower) ||
            Texture.Group.ToLower().Contains(SearchLower) ||
            Texture.Compression.ToLower().Contains(SearchLower))
        {
            Filtered.Add(Texture);
        }
    }
    
    return Filtered;
}

TArray<FUITextureRow> SMagicOptimizerMainPanel::GetSelectedTextures() const
{
    TArray<FUITextureRow> Selected;
    
    for (const FString& Id : SelectedTextureIds)
    {
        for (const FUITextureRow& Texture : TextureData)
        {
            if (Texture.Id == Id)
            {
                Selected.Add(Texture);
                break;
            }
        }
    }
    
    return Selected;
}

FReply SMagicOptimizerMainPanel::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();
    const bool bCtrl = InKeyEvent.IsControlDown();
    const bool bShift = InKeyEvent.IsShiftDown();
    
    // Handle Ctrl+Shift+P for command palette
    if (Key == EKeys::P && bCtrl && bShift)
    {
        OpenCommandPalette();
        return FReply::Handled();
    }
    
    return FReply::Unhandled();
}

void SMagicOptimizerMainPanel::BindCommands()
{
    Commands = MakeShared<FUICommandList>();
    
    // Ctrl+Shift+P opens command palette
    const FInputChord OpenPaletteChord(EKeys::P, EModifierKey::Control | EModifierKey::Shift);
    Commands->MapAction(
        FExecuteAction::CreateSP(this, &SMagicOptimizerMainPanel::OpenCommandPalette),
        FCanExecuteAction(),
        FIsActionChecked(),
        OpenPaletteChord
    );
    
    // Register commands with Slate
    FSlateApplication::Get().SetKeyboardFocus(AsShared(), EFocusCause::SetDirectly);
}

// Event handlers
void SMagicOptimizerMainPanel::OnPresetChanged(const FString& NewPreset)
{
    CurrentPreset = NewPreset;
    OnPresetChangedDelegate.ExecuteIfBound(NewPreset);
}

void SMagicOptimizerMainPanel::OnTargetToggled(const FString& TargetId)
{
    if (SelectedTargets.Contains(TargetId))
    {
        SelectedTargets.Remove(TargetId);
    }
    else
    {
        SelectedTargets.Add(TargetId);
    }
    OnTargetToggledDelegate.ExecuteIfBound(TargetId);
}

void SMagicOptimizerMainPanel::OnScopeChanged(const FString& NewScope)
{
    ScopeSummary = NewScope;
    OnScopeChangedDelegate.ExecuteIfBound(NewScope);
}

void SMagicOptimizerMainPanel::OnSearchTextChanged(const FText& NewText)
{
    SearchText = NewText.ToString();
    OnSearchTextChangedDelegate.ExecuteIfBound(NewText);
    
    // Refresh center tabs with filtered data
    if (CenterTabsWidget.IsValid())
    {
        CenterTabsWidget->RefreshData(GetFilteredTextures());
    }
}

void SMagicOptimizerMainPanel::OnTextureSelectionChanged(const TArray<FString>& NewSelection)
{
    SelectedTextureIds = NewSelection;
    OnTextureSelectionChangedDelegate.ExecuteIfBound(NewSelection);
    
    // Refresh right shelf with new selection
    if (RightShelfWidget.IsValid())
    {
        RightShelfWidget->RefreshSelection(NewSelection);
    }
}

void SMagicOptimizerMainPanel::OnTabChanged(int32 NewTabIndex)
{
    ActiveTabIndex = NewTabIndex;
    OnTabChangedDelegate.ExecuteIfBound(NewTabIndex);
}

// Actions
void SMagicOptimizerMainPanel::OnRunScan()
{
    ProgressData.bRunning = true;
    ProgressData.Progress = 0.0f;
    ProgressData.Message = TEXT("Initializing scan...");
    
    // Start progress animation
    ProgressAnim.Play(this->AsShared());
    
    // Create audit configuration
    FAuditConfig Config;
    Config.bIncludeTextures = true;
    Config.bIncludeMeshes = true;
    Config.bIncludeMaterials = true;
    Config.bIncludeAnimations = true;
    Config.bIncludeAudio = true;
    Config.MaxAssetsToProcess = 10000; // Reasonable limit for UI responsiveness
    
    // Apply scope filter if set
    if (!ScopeSummary.IsEmpty())
    {
        // Parse scope summary and set filters accordingly
        // For now, use default broad scope
        Config.AssetPathFilters.Add(TEXT("/Game/"));
    }
    
    // Start the audit
    bool bStarted = UMagicOptimizerAuditManager::StartAssetAudit(Config);
    if (!bStarted)
    {
        ProgressData.bRunning = false;
        ProgressData.Message = TEXT("Failed to start scan");
        UE_LOG(LogTemp, Error, TEXT("Failed to start asset audit"));
    }
    
    OnRunScanDelegate.ExecuteIfBound();
}

void SMagicOptimizerMainPanel::OnAutoFix()
{
    ProgressData.bRunning = true;
    ProgressData.Progress = 0.0f;
    ProgressData.Message = TEXT("Auto fixing...");
    
    // Start progress animation
    ProgressAnim.Play(this->AsShared());
    
    OnAutoFixDelegate.ExecuteIfBound();
}

void SMagicOptimizerMainPanel::OnPreview()
{
    OpenPlanDialog();
    OnPreviewDelegate.ExecuteIfBound();
}

void SMagicOptimizerMainPanel::OnTasks()
{
    OpenTaskDrawer();
    OnTasksDelegate.ExecuteIfBound();
}

void SMagicOptimizerMainPanel::OpenCommandPalette()
{
    bCommandPaletteOpen = true;
    if (CommandPaletteWidget.IsValid())
    {
        CommandPaletteWidget->SetIsOpen(true);
    }
}

void SMagicOptimizerMainPanel::ToggleDrawer()
{
    bDrawerOpen = !bDrawerOpen;
    if (SlideOverWidget.IsValid())
    {
        SlideOverWidget->SetIsOpen(bDrawerOpen);
    }
    
    if (bDrawerOpen)
    {
        DrawerAnim.Play(this->AsShared());
    }
    else
    {
        DrawerAnim.Reverse();
    }
}

void SMagicOptimizerMainPanel::OpenScopeSheet()
{
    bScopeSheetOpen = true;
    if (ScopeSheetWidget.IsValid())
    {
        ScopeSheetWidget->SetIsOpen(true);
    }
}

void SMagicOptimizerMainPanel::OpenPlanDialog()
{
    bPlanDialogOpen = true;
    if (PlanDialogWidget.IsValid())
    {
        PlanDialogWidget->SetIsOpen(true);
    }
}

void SMagicOptimizerMainPanel::OpenTaskDrawer()
{
    bTaskDrawerOpen = true;
    if (TaskDrawerWidget.IsValid())
    {
        TaskDrawerWidget->SetIsOpen(true);
    }
}

void SMagicOptimizerMainPanel::OnCloseDrawer()
{
    bDrawerOpen = false;
    if (SlideOverWidget.IsValid())
    {
        SlideOverWidget->SetIsOpen(false);
    }
    DrawerAnim.Reverse();
    OnCloseDrawerDelegate.ExecuteIfBound();
}

void SMagicOptimizerMainPanel::OnCloseScopeSheet()
{
    bScopeSheetOpen = false;
    if (ScopeSheetWidget.IsValid())
    {
        ScopeSheetWidget->SetIsOpen(false);
    }
    OnCloseScopeSheetDelegate.ExecuteIfBound();
}

void SMagicOptimizerMainPanel::OnClosePlanDialog()
{
    bPlanDialogOpen = false;
    if (PlanDialogWidget.IsValid())
    {
        PlanDialogWidget->SetIsOpen(false);
    }
    OnClosePlanDialogDelegate.ExecuteIfBound();
}

void SMagicOptimizerMainPanel::OnCloseTaskDrawer()
{
    bTaskDrawerOpen = false;
    if (TaskDrawerWidget.IsValid())
    {
        TaskDrawerWidget->SetIsOpen(false);
    }
    OnCloseTaskDrawerDelegate.ExecuteIfBound();
}

void SMagicOptimizerMainPanel::OnCloseCommandPalette()
{
    bCommandPaletteOpen = false;
    if (CommandPaletteWidget.IsValid())
    {
        CommandPaletteWidget->SetIsOpen(false);
    }
    OnCloseCommandPaletteDelegate.ExecuteIfBound();
}

void SMagicOptimizerMainPanel::OnExecuteCommand(const FString& CommandId)
{
    // Execute commands from the command palette
    if (CommandId == TEXT("run_scan"))
    {
        OnRunScan();
    }
    else if (CommandId == TEXT("auto_fix"))
    {
        OnAutoFix();
    }
    else if (CommandId == TEXT("preview"))
    {
        OnPreview();
    }
    else if (CommandId == TEXT("export_csv"))
    {
        // TODO: Implement CSV export
        UE_LOG(LogTemp, Log, TEXT("Export CSV command executed"));
    }
    else if (CommandId == TEXT("export_json"))
    {
        // TODO: Implement JSON export
        UE_LOG(LogTemp, Log, TEXT("Export JSON command executed"));
    }
    else if (CommandId == TEXT("export_html"))
    {
        // TODO: Implement HTML export
        UE_LOG(LogTemp, Log, TEXT("Export HTML command executed"));
    }
    else if (CommandId == TEXT("copy_commandlet"))
    {
        // Copy commandlet to clipboard
        FString Commandlet = FString::Printf(TEXT("UE4Editor-Cmd.exe Project.uproject -run=MagicOptimizer -mode=audit -preset=\"%s\" -scope=\"%s\" -report"), 
            *CurrentPreset, *ScopeSummary);
        FPlatformApplicationMisc::ClipboardCopy(*Commandlet);
        UE_LOG(LogTemp, Log, TEXT("Commandlet copied to clipboard: %s"), *Commandlet);
    }
    else if (CommandId == TEXT("goto_audit"))
    {
        OnTabChanged(0);
    }
    else if (CommandId == TEXT("goto_budgets"))
    {
        OnTabChanged(1);
    }
    else if (CommandId == TEXT("goto_recommend"))
    {
        OnTabChanged(2);
    }
    else if (CommandId == TEXT("goto_apply"))
    {
        OnTabChanged(3);
    }
    else if (CommandId == TEXT("goto_verify"))
    {
        OnTabChanged(4);
    }
    else if (CommandId == TEXT("goto_reports"))
    {
        OnTabChanged(5);
    }
    else if (CommandId == TEXT("toggle_rules"))
    {
        // TODO: Implement toggle rules
        UE_LOG(LogTemp, Log, TEXT("Toggle rules command executed"));
    }
    else if (CommandId == TEXT("open_scope"))
    {
        OpenScopeSheet();
    }
    else if (CommandId == TEXT("copy_share_link"))
    {
        // Generate and copy share link
        FString ShareLink = FString::Printf(TEXT("https://reports.magicoptimizer.com/project/%s"), 
            *FGuid::NewGuid().ToString());
        FPlatformApplicationMisc::ClipboardCopy(*ShareLink);
        UE_LOG(LogTemp, Log, TEXT("Share link copied to clipboard: %s"), *ShareLink);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Unknown command ID: %s"), *CommandId);
    }
}

void SMagicOptimizerMainPanel::OnApplyTextureSettings(const FString& TextureId, const FTextureSettings& Settings)
{
    // Find and update the texture with new settings
    for (FUITextureRow& Texture : TextureData)
    {
        if (Texture.Id == TextureId)
        {
            // Apply the new settings to the texture
            Texture.Compression = Settings.Compression;
            
            // Update size based on MaxTextureSize
            if (Settings.MaxTextureSize >= 4096)
            {
                Texture.Size = TEXT("4k");
            }
            else if (Settings.MaxTextureSize >= 2048)
            {
                Texture.Size = TEXT("2k");
            }
            else if (Settings.MaxTextureSize >= 1024)
            {
                Texture.Size = TEXT("1k");
            }
            else
            {
                Texture.Size = TEXT("512");
            }
            
            Texture.LOD = Settings.LODBias;
            
            // Log the applied settings
            UE_LOG(LogTemp, Log, TEXT("Applied settings to texture %s: Compression=%s, MaxSize=%d, LODBias=%d"), 
                *TextureId, *Settings.Compression, Settings.MaxTextureSize, Settings.LODBias);
            
            // Refresh UI with updated data
            RefreshData();
            
            // Close the slide-over
            OnCloseDrawer();
            
            break;
        }
    }
}

void SMagicOptimizerMainPanel::RefreshData()
{
    GenerateMockData();
    
    if (CenterTabsWidget.IsValid())
    {
        CenterTabsWidget->RefreshData(GetFilteredTextures());
    }
    
    if (RightShelfWidget.IsValid())
    {
        RightShelfWidget->RefreshSelection(SelectedTextureIds);
    }
}

void SMagicOptimizerMainPanel::StartScan(bool bAutoFix, bool bPreview)
{
    if (bPreview)
    {
        OpenPlanDialog();
        return;
    }
    
    if (bAutoFix)
    {
        OnAutoFix();
    }
    else
    {
        OnRunScan();
    }
}

void SMagicOptimizerMainPanel::StopScan()
{
    ProgressData.bRunning = false;
    ProgressData.Progress = 100.0f;
    ProgressData.Message = TEXT("Scan complete");
}

void SMagicOptimizerMainPanel::OnDrawerAnimationFinished()
{
    // Animation finished callback
}

void SMagicOptimizerMainPanel::OnProgressAnimationFinished()
{
    // Progress animation finished callback
}

#undef LOCTEXT_NAMESPACE
