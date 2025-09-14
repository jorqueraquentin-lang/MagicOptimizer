#include "UI/SBarChart.h"
#include "Rendering/DrawElements.h"
#include "Rendering/RenderingCommon.h"
#include "Math/UnrealMathUtility.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"

void SBarChart::Construct(const FArguments& InArgs)
{
    Data = InArgs._Data;
    BeforeColor = InArgs._BeforeColor;
    AfterColor = InArgs._AfterColor;
    TextColor = InArgs._TextColor;
}

int32 SBarChart::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    if (Data.Num() == 0)
    {
        return LayerId;
    }

    const FVector2D Size = AllottedGeometry.GetLocalSize();
    const float BarWidth = Size.X / (Data.Num() * 2.0f); // Two bars per data point
    const float BarSpacing = BarWidth * 0.1f;
    const float MaxValue = 1.0f; // Normalize to max value
    
    // Find the maximum value for normalization
    double ActualMaxValue = 0.0;
    for (const FBarData& Bar : Data)
    {
        ActualMaxValue = FMath::Max(ActualMaxValue, FMath::Max(Bar.Before, Bar.After));
    }
    
    if (ActualMaxValue <= 0.0)
    {
        return LayerId;
    }

    const FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", 10);
    const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

    for (int32 i = 0; i < Data.Num(); ++i)
    {
        const FBarData& Bar = Data[i];
        const float X = i * (BarWidth * 2.0f + BarSpacing);
        const float Y = Size.Y * 0.5f;
        
        // Draw Before bar
        const float BeforeHeight = (Bar.Before / ActualMaxValue) * (Size.Y * 0.4f);
        const FVector2D BeforePos(X, Y - BeforeHeight);
        const FVector2D BeforeSize(BarWidth, BeforeHeight);
        
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(BeforePos), FSlateRenderTransform(), FVector2D(BeforeSize)),
            FCoreStyle::Get().GetBrush("Border"),
            ESlateDrawEffect::None,
            BeforeColor
        );
        
        // Draw After bar
        const float AfterHeight = (Bar.After / ActualMaxValue) * (Size.Y * 0.4f);
        const FVector2D AfterPos(X + BarWidth + BarSpacing, Y - AfterHeight);
        const FVector2D AfterSize(BarWidth, AfterHeight);
        
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(AfterPos), FSlateRenderTransform(), FVector2D(AfterSize)),
            FCoreStyle::Get().GetBrush("Border"),
            ESlateDrawEffect::None,
            AfterColor
        );
        
        // Draw labels
        const FString BeforeText = FString::Printf(TEXT("Before: %.0f"), Bar.Before);
        const FString AfterText = FString::Printf(TEXT("After: %.0f"), Bar.After);
        
        const FVector2D BeforeTextSize = FontMeasure->Measure(BeforeText, FontInfo);
        const FVector2D AfterTextSize = FontMeasure->Measure(AfterText, FontInfo);
        
        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId + 1,
            AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(FVector2D(X, Y + 5.0f)), FSlateRenderTransform(), BeforeTextSize),
            BeforeText,
            FontInfo,
            ESlateDrawEffect::None,
            TextColor
        );
        
        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId + 1,
            AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(FVector2D(X + BarWidth + BarSpacing, Y + 5.0f)), FSlateRenderTransform(), AfterTextSize),
            AfterText,
            FontInfo,
            ESlateDrawEffect::None,
            TextColor
        );
    }

    return LayerId + 2;
}

FVector2D SBarChart::ComputeDesiredSize(float) const
{
    return FVector2D(400.0f, 200.0f);
}
