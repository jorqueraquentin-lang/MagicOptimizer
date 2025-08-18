// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"

// Performance statistics group for MagicOptimizer
DECLARE_STATS_GROUP(TEXT("MagicOptimizer"), STATGROUP_MagicOptimizer, STATCAT_Advanced);

// Cycle counters for optimization operations
DECLARE_CYCLE_STAT(TEXT("Optimization Pass Time"), STAT_MagicOpt_PassTime, STATGROUP_MagicOptimizer);
DECLARE_CYCLE_STAT(TEXT("Texture Optimization Time"), STAT_MagicOpt_TextureTime, STATGROUP_MagicOptimizer);
DECLARE_CYCLE_STAT(TEXT("Mesh Optimization Time"), STAT_MagicOpt_MeshTime, STATGROUP_MagicOptimizer);
DECLARE_CYCLE_STAT(TEXT("Material Optimization Time"), STAT_MagicOpt_MaterialTime, STATGROUP_MagicOptimizer);

// Memory statistics
DECLARE_DWORD_COUNTER_STAT(TEXT("Assets Processed"), STAT_MagicOpt_AssetsProcessed, STATGROUP_MagicOptimizer);
DECLARE_DWORD_COUNTER_STAT(TEXT("Assets Optimized"), STAT_MagicOpt_AssetsOptimized, STATGROUP_MagicOptimizer);
DECLARE_DWORD_COUNTER_STAT(TEXT("Memory Saved (MB)"), STAT_MagicOpt_MemorySaved, STATGROUP_MagicOptimizer);

// Error and warning counters
DECLARE_DWORD_COUNTER_STAT(TEXT("Optimization Errors"), STAT_MagicOpt_Errors, STATGROUP_MagicOptimizer);
DECLARE_DWORD_COUNTER_STAT(TEXT("Optimization Warnings"), STAT_MagicOpt_Warnings, STATGROUP_MagicOptimizer);

// Performance tracking macros for easy use
#define MAGICOPT_SCOPE_CYCLE_COUNTER(StatName) \
    SCOPE_CYCLE_COUNTER(StatName)

#define MAGICOPT_QUICK_SCOPE_CYCLE_COUNTER(StatName) \
    QUICK_SCOPE_CYCLE_COUNTER(StatName)

// Memory tracking macros
#define MAGICOPT_TRACK_MEMORY(StatName, Value) \
    INC_DWORD_STAT_BY(StatName, Value)

#define MAGICOPT_TRACK_ASSETS_PROCESSED(Count) \
    INC_DWORD_STAT_BY(STAT_MagicOpt_AssetsProcessed, Count)

#define MAGICOPT_TRACK_ASSETS_OPTIMIZED(Count) \
    INC_DWORD_STAT_BY(STAT_MagicOpt_AssetsOptimized, Count)

#define MAGICOPT_TRACK_MEMORY_SAVED(MegaBytes) \
    INC_DWORD_STAT_BY(STAT_MagicOpt_MemorySaved, MegaBytes)

#define MAGICOPT_TRACK_ERRORS(Count) \
    INC_DWORD_STAT_BY(STAT_MagicOpt_Errors, Count)

#define MAGICOPT_TRACK_WARNINGS(Count) \
    INC_DWORD_STAT_BY(STAT_MagicOpt_Warnings, Count)
