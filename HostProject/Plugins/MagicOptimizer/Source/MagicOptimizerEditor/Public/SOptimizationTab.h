// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Dedicated tab widget for general optimization and workflow functionality
 * Provides a clean, focused interface for optimization operations and settings
 */
class SOptimizationTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SOptimizationTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SOptimizationTab();

protected:
	// TODO: Implement general optimization functionality
};
