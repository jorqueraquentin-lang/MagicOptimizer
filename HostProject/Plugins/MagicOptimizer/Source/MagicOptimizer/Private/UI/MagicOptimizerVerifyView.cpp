#include "UI/MagicOptimizerVerifyView.h"
#include "Slate.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSeparator.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerVerifyView"

// ============================================================================
// SMagicOptimizerBarChart Implementation
// ============================================================================

void SMagicOptimizerBarChart::Construct(const FArguments& InArgs)
{
    Data = InArgs._Data;
}

int32 SMagicOptimizerBarChart::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
    int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const TArray<FBeforeAfterData>& ChartData = Data.Get();
    if (ChartData.Num() == 0)
    {
        return LayerId;
    }

    const FVector2D Size = AllottedGeometry.GetLocalSize();
    const float Padding = 60.0f;
    const float ChartWidth = Size.X - (Padding * 2);
    const float ChartHeight = Size.Y - (Padding * 2);
    const float BarWidth = ChartWidth / (ChartData.Num() * 2.2f); // Space for before/after pairs
    
    // Find max value for scaling
    float MaxValue = 0.0f;
    for (const FBeforeAfterData& Item : ChartData)
    {
        MaxValue = FMath::Max(MaxValue, FMath::Max(Item.BeforeValue, Item.AfterValue));
    }

    if (MaxValue <= 0.0f) MaxValue = 1.0f;

    // Draw bars
    for (int32 i = 0; i < ChartData.Num(); ++i)
    {
        const FBeforeAfterData& Item = ChartData[i];
        const float BaseX = Padding + (i * ChartWidth / ChartData.Num());
        
        // Before bar (left)
        const float BeforeHeight = (Item.BeforeValue / MaxValue) * ChartHeight;
        const FVector2D BeforePos = FVector2D(BaseX, Padding + ChartHeight - BeforeHeight);
        const FVector2D BeforeSize = FVector2D(BarWidth * 0.8f, BeforeHeight);
        
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId++,
            AllottedGeometry.ToPaintGeometry(BeforeSize, FSlateLayoutTransform(BeforePos)),
            FAppStyle::GetBrush("ToolPanel.GroupBorder"),
            ESlateDrawEffect::None,
            FLinearColor(0.6f, 0.6f, 0.6f, 1.0f) // Gray for "Before"
        );
        
        // After bar (right)
        const float AfterHeight = (Item.AfterValue / MaxValue) * ChartHeight;
        const FVector2D AfterPos = FVector2D(BaseX + BarWidth, Padding + ChartHeight - AfterHeight);
        const FVector2D AfterSize = FVector2D(BarWidth * 0.8f, AfterHeight);
        
        const float Improvement = Item.GetImprovement();
        const FLinearColor AfterColor = Improvement > 0 ? 
            FLinearColor(0.2f, 0.7f, 0.2f, 1.0f) : // Green for improvement
            FLinearColor(0.8f, 0.3f, 0.2f, 1.0f);   // Red for regression
        
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId++,
            AllottedGeometry.ToPaintGeometry(AfterSize, FSlateLayoutTransform(AfterPos)),
            FAppStyle::GetBrush("ToolPanel.GroupBorder"),
            ESlateDrawEffect::None,
            AfterColor
        );
        
        // Labels
        const FSlateFontInfo Font = FAppStyle::GetFontStyle("SmallFont");
        const FString LabelText = Item.MetricName;
        const FVector2D TextSize = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(LabelText, Font);
        const FVector2D TextPos = FVector2D(BaseX, Padding + ChartHeight + 5);
        
        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId++,
            AllottedGeometry.MakeChild(TextSize, FSlateLayoutTransform(TextPos)).ToPaintGeometry(),
            LabelText,
            Font,
            ESlateDrawEffect::None,
            FLinearColor(0.8f, 0.8f, 0.8f, 1.0f)
        );
    }

    return LayerId;
}

FVector2D SMagicOptimizerBarChart::ComputeDesiredSize(float) const
{
    return FVector2D(400.0f, 250.0f);
}

// ============================================================================
// SMagicOptimizerVerifyView Implementation
// ============================================================================

void SMagicOptimizerVerifyView::Construct(const FArguments& InArgs)
{
    InitializeMockData();

    ChildSlot
    [
        SNew(SHorizontalBox)
        
        // Before and After section (left)
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .Padding(0, 0, 12, 0)
        [
            MakeBeforeAfterSection()
        ]
        
        // Regressions section (right)
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        [
            MakeRegressionsSection()
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerVerifyView::MakeBeforeAfterSection()
{
    return SNew(SBorder)
    .Padding(FMargin(20))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    [
        SNew(SVerticalBox)
        
        // Title
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 16)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("BeforeAfterTitle", "Before and After"))
            .Font(FAppStyle::GetFontStyle("NormalFontBold"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        // Chart
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Top)
        [
            SNew(SMagicOptimizerBarChart)
            .Data(BeforeAfterData)
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerVerifyView::MakeRegressionsSection()
{
    return SNew(SBorder)
    .Padding(FMargin(20))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    [
        SNew(SVerticalBox)
        
        // Title
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 16)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("RegressionsTitle", "Regressions"))
            .Font(FAppStyle::GetFontStyle("NormalFontBold"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        // Content
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            RegressionWarnings.Num() > 0 ?
            // Show warnings if any
            [this]() -> TSharedRef<SWidget>
            {
                TSharedRef<SVerticalBox> WarningsBox = SNew(SVerticalBox);
                
                for (const FString& Warning : RegressionWarnings)
                {
                    WarningsBox->AddSlot()
                    .AutoHeight()
                    .Padding(0, 0, 0, 8)
                    [
                        SNew(SBorder)
                        .Padding(FMargin(12, 8, 12, 8))
                        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                        .ColorAndOpacity(FLinearColor(0.8f, 0.3f, 0.2f, 0.3f))
                        .BorderBackgroundColor(FLinearColor(0.8f, 0.3f, 0.2f, 0.5f))
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(Warning))
                            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                            .ColorAndOpacity(FLinearColor(0.9f, 0.7f, 0.7f, 1.0f))
                            .AutoWrapText(true)
                        ]
                    ];
                }
                
                return WarningsBox;
            }() :
            // No regressions detected message
            StaticCastSharedRef<SWidget>(
                SNew(STextBlock)
                .Text(LOCTEXT("NoRegressionsDetected", "No regressions detected."))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            )
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerVerifyView::MakeMetricCard(const FBeforeAfterData& Data)
{
    const float Improvement = Data.GetImprovement();
    const FLinearColor ImprovementColor = GetImprovementColor(Improvement);
    
    return SNew(SBorder)
    .Padding(FMargin(12))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
    [
        SNew(SVerticalBox)
        
        // Metric name
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 8)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Data.MetricName))
            .Font(FAppStyle::GetFontStyle("NormalFontBold"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        // Before/After values
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 4)
        [
            SNew(SHorizontalBox)
            
            // Before
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(FString::Printf(TEXT("Before: %.0f %s"), Data.BeforeValue, *Data.Unit)))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
            ]
            
            // After
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(FString::Printf(TEXT("After: %.0f %s"), Data.AfterValue, *Data.Unit)))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(ImprovementColor)
            ]
        ]
        
        // Improvement percentage
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(STextBlock)
            .Text(FText::FromString(FString::Printf(TEXT("%+.1f%%"), Improvement)))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 16))
            .Font(FAppStyle::GetFontStyle("BoldFont"))
            .ColorAndOpacity(ImprovementColor)
        ]
    ];
}

void SMagicOptimizerVerifyView::InitializeMockData()
{
    // Initialize mock data matching the React mock
    BeforeAfterData.Add(FBeforeAfterData(TEXT("Disk MB"), 3120.0f, 2710.0f, TEXT("MB")));
    BeforeAfterData.Add(FBeforeAfterData(TEXT("GPU Mem MB"), 2240.0f, 1820.0f, TEXT("MB")));
    BeforeAfterData.Add(FBeforeAfterData(TEXT("Draw Calls"), 14500.0f, 11700.0f, TEXT("calls")));
    BeforeAfterData.Add(FBeforeAfterData(TEXT("Shader Cost"), 1.0f, 0.78f, TEXT("units")));
    
    RegressionWarnings = DetectRegressions();
}

TArray<FString> SMagicOptimizerVerifyView::DetectRegressions() const
{
    TArray<FString> Warnings;
    
    // Check each metric for regressions
    for (const FBeforeAfterData& Data : BeforeAfterData)
    {
        const float Improvement = Data.GetImprovement();
        
        // If improvement is negative (worse performance), it's a regression
        if (Improvement < -5.0f) // More than 5% worse
        {
            Warnings.Add(FString::Printf(TEXT("Regression in %s: %.1f%% worse performance"), *Data.MetricName, -Improvement));
        }
    }
    
    return Warnings;
}

FLinearColor SMagicOptimizerVerifyView::GetImprovementColor(float ImprovementPercent) const
{
    if (ImprovementPercent > 0.0f)
    {
        // Green for improvements
        return FLinearColor(0.2f, 0.7f, 0.2f, 1.0f);
    }
    else if (ImprovementPercent < -5.0f)
    {
        // Red for significant regressions
        return FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);
    }
    else
    {
        // Yellow/orange for neutral or slight regressions
        return FLinearColor(0.8f, 0.6f, 0.2f, 1.0f);
    }
}

#undef LOCTEXT_NAMESPACE
