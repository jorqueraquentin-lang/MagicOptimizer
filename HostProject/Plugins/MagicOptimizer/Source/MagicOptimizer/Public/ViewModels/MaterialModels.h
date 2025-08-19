// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"

// Material sort column enum
UENUM(BlueprintType)
enum class EMaterialSortColumn : uint8
{
	Path UMETA(DisplayName = "Path"),
	TextureCount UMETA(DisplayName = "Texture Count"),
	ShaderComplexity UMETA(DisplayName = "Shader Complexity"),
	Issues UMETA(DisplayName = "Issues")
};

// Material audit row
typedef TSharedPtr<struct FMaterialAuditRow> FMaterialAuditRowPtr;

struct FMaterialAuditRow
{
	FString Path;
	int32 TextureCount = 0;
	int32 ShaderComplexity = 0;
	FString Issues;
};

// Material recommendation row
typedef TSharedPtr<struct FMaterialRecRow> FMaterialRecRowPtr;

struct FMaterialRecRow
{
	FString Path;
	int32 TextureCount = 0;
	int32 ShaderComplexity = 0;
	FString Issues;
	FString Recommendations;
};
