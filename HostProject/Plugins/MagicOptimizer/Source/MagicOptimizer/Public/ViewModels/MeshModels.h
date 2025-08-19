// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"

// Mesh sort column enum
UENUM(BlueprintType)
enum class EMeshSortColumn : uint8
{
	Path UMETA(DisplayName = "Path"),
	VertexCount UMETA(DisplayName = "Vertex Count"),
	TriangleCount UMETA(DisplayName = "Triangle Count"),
	LODCount UMETA(DisplayName = "LOD Count")
};

// Mesh audit row
typedef TSharedPtr<struct FMeshAuditRow> FMeshAuditRowPtr;

struct FMeshAuditRow
{
	FString Path;
	int32 VertexCount = 0;
	int32 TriangleCount = 0;
	int32 LODCount = 0;
	FString Issues;
};

// Mesh recommendation row
typedef TSharedPtr<struct FMeshRecRow> FMeshRecRowPtr;

struct FMeshRecRow
{
	FString Path;
	int32 VertexCount = 0;
	int32 TriangleCount = 0;
	int32 LODCount = 0;
	FString Issues;
	FString Recommendations;
};
