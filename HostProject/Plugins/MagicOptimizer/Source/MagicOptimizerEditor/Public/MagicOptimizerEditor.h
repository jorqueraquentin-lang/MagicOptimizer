#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class MAGICOPTIMIZEREDITOR_API FMagicOptimizerEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
    void AddMenuExtension(class FMenuBuilder& Builder);
    void OpenOptimizerPanel();

private:
    void RegisterConsoleCommands();
    void UnregisterConsoleCommands();
    void ConsoleRun(const TArray<FString>& Args);

    struct IConsoleCommand* ConsoleRunCommand = nullptr;
    struct IConsoleCommand* ConsoleOpenCommand = nullptr;
};


