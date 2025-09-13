#include "MagicOptimizer.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "EditorStyleSet.h"
#include "Styling/AppStyle.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "OptimizerSettings.h"
#include "MagicOptimizerDiagnostics.h"
#include "MagicOptimizerAuditManager.h"
#include "MagicOptimizerBranding.h"
#include "UI/MagicOptimizerTextureAuditWidget.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/App.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FMagicOptimizerModule"

// Define the custom log category
DEFINE_LOG_CATEGORY(LogMagicOptimizer);

static const FName MagicOptimizerTabName("MagicOptimizerPlugin");

// Console command for testing
static void TestMagicOptimizerCommand()
{
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Console command executed successfully!"));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Magic Optimizer Plugin is working!"));
    }
    
    // Also try to open the tab
    FGlobalTabmanager::Get()->TryInvokeTab(FName("MagicOptimizerPlugin"));
}

static FAutoConsoleCommand TestMagicOptimizerCmd(
    TEXT("MagicOptimizer.Test"),
    TEXT("Test the Magic Optimizer plugin functionality"),
    FConsoleCommandDelegate::CreateStatic(&TestMagicOptimizerCommand)
);

// Console command for branding info
static void MagicOptimizerBrandingCommand()
{
    FString PluginName = UMagicOptimizerBranding::GetBrandedPluginName();
    FString CompanyName = UMagicOptimizerBranding::GetCompanyName();
    FString CompanyURL = UMagicOptimizerBranding::GetCompanyURL();
    
    UE_LOG(LogTemp, Log, TEXT("Magic Optimizer Branding Info:"));
    UE_LOG(LogTemp, Log, TEXT("  Plugin: %s"), *PluginName);
    UE_LOG(LogTemp, Log, TEXT("  Company: %s"), *CompanyName);
    UE_LOG(LogTemp, Log, TEXT("  Website: %s"), *CompanyURL);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, 
            FString::Printf(TEXT("%s\n%s - %s"), *PluginName, *CompanyName, *CompanyURL));
    }
}

static FAutoConsoleCommand MagicOptimizerBrandingCmd(
    TEXT("MagicOptimizer.Branding"),
    TEXT("Display Magic Optimizer branding information"),
    FConsoleCommandDelegate::CreateStatic(&MagicOptimizerBrandingCommand)
);

void FMagicOptimizerModule::StartupModule()
{
    // Log branding information
    MAGIC_LOG(General, FString::Printf(TEXT("Plugin: %s"), *UMagicOptimizerBranding::GetBrandedPluginName()));
    MAGIC_LOG(General, FString::Printf(TEXT("Company: %s"), *UMagicOptimizerBranding::GetCompanyName()));
    MAGIC_LOG(General, FString::Printf(TEXT("Website: %s"), *UMagicOptimizerBranding::GetCompanyURL()));
    
    // Initialize diagnostics system first
    FMagicOptimizerDiagnostics::Initialize();
    
    // Initialize audit system
    UMagicOptimizerAuditManager::Initialize();
    
    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
    MAGIC_LOG(General, TEXT("Module started successfully"));
    MAGIC_LOG_SCREEN(General, TEXT("MagicOptimizer Plugin Loaded"), 5.0f);
    
    // CRITICAL: Settings are automatically registered by UDeveloperSettings
    // DO NOT manually register them to prevent duplicate settings windows
    // This is the ONLY way to ensure single settings registration
    
    MAGIC_LOG(Settings, TEXT("Settings auto-registration enabled via UDeveloperSettings"));
    MAGIC_LOG(Settings, TEXT("NO manual settings registration - preventing duplicates"));
    
    // Register the tab first - check if already registered to prevent duplicates
    if (!FGlobalTabmanager::Get()->HasTabSpawner(MagicOptimizerTabName))
    {
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Registering tab spawner"));
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MagicOptimizerTabName, FOnSpawnTab::CreateRaw(this, &FMagicOptimizerModule::OnSpawnPluginTab))
        .SetDisplayName(LOCTEXT("FMagicOptimizerTabTitle", "Magic Optimizer"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);
    
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Tab spawner registered successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: Tab spawner already registered, skipping duplicate registration"));
    }
    
    // Register UI components immediately - LevelEditor should be available by now
    RegisterMenus();
}

void FMagicOptimizerModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module. For modules that support dynamic reloading,
    // we call this function before unloading the module.
    
    // CRITICAL: Settings are automatically unregistered by UDeveloperSettings
    // DO NOT manually unregister them to prevent conflicts
    
    MAGIC_LOG(Settings, TEXT("Settings auto-unregistration enabled via UDeveloperSettings"));
    MAGIC_LOG(General, TEXT("Module shutdown successfully"));
    
    // Unregister UI components
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MagicOptimizerTabName);
    
    // Shutdown audit system
    UMagicOptimizerAuditManager::Shutdown();
    
    // Shutdown diagnostics system
    FMagicOptimizerDiagnostics::Shutdown();
}

void FMagicOptimizerModule::PluginButtonClicked()
{
    FGlobalTabmanager::Get()->TryInvokeTab(MagicOptimizerTabName);
}

void FMagicOptimizerModule::AddMenuExtension(FMenuBuilder& Builder)
{
    Builder.AddMenuEntry(
        LOCTEXT("MagicOptimizerMenuEntryTitle", "Magic Optimizer"),
        LOCTEXT("MagicOptimizerMenuEntryTooltip", "Open the Magic Optimizer window"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Toolbar.Play"),
        FUIAction(FExecuteAction::CreateRaw(this, &FMagicOptimizerModule::PluginButtonClicked))
    );
}

void FMagicOptimizerModule::AddToolbarButton(FToolBarBuilder& Builder)
{
    Builder.AddToolBarButton(
        FUIAction(FExecuteAction::CreateRaw(this, &FMagicOptimizerModule::PluginButtonClicked)),
        NAME_None,
        LOCTEXT("MagicOptimizerToolbarButtonText", "Magic Optimizer"),
        LOCTEXT("MagicOptimizerToolbarButtonTooltip", "Open the Magic Optimizer window"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Toolbar.Play")
    );
}

void FMagicOptimizerModule::OpenSettings()
{
    MAGIC_LOG(Settings, TEXT("Opening settings..."));
    MAGIC_LOG_PERF_START(TEXT("OpenSettings"));
    
    // Use the reliable SettingsEditor module approach
    FModuleManager& ModuleManager = FModuleManager::Get();
    if (ModuleManager.IsModuleLoaded("SettingsEditor"))
    {
        MAGIC_LOG(Settings, TEXT("SettingsEditor module found, opening plugin settings"));
        ISettingsModule& SettingsModule = ModuleManager.GetModuleChecked<ISettingsModule>("SettingsEditor");
        
        // Open the settings viewer with the correct parameters
        SettingsModule.ShowViewer("Project", "Plugins", "Magic Optimizer");
        MAGIC_LOG(Settings, TEXT("Plugin settings opened successfully"));
    }
    else
    {
        MAGIC_LOG(Settings, TEXT("SettingsEditor module not loaded, trying fallback"));
        
        // Fallback: Try to load the module first
        if (ModuleManager.LoadModule("SettingsEditor"))
        {
            MAGIC_LOG(Settings, TEXT("SettingsEditor module loaded successfully"));
            ISettingsModule& SettingsModule = ModuleManager.GetModuleChecked<ISettingsModule>("SettingsEditor");
            SettingsModule.ShowViewer("Project", "Plugins", "Magic Optimizer");
            MAGIC_LOG(Settings, TEXT("Plugin settings opened after module load"));
        }
        else
        {
            MAGIC_LOG_ERROR(TEXT("Failed to load SettingsEditor module"), TEXT("OpenSettings"));
            
            // Final fallback: Open general project settings
            MAGIC_LOG(Settings, TEXT("Attempting final fallback to project settings"));
            FGlobalTabmanager::Get()->TryInvokeTab(FName("ProjectSettings"));
        }
    }
    
    // Verify settings object exists
    if (UOptimizerSettings* Settings = GetMutableDefault<UOptimizerSettings>())
    {
        MAGIC_LOG(Settings, FString::Printf(TEXT("Settings object verified - DefaultProfile: %s, MaxAssetsPerBatch: %d"), 
               *Settings->DefaultProfile, Settings->MaxAssetsPerBatch));
        
        // Log detailed settings state
        FMagicOptimizerDiagnostics::LogSettingsState();
        
        // Validate settings
        FMagicOptimizerDiagnostics::ValidateSettings();
    }
    else
    {
        MAGIC_LOG_ERROR(TEXT("Settings object not found! This indicates a registration issue"), TEXT("OpenSettings"));
    }
    
    // Log memory usage after operation
    FMagicOptimizerDiagnostics::LogMemoryUsage(TEXT("After Settings Open"));
    
    MAGIC_LOG_PERF_END(TEXT("OpenSettings"));
}

void FMagicOptimizerModule::RegisterMenus()
{
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: RegisterMenus called"));
    
    // Check if LevelEditor module is available
    if (!FModuleManager::Get().IsModuleLoaded("LevelEditor"))
    {
        UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: LevelEditor module not loaded yet, deferring menu registration"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: LevelEditor module is loaded, proceeding with menu registration"));
    
    // Use modern UToolMenus approach for UE 5.6 - this prevents duplicates
    if (UToolMenus::IsToolMenuUIEnabled())
    {
        UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Using UToolMenus for menu registration"));
        
        UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
        if (WindowMenu)
        {
            FToolMenuSection& Section = WindowMenu->FindOrAddSection("WindowLayout");
            Section.AddMenuEntry(
                NAME_None,
                LOCTEXT("MagicOptimizerMenuEntryTitle", "Magic Optimizer"),
                LOCTEXT("MagicOptimizerMenuEntryTooltip", "Open the Magic Optimizer window"),
                FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Toolbar.Play"),
                FUIAction(FExecuteAction::CreateRaw(this, &FMagicOptimizerModule::PluginButtonClicked))
            );
            
            UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Menu entry added to Window menu"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: UToolMenus not available, skipping menu registration"));
    }
    
}

TSharedRef<SDockTab> FMagicOptimizerModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: OnSpawnPluginTab called"));
    
    // Create the texture audit widget
    UMagicOptimizerTextureAuditWidget* TextureAuditWidget = CreateWidget<UMagicOptimizerTextureAuditWidget>(GEditor->GetEditorWorldContext().World());
    if (TextureAuditWidget)
    {
        TextureAuditWidget->InitializeWidget();
        UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: UMG TextureAuditWidget created and initialized"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MagicOptimizer: Failed to create UMG TextureAuditWidget"));
    }

    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        .Label(LOCTEXT("MagicOptimizerTabTitle", "Magic Optimizer"))
        [
            SNew(SBox)
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            .Padding(8.0f)
            [
                // Professional Header
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 0, 0, 8)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .FillWidth(1.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("MagicOptimizerTitle", "🎯 Magic Optimizer"))
                        .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                        .Justification(ETextJustify::Left)
                    ]
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(8, 0, 0, 0)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(0, 0, 4, 0)
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("RunScanButtonText", "🔍 Run Scan"))
                            .OnClicked_Lambda([this]()
                            {
                                MAGIC_LOG(UI, TEXT("Run Scan button clicked"));
                                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Starting asset scan..."));
                                return FReply::Handled();
                            })
                        ]
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(0, 0, 4, 0)
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("AutoFixButtonText", "🪄 Auto Fix"))
                            .OnClicked_Lambda([this]()
                            {
                                MAGIC_LOG(UI, TEXT("Auto Fix button clicked"));
                                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Starting auto fix..."));
                                return FReply::Handled();
                            })
                        ]
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("PreviewButtonText", "👁️ Preview"))
                            .OnClicked_Lambda([this]()
                            {
                                MAGIC_LOG(UI, TEXT("Preview button clicked"));
                                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Opening preview..."));
                                return FReply::Handled();
                            })
                        ]
                    ]
                ]
                
                // Tab System
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .FillWidth(1.0f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 0, 0, 8)
                        [
                            // Tab Buttons
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .Padding(0, 0, 4, 0)
                            [
                                SNew(SButton)
                                .Text(LOCTEXT("AuditTabText", "Audit"))
                                .OnClicked_Lambda([this]()
                                {
                                    MAGIC_LOG(UI, TEXT("Audit tab selected"));
                                    return FReply::Handled();
                                })
                            ]
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .Padding(0, 0, 4, 0)
                            [
                                SNew(SButton)
                                .Text(LOCTEXT("BudgetsTabText", "Budgets"))
                                .OnClicked_Lambda([this]()
                                {
                                    MAGIC_LOG(UI, TEXT("Budgets tab selected"));
                                    return FReply::Handled();
                                })
                            ]
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .Padding(0, 0, 4, 0)
                            [
                                SNew(SButton)
                                .Text(LOCTEXT("RecommendTabText", "Recommend"))
                                .OnClicked_Lambda([this]()
                                {
                                    MAGIC_LOG(UI, TEXT("Recommend tab selected"));
                                    return FReply::Handled();
                                })
                            ]
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .Padding(0, 0, 4, 0)
                            [
                                SNew(SButton)
                                .Text(LOCTEXT("ApplyTabText", "Apply"))
                                .OnClicked_Lambda([this]()
                                {
                                    MAGIC_LOG(UI, TEXT("Apply tab selected"));
                                    return FReply::Handled();
                                })
                            ]
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .Padding(0, 0, 4, 0)
                            [
                                SNew(SButton)
                                .Text(LOCTEXT("VerifyTabText", "Verify"))
                                .OnClicked_Lambda([this]()
                                {
                                    MAGIC_LOG(UI, TEXT("Verify tab selected"));
                                    return FReply::Handled();
                                })
                            ]
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            [
                                SNew(SButton)
                                .Text(LOCTEXT("ReportsTabText", "Reports"))
                                .OnClicked_Lambda([this]()
                                {
                                    MAGIC_LOG(UI, TEXT("Reports tab selected"));
                                    return FReply::Handled();
                                })
                            ]
                        ]
                        
                        // Main Content Area
                        + SVerticalBox::Slot()
                        .FillHeight(1.0f)
            [
                SNew(SScrollBox)
                + SScrollBox::Slot()
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Top)
                            .Padding(0, 8, 0, 0)
                            [
                                // Audit View Content
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 8)
                                [
                                    SNew(STextBlock)
                                    .Text(LOCTEXT("AuditViewTitle", "Asset Audit - Comprehensive Analysis"))
                                    .TextStyle(FAppStyle::Get(), "LargeText")
                                    .Justification(ETextJustify::Left)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 16)
                                [
                                    SNew(STextBlock)
                                    .Text(LOCTEXT("AuditViewDescription", "Scan your project assets for optimization opportunities. Identify performance bottlenecks and memory usage issues."))
                                    .TextStyle(FAppStyle::Get(), "NormalText")
                                    .Justification(ETextJustify::Left)
                                ]
                                
                                // Health Overview Cards
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 16)
                                [
                                    SNew(SHorizontalBox)
                                    + SHorizontalBox::Slot()
                                    .FillWidth(1.0f)
                                    .Padding(0, 0, 8, 0)
                                    [
                                        SNew(SBorder)
                                        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                                        .Padding(12)
                                        [
                                            SNew(SVerticalBox)
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 8)
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("HealthCardTitle", "Health Score"))
                                                .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            [
                                                SNew(SHorizontalBox)
                                                + SHorizontalBox::Slot()
                                                .AutoWidth()
                                                [
                                                    SNew(STextBlock)
                                                    .Text(LOCTEXT("HealthScore", "78"))
                                                    .TextStyle(FAppStyle::Get(), "LargeText")
                                                    .ColorAndOpacity(FLinearColor::Green)
                                                ]
                                                + SHorizontalBox::Slot()
                                                .AutoWidth()
                                                .Padding(4, 0, 0, 0)
                                                [
                                                    SNew(STextBlock)
                                                    .Text(LOCTEXT("HealthScoreOutOf", "/100"))
                                                    .TextStyle(FAppStyle::Get(), "NormalText")
                                                    .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
                                                ]
                                            ]
                                        ]
                                    ]
                                    + SHorizontalBox::Slot()
                                    .FillWidth(1.0f)
                                    .Padding(0, 0, 8, 0)
                                    [
                                        SNew(SBorder)
                                        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                                        .Padding(12)
                                        [
                                            SNew(SVerticalBox)
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 8)
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("IssuesCardTitle", "Critical Issues"))
                                                .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("CriticalIssuesCount", "12"))
                                                .TextStyle(FAppStyle::Get(), "LargeText")
                                                .ColorAndOpacity(FLinearColor::Red)
                                            ]
                                        ]
                                    ]
                                    + SHorizontalBox::Slot()
                                    .FillWidth(1.0f)
                                    [
                                        SNew(SBorder)
                                        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                                        .Padding(12)
                                        [
                                            SNew(SVerticalBox)
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 8)
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("AssetsCardTitle", "Assets Scanned"))
                                                .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                [
                    SNew(STextBlock)
                                                .Text(LOCTEXT("AssetsScannedCount", "1,247"))
                    .TextStyle(FAppStyle::Get(), "LargeText")
                                                .ColorAndOpacity(FLinearColor::Blue)
                                            ]
                                        ]
                                    ]
                                ]
                                
                                // Progress Bar Section
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 16)
                                [
                                    SNew(SBorder)
                                    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                                    .Padding(12)
                                    [
                                        SNew(SVerticalBox)
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        .Padding(0, 0, 0, 8)
                                        [
                                            SNew(STextBlock)
                                            .Text(LOCTEXT("ProgressTitle", "Audit Progress"))
                                            .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        .Padding(0, 0, 0, 4)
                                        [
                                            SNew(SHorizontalBox)
                                            + SHorizontalBox::Slot()
                                            .FillWidth(1.0f)
                                            [
                                                SNew(SProgressBar)
                                                .Percent(0.65f)
                                                .BarFillType(EProgressBarFillType::TopToBottom)
                                            ]
                                            + SHorizontalBox::Slot()
                                            .AutoWidth()
                                            .Padding(8, 0, 0, 0)
                [
                    SNew(STextBlock)
                                                .Text(LOCTEXT("ProgressText", "65%"))
                    .TextStyle(FAppStyle::Get(), "NormalText")
                                                .ColorAndOpacity(FLinearColor::Green)
                                            ]
                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        .Padding(0, 4, 0, 0)
                                        [
                                            SNew(STextBlock)
                                            .Text(LOCTEXT("ProgressStatus", "Scanning textures... 847/1,247 assets processed"))
                                            .TextStyle(FAppStyle::Get(), "SmallText")
                                            .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
                                        ]
                                    ]
                                ]
                                
                                // Action Buttons
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 16)
                                [
                                    SNew(SHorizontalBox)
                                    + SHorizontalBox::Slot()
                                    .AutoWidth()
                                    .Padding(0, 0, 8, 0)
                [
                    SNew(SButton)
                                        .Text(LOCTEXT("StartAuditButtonText", "🔍 Start Asset Audit"))
                    .OnClicked_Lambda([this]()
                    {
                                            MAGIC_LOG(UI, TEXT("Start Asset Audit clicked"));
                        
                        // Start asset audit
                        FAuditConfig Config;
                        Config.TargetPlatform = TEXT("WindowsEditor");
                        Config.QualityLevel = EQualityLevel::High;
                        Config.bDryRunMode = true;
                        Config.bGenerateReports = true;
                                            Config.IncludeAssetTypes.Add(EAssetType::Texture);
                        
                        if (UMagicOptimizerAuditManager::StartAssetAudit(Config))
                        {
                            MAGIC_LOG(AssetProcessing, TEXT("Asset audit started successfully"));
                                                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Asset Audit: Started successfully!"));
                        }
                        else
                        {
                            MAGIC_LOG_ERROR(TEXT("Failed to start asset audit"), TEXT("AssetAuditButtonClick"));
                                                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Asset Audit: Failed to start!"));
                        }
                        
                        return FReply::Handled();
                    })
                                    ]
                                    + SHorizontalBox::Slot()
                                    .AutoWidth()
                                    .Padding(0, 0, 8, 0)
                [
                    SNew(SButton)
                                        .Text(LOCTEXT("GenerateReportButtonText", "📊 Generate Report"))
                    .OnClicked_Lambda([this]()
                    {
                                            MAGIC_LOG(UI, TEXT("Generate Report clicked"));
                                            
                                            // Generate comprehensive diagnostic report
                                            FMagicOptimizerDiagnostics::GenerateDiagnosticReport();
                                            
                                            // Show success message
                                            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Diagnostic report generated successfully!"));
                                            
                                            return FReply::Handled();
                                        })
                                    ]
                                    + SHorizontalBox::Slot()
                                    .AutoWidth()
                                    .Padding(0, 0, 8, 0)
                                    [
                                        SNew(SButton)
                                        .Text(LOCTEXT("OpenSettingsButtonText", "⚙️ Settings"))
                                        .OnClicked_Lambda([this]()
                                        {
                                            MAGIC_LOG(UI, TEXT("Settings button clicked"));
                                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Opening settings..."));
                                            return FReply::Handled();
                                        })
                                    ]
                                    + SHorizontalBox::Slot()
                                    .AutoWidth()
                                    [
                                        SNew(SButton)
                                        .Text(LOCTEXT("StopAuditButtonText", "⏹️ Stop"))
                                        .OnClicked_Lambda([this]()
                                        {
                                            MAGIC_LOG(UI, TEXT("Stop Audit clicked"));
                                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("Audit stopped by user"));
                                            return FReply::Handled();
                                        })
                                    ]
                                ]
                                
                                // Asset Table Section
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 8)
                                [
                                    SNew(STextBlock)
                                    .Text(LOCTEXT("AssetTableTitle", "Asset Analysis Results"))
                                    .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                                    .Justification(ETextJustify::Left)
                                ]
                                
                                // Advanced Search and Filter Bar
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 8)
                                [
                                    SNew(SVerticalBox)
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    .Padding(0, 0, 0, 8)
                                    [
                                        SNew(SHorizontalBox)
                                        + SHorizontalBox::Slot()
                                        .FillWidth(1.0f)
                                        .Padding(0, 0, 8, 0)
                                        [
                                            SNew(SEditableTextBox)
                                            .HintText(LOCTEXT("SearchHint", "Search assets by name, path, or type..."))
                                            .OnTextChanged_Lambda([this](const FText& InText)
                                            {
                                                MAGIC_LOG(UI, FString::Printf(TEXT("Search text changed: %s"), *InText.ToString()));
                                            })
                                        ]
                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .Padding(0, 0, 8, 0)
                                        [
                                            SNew(SButton)
                                            .Text(LOCTEXT("AdvancedFilterButtonText", "🔧 Advanced"))
                                            .OnClicked_Lambda([this]()
                                            {
                                                MAGIC_LOG(UI, TEXT("Advanced Filter button clicked"));
                                                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Opening advanced filters..."));
                                                return FReply::Handled();
                                            })
                                        ]
                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .Padding(0, 0, 8, 0)
                                        [
                                            SNew(SButton)
                                            .Text(LOCTEXT("SortButtonText", "📊 Sort"))
                                            .OnClicked_Lambda([this]()
                                            {
                                                MAGIC_LOG(UI, TEXT("Sort button clicked"));
                                                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Opening sort options..."));
                                                return FReply::Handled();
                                            })
                                        ]
                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        [
                                            SNew(SButton)
                                            .Text(LOCTEXT("ExportButtonText", "📤 Export"))
                                            .OnClicked_Lambda([this]()
                                            {
                                                MAGIC_LOG(UI, TEXT("Export button clicked"));
                                                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Exporting results..."));
                                                return FReply::Handled();
                                            })
                                        ]
                                    ]
                                    
                                    // Filter Row
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    [
                                        SNew(SHorizontalBox)
                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .Padding(0, 0, 8, 0)
                                        [
                                            SNew(STextBlock)
                                            .Text(LOCTEXT("FilterLabel", "Quick Filters:"))
                                            .TextStyle(FAppStyle::Get(), "SmallText")
                                            .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
                                        ]
                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .Padding(0, 0, 8, 0)
                                        [
                                            SNew(SButton)
                                            .Text(LOCTEXT("CriticalFilterText", "🔴 Critical"))
                                            .OnClicked_Lambda([this]()
                                            {
                                                MAGIC_LOG(UI, TEXT("Critical filter clicked"));
                                                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Filtering critical issues..."));
                                                return FReply::Handled();
                                            })
                                        ]
                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .Padding(0, 0, 8, 0)
                                        [
                                            SNew(SButton)
                                            .Text(LOCTEXT("WarningFilterText", "🟡 Warnings"))
                                            .OnClicked_Lambda([this]()
                                            {
                                                MAGIC_LOG(UI, TEXT("Warning filter clicked"));
                                                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Filtering warnings..."));
                                                return FReply::Handled();
                                            })
                                        ]
                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .Padding(0, 0, 8, 0)
                                        [
                                            SNew(SButton)
                                            .Text(LOCTEXT("LargeAssetsFilterText", "📏 Large Assets"))
                                            .OnClicked_Lambda([this]()
                                            {
                                                MAGIC_LOG(UI, TEXT("Large assets filter clicked"));
                                                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TEXT("Filtering large assets..."));
                                                return FReply::Handled();
                                            })
                                        ]
                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .Padding(0, 0, 8, 0)
                                        [
                                            SNew(SButton)
                                            .Text(LOCTEXT("ClearFiltersText", "❌ Clear"))
                                            .OnClicked_Lambda([this]()
                                            {
                                                MAGIC_LOG(UI, TEXT("Clear filters clicked"));
                                                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, TEXT("Clearing all filters..."));
                        return FReply::Handled();
                    })
                                        ]
                                        + SHorizontalBox::Slot()
                                        .FillWidth(1.0f)
                                        [
                                            SNew(SSpacer)
                                        ]
                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        [
                                            SNew(STextBlock)
                                            .Text(LOCTEXT("ResultsCount", "Showing 5 of 1,247 assets"))
                                            .TextStyle(FAppStyle::Get(), "SmallText")
                                            .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
                                        ]
                                    ]
                                ]
                                
                                // Asset Table
                                + SVerticalBox::Slot()
                                .FillHeight(1.0f)
                                [
                                    SNew(SBorder)
                                    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                                    .Padding(8)
                                    [
                                        SNew(SScrollBox)
                + SScrollBox::Slot()
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Top)
                                        [
                                            // Mock Asset Data
                                            SNew(SVerticalBox)
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 4)
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("AssetTableHeader", "Path | Size | Group | Compression | sRGB | Mips | VT | LOD | Issues"))
                                                .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                                                .Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 2)
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("AssetRow1", "/Game/Props/T_001 | 2k | Props | Default | On | Yes | VT | 0 | 2"))
                                                .TextStyle(FAppStyle::Get(), "NormalText")
                                                .Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
                                                .ColorAndOpacity(FLinearColor::Red)
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 2)
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("AssetRow2", "/Game/UI/T_Button | 1k | UI | UserInterface | On | Yes | — | 0 | 0"))
                                                .TextStyle(FAppStyle::Get(), "NormalText")
                                                .Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
                                                .ColorAndOpacity(FLinearColor::Green)
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 2)
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("AssetRow3", "/Game/World/T_Ground | 4k | World | Default | Off | Yes | VT | 1 | 1"))
                                                .TextStyle(FAppStyle::Get(), "NormalText")
                                                .Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
                                                .ColorAndOpacity(FLinearColor::Yellow)
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 2)
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("AssetRow4", "/Game/Characters/T_Skin | 2k | Characters | Default | On | Yes | — | 0 | 3"))
                                                .TextStyle(FAppStyle::Get(), "NormalText")
                                                .Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
                                                .ColorAndOpacity(FLinearColor::Red)
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 2)
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("AssetRow5", "/Game/Effects/T_Fire | 1k | Effects | Default | On | No | VT | 2 | 0"))
                                                .TextStyle(FAppStyle::Get(), "NormalText")
                                                .Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
                                                .ColorAndOpacity(FLinearColor::Green)
                                            ]
                                        ]
                                    ]
                                ]
                                
                                // Table Footer
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 8, 0, 16)
                                [
                                    SNew(SHorizontalBox)
                                    + SHorizontalBox::Slot()
                                    .FillWidth(1.0f)
                                    [
                                        SNew(STextBlock)
                                        .Text(LOCTEXT("TableFooter", "Showing 5 of 1,247 assets • 12 critical issues found"))
                                        .TextStyle(FAppStyle::Get(), "SmallText")
                                        .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
                                    ]
                                    + SHorizontalBox::Slot()
                                    .AutoWidth()
                [
                    SNew(SButton)
                                        .Text(LOCTEXT("LoadMoreButtonText", "Load More"))
                    .OnClicked_Lambda([this]()
                    {
                                            MAGIC_LOG(UI, TEXT("Load More button clicked"));
                                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Loading more assets..."));
                                            return FReply::Handled();
                                        })
                                    ]
                                ]
                                
                                // Performance Analytics Section
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 8)
                                [
                                    SNew(STextBlock)
                                    .Text(LOCTEXT("AnalyticsTitle", "Performance Analytics"))
                                    .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                                    .Justification(ETextJustify::Left)
                                ]
                                
                                // Analytics Cards
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 16)
                                [
                                    SNew(SHorizontalBox)
                                    + SHorizontalBox::Slot()
                                    .FillWidth(1.0f)
                                    .Padding(0, 0, 8, 0)
                                    [
                                        SNew(SBorder)
                                        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                                        .Padding(12)
                                        [
                                            SNew(SVerticalBox)
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 8)
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("MemoryUsageTitle", "Memory Usage"))
                                                .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 4)
                                            [
                                                SNew(SProgressBar)
                                                .Percent(0.72f)
                                                .BarFillType(EProgressBarFillType::LeftToRight)
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("MemoryUsageText", "2.4 GB / 3.2 GB (72%)"))
                                                .TextStyle(FAppStyle::Get(), "SmallText")
                                                .ColorAndOpacity(FLinearColor::Yellow)
                                            ]
                                        ]
                                    ]
                                    + SHorizontalBox::Slot()
                                    .FillWidth(1.0f)
                                    .Padding(0, 0, 8, 0)
                                    [
                                        SNew(SBorder)
                                        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                                        .Padding(12)
                                        [
                                            SNew(SVerticalBox)
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 8)
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("DiskUsageTitle", "Disk Usage"))
                                                .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 4)
                                            [
                                                SNew(SProgressBar)
                                                .Percent(0.45f)
                                                .BarFillType(EProgressBarFillType::LeftToRight)
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("DiskUsageText", "1.8 GB / 4.0 GB (45%)"))
                                                .TextStyle(FAppStyle::Get(), "SmallText")
                                                .ColorAndOpacity(FLinearColor::Green)
                                            ]
                                        ]
                                    ]
                                    + SHorizontalBox::Slot()
                                    .FillWidth(1.0f)
                                    [
                                        SNew(SBorder)
                                        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                                        .Padding(12)
                                        [
                                            SNew(SVerticalBox)
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 8)
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("PerformanceScoreTitle", "Performance Score"))
                                                .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(0, 0, 0, 4)
                                            [
                                                SNew(SProgressBar)
                                                .Percent(0.78f)
                                                .BarFillType(EProgressBarFillType::LeftToRight)
                                            ]
                                            + SVerticalBox::Slot()
                                            .AutoHeight()
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("PerformanceScoreText", "78/100 (Good)"))
                                                .TextStyle(FAppStyle::Get(), "SmallText")
                                                .ColorAndOpacity(FLinearColor::Green)
                                            ]
                                        ]
                                    ]
                                ]
                            ]
                        ]
                    ]
                    
                    // Right Panel (Quick Actions)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(16, 0, 0, 0)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 0, 0, 8)
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("QuickActionsTitle", "Quick Actions"))
                            .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                            .Justification(ETextJustify::Left)
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 0, 0, 4)
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("OpenAssetAuditText", "Open in Asset Audit"))
                            .OnClicked_Lambda([this]()
                            {
                                MAGIC_LOG(UI, TEXT("Open in Asset Audit clicked"));
                                return FReply::Handled();
                            })
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 0, 0, 4)
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("OpenSizeMapText", "Open Size Map"))
                            .OnClicked_Lambda([this]()
                            {
                                MAGIC_LOG(UI, TEXT("Open Size Map clicked"));
                                return FReply::Handled();
                            })
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 0, 0, 4)
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("StartTraceText", "Start 30s Insights Trace"))
                            .OnClicked_Lambda([this]()
                            {
                                MAGIC_LOG(UI, TEXT("Start Insights Trace clicked"));
                        return FReply::Handled();
                    })
                        ]
                    ]
                ]
            ]
        ];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMagicOptimizerModule, MagicOptimizer)

