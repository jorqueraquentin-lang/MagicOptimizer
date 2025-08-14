// Copyright Epic Games, Inc. All Rights Reserved.

#include "MagicOptimizer.h"
#include "SOptimizerPanel.h"
#include "OptimizerSettings.h"
#include "PythonBridge.h"
#include "OptimizerRun.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "LevelEditor.h"

#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Docking/SDockTab.h"
#include "Styling/AppStyle.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FMagicOptimizerModule"

void FMagicOptimizerModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Starting module initialization..."));

	// Check if we're in the editor
	if (!GIsEditor)
	{
		UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: Not in editor mode, skipping initialization"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Editor mode confirmed, proceeding with initialization"));

	// Check if required modules are available
	if (!FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: LevelEditor module not loaded, attempting to load it"));
		FModuleManager::Get().LoadModule("LevelEditor");
	}

	if (!FModuleManager::Get().IsModuleLoaded("PythonScriptPlugin"))
	{
		UE_LOG(LogTemp, Warning, TEXT("MagicOptimizer: PythonScriptPlugin module not loaded, attempting to load it"));
		FModuleManager::Get().LoadModule("PythonScriptPlugin");
	}

	// Register settings with error handling
	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Initializing settings..."));
	
	if (UOptimizerSettings* Settings = UOptimizerSettings::Get())
	{
		Settings->GetDefaultSettings();
		UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Settings initialized successfully"));

		// Ensure settings section is registered so we can deep-link the Project Settings view
		if (FModuleManager::Get().IsModuleLoaded("Settings"))
		{
			ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
			SettingsModule.RegisterSettings(
				TEXT("Project"),
				TEXT("Plugins"),
				TEXT("MagicOptimizer"),
				LOCTEXT("MagicOptimizerSettingsName", "Magic Optimizer"),
				LOCTEXT("MagicOptimizerSettingsDesc", "Settings for the Magic Optimizer plugin."),
				Settings
			);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MagicOptimizer: Failed to get OptimizerSettings instance"));
	}

	// Register tab spawner with error handling
	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Registering tab spawner..."));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("MagicOptimizer"),
		FOnSpawnTab::CreateRaw(this, &FMagicOptimizerModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("MagicOptimizerTab", "Magic Optimizer"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Tab spawner registered successfully"));

	// Add menu extension to Window menu with comprehensive error handling
	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Adding Window menu extension..."));
	
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		
		if (LevelEditorModule.GetMenuExtensibilityManager())
		{
			TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
			MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateRaw(this, &FMagicOptimizerModule::AddMenuExtension));
			LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

			UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Window menu extension added successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("MagicOptimizer: MenuExtensibilityManager not available"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MagicOptimizer: LevelEditor module not available for menu extension"));
	}

	// Alternative: Add to Tools menu as well
	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Adding Tools menu extension..."));
	
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule2 = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		
		if (LevelEditorModule2.GetMenuExtensibilityManager())
		{
			TSharedPtr<FExtender> ToolsMenuExtender = MakeShareable(new FExtender());
			ToolsMenuExtender->AddMenuExtension("AssetEditorToolbar", EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateRaw(this, &FMagicOptimizerModule::AddMenuExtension));
			LevelEditorModule2.GetMenuExtensibilityManager()->AddExtender(ToolsMenuExtender);

			UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Tools menu extension added successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("MagicOptimizer: MenuExtensibilityManager not available for Tools menu"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MagicOptimizer: LevelEditor module not available for Tools menu extension"));
	}

	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer module started successfully"));

	// Register console commands for headless/manual triggering
	RegisterConsoleCommands();
}

void FMagicOptimizerModule::ShutdownModule()
{
	UnregisterConsoleCommands();
	// Unregister settings section if present
	if (FModuleManager::Get().IsModuleLoaded("Settings"))
	{
		ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
		SettingsModule.UnregisterSettings(TEXT("Project"), TEXT("Plugins"), TEXT("MagicOptimizer"));
	}

	// Unregister tab spawner
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("MagicOptimizer"));

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
		LOCTEXT("MagicOptimizerMenuTooltip", "Open Magic Optimizer panel for asset optimization"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.BlueprintCore"),
		FUIAction(FExecuteAction::CreateRaw(this, &FMagicOptimizerModule::OpenOptimizerPanel))
	);

	// Add a separator for better visibility
	Builder.AddSeparator();
}

void FMagicOptimizerModule::OpenOptimizerPanel()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("MagicOptimizer"));
}

void FMagicOptimizerModule::RegisterConsoleCommands()
{
	if (ConsoleRunCommand == nullptr)
	{
		ConsoleRunCommand = IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("MagicOptimizer.Run"),
			TEXT("Run MagicOptimizer phase inside the open editor. Usage: MagicOptimizer.Run [Phase=Audit] [Categories=Textures,Meshes,Materials]"),
			FConsoleCommandWithArgsDelegate::CreateRaw(this, &FMagicOptimizerModule::ConsoleRun),
			ECVF_Default);
	}
}

void FMagicOptimizerModule::UnregisterConsoleCommands()
{
	if (ConsoleRunCommand)
	{
		IConsoleManager::Get().UnregisterConsoleObject(ConsoleRunCommand);
		ConsoleRunCommand = nullptr;
	}
}

void FMagicOptimizerModule::ConsoleRun(const TArray<FString>& Args)
{
	FString Phase = Args.Num() > 0 ? Args[0] : TEXT("Audit");
	FString CategoriesCsv = Args.Num() > 1 ? Args[1] : TEXT("Textures,Meshes,Materials");

	TArray<FString> Categories;
	CategoriesCsv.ParseIntoArray(Categories, TEXT(","), /*CullEmpty*/ true);

	UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: ConsoleRun Phase=%s Categories=%s"), *Phase, *CategoriesCsv);

	UOptimizerRun* Runner = NewObject<UOptimizerRun>();
	if (Runner)
	{
		Runner->Run(Phase, Categories);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMagicOptimizerModule, MagicOptimizer)