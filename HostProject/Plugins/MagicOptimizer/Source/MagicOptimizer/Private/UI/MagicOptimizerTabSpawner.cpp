#include "UI/MagicOptimizerTabSpawner.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "UI/MagicOptimizerMainPanel.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "MagicOptimizerTabSpawner"

FName FMagicOptimizerTabSpawner::TabName = FName("MagicOptimizer");
TSharedPtr<SMagicOptimizerMainPanel> FMagicOptimizerTabSpawner::MainPanel = nullptr;

void FMagicOptimizerTabSpawner::RegisterTabSpawner()
{
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        TabName,
        FOnSpawnTab::CreateStatic(&FMagicOptimizerTabSpawner::SpawnTab)
    )
    .SetDisplayName(LOCTEXT("MagicOptimizerTabName", "Magic Optimizer"))
    .SetMenuType(ETabSpawnerMenuType::Hidden)
    .SetIcon(FSlateIcon("EditorStyle", "LevelEditor.Tabs.Viewports"));
}

void FMagicOptimizerTabSpawner::UnregisterTabSpawner()
{
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
}

TSharedRef<SDockTab> FMagicOptimizerTabSpawner::SpawnTab(const FSpawnTabArgs& SpawnTabArgs)
{
    // Create the main panel
    MainPanel = SNew(SMagicOptimizerMainPanel);
    
    TSharedRef<SDockTab> Tab = SNew(SDockTab)
    .TabRole(ETabRole::NomadTab)
    .Label(LOCTEXT("MagicOptimizerTabName", "Magic Optimizer"))
    [
        MainPanel.ToSharedRef()
    ];
    
    // Set the tab icon using the new UE5.6 API
    Tab->SetTabIcon(FAppStyle::GetBrush("LevelEditor.Tabs.Viewports"));
    
    return Tab;
}

#undef LOCTEXT_NAMESPACE
