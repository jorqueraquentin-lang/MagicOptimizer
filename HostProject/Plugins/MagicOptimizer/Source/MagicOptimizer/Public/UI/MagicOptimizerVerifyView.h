#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SLeafWidget.h"
#include "UI/UIDataTypes.h"

// Data structures are now defined in UIDataTypes.h

/**
 * Simple bar chart widget for before/after visualization
 */
class MAGICOPTIMIZER_API SMagicOptimizerBarChart : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerBarChart) {}
    
    SLATE_ATTRIBUTE(TArray<FBeforeAfterData>, Data)
    
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // SWidget interface
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, 
        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, 
        int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

    virtual FVector2D ComputeDesiredSize(float) const override;

private:
    TAttribute<TArray<FBeforeAfterData>> Data;
};

/**
 * Verify view widget for MagicOptimizer that contains:
 * - Before and After comparison chart
 * - Regression detection and warnings
 * - Performance impact visualization
 */
class MAGICOPTIMIZER_API SMagicOptimizerVerifyView : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerVerifyView) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    // Data
    TArray<FBeforeAfterData> BeforeAfterData;
    TArray<FString> RegressionWarnings;

    // Widget creation
    TSharedRef<SWidget> MakeBeforeAfterSection();
    TSharedRef<SWidget> MakeRegressionsSection();
    TSharedRef<SWidget> MakeMetricCard(const FBeforeAfterData& Data);

    // Data processing
    void InitializeMockData();
    TArray<FString> DetectRegressions() const;
    FLinearColor GetImprovementColor(float ImprovementPercent) const;
};
