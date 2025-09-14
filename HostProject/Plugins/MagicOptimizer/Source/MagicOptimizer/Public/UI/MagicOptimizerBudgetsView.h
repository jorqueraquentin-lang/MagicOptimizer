#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "UI/MagicOptimizerMainPanel.h"
#include "MagicOptimizerBudgetsView.generated.h"

// Forward declarations
class SMagicOptimizerRadialChart;

/**
 * Budget offender data for TextureGroups
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FBudgetOffender
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Group;

    UPROPERTY(BlueprintReadOnly)
    float WeightedCost;

    UPROPERTY(BlueprintReadOnly)
    FString SuggestedAction;

    UPROPERTY(BlueprintReadOnly)
    int32 AssetCount;

    FBudgetOffender()
        : Group(TEXT(""))
        , WeightedCost(0.0f)
        , SuggestedAction(TEXT(""))
        , AssetCount(0)
    {}

    FBudgetOffender(const FString& InGroup, float InWeightedCost, const FString& InSuggestedAction, int32 InAssetCount)
        : Group(InGroup)
        , WeightedCost(InWeightedCost)
        , SuggestedAction(InSuggestedAction)
        , AssetCount(InAssetCount)
    {}
};

/**
 * Custom radial chart widget for displaying budget usage as circular progress
 */
class MAGICOPTIMIZER_API SMagicOptimizerRadialChart : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerRadialChart) {}
    
    SLATE_ATTRIBUTE(float, Value) // 0.0 to 100.0
    SLATE_ATTRIBUTE(FLinearColor, PrimaryColor)
    SLATE_ATTRIBUTE(FLinearColor, BackgroundColor)
    SLATE_ATTRIBUTE(float, Thickness)
    SLATE_ATTRIBUTE(FString, CenterText)
    SLATE_ATTRIBUTE(FString, Label)
    
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // SWidget interface
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, 
        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, 
        int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

    virtual FVector2D ComputeDesiredSize(float) const override;

private:
    TAttribute<float> Value;
    TAttribute<FLinearColor> PrimaryColor;
    TAttribute<FLinearColor> BackgroundColor;
    TAttribute<float> Thickness;
    TAttribute<FString> CenterText;
    TAttribute<FString> Label;
};

/**
 * Budgets view widget for MagicOptimizer that contains:
 * - Three radial charts for Texture Pool, VT Pool, and Disk Footprint
 * - Top offenders table by TextureGroup with weighted cost analysis
 */
class MAGICOPTIMIZER_API SMagicOptimizerBudgetsView : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerBudgetsView) {}
    SLATE_ATTRIBUTE(TArray<FUITextureRow>, TextureData)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void RefreshData(const TArray<FUITextureRow>& NewTextureData);

private:
    // Attributes
    TAttribute<TArray<FUITextureRow>> TextureData;

    // Data
    float TexturePoolUsage = 86.0f;
    float VTPoolUsage = 72.0f;
    float DiskUsage = 64.0f;
    TArray<TSharedPtr<FBudgetOffender>> OffenderData;

    // Widget creation
    TSharedRef<SWidget> MakeBudgetCards();
    TSharedRef<SWidget> MakeBudgetCard(const FString& Title, const FString& Icon, float Value, const FString& Description, const FLinearColor& Color);
    TSharedRef<SWidget> MakeOffendersTable();
    TSharedRef<ITableRow> OnGenerateOffenderRow(TSharedPtr<FBudgetOffender> InItem, const TSharedRef<STableViewBase>& OwnerTable);

    // Data processing
    void UpdateBudgetData();
    TArray<TSharedPtr<FBudgetOffender>> CalculateTopOffenders() const;
};
