#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SWidget.h"

// Custom log category for MagicOptimizer
DECLARE_LOG_CATEGORY_EXTERN(LogMagicOptimizer, Log, All);

/**
 * Magic Optimizer Plugin Module
 * 
 * Developed by Perseus XR PTY LTD
 * Website: https://perseusxr.com
 * 
 * CRITICAL DESIGN PRINCIPLES TO PREVENT DUPLICATE SETTINGS:
 * 1. NEVER manually register settings - let UDeveloperSettings handle it automatically
 * 2. NEVER use ISettingsModule directly in plugin code
 * 3. ALWAYS use config = Editor (not defaultconfig) to prevent multiple config sections
 * 4. ALWAYS consolidate settings in ONE location (plugin config directory)
 * 5. ALWAYS force rebuild after settings changes
 * 
 * This module provides the core functionality for the Magic Optimizer plugin
 * without any manual settings registration to avoid conflicts.
 */
class FMagicOptimizerModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    /** This function will be bound to Command (by default it will bring up plugin window) */
    void PluginButtonClicked();

private:
    void AddMenuExtension(FMenuBuilder& Builder);
    void AddToolbarButton(FToolBarBuilder& Builder);
    void RegisterMenus();
    TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);
    void OpenSettings();
    
    // Manager lifecycle management
    class UMagicOptimizerUIDataManager* GetOrCreateUIDataManager();
    void OnTabClosed(TSharedRef<SDockTab> Tab);

    TSharedPtr<class FUICommandList> PluginCommands;
    TSharedPtr<SDockTab> MagicOptimizerTab;
    TSharedPtr<class SWidget> MagicOptimizerWidget;

    // Strong references to prevent GC
    TStrongObjectPtr<class UMagicOptimizerMainWidget> MainWidgetKeeper;
    TStrongObjectPtr<class UMagicOptimizerUIDataManager> UIDataManagerKeeper;

    // NO SETTINGS REGISTRATION FUNCTIONS - This prevents duplicate registration
    // UDeveloperSettings automatically handles all settings registration
};
