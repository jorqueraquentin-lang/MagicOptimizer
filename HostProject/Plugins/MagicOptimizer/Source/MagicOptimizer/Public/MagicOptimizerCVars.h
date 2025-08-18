// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Console variables and commands for MagicOptimizer runtime control.
 * This provides a clean interface for runtime configuration and debugging.
 */
namespace MagicOptimizerCVars
{
    // CVar access functions
    bool IsEnabled();
    int32 GetMaxChanges();
    float GetTimeout();
    bool IsVerbose();
    bool IsDryRun();
    bool IsPerformanceTrackingEnabled();
}
