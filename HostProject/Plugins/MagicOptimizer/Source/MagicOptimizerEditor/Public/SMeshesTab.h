// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Dedicated tab widget for mesh optimization functionality
 * Provides a clean, focused interface for mesh-related operations
 */
class SMeshesTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMeshesTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SMeshesTab();

protected:
	// TODO: Implement mesh optimization functionality
};
