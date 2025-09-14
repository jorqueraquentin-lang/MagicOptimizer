#include "MagicOptimizer.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
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
#include "UI/MagicOptimizerUIDataManager.h"
#include "UI/MagicOptimizerMainWidget.h"
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
    
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;

    // **STEP 1: Create or get the one manager instance for the editor**
    UMagicOptimizerUIDataManager* Manager = GetOrCreateUIDataManager();
    UIDataManagerKeeper = TStrongObjectPtr<UMagicOptimizerUIDataManager>(Manager);

    // **STEP 2: Create the widget**
    UMagicOptimizerMainWidget* MainWidget = World 
        ? CreateWidget<UMagicOptimizerMainWidget>(World)
        : NewObject<UMagicOptimizerMainWidget>();
    MainWidgetKeeper = TStrongObjectPtr<UMagicOptimizerMainWidget>(MainWidget);
    
    if (MainWidget)
    {
        UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Main Widget created successfully"));
        
        // **STEP 3: Inject the manager BEFORE Slate wraps it**
        MainWidget->SetUIDataManager(Manager);
        UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Injected manager into main widget"));
        
        // **STEP 4: Create the tab with Slate stretching wrapper (GPT-5 recommendation)**
        TSharedRef<SDockTab> Tab = SNew(SDockTab)
            .TabRole(ETabRole::NomadTab)
            .Label(LOCTEXT("MagicOptimizerTabTitle", "Magic Optimizer"))
            [
                SNew(SBorder).Padding(0)
                [
                    SNew(SBox).HAlign(HAlign_Fill).VAlign(VAlign_Fill)
                    [
                        MainWidget->TakeWidget()
                    ]
                ]
            ];
            
        MagicOptimizerTab = Tab;
        return Tab;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MagicOptimizer: Failed to create Main Widget"));
    }

    // If we get here, something went wrong - create a fallback
    UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: All widget creation attempts failed, using fallback Slate UI"));
    
    // Fallback to simple Slate UI
    TSharedRef<SWidget> FallbackWidget = SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(10)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("ErrorText", "Magic Optimizer - Widget Creation Failed"))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
            .ColorAndOpacity(FLinearColor::Red)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(10)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("DebugText", "Check the output log for detailed error messages"))
            .ColorAndOpacity(FLinearColor::Yellow)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(10)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("InfoText", "This is a fallback Slate UI while debugging the UMG widget issue"))
            .ColorAndOpacity(FLinearColor::White)
        ];

    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        .Label(LOCTEXT("MagicOptimizerTabTitle", "Magic Optimizer"))
        [
            FallbackWidget
        ];
}

UMagicOptimizerUIDataManager* FMagicOptimizerModule::GetOrCreateUIDataManager()
{
    // Check if we already have a valid manager
    if (UIDataManagerKeeper.IsValid())
    {
        return UIDataManagerKeeper.Get();
    }

    // Create new manager instance
    UMagicOptimizerUIDataManager* Manager = NewObject<UMagicOptimizerUIDataManager>(GetTransientPackage(), UMagicOptimizerUIDataManager::StaticClass());
    Manager->AddToRoot();     // Prevent GC since outer is transient
    Manager->Initialize();    // Initialize the manager
    
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Created new UIDataManager instance"));
    return Manager;
}

void FMagicOptimizerModule::OnTabClosed(TSharedRef<SDockTab> Tab)
{
    UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Tab closed, cleaning up references"));
    
    // Clean up strong references when tab is closed
    MainWidgetKeeper.Reset();
    UIDataManagerKeeper.Reset();
    MagicOptimizerTab.Reset();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMagicOptimizerModule, MagicOptimizer)
