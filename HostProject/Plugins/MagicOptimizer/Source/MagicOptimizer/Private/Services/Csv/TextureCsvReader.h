// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

/*
  TextureCsvReader.h
  Part of the MagicOptimizer Unreal Engine plugin.
  Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.
*/
#pragma once

#include "CoreMinimal.h"
#include "ViewModels/TextureModels.h"

class UOptimizerSettings;

namespace TextureCsvReader
{
	// Reads audit CSV (textures.csv) into OutRows. Returns true if file existed and was parsed.
	bool ReadAuditCsv(const UOptimizerSettings* OptimizerSettings, TArray<FTextureAuditRowPtr>& OutRows);

	// Reads recommendations CSV (textures_recommend.csv) into OutRows. Returns true if file existed and was parsed.
	bool ReadRecommendationsCsv(const UOptimizerSettings* OptimizerSettings, TArray<FTextureRecRowPtr>& OutRows);
}


