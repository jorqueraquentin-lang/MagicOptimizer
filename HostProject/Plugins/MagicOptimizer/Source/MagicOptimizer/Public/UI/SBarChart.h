#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

/**
 * Bar chart widget for displaying before/after comparisons
 * Matches the React mock design for performance metrics visualization
 */
class MAGICOPTIMIZER_API SBarChart : public SLeafWidget
{
public:
    struct FBarData
    {
        FString Name;
        double Before = 0.0;
        double After = 0.0;
        
        FBarData() = default;
        FBarData(const FString& InName, double InBefore, double InAfter)
            : Name(InName), Before(InBefore), After(InAfter) {}
    };

    SLATE_BEGIN_ARGS(SBarChart) 
        : _Data()
        , _BeforeColor(FLinearColor(0.2f, 0.6f, 1.0f, 0.7f))
        , _AfterColor(FLinearColor(0.2f, 0.8f, 0.4f, 0.7f))
        , _TextColor(FLinearColor::White)
    {}
        SLATE_ARGUMENT(TArray<FBarData>, Data)
        SLATE_ARGUMENT(FLinearColor, BeforeColor)
        SLATE_ARGUMENT(FLinearColor, AfterColor)
        SLATE_ARGUMENT(FLinearColor, TextColor)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
    virtual FVector2D ComputeDesiredSize(float) const override;

private:
    TArray<FBarData> Data;
    FLinearColor BeforeColor;
    FLinearColor AfterColor;
    FLinearColor TextColor;
};
