// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMagicOptimizerModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Tab spawner callback */
	TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);

	/** Menu extension callback */
	void AddMenuExtension(FMenuBuilder& Builder);

	/** Open optimizer panel */
	void OpenOptimizerPanel();

private:
	// Console command: MagicOptimizer.Run [Phase] [CategoriesCsv]
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();
	void ConsoleRun(const TArray<FString>& Args);

	struct IConsoleCommand* ConsoleRunCommand = nullptr;
	struct IConsoleCommand* ConsoleOpenCommand = nullptr;
};
