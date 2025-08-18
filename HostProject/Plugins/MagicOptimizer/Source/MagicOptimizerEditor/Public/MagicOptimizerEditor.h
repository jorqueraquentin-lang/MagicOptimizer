#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Framework/Commands/UICommandList.h"

class MAGICOPTIMIZEREDITOR_API FMagicOptimizerEditorModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
    void AddMenuExtension(class FMenuBuilder& Builder);
    void OpenOptimizerPanel();

private:
    void RegisterMenus();
    void RegisterConsoleCommands();
    void UnregisterConsoleCommands();
    void ConsoleRun(const TArray<FString>& Args);

    // Command execution methods
    void ExecuteRunOptimization();
    void ExecuteRunTextureOptimization();
    void ExecuteRunMeshOptimization();
    void ExecuteRunMaterialOptimization();
    void ShowOptimizationStatus();

    struct IConsoleCommand* ConsoleRunCommand = nullptr;
    struct IConsoleCommand* ConsoleOpenCommand = nullptr;
    
    TSharedPtr<FUICommandList> CommandList;
};


