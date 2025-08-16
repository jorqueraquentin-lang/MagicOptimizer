/*
  OptimizerLogging.h
  Part of the MagicOptimizer Unreal Engine plugin.
  Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.
*/
#pragma once

#include "CoreMinimal.h"

// Export free functions from the runtime module for use by other modules
namespace MagicOptimizerLog
{
	// Returns the absolute path to the plugin log file under Project/Saved/MagicOptimizer
	MAGICOPTIMIZER_API FString GetLogFilePath();

	// Returns the absolute path to the assistant-only backlog file
	MAGICOPTIMIZER_API FString GetBacklogFilePath();

	// Appends a single line with timestamp to the runtime log (creates directories on demand)
	MAGICOPTIMIZER_API void AppendLine(const FString& Line);

	// Appends a single line with timestamp to the assistant-only backlog
	MAGICOPTIMIZER_API void AppendBacklog(const FString& Line);
}
