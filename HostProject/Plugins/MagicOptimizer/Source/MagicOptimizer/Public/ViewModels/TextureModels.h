// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Texture audit row
typedef TSharedPtr<struct FTextureAuditRow> FTextureAuditRowPtr;

struct FTextureAuditRow
{
	FString Path;
	int32 Width = 0;
	int32 Height = 0;
	FString Format;
};

// Texture recommendation row
typedef TSharedPtr<struct FTextureRecRow> FTextureRecRowPtr;

struct FTextureRecRow
{
	FString Path;
	int32 Width = 0;
	int32 Height = 0;
	FString Format;
	FString Issues;
	FString Recommendations;
};


