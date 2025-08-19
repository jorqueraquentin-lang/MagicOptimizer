// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#include "MagicOptimizerSubsystem.h"
#include "OptimizerSettings.h"
#include "MagicOptimizerCVars.h"
#include "MagicOptimizerStats.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Tasks/Task.h"
#include "Async/Async.h"
#include "HAL/PlatformTime.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "MagicOptimizerLogging.h"

UMagicOptimizerSubsystem::UMagicOptimizerSubsystem()
{
    bOptimizationRunning = false;
    LastOptimizationTime = 0.0f;
    OptimizedAssetCount = 0;
    OptimizationStartTime = 0.0;
    CurrentOptimizationType = TEXT("");
}

void UMagicOptimizerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogMagicOptimizer, Log, TEXT("MagicOptimizer Subsystem initialized"));
    
    // Initialize settings
    if (UOptimizerSettings* Settings = UOptimizerSettings::Get())
    {
        Settings->GetDefaultSettings();
    }
    
    // Log CVar status if verbose mode is enabled
    if (MagicOptimizerCVars::IsVerbose())
    {
        UE_LOG(LogMagicOptimizer, Log, TEXT("MagicOptimizer CVars: Enabled=%d, MaxChanges=%d, Timeout=%.1f, DryRun=%d"),
            MagicOptimizerCVars::IsEnabled(),
            MagicOptimizerCVars::GetMaxChanges(),
            MagicOptimizerCVars::GetTimeout(),
            MagicOptimizerCVars::IsDryRun());
    }
}

void UMagicOptimizerSubsystem::Deinitialize()
{
    // Ensure any running optimizations are stopped
    if (bOptimizationRunning)
    {
        UE_LOG(LogMagicOptimizer, Warning, TEXT("MagicOptimizer Subsystem deinitializing while optimization is running"));
    }
    
    Super::Deinitialize();
    
    UE_LOG(LogMagicOptimizer, Log, TEXT("MagicOptimizer Subsystem deinitialized"));
}

void UMagicOptimizerSubsystem::RunOptimizationPass()
{
    if (!MagicOptimizerCVars::IsEnabled())
    {
        UE_LOG(LogMagicOptimizer, Warning, TEXT("MagicOptimizer is disabled via CVar"));
        return;
    }
    
    if (bOptimizationRunning)
    {
        UE_LOG(LogMagicOptimizer, Warning, TEXT("Optimization already running, skipping request"));
        return;
    }

    CurrentOptimizationType = TEXT("Full");
    ExecuteOptimizationPass();
}

void UMagicOptimizerSubsystem::RunTextureOptimization()
{
    if (!MagicOptimizerCVars::IsEnabled())
    {
        UE_LOG(LogMagicOptimizer, Warning, TEXT("MagicOptimizer is disabled via CVar"));
        return;
    }
    
    if (bOptimizationRunning)
    {
        UE_LOG(LogMagicOptimizer, Warning, TEXT("Optimization already running, skipping request"));
        return;
    }

    CurrentOptimizationType = TEXT("Texture");
    ExecuteOptimizationPass();
}

void UMagicOptimizerSubsystem::RunMeshOptimization()
{
    if (!MagicOptimizerCVars::IsEnabled())
    {
        UE_LOG(LogMagicOptimizer, Warning, TEXT("MagicOptimizer is disabled via CVar"));
        return;
    }
    
    if (bOptimizationRunning)
    {
        UE_LOG(LogMagicOptimizer, Warning, TEXT("Optimization already running, skipping request"));
        return;
    }

    CurrentOptimizationType = TEXT("Mesh");
    ExecuteOptimizationPass();
}

void UMagicOptimizerSubsystem::RunMaterialOptimization()
{
    if (!MagicOptimizerCVars::IsEnabled())
    {
        UE_LOG(LogMagicOptimizer, Warning, TEXT("MagicOptimizer is disabled via CVar"));
        return;
    }
    
    if (bOptimizationRunning)
    {
        UE_LOG(LogMagicOptimizer, Warning, TEXT("Optimization already running, skipping request"));
        return;
    }

    CurrentOptimizationType = TEXT("Material");
    ExecuteOptimizationPass();
}

void UMagicOptimizerSubsystem::ExecuteOptimizationPass()
{
    bOptimizationRunning = true;
    StartPerformanceTracking();
    
    UE_LOG(LogMagicOptimizer, Log, TEXT("Starting %s optimization pass"), *CurrentOptimizationType);
    
    // Track assets processed
    MAGICOPT_TRACK_ASSETS_PROCESSED(1);
    
    // Launch optimization work on background thread using UE5.6 Task system
    UE::Tasks::Launch(UE_SOURCE_LOCATION, [this]()
    {
        // Simulate heavy optimization work (replace with actual optimization logic)
        FPlatformProcess::Sleep(2.0f); // Simulate work
        
        // Marshal back to game thread for completion
        AsyncTask(ENamedThreads::GameThread, [this]()
        {
            OnOptimizationComplete(true, FString::Printf(TEXT("%s optimization completed successfully"), *CurrentOptimizationType));
        });
    });
}

void UMagicOptimizerSubsystem::OnOptimizationComplete(bool bSuccess, const FString& ResultMessage)
{
    EndPerformanceTracking();
    bOptimizationRunning = false;
    
    if (bSuccess)
    {
        OptimizedAssetCount += 10; // Simulate optimized assets
        MAGICOPT_TRACK_ASSETS_OPTIMIZED(10);
        MAGICOPT_TRACK_MEMORY_SAVED(5); // Simulate 5MB saved
        
        UE_LOG(LogMagicOptimizer, Log, TEXT("%s"), *ResultMessage);
    }
    else
    {
        MAGICOPT_TRACK_ERRORS(1);
        UE_LOG(LogMagicOptimizer, Error, TEXT("Optimization failed: %s"), *ResultMessage);
    }
}

void UMagicOptimizerSubsystem::StartPerformanceTracking()
{
    OptimizationStartTime = FPlatformTime::Seconds();
    
    // Start cycle counter based on optimization type
    if (MagicOptimizerCVars::IsPerformanceTrackingEnabled())
    {
        if (CurrentOptimizationType == TEXT("Texture"))
        {
            MAGICOPT_SCOPE_CYCLE_COUNTER(STAT_MagicOpt_TextureTime);
        }
        else if (CurrentOptimizationType == TEXT("Mesh"))
        {
            MAGICOPT_SCOPE_CYCLE_COUNTER(STAT_MagicOpt_MeshTime);
        }
        else if (CurrentOptimizationType == TEXT("Material"))
        {
            MAGICOPT_SCOPE_CYCLE_COUNTER(STAT_MagicOpt_MaterialTime);
        }
        else
        {
            MAGICOPT_SCOPE_CYCLE_COUNTER(STAT_MagicOpt_PassTime);
        }
    }
}

void UMagicOptimizerSubsystem::EndPerformanceTracking()
{
    if (OptimizationStartTime > 0.0)
    {
        LastOptimizationTime = (float)(FPlatformTime::Seconds() - OptimizationStartTime);
        OptimizationStartTime = 0.0;
    }
}

UOptimizerSettings* UMagicOptimizerSubsystem::GetOptimizerSettings() const
{
    return UOptimizerSettings::Get();
}
