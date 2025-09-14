#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

/**
 * Radial gauge widget for displaying budget usage and health metrics
 * Matches the React mock design for circular progress indicators
 */
class MAGICOPTIMIZER_API SRadialGauge : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SRadialGauge) 
        : _ValuePct(0)
        , _MaxValue(100)
        , _GaugeColor(FLinearColor::White)
        , _BackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.08f))
        , _Thickness(10.0f)
    {}
        SLATE_ARGUMENT(int32, ValuePct) // 0..100
        SLATE_ARGUMENT(int32, MaxValue)
        SLATE_ARGUMENT(FLinearColor, GaugeColor)
        SLATE_ARGUMENT(FLinearColor, BackgroundColor)
        SLATE_ARGUMENT(float, Thickness)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
    virtual FVector2D ComputeDesiredSize(float) const override;

private:
    int32 ValuePct;
    int32 MaxValue;
    FLinearColor GaugeColor;
    FLinearColor BackgroundColor;
    float Thickness;
};
