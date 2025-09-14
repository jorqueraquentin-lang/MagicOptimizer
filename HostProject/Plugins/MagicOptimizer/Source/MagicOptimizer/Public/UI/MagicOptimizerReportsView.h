#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SButton.h"
#include "UI/MagicOptimizerMainPanel.h"
#include "MagicOptimizerReportsView.generated.h"

/**
 * Report run data
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FReportRun
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Id;

    UPROPERTY(BlueprintReadOnly)
    FString Date;

    UPROPERTY(BlueprintReadOnly)
    FString Preset;

    UPROPERTY(BlueprintReadOnly)
    FString Notes;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Artifacts;

    FReportRun()
        : Id(TEXT(""))
        , Date(TEXT(""))
        , Preset(TEXT(""))
        , Notes(TEXT(""))
    {}

    FReportRun(const FString& InId, const FString& InDate, const FString& InPreset, const FString& InNotes, const TArray<FString>& InArtifacts)
        : Id(InId)
        , Date(InDate)
        , Preset(InPreset)
        , Notes(InNotes)
        , Artifacts(InArtifacts)
    {}
};

/**
 * Reports view widget for MagicOptimizer that contains:
 * - Run history list
 * - Artifact download buttons
 * - Report sharing capabilities
 */
class MAGICOPTIMIZER_API SMagicOptimizerReportsView : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerReportsView) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    // Data
    TArray<TSharedPtr<FReportRun>> RunHistory;
    TSharedPtr<FReportRun> SelectedRun;

    // Widget creation
    TSharedRef<SWidget> MakeRunHistorySection();
    TSharedRef<SWidget> MakeArtifactsSection();
    TSharedRef<ITableRow> OnGenerateRunRow(TSharedPtr<FReportRun> InItem, const TSharedRef<STableViewBase>& OwnerTable);
    TSharedRef<SWidget> MakeArtifactButton(const FString& Format, const FString& Description);

    // Event handlers
    void OnRunSelectionChanged(TSharedPtr<FReportRun> SelectedItem, ESelectInfo::Type SelectInfo);
    void OnDownloadArtifact(const FString& Format);
    void OnCopyShareLink();

    // Data initialization
    void InitializeMockData();
};
