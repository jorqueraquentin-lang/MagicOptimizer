// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

/*
  MeshCsvReader.h
  Part of the MagicOptimizer Unreal Engine plugin.
  Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.
*/
#pragma once

#include "CoreMinimal.h"
#include "ViewModels/MeshModels.h"

class UOptimizerSettings;

namespace MeshCsvReader
{
	// Reads audit CSV (meshes.csv) into OutRows. Returns true if file existed and was parsed.
	MAGICOPTIMIZER_API bool ReadAuditCsv(const UOptimizerSettings* OptimizerSettings, TArray<FMeshAuditRowPtr>& OutRows);

	// Reads recommendations CSV (meshes_recommend.csv) into OutRows. Returns true if file existed and was parsed.
	MAGICOPTIMIZER_API bool ReadRecommendationsCsv(const UOptimizerSettings* OptimizerSettings, TArray<FMeshRecRowPtr>& OutRows);
}
