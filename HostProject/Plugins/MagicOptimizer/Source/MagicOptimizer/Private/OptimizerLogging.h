#pragma once

#include "CoreMinimal.h"

namespace MagicOptimizerLog
{
	// Returns the absolute path to the plugin log file under Project/Saved/MagicOptimizer
	FString GetLogFilePath();

	// Appends a single line with timestamp to the log file (creates directories on demand)
	void AppendLine(const FString& Line);
}


