#include "MagicOptimizerEditor.h"
#include "SOptimizerPanel.h"
#include "OptimizerSettings.h"
#include "MagicOptimizerSubsystem.h"
#include "MagicOptimizerLogging.h"
#include "LevelEditor.h"
#include "ISettingsModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Styling/AppStyle.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "ToolMenus.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandList.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "FMagicOptimizerEditorModule"

// Modern ToolMenus command system
class FMagicOptimizerCommands final : public TCommands<FMagicOptimizerCommands>
{
public:
    FMagicOptimizerCommands()
        : TCommands<FMagicOptimizerCommands>(
            TEXT("MagicOptimizer"), 
            NSLOCTEXT("MagicOptimizer", "MagicOptimizer", "Magic Optimizer"),
            NAME_None, 
            TEXT("EditorStyle"))
    {}

    virtual void RegisterCommands() override
    {
        UI_COMMAND(RunOptimization, "Run Optimization", "Run full optimization pass", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(RunTextureOptimization, "Texture Optimization", "Run texture optimization", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(RunMeshOptimization, "Mesh Optimization", "Run mesh optimization", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(RunMaterialOptimization, "Material Optimization", "Run material optimization", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(OpenPanel, "Open Panel", "Open Magic Optimizer panel", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(ShowStatus, "Show Status", "Show optimization status", EUserInterfaceActionType::Button, FInputChord());
    }

    TSharedPtr<FUICommandInfo> RunOptimization;
    TSharedPtr<FUICommandInfo> RunTextureOptimization;
    TSharedPtr<FUICommandInfo> RunMeshOptimization;
    TSharedPtr<FUICommandInfo> RunMaterialOptimization;
    TSharedPtr<FUICommandInfo> OpenPanel;
    TSharedPtr<FUICommandInfo> ShowStatus;
};

void FMagicOptimizerEditorModule::StartupModule()
{
    if (!GIsEditor)
    {
        return;
    }

    // Register commands
    FMagicOptimizerCommands::Register();
    CommandList = MakeShared<FUICommandList>();

    // Bind commands to actions
    CommandList->MapAction(
        FMagicOptimizerCommands::Get().RunOptimization,
        FExecuteAction::CreateRaw(this, &FMagicOptimizerEditorModule::ExecuteRunOptimization)
    );
    
    CommandList->MapAction(
        FMagicOptimizerCommands::Get().RunTextureOptimization,
        FExecuteAction::CreateRaw(this, &FMagicOptimizerEditorModule::ExecuteRunTextureOptimization)
    );
    
    CommandList->MapAction(
        FMagicOptimizerCommands::Get().RunMeshOptimization,
        FExecuteAction::CreateRaw(this, &FMagicOptimizerEditorModule::ExecuteRunMeshOptimization)
    );
    
    CommandList->MapAction(
        FMagicOptimizerCommands::Get().RunMaterialOptimization,
        FExecuteAction::CreateRaw(this, &FMagicOptimizerEditorModule::ExecuteRunMaterialOptimization)
    );
    
    CommandList->MapAction(
        FMagicOptimizerCommands::Get().OpenPanel,
        FExecuteAction::CreateRaw(this, &FMagicOptimizerEditorModule::OpenOptimizerPanel)
    );
    
    CommandList->MapAction(
        FMagicOptimizerCommands::Get().ShowStatus,
        FExecuteAction::CreateRaw(this, &FMagicOptimizerEditorModule::ShowOptimizationStatus)
    );

    // Register settings
    if (UOptimizerSettings* Settings = UOptimizerSettings::Get())
    {
        Settings->GetDefaultSettings();
        if (FModuleManager::Get().IsModuleLoaded("Settings"))
        {
            ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
            SettingsModule.RegisterSettings(TEXT("Project"), TEXT("Plugins"), TEXT("MagicOptimizer"), LOCTEXT("MagicOptimizerSettingsName", "Magic Optimizer"), LOCTEXT("MagicOptimizerSettingsDesc", "Settings for the Magic Optimizer plugin."), Settings);
        }
    }

    // Register tab spawner
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("MagicOptimizer"), FOnSpawnTab::CreateRaw(this, &FMagicOptimizerEditorModule::OnSpawnPluginTab))
        .SetDisplayName(LOCTEXT("MagicOptimizerTab", "Magic Optimizer"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);

    // Register ToolMenus callback
    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMagicOptimizerEditorModule::RegisterMenus));

    // Legacy menu extension for backward compatibility
    if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
    {
        FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
        if (LevelEditorModule.GetMenuExtensibilityManager())
        {
            TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
            MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateRaw(this, &FMagicOptimizerEditorModule::AddMenuExtension));
            LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
        }
    }
}

void FMagicOptimizerEditorModule::ShutdownModule()
{
    // Unregister ToolMenus
    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);
    
    // Unregister commands
    FMagicOptimizerCommands::Unregister();
    
    // Unregister settings
    if (FModuleManager::Get().IsModuleLoaded("Settings"))
    {
        ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
        SettingsModule.UnregisterSettings(TEXT("Project"), TEXT("Plugins"), TEXT("MagicOptimizer"));
    }
    
    // Unregister tab spawner
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("MagicOptimizer"));
}

void FMagicOptimizerEditorModule::RegisterMenus()
{
    // Add to main menu
    UToolMenu* MainMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
    FToolMenuSection& Section = MainMenu->AddSection("MagicOptimizer", LOCTEXT("MagicOptimizer", "Magic Optimizer"));
    
    Section.AddMenuEntry(FMagicOptimizerCommands::Get().OpenPanel);
    Section.AddSeparator("OptimizationActions");
    Section.AddMenuEntry(FMagicOptimizerCommands::Get().RunOptimization);
    Section.AddMenuEntry(FMagicOptimizerCommands::Get().RunTextureOptimization);
    Section.AddMenuEntry(FMagicOptimizerCommands::Get().RunMeshOptimization);
    Section.AddMenuEntry(FMagicOptimizerCommands::Get().RunMaterialOptimization);
    Section.AddSeparator("StatusActions");
    Section.AddMenuEntry(FMagicOptimizerCommands::Get().ShowStatus);
    
    // Add to toolbar
    UToolMenu* Toolbar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
    FToolMenuSection& ToolbarSection = Toolbar->AddSection("MagicOptimizer");
    ToolbarSection.AddMenuEntry(FMagicOptimizerCommands::Get().RunOptimization);
}

TSharedRef<SDockTab> FMagicOptimizerEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SOptimizerPanel)
        ];
}

void FMagicOptimizerEditorModule::AddMenuExtension(FMenuBuilder& Builder)
{
    Builder.AddMenuEntry(
        LOCTEXT("MagicOptimizerMenu", "Magic Optimizer"),
        LOCTEXT("MagicOptimizerMenuTooltip", "Open Magic Optimizer panel for asset optimization"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.BlueprintCore"),
        FUIAction(FExecuteAction::CreateRaw(this, &FMagicOptimizerEditorModule::OpenOptimizerPanel))
    );
}

void FMagicOptimizerEditorModule::OpenOptimizerPanel()
{
    FGlobalTabmanager::Get()->TryInvokeTab(FName("MagicOptimizer"));
}

// Command execution methods
void FMagicOptimizerEditorModule::ExecuteRunOptimization()
{
    if (GEngine)
    {
        if (UMagicOptimizerSubsystem* Subsystem = GEngine->GetEngineSubsystem<UMagicOptimizerSubsystem>())
        {
            Subsystem->RunOptimizationPass();
            UE_LOG(LogMagicOptimizer, Display, TEXT("Full optimization pass started from editor"));
        }
        else
        {
            UE_LOG(LogMagicOptimizer, Error, TEXT("Magic Optimizer subsystem not found"));
        }
    }
}

void FMagicOptimizerEditorModule::ExecuteRunTextureOptimization()
{
    if (GEngine)
    {
        if (UMagicOptimizerSubsystem* Subsystem = GEngine->GetEngineSubsystem<UMagicOptimizerSubsystem>())
        {
            Subsystem->RunTextureOptimization();
            UE_LOG(LogMagicOptimizer, Display, TEXT("Texture optimization started from editor"));
        }
        else
        {
            UE_LOG(LogMagicOptimizer, Error, TEXT("Magic Optimizer subsystem not found"));
        }
    }
}

void FMagicOptimizerEditorModule::ExecuteRunMeshOptimization()
{
    if (GEngine)
    {
        if (UMagicOptimizerSubsystem* Subsystem = GEngine->GetEngineSubsystem<UMagicOptimizerSubsystem>())
        {
            Subsystem->RunMeshOptimization();
            UE_LOG(LogMagicOptimizer, Display, TEXT("Mesh optimization started from editor"));
        }
        else
        {
            UE_LOG(LogMagicOptimizer, Error, TEXT("Magic Optimizer subsystem not found"));
        }
    }
}

void FMagicOptimizerEditorModule::ExecuteRunMaterialOptimization()
{
    if (GEngine)
    {
        if (UMagicOptimizerSubsystem* Subsystem = GEngine->GetEngineSubsystem<UMagicOptimizerSubsystem>())
        {
            Subsystem->RunMaterialOptimization();
            UE_LOG(LogMagicOptimizer, Display, TEXT("Material optimization started from editor"));
        }
        else
        {
            UE_LOG(LogMagicOptimizer, Error, TEXT("Magic Optimizer subsystem not found"));
        }
    }
}

void FMagicOptimizerEditorModule::ShowOptimizationStatus()
{
    if (GEngine)
    {
        if (UMagicOptimizerSubsystem* Subsystem = GEngine->GetEngineSubsystem<UMagicOptimizerSubsystem>())
        {
            UE_LOG(LogMagicOptimizer, Display, TEXT("Magic Optimizer Status:"));
            UE_LOG(LogMagicOptimizer, Display, TEXT("  Running: %s"), Subsystem->IsOptimizationRunning() ? TEXT("Yes") : TEXT("No"));
            UE_LOG(LogMagicOptimizer, Display, TEXT("  Last Time: %.2f seconds"), Subsystem->GetLastOptimizationTime());
            UE_LOG(LogMagicOptimizer, Display, TEXT("  Assets Optimized: %d"), Subsystem->GetOptimizedAssetCount());
        }
        else
        {
            UE_LOG(LogMagicOptimizer, Error, TEXT("Magic Optimizer subsystem not found"));
        }
    }
}

void FMagicOptimizerEditorModule::RegisterConsoleCommands() {}
void FMagicOptimizerEditorModule::UnregisterConsoleCommands() {}
void FMagicOptimizerEditorModule::ConsoleRun(const TArray<FString>&) {}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMagicOptimizerEditorModule, MagicOptimizerEditor)


