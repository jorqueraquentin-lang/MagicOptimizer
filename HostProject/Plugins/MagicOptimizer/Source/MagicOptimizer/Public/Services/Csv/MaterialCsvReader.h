// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

/*
  MaterialCsvReader.h
  Part of the MagicOptimizer Unreal Engine plugin.
  Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.
*/
#pragma once

#include "CoreMinimal.h"
#include "ViewModels/MaterialModels.h"

class UOptimizerSettings;

namespace MaterialCsvReader
{
	// Reads audit CSV (materials.csv) into OutRows. Returns true if file existed and was parsed.
	MAGICOPTIMIZER_API bool ReadAuditCsv(const UOptimizerSettings* OptimizerSettings, TArray<FMaterialAuditRowPtr>& OutRows);

	// Reads recommendations CSV (materials_recommend.csv) into OutRows. Returns true if file existed and was parsed.
	MAGICOPTIMIZER_API bool ReadRecommendationsCsv(const UOptimizerSettings* OptimizerSettings, TArray<FMaterialRecRowPtr>& OutRows);
}
