// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

class FMenuExtensibilityManager;
class FExtender;

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
	/** Menu extensibility manager */
	TSharedPtr<FMenuExtensibilityManager> MenuExtensibilityManager;

	/** Menu extender */
	TSharedPtr<FExtender> MenuExtender;
};
