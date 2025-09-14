#include "UI/SRadialGauge.h"
#include "Rendering/DrawElements.h"
#include "Rendering/RenderingCommon.h"
#include "Math/UnrealMathUtility.h"

void SRadialGauge::Construct(const FArguments& InArgs)
{
    ValuePct = FMath::Clamp(InArgs._ValuePct, 0, InArgs._MaxValue);
    MaxValue = InArgs._MaxValue;
    GaugeColor = InArgs._GaugeColor;
    BackgroundColor = InArgs._BackgroundColor;
    Thickness = InArgs._Thickness;
}

int32 SRadialGauge::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const FVector2D Size = AllottedGeometry.GetLocalSize();
    const FVector2D Center = Size * 0.5f;
    const float Radius = FMath::Min(Center.X, Center.Y) - 8.0f;
    
    if (Radius <= 0.0f)
    {
        return LayerId;
    }

    // Draw background circle
    FSlateDrawElement::MakeBox(
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry(),
        FCoreStyle::Get().GetBrush("Border"),
        ESlateDrawEffect::None,
        BackgroundColor
    );

    // Calculate arc points
    const float StartAngle = -90.0f; // Start from top
    const float EndAngle = StartAngle + (360.0f * (float(ValuePct) / float(MaxValue)));
    const int32 NumSegments = 64;
    
    TArray<FVector2D> ArcPoints;
    ArcPoints.Reserve(NumSegments + 1);
    
    for (int32 i = 0; i <= NumSegments * ValuePct / MaxValue; ++i)
    {
        const float Angle = FMath::DegreesToRadians(StartAngle + (EndAngle - StartAngle) * (float(i) / float(NumSegments)));
        const FVector2D Point = Center + FVector2D(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius);
        ArcPoints.Add(Point);
    }

    // Draw the gauge arc
    if (ArcPoints.Num() > 1)
    {
        FSlateDrawElement::MakeLines(
            OutDrawElements,
            LayerId + 1,
            AllottedGeometry.ToPaintGeometry(),
            ArcPoints,
            ESlateDrawEffect::None,
            GaugeColor,
            true, // bAntialias
            Thickness
        );
    }

    return LayerId + 2;
}

FVector2D SRadialGauge::ComputeDesiredSize(float) const
{
    return FVector2D(160.0f, 160.0f);
}
