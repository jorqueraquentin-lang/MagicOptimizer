#include "UI/MagicOptimizerBudgetsView.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Layout/SSeparator.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerBudgetsView"

// ============================================================================
// SMagicOptimizerRadialChart Implementation
// ============================================================================

void SMagicOptimizerRadialChart::Construct(const FArguments& InArgs)
{
    Value = InArgs._Value;
    PrimaryColor = InArgs._PrimaryColor;
    BackgroundColor = InArgs._BackgroundColor;
    Thickness = InArgs._Thickness;
    CenterText = InArgs._CenterText;
    Label = InArgs._Label;
}

int32 SMagicOptimizerRadialChart::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
    int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const FVector2D Size = AllottedGeometry.GetLocalSize();
    const FVector2D Center = Size * 0.5f;
    const float Radius = FMath::Min(Size.X, Size.Y) * 0.35f;
    const float InnerRadius = Radius - Thickness.Get(12.0f);
    
    // Calculate progress angle (0-360 degrees)
    const float ProgressAngle = (Value.Get() / 100.0f) * 360.0f;
    
    // Draw background circle
    const int32 NumSegments = 64;
    TArray<FSlateVertex> BackgroundVertices;
    TArray<SlateIndex> BackgroundIndices;
    
    // Create background circle vertices
    for (int32 i = 0; i <= NumSegments; ++i)
    {
        const float Angle = (i / (float)NumSegments) * 2.0f * PI;
        const FVector2D OuterPoint = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius;
        const FVector2D InnerPoint = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * InnerRadius;
        
        BackgroundVertices.Add(FSlateVertex::Make(
            FSlateRenderTransform(),
            FVector2f(OuterPoint),
            FVector2f::ZeroVector,
            BackgroundColor.Get(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f)).ToFColor(false)
        ));
        
        BackgroundVertices.Add(FSlateVertex::Make(
            FSlateRenderTransform(),
            FVector2f(InnerPoint),
            FVector2f::ZeroVector,
            BackgroundColor.Get(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f)).ToFColor(false)
        ));
        
        if (i < NumSegments)
        {
            const int32 BaseIndex = i * 2;
            // Create two triangles for each segment
            BackgroundIndices.Add(BaseIndex);
            BackgroundIndices.Add(BaseIndex + 1);
            BackgroundIndices.Add(BaseIndex + 2);
            
            BackgroundIndices.Add(BaseIndex + 1);
            BackgroundIndices.Add(BaseIndex + 3);
            BackgroundIndices.Add(BaseIndex + 2);
        }
    }
    
    // Draw background
    FSlateDrawElement::MakeCustomVerts(
        OutDrawElements,
        LayerId++,
        FSlateResourceHandle(),
        BackgroundVertices,
        BackgroundIndices,
        nullptr,
        0,
        0
    );
    
    // Draw progress arc
    if (ProgressAngle > 0.0f)
    {
        const int32 ProgressSegments = FMath::Max(1, (int32)(NumSegments * (ProgressAngle / 360.0f)));
        TArray<FSlateVertex> ProgressVertices;
        TArray<SlateIndex> ProgressIndices;
        
        for (int32 i = 0; i <= ProgressSegments; ++i)
        {
            const float Angle = ((i / (float)NumSegments) * 2.0f * PI) - (PI * 0.5f); // Start from top
            const FVector2D OuterPoint = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius;
            const FVector2D InnerPoint = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * InnerRadius;
            
            ProgressVertices.Add(FSlateVertex::Make(
                FSlateRenderTransform(),
                FVector2f(OuterPoint),
                FVector2f::ZeroVector,
                PrimaryColor.Get(FLinearColor(0.2f, 0.7f, 0.2f, 1.0f)).ToFColor(false)
            ));
            
            ProgressVertices.Add(FSlateVertex::Make(
                FSlateRenderTransform(),
                FVector2f(InnerPoint),
                FVector2f::ZeroVector,
                PrimaryColor.Get(FLinearColor(0.2f, 0.7f, 0.2f, 1.0f)).ToFColor(false)
            ));
            
            if (i < ProgressSegments)
            {
                const int32 BaseIndex = i * 2;
                ProgressIndices.Add(BaseIndex);
                ProgressIndices.Add(BaseIndex + 1);
                ProgressIndices.Add(BaseIndex + 2);
                
                ProgressIndices.Add(BaseIndex + 1);
                ProgressIndices.Add(BaseIndex + 3);
                ProgressIndices.Add(BaseIndex + 2);
            }
        }
        
        FSlateDrawElement::MakeCustomVerts(
            OutDrawElements,
            LayerId++,
            FSlateResourceHandle(),
            ProgressVertices,
            ProgressIndices,
            nullptr,
            0,
            0
        );
    }
    
    // Draw center text
    if (!CenterText.Get().IsEmpty())
    {
        const FSlateFontInfo Font = FAppStyle::GetFontStyle("BoldFont");
        const FVector2D TextSize = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(CenterText.Get(), Font);
        const FVector2D TextPosition = Center - (TextSize * 0.5f);
        
        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId++,
            AllottedGeometry.MakeChild(TextSize, FSlateLayoutTransform(TextPosition)).ToPaintGeometry(),
            CenterText.Get(),
            Font,
            ESlateDrawEffect::None,
            FLinearColor::White
        );
    }
    
    return LayerId;
}

FVector2D SMagicOptimizerRadialChart::ComputeDesiredSize(float) const
{
    return FVector2D(160.0f, 160.0f);
}

// ============================================================================
// SMagicOptimizerBudgetsView Implementation  
// ============================================================================

void SMagicOptimizerBudgetsView::Construct(const FArguments& InArgs)
{
    // Bind attributes
    TextureData = InArgs._TextureData;
    
    // Initialize data
    UpdateBudgetData();
    
    ChildSlot
    [
        SNew(SVerticalBox)
        
        // Budget cards (3-column radial charts)
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 24)
        [
            MakeBudgetCards()
        ]
        
        // Top offenders table
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            MakeOffendersTable()
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerBudgetsView::MakeBudgetCards()
{
    return SNew(SHorizontalBox)
    
    // Texture Pool card
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .Padding(0, 0, 12, 0)
    [
        MakeBudgetCard(
            TEXT("Texture Pool"),
            TEXT("📊"), // Gauge icon replacement
            TexturePoolUsage,
            TEXT("Usage of streaming pool"),
            FLinearColor(0.2f, 0.7f, 0.2f, 1.0f)
        )
    ]
    
    // VT Pool card  
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .Padding(0, 0, 12, 0)
    [
        MakeBudgetCard(
            TEXT("VT Pool"),
            TEXT("📈"), // Activity icon replacement
            VTPoolUsage,
            TEXT("Virtual texturing residency"),
            FLinearColor(0.2f, 0.5f, 0.8f, 1.0f)
        )
    ]
    
    // Disk Footprint card
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    [
        MakeBudgetCard(
            TEXT("Disk Footprint"),
            TEXT("💽"), // HardDrive icon replacement
            DiskUsage,
            TEXT("Cooked content vs target"),
            FLinearColor(0.8f, 0.4f, 0.2f, 1.0f)
        )
    ];
}

TSharedRef<SWidget> SMagicOptimizerBudgetsView::MakeBudgetCard(const FString& Title, const FString& Icon, float Value, const FString& Description, const FLinearColor& Color)
{
    return SNew(SBorder)
    .Padding(FMargin(20))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    [
        SNew(SVerticalBox)
        
        // Title with icon
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 16)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 8, 0)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Icon))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 16))
                .ColorAndOpacity(Color)
            ]
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Title))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14))
                .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            ]
        ]
        
        // Radial chart
        +SVerticalBox::Slot()
        .AutoHeight()
        .HAlign(HAlign_Center)
        .Padding(0, 0, 0, 16)
        [
            SNew(SMagicOptimizerRadialChart)
            .Value(Value)
            .PrimaryColor(Color)
            .BackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
            .Thickness(14.0f)
            .CenterText(FString::Printf(TEXT("%.0f%%"), Value))
        ]
        
        // Description
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(STextBlock)
            .Text(FText::FromString(Description))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            .Justification(ETextJustify::Center)
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerBudgetsView::MakeOffendersTable()
{
    return SNew(SBorder)
    .Padding(FMargin(0))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    [
        SNew(SVerticalBox)
        
        // Header
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(20, 16, 20, 8)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("TopOffendersTitle", "Top Offenders by TextureGroup"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        // Separator
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(20, 0, 20, 16)
        [
            SNew(SSeparator)
            .ColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
        ]
        
        // Table
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(20, 0, 20, 20)
        [
            SNew(SListView<TSharedPtr<FBudgetOffender>>)
            .ListItemsSource(&OffenderData)
            .OnGenerateRow(this, &SMagicOptimizerBudgetsView::OnGenerateOffenderRow)
            .SelectionMode(ESelectionMode::None)
            .ScrollbarVisibility(EVisibility::Collapsed)
        ]
    ];
}

TSharedRef<ITableRow> SMagicOptimizerBudgetsView::OnGenerateOffenderRow(TSharedPtr<FBudgetOffender> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FBudgetOffender>>, OwnerTable)
    [
        SNew(SBorder)
        .Padding(FMargin(0, 8, 0, 8))
        .BorderImage(FAppStyle::GetBrush("NoBorder"))
        .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
        [
            SNew(SHorizontalBox)
            
            // Group name
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 24, 0)
            [
                SNew(STextBlock)
                .Text(FText::FromString(InItem->Group))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                .MinDesiredWidth(120)
            ]
            
            // Weighted cost progress bar
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .VAlign(VAlign_Center)
            .Padding(0, 0, 24, 0)
            [
                SNew(SProgressBar)
                .Percent(InItem->WeightedCost / 100.0f)
                .FillColorAndOpacity(FLinearColor(0.8f, 0.4f, 0.2f, 1.0f))
            ]
            
            // Suggested action
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString(InItem->SuggestedAction))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
                .MinDesiredWidth(300)
            ]
        ]
    ];
}

void SMagicOptimizerBudgetsView::UpdateBudgetData()
{
    // Calculate realistic budget data from texture data
    const TArray<FUITextureRow>& Textures = TextureData.Get();
    
    // Mock calculations - in real implementation, these would be calculated from actual data
    TexturePoolUsage = 86.0f;
    VTPoolUsage = 72.0f;  
    DiskUsage = 64.0f;
    
    // Generate top offenders data
    OffenderData = CalculateTopOffenders();
}

TArray<TSharedPtr<FBudgetOffender>> SMagicOptimizerBudgetsView::CalculateTopOffenders() const
{
    TArray<TSharedPtr<FBudgetOffender>> Offenders;
    
    // Calculate weighted costs by texture group (mock data from React example)
    Offenders.Add(MakeShared<FBudgetOffender>(TEXT("World"), 85.0f, TEXT("Lower MaxTextureSize or increase LODBias"), 342));
    Offenders.Add(MakeShared<FBudgetOffender>(TEXT("Characters"), 72.0f, TEXT("Lower MaxTextureSize or increase LODBias"), 156));  
    Offenders.Add(MakeShared<FBudgetOffender>(TEXT("Props"), 58.0f, TEXT("Lower MaxTextureSize or increase LODBias"), 789));
    Offenders.Add(MakeShared<FBudgetOffender>(TEXT("UI"), 34.0f, TEXT("Verify crispness / consider 2k cap"), 23));
    Offenders.Add(MakeShared<FBudgetOffender>(TEXT("Effects"), 28.0f, TEXT("Lower MaxTextureSize or increase LODBias"), 67));
    
    return Offenders;
}

void SMagicOptimizerBudgetsView::RefreshData(const TArray<FUITextureRow>& NewTextureData)
{
    UpdateBudgetData();
}

#undef LOCTEXT_NAMESPACE
