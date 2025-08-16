#include "MagicOptimizerEditor.h"
#include "SOptimizerPanel.h"
#include "OptimizerSettings.h"
#include "LevelEditor.h"
#include "ISettingsModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Styling/AppStyle.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"

#define LOCTEXT_NAMESPACE "FMagicOptimizerEditorModule"

void FMagicOptimizerEditorModule::StartupModule()
{
    if (!GIsEditor)
    {
        return;
    }

    if (UOptimizerSettings* Settings = UOptimizerSettings::Get())
    {
        Settings->GetDefaultSettings();
        if (FModuleManager::Get().IsModuleLoaded("Settings"))
        {
            ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
            SettingsModule.RegisterSettings(TEXT("Project"), TEXT("Plugins"), TEXT("MagicOptimizer"), LOCTEXT("MagicOptimizerSettingsName", "Magic Optimizer"), LOCTEXT("MagicOptimizerSettingsDesc", "Settings for the Magic Optimizer plugin."), Settings);
        }
    }

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("MagicOptimizer"), FOnSpawnTab::CreateRaw(this, &FMagicOptimizerEditorModule::OnSpawnPluginTab))
        .SetDisplayName(LOCTEXT("MagicOptimizerTab", "Magic Optimizer"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);

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
    if (FModuleManager::Get().IsModuleLoaded("Settings"))
    {
        ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
        SettingsModule.UnregisterSettings(TEXT("Project"), TEXT("Plugins"), TEXT("MagicOptimizer"));
    }
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("MagicOptimizer"));
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

void FMagicOptimizerEditorModule::RegisterConsoleCommands() {}
void FMagicOptimizerEditorModule::UnregisterConsoleCommands() {}
void FMagicOptimizerEditorModule::ConsoleRun(const TArray<FString>&) {}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMagicOptimizerEditorModule, MagicOptimizerEditor)


