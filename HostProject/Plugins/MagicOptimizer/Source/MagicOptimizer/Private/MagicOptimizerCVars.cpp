// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#include "MagicOptimizerCVars.h"
#include "HAL/IConsoleManager.h"
#include "MagicOptimizerSubsystem.h"
#include "Engine/Engine.h"
#include "MagicOptimizerLogging.h"

// Console variables for MagicOptimizer runtime control
namespace MagicOptimizerCVars
{
    // Enable/disable the optimizer
    static int32 GMagicOptEnabled = 1;
    static FAutoConsoleVariableRef CVarMagicOptEnabled(
        TEXT("magicopt.Enabled"),
        GMagicOptEnabled,
        TEXT("Enable Magic Optimizer at runtime (0=disabled, 1=enabled)"),
        FConsoleVariableDelegate(),
        ECVF_Default);

    // Maximum number of changes per optimization pass
    static int32 GMagicOptMaxChanges = 100;
    static FAutoConsoleVariableRef CVarMagicOptMaxChanges(
        TEXT("magicopt.MaxChanges"),
        GMagicOptMaxChanges,
        TEXT("Maximum number of changes per optimization pass (default: 100)"),
        FConsoleVariableDelegate(),
        ECVF_Default);

    // Optimization timeout in seconds
    static float GMagicOptTimeout = 300.0f;
    static FAutoConsoleVariableRef CVarMagicOptTimeout(
        TEXT("magicopt.Timeout"),
        GMagicOptTimeout,
        TEXT("Optimization timeout in seconds (default: 300)"),
        FConsoleVariableDelegate(),
        ECVF_Default);

    // Verbose logging
    static int32 GMagicOptVerbose = 0;
    static FAutoConsoleVariableRef CVarMagicOptVerbose(
        TEXT("magicopt.Verbose"),
        GMagicOptVerbose,
        TEXT("Enable verbose logging (0=normal, 1=verbose)"),
        FConsoleVariableDelegate(),
        ECVF_Default);

    // Dry run mode
    static int32 GMagicOptDryRun = 1;
    static FAutoConsoleVariableRef CVarMagicOptDryRun(
        TEXT("magicopt.DryRun"),
        GMagicOptDryRun,
        TEXT("Enable dry run mode - don't apply changes (0=apply, 1=dry run)"),
        FConsoleVariableDelegate(),
        ECVF_Default);

    // Performance tracking
    static int32 GMagicOptPerformanceTracking = 1;
    static FAutoConsoleVariableRef CVarMagicOptPerformanceTracking(
        TEXT("magicopt.PerformanceTracking"),
        GMagicOptPerformanceTracking,
        TEXT("Enable performance tracking (0=disabled, 1=enabled)"),
        FConsoleVariableDelegate(),
        ECVF_Default);
}

// Console commands for MagicOptimizer
namespace MagicOptimizerCommands
{
    // Command to run optimization pass
    static FAutoConsoleCommand CmdMagicOptRun(
        TEXT("magicopt.Run"),
        TEXT("Run Magic Optimizer optimization pass"),
        FConsoleCommandDelegate::CreateLambda([]()
        {
            if (GEngine)
            {
                if (UMagicOptimizerSubsystem* Subsystem = GEngine->GetEngineSubsystem<UMagicOptimizerSubsystem>())
                {
                    Subsystem->RunOptimizationPass();
                    UE_LOG(LogMagicOptimizer, Display, TEXT("Magic Optimizer pass started via console command"));
                }
                else
                {
                    UE_LOG(LogMagicOptimizer, Error, TEXT("Magic Optimizer subsystem not found"));
                }
            }
        }));

    // Command to run texture optimization
    static FAutoConsoleCommand CmdMagicOptTextures(
        TEXT("magicopt.Textures"),
        TEXT("Run Magic Optimizer texture optimization"),
        FConsoleCommandDelegate::CreateLambda([]()
        {
            if (GEngine)
            {
                if (UMagicOptimizerSubsystem* Subsystem = GEngine->GetEngineSubsystem<UMagicOptimizerSubsystem>())
                {
                    Subsystem->RunTextureOptimization();
                    UE_LOG(LogMagicOptimizer, Display, TEXT("Magic Optimizer texture optimization started via console command"));
                }
                else
                {
                    UE_LOG(LogMagicOptimizer, Error, TEXT("Magic Optimizer subsystem not found"));
                }
            }
        }));

    // Command to run mesh optimization
    static FAutoConsoleCommand CmdMagicOptMeshes(
        TEXT("magicopt.Meshes"),
        TEXT("Run Magic Optimizer mesh optimization"),
        FConsoleCommandDelegate::CreateLambda([]()
        {
            if (GEngine)
            {
                if (UMagicOptimizerSubsystem* Subsystem = GEngine->GetEngineSubsystem<UMagicOptimizerSubsystem>())
                {
                    Subsystem->RunMeshOptimization();
                    UE_LOG(LogMagicOptimizer, Display, TEXT("Magic Optimizer mesh optimization started via console command"));
                }
                else
                {
                    UE_LOG(LogMagicOptimizer, Error, TEXT("Magic Optimizer subsystem not found"));
                }
            }
        }));

    // Command to run material optimization
    static FAutoConsoleCommand CmdMagicOptMaterials(
        TEXT("magicopt.Materials"),
        TEXT("Run Magic Optimizer material optimization"),
        FConsoleCommandDelegate::CreateLambda([]()
        {
            if (GEngine)
            {
                if (UMagicOptimizerSubsystem* Subsystem = GEngine->GetEngineSubsystem<UMagicOptimizerSubsystem>())
                {
                    Subsystem->RunMaterialOptimization();
                    UE_LOG(LogMagicOptimizer, Display, TEXT("Magic Optimizer material optimization started via console command"));
                }
                else
                {
                    UE_LOG(LogMagicOptimizer, Error, TEXT("Magic Optimizer subsystem not found"));
                }
            }
        }));

    // Command to show status
    static FAutoConsoleCommand CmdMagicOptStatus(
        TEXT("magicopt.Status"),
        TEXT("Show Magic Optimizer status"),
        FConsoleCommandDelegate::CreateLambda([]()
        {
            if (GEngine)
            {
                if (UMagicOptimizerSubsystem* Subsystem = GEngine->GetEngineSubsystem<UMagicOptimizerSubsystem>())
                {
                    UE_LOG(LogMagicOptimizer, Display, TEXT("Magic Optimizer Status:"));
                    UE_LOG(LogMagicOptimizer, Display, TEXT("  Running: %s"), Subsystem->IsOptimizationRunning() ? TEXT("Yes") : TEXT("No"));
                    UE_LOG(LogMagicOptimizer, Display, TEXT("  Last Time: %.2f seconds"), Subsystem->GetLastOptimizationTime());
                    UE_LOG(LogMagicOptimizer, Display, TEXT("  Assets Optimized: %d"), Subsystem->GetOptimizedAssetCount());
                }
                else
                {
                    UE_LOG(LogMagicOptimizer, Error, TEXT("Magic Optimizer subsystem not found"));
                }
            }
        }));

    // Command to show CVars
    static FAutoConsoleCommand CmdMagicOptCVars(
        TEXT("magicopt.CVars"),
        TEXT("Show Magic Optimizer console variables"),
        FConsoleCommandDelegate::CreateLambda([]()
        {
            UE_LOG(LogMagicOptimizer, Display, TEXT("Magic Optimizer Console Variables:"));
            UE_LOG(LogMagicOptimizer, Display, TEXT("  magicopt.Enabled: %d"), MagicOptimizerCVars::GMagicOptEnabled);
            UE_LOG(LogMagicOptimizer, Display, TEXT("  magicopt.MaxChanges: %d"), MagicOptimizerCVars::GMagicOptMaxChanges);
            UE_LOG(LogMagicOptimizer, Display, TEXT("  magicopt.Timeout: %.1f"), MagicOptimizerCVars::GMagicOptTimeout);
            UE_LOG(LogMagicOptimizer, Display, TEXT("  magicopt.Verbose: %d"), MagicOptimizerCVars::GMagicOptVerbose);
            UE_LOG(LogMagicOptimizer, Display, TEXT("  magicopt.DryRun: %d"), MagicOptimizerCVars::GMagicOptDryRun);
            UE_LOG(LogMagicOptimizer, Display, TEXT("  magicopt.PerformanceTracking: %d"), MagicOptimizerCVars::GMagicOptPerformanceTracking);
        }));
}

// Helper functions to access CVars from other parts of the plugin
namespace MagicOptimizerCVars
{
    bool IsEnabled() { return GMagicOptEnabled != 0; }
    int32 GetMaxChanges() { return GMagicOptMaxChanges; }
    float GetTimeout() { return GMagicOptTimeout; }
    bool IsVerbose() { return GMagicOptVerbose != 0; }
    bool IsDryRun() { return GMagicOptDryRun != 0; }
    bool IsPerformanceTrackingEnabled() { return GMagicOptPerformanceTracking != 0; }
}
