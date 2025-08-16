// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#include "MagicOptimizer.h"
#include "MagicOptimizerLogging.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FMagicOptimizerModule"

void FMagicOptimizerModule::StartupModule()
{
	UE_LOG(LogMagicOptimizer, Log, TEXT("MagicOptimizer (Runtime) module startup"));
}

void FMagicOptimizerModule::ShutdownModule()
{
	UE_LOG(LogMagicOptimizer, Log, TEXT("MagicOptimizer (Runtime) module shutdown"));
}

// The runtime module deliberately contains no editor-only registrations.

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMagicOptimizerModule, MagicOptimizer)