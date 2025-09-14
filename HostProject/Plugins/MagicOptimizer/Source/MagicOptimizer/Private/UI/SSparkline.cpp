#include "UI/SSparkline.h"
#include "Rendering/DrawElements.h"
#include "Rendering/RenderingCommon.h"

void SSparkline::Construct(const FArguments& InArgs)
{
    Points = InArgs._Points;
    LineColor = InArgs._LineColor;
    LineThickness = InArgs._LineThickness;
}

int32 SSparkline::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    if (Points.Num() < 2)
    {
        return LayerId;
    }

    const FVector2D Size = AllottedGeometry.GetLocalSize();
    TArray<FVector2D> Polyline;
    Polyline.Reserve(Points.Num());

    // Convert normalized coordinates to actual pixel coordinates
    for (const FVector2D& Point : Points)
    {
        FVector2D PixelPoint;
        PixelPoint.X = Point.X * Size.X;
        PixelPoint.Y = (1.0f - Point.Y) * Size.Y; // Flip Y coordinate for proper display
        Polyline.Add(PixelPoint);
    }

    // Draw the sparkline
    FSlateDrawElement::MakeLines(
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry(),
        Polyline,
        ESlateDrawEffect::None,
        LineColor,
        true, // bAntialias
        LineThickness
    );

    return LayerId + 1;
}

FVector2D SSparkline::ComputeDesiredSize(float) const
{
    return FVector2D(200.0f, 64.0f);
}
