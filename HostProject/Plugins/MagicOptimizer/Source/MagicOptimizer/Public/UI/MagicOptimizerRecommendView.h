#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SButton.h"
#include "UI/UIDataTypes.h"
#include "UI/MagicOptimizerMainPanel.h"
#include "MagicOptimizerRecommendView.generated.h"

/**
 * Playbook recommendation data
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FPlaybookRecommendation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Title;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> SuggestedActions;

    UPROPERTY(BlueprintReadOnly)
    FString Severity; // "warning", "info", "critical"

    FPlaybookRecommendation()
        : Title(TEXT(""))
        , Description(TEXT(""))
        , Severity(TEXT("info"))
    {}

    FPlaybookRecommendation(const FString& InTitle, const FString& InDescription, const TArray<FString>& InActions, const FString& InSeverity)
        : Title(InTitle)
        , Description(InDescription)
        , SuggestedActions(InActions)
        , Severity(InSeverity)
    {}
};

/**
 * Proposed change data
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FProposedChange
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Id;

    UPROPERTY(BlueprintReadOnly)
    FString Label;

    UPROPERTY(BlueprintReadOnly)
    TArray<FUITextureRow> AffectedTextures;

    UPROPERTY(BlueprintReadOnly)
    float EstimatedDiskSaving; // MB

    UPROPERTY(BlueprintReadOnly)
    float EstimatedMemorySaving; // MB

    FProposedChange()
        : Id(TEXT(""))
        , Label(TEXT(""))
        , EstimatedDiskSaving(0.0f)
        , EstimatedMemorySaving(0.0f)
    {}

    FProposedChange(const FString& InId, const FString& InLabel, const TArray<FUITextureRow>& InTextures, float InDiskSaving, float InMemorySaving)
        : Id(InId)
        , Label(InLabel)
        , AffectedTextures(InTextures)
        , EstimatedDiskSaving(InDiskSaving)
        , EstimatedMemorySaving(InMemorySaving)
    {}
};

/**
 * Cost estimation summary
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FOptimizationSummary
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float TotalDiskSaving; // MB

    UPROPERTY(BlueprintReadOnly)
    float TotalMemorySaving; // MB

    UPROPERTY(BlueprintReadOnly)
    int32 TotalChanges;

    UPROPERTY(BlueprintReadOnly)
    FString RiskNotes;

    FOptimizationSummary()
        : TotalDiskSaving(0.0f)
        , TotalMemorySaving(0.0f)
        , TotalChanges(0)
        , RiskNotes(TEXT(""))
    {}
};

/**
 * Recommend view widget for MagicOptimizer that contains:
 * - Playbook recommendations based on detected patterns
 * - Summary of estimated savings and risks
 * - List of proposed changes with item counts
 * - Action buttons for previewing and applying changes
 */
class MAGICOPTIMIZER_API SMagicOptimizerRecommendView : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerRecommendView) {}
    
    SLATE_ATTRIBUTE(TArray<FUITextureRow>, TextureData)
    SLATE_ATTRIBUTE(TArray<FString>, SelectedTextureIds)
    
    SLATE_EVENT(FOnPreview, OnPreview)
    SLATE_EVENT(FOnAutoFix, OnAutoFix)
    
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void RefreshData(const TArray<FUITextureRow>& NewTextureData);

private:
    // Attributes
    TAttribute<TArray<FUITextureRow>> TextureData;
    TAttribute<TArray<FString>> SelectedTextureIds;

    // Events
    FOnPreview OnPreview;
    FOnAutoFix OnAutoFix;

    // Data
    TArray<FPlaybookRecommendation> PlaybookRecommendations;
    TArray<FProposedChange> ProposedChanges;
    FOptimizationSummary OptimizationSummary;

    // Widget creation
    TSharedRef<SWidget> MakePlaybookSection();
    TSharedRef<SWidget> MakePlaybookCard(const FPlaybookRecommendation& Recommendation);
    TSharedRef<SWidget> MakeSummarySection();
    TSharedRef<SWidget> MakeSummaryCard(const FString& Title, const FString& Value, const FString& Subtitle);
    TSharedRef<SWidget> MakeProposedChangesSection();
    TSharedRef<SWidget> MakeProposedChangeRow(const FProposedChange& Change);
    TSharedRef<SWidget> MakeActionButtons();

    // Data processing
    void UpdateRecommendations();
    TArray<FPlaybookRecommendation> AnalyzePlaybook() const;
    TArray<FProposedChange> BuildProposedChanges() const;
    FOptimizationSummary CalculateOptimizationSummary() const;
    TArray<FUITextureRow> GetSelectedTextures() const;

    // Event handlers
    void OnPreviewPlan();
    void OnApplyChanges();
    void OnCopyCommandlet();
    void OnShowDetails(const FString& ChangeId);
};
