/*
  OptimizerLogging.h
  Part of the MagicOptimizer Unreal Engine plugin.
  Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.
*/
#pragma once

#include "CoreMinimal.h"

namespace MagicOptimizerLog
{
	FString GetLogFilePath();
	FString GetBacklogFilePath();
	void AppendLine(const FString& Line);
	void AppendBacklog(const FString& Line);
}

#pragma once

#include "CoreMinimal.h"

namespace MagicOptimizerLog
{
	// Returns the absolute path to the plugin log file under Project/Saved/MagicOptimizer
	FString GetLogFilePath();

	// Returns the absolute path to the assistant-only backlog file
	FString GetBacklogFilePath();

	// Appends a single line with timestamp to the runtime log (creates directories on demand)
	void AppendLine(const FString& Line);

	// Appends a single line with timestamp to the assistant-only backlog
	void AppendBacklog(const FString& Line);
}


