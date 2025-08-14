// Copyright Epic Games, Inc. All Rights Reserved.

#include "MagicOptimizer.h"
#include "SOptimizerPanel.h"
#include "OptimizerSettings.h"
#include "PythonBridge.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidget.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Docking/SDockTab.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "FMagicOptimizerModule"

void FMagicOptimizerModule::StartupModule()
{
	// Register settings
	if (UOptimizerSettings* Settings = UOptimizerSettings::Get())
	{
		Settings->GetDefaultSettings();
	}

	// Register menu extension
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	// Add menu extension
	MenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
	MenuExtensibilityManager->AddExtender(MenuExtender);

	// Register tab spawner
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		"MagicOptimizer",
		FOnSpawnTab::CreateRaw(this, &FMagicOptimizerModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("MagicOptimizerTab", "Magic Optimizer"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	// Add menu item
	FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
	MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateRaw(this, &FMagicOptimizerModule::AddMenuExtension));
	LevelEditor.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer module started"));
}

void FMagicOptimizerModule::ShutdownModule()
{
	// Unregister tab spawner
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("MagicOptimizer");

	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer module shutdown"));
}

TSharedRef<SDockTab> FMagicOptimizerModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SOptimizerPanel)
		];
}

void FMagicOptimizerModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(
		LOCTEXT("MagicOptimizerMenu", "Magic Optimizer"),
		LOCTEXT("MagicOptimizerMenuTooltip", "Open Magic Optimizer panel"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.BlueprintCore"),
		FUIAction(FExecuteAction::CreateRaw(this, &FMagicOptimizerModule::OpenOptimizerPanel))
	);
}

void FMagicOptimizerModule::OpenOptimizerPanel()
{
	FGlobalTabmanager::Get()->TryInvokeTab("MagicOptimizer");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMagicOptimizerModule, MagicOptimizer)