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
#include "HAL/PlatformFilemanager.h"
#include "Misc/App.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FMagicOptimizerModule"

// Define the custom log category
DEFINE_LOG_CATEGORY(LogMagicOptimizer);

static const FName MagicOptimizerTabName("MagicOptimizer");

// Console command for testing
static void TestMagicOptimizerCommand()
{
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Console command executed successfully!"));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Magic Optimizer Plugin is working!"));
    }
    
    // Also try to open the tab
    FGlobalTabmanager::Get()->TryInvokeTab(FName("MagicOptimizer"));
}

static FAutoConsoleCommand TestMagicOptimizerCmd(
    TEXT("MagicOptimizer.Test"),
    TEXT("Test the Magic Optimizer plugin functionality"),
    FConsoleCommandDelegate::CreateStatic(&TestMagicOptimizerCommand)
);

void FMagicOptimizerModule::StartupModule()
{
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
    
    // Register the tab first
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Registering tab spawner"));
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MagicOptimizerTabName, FOnSpawnTab::CreateRaw(this, &FMagicOptimizerModule::OnSpawnPluginTab))
        .SetDisplayName(LOCTEXT("FMagicOptimizerTabTitle", "Magic Optimizer"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);
    
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Tab spawner registered successfully"));
    
    // Register UI components immediately - LevelEditor should be available by now
    RegisterMenus();
    
    // Also add a fallback registration after editor is fully loaded
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float DeltaTime)
    {
        UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Fallback menu registration attempt"));
        RegisterMenus();
        return false; // Only run once
    }), 3.0f); // 3 second delay as fallback
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
    
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        MAGIC_LOG(Settings, TEXT("Settings module found, attempting to open viewer"));
        
        // Try to open the settings viewer
        MAGIC_LOG(Settings, TEXT("Attempting to open Magic Optimizer settings"));
        SettingsModule->ShowViewer("Project", "Plugins", "Magic Optimizer");
        MAGIC_LOG(Settings, TEXT("Settings viewer command executed"));
        
        // Note: ShowViewer returns void, so we can't check success/failure directly
        // The settings should open if the plugin is properly registered
    }
    else
    {
        MAGIC_LOG_ERROR(TEXT("Settings module not found! Cannot open settings"), TEXT("OpenSettings"));
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
    
    // Use FExtensibilityManager approach - more reliable in UE 5.6
    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    
    // Create menu extender
    TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);
    MenuExtender->AddMenuExtension(
        "WindowLayout",
        EExtensionHook::After,
        nullptr,
        FMenuExtensionDelegate::CreateRaw(this, &FMagicOptimizerModule::AddMenuExtension)
    );
    
    // Add to level editor
    LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
    
    // Also add a toolbar button
    TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
    ToolbarExtender->AddToolBarExtension(
        "Settings",
        EExtensionHook::After,
        nullptr,
        FToolBarExtensionDelegate::CreateRaw(this, &FMagicOptimizerModule::AddToolbarButton)
    );
    LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
    
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Menu and toolbar extenders added to LevelEditor"));
    
    // Also try the modern UToolMenus approach as backup
    if (UToolMenus::IsToolMenuUIEnabled())
    {
        UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: UToolMenus is enabled, adding backup registration"));
        
        UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
        if (WindowMenu)
        {
            FToolMenuSection& Section = WindowMenu->FindOrAddSection("WindowLayout");
            Section.AddMenuEntry(
                "MagicOptimizerBackup",
                LOCTEXT("MagicOptimizerMenuEntryTitle", "Magic Optimizer (Backup)"),
                LOCTEXT("MagicOptimizerMenuEntryTooltip", "Open the Magic Optimizer window"),
                FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Toolbar.Play"),
                FUIAction(FExecuteAction::CreateRaw(this, &FMagicOptimizerModule::PluginButtonClicked))
            );
            UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Backup menu entry added"));
        }
    }
}

TSharedRef<SDockTab> FMagicOptimizerModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SBox)
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            [
                SNew(SScrollBox)
                + SScrollBox::Slot()
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Top)
                .Padding(10)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("MagicOptimizerWelcomeText", "🎯 Magic Optimizer - Ultimate UE5.6 Asset Optimization Platform"))
                    .TextStyle(FAppStyle::Get(), "LargeText")
                    .Justification(ETextJustify::Center)
                ]
                + SScrollBox::Slot()
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Top)
                .Padding(10)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("MagicOptimizerDescriptionText", "Comprehensive asset optimization with AI-powered intelligence and self-learning capabilities."))
                    .TextStyle(FAppStyle::Get(), "NormalText")
                    .Justification(ETextJustify::Center)
                ]
                + SScrollBox::Slot()
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Top)
                .Padding(20)
                [
                    SNew(SButton)
                    .Text(LOCTEXT("RunAuditButtonText", "🔍 Run Asset Audit"))
                    .OnClicked_Lambda([this]()
                    {
                        MAGIC_LOG(UI, TEXT("Asset Audit button clicked"));
                        MAGIC_LOG_PERF_START(TEXT("AssetAuditButtonClick"));
                        
                        // Log UI state
                        FMagicOptimizerDiagnostics::LogButtonClick(TEXT("Asset Audit"), true);
                        FMagicOptimizerDiagnostics::LogUIState(TEXT("Asset Audit Button"), TEXT("Clicked"));
                        
                        // Log current settings for context
                        FMagicOptimizerDiagnostics::LogSettingsState();
                        
                        // Log memory usage before operation
                        FMagicOptimizerDiagnostics::LogMemoryUsage(TEXT("Before Asset Audit"));
                        
                        // Start asset audit
                        FAuditConfig Config;
                        Config.TargetPlatform = TEXT("WindowsEditor");
                        Config.QualityLevel = EQualityLevel::High;
                        Config.bDryRunMode = true;
                        Config.bGenerateReports = true;
                        
                        if (UMagicOptimizerAuditManager::StartAssetAudit(Config))
                        {
                            MAGIC_LOG(AssetProcessing, TEXT("Asset audit started successfully"));
                            MAGIC_LOG_SCREEN(Info, TEXT("Asset Audit: Started successfully!"), 3.0f);
                        }
                        else
                        {
                            MAGIC_LOG_ERROR(TEXT("Failed to start asset audit"), TEXT("AssetAuditButtonClick"));
                            MAGIC_LOG_SCREEN(Error, TEXT("Asset Audit: Failed to start!"), 5.0f);
                        }
                        
                        MAGIC_LOG_PERF_END(TEXT("AssetAuditButtonClick"));
                        return FReply::Handled();
                    })
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Center)
                ]
                + SScrollBox::Slot()
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Top)
                .Padding(20)
                [
                    SNew(SButton)
                    .Text(LOCTEXT("ViewSettingsButtonText", "⚙️ View Settings"))
                    .OnClicked_Lambda([this]()
                    {
                        MAGIC_LOG(UI, TEXT("View Settings button clicked"));
                        MAGIC_LOG_PERF_START(TEXT("ViewSettingsButtonClick"));
                        
                        // Log UI state
                        FMagicOptimizerDiagnostics::LogButtonClick(TEXT("View Settings"), true);
                        FMagicOptimizerDiagnostics::LogUIState(TEXT("View Settings Button"), TEXT("Clicked"));
                        
                        // Log memory usage before operation
                        FMagicOptimizerDiagnostics::LogMemoryUsage(TEXT("Before Settings Open"));
                        
                        // Open settings with enhanced logging
                        OpenSettings();
                        
                        MAGIC_LOG_PERF_END(TEXT("ViewSettingsButtonClick"));
                        return FReply::Handled();
                    })
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Center)
                ]
                + SScrollBox::Slot()
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Top)
                .Padding(20)
                [
                    SNew(SButton)
                    .Text(LOCTEXT("DiagnosticsButtonText", "🔧 Generate Diagnostic Report"))
                    .OnClicked_Lambda([this]()
                    {
                        MAGIC_LOG(UI, TEXT("Diagnostics button clicked"));
                        MAGIC_LOG_PERF_START(TEXT("DiagnosticsButtonClick"));
                        
                        // Log UI state
                        FMagicOptimizerDiagnostics::LogButtonClick(TEXT("Diagnostics"), true);
                        FMagicOptimizerDiagnostics::LogUIState(TEXT("Diagnostics Button"), TEXT("Clicked"));
                        
                        // Generate comprehensive diagnostic report
                        FMagicOptimizerDiagnostics::GenerateDiagnosticReport();
                        
                        // Show success message
                        MAGIC_LOG_SCREEN(Info, TEXT("Diagnostic report generated successfully!"), 5.0f);
                        
                        MAGIC_LOG_PERF_END(TEXT("DiagnosticsButtonClick"));
                        return FReply::Handled();
                    })
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Center)
                ]
            ]
        ];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMagicOptimizerModule, MagicOptimizer)
