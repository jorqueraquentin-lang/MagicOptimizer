#pragma once

#include "CoreMinimal.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "UI/MagicOptimizerMainPanel.h"

/**
 * Tab spawner for MagicOptimizer that creates and manages the main panel tab
 */
class MAGICOPTIMIZER_API FMagicOptimizerTabSpawner
{
public:
    // Register the tab spawner
    static void RegisterTabSpawner();
    
    // Unregister the tab spawner
    static void UnregisterTabSpawner();
    
    // Spawn the tab
    static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& SpawnTabArgs);

private:
    static FName TabName;
    static TSharedPtr<SMagicOptimizerMainPanel> MainPanel;
};
