#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

/**
 * Sparkline widget for displaying health history trends
 * Matches the React mock design for health data visualization
 */
class MAGICOPTIMIZER_API SSparkline : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SSparkline) 
        : _Points()
        , _LineColor(FLinearColor::White)
        , _LineThickness(2.0f)
    {}
        SLATE_ARGUMENT(TArray<FVector2D>, Points) // x in [0..1], y in [0..1]
        SLATE_ARGUMENT(FLinearColor, LineColor)
        SLATE_ARGUMENT(float, LineThickness)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
    virtual FVector2D ComputeDesiredSize(float) const override;

private:
    TArray<FVector2D> Points;
    FLinearColor LineColor;
    float LineThickness;
};
