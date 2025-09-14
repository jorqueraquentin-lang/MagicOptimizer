#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Slate.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SButton.h"
#include "Styling/AppStyle.h"
#include "UI/MagicOptimizerMainPanel.h"

// Forward declarations
class SMagicOptimizerHealthCard;
class SMagicOptimizerTextureTable;
class SMagicOptimizerLineChart;

/**
 * Audit view widget for MagicOptimizer that contains:
 * - Health overview cards (score, top offenders, summary)
 * - Texture table with search and filtering
 * - Line chart for health history
 */
class MAGICOPTIMIZER_API SMagicOptimizerAuditView : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerAuditView) {}
    
    SLATE_ATTRIBUTE(TArray<FUITextureRow>, TextureData)
    SLATE_ATTRIBUTE(TArray<FString>, SelectedTextureIds)
    SLATE_ATTRIBUTE(FString, SearchText)
    
    SLATE_EVENT(FOnSearchTextChanged, OnSearchTextChanged)
    SLATE_EVENT(FOnTextureSelectionChanged, OnTextureSelectionChanged)
    
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // Public interface
    void RefreshData(const TArray<FUITextureRow>& NewTextureData);

private:
    // Attributes
    TAttribute<TArray<FUITextureRow>> TextureData;
    TAttribute<TArray<FString>> SelectedTextureIds;
    TAttribute<FString> SearchText;

    // Events
    FOnSearchTextChanged OnSearchTextChangedDelegate;
    FOnTextureSelectionChanged OnTextureSelectionChangedDelegate;

    // Data
    FHealthData HealthData;
    TArray<FUITextureRow> FilteredTextures;
    FString SeverityFilter = TEXT("");

    // Widget references
    TSharedPtr<SMagicOptimizerTextureTable> TextureTableWidget;
    TSharedPtr<SListView<TSharedPtr<FUITextureRow>>> TextureListView;

    // Widget creation
    TSharedRef<SWidget> MakeHealthCards();
    TSharedRef<SWidget> MakeHealthScoreCard();
    TSharedRef<SWidget> MakeTopOffendersCard();
    TSharedRef<SWidget> MakeSummaryCard();
    TSharedRef<SWidget> MakeHealthTrendChart();
    TSharedRef<SWidget> MakeTextureTable();
    TSharedRef<SWidget> MakeSearchAndFilter();
    TSharedRef<SWidget> MakeIssueBadge(int32 IssueCount);
    TSharedRef<SWidget> MakeTextureList();
    TSharedRef<SWidget> MakeTextureRows();
    TSharedRef<SWidget> MakeTextureRow(const FUITextureRow& Texture);

    // Event handlers
    void OnTextureCheckboxChanged(const FString& TextureId, bool bSelected);
    void OnTextureRowClicked(const FString& TextureId);
    
    // Data processing
    void UpdateFilteredTextures();
    FHealthData CalculateHealthData() const;
    TArray<FUITextureRow> GetFilteredTextures() const;

    // Event handlers
    void OnSearchTextChanged(const FText& NewText);
    void OnSeverityFilterChanged(const FString& NewFilter);
    void OnTextureSelectionChanged(const TArray<FString>& NewSelection);
};

/**
 * Health card widget that displays a single health metric
 */
class MAGICOPTIMIZER_API SMagicOptimizerHealthCard : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerHealthCard) {}
    
    SLATE_ATTRIBUTE(FString, Title)
    SLATE_ATTRIBUTE(FString, Value)
    SLATE_ATTRIBUTE(FString, SubValue)
    SLATE_ATTRIBUTE(FLinearColor, Color)
    SLATE_ATTRIBUTE(TSharedPtr<SWidget>, ChartWidget)
    
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    TAttribute<FString> Title;
    TAttribute<FString> Value;
    TAttribute<FString> SubValue;
    TAttribute<FLinearColor> Color;
    TAttribute<TSharedPtr<SWidget>> ChartWidget;
};

/**
 * Texture table widget that displays texture data in a table format
 */
class MAGICOPTIMIZER_API SMagicOptimizerTextureTable : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerTextureTable) {}
    
    SLATE_ATTRIBUTE(TArray<FUITextureRow>, TextureData)
    SLATE_ATTRIBUTE(TArray<FString>, SelectedTextureIds)
    SLATE_ATTRIBUTE(FString, SeverityFilter)
    
    SLATE_EVENT(FOnTextureSelectionChanged, OnTextureSelectionChanged)
    SLATE_EVENT(FOnTextureRowClicked, OnTextureRowClicked)
    
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // Public interface
    void RefreshData(const TArray<FUITextureRow>& NewTextureData);

private:
    // Attributes
    TAttribute<TArray<FUITextureRow>> TextureData;
    TAttribute<TArray<FString>> SelectedTextureIds;
    TAttribute<FString> SeverityFilter;

    // Events
    FOnTextureSelectionChanged OnTextureSelectionChangedDelegate;
    FOnTextureRowClicked OnTextureRowClickedDelegate;

    // Widget creation
    TSharedRef<SWidget> MakeTableHeader();
    TSharedRef<SWidget> MakeTableBody();
    TSharedRef<SWidget> MakeTextureRow(const FUITextureRow& Texture);
    TSharedRef<SWidget> MakeIssueBadge(int32 IssueCount);

    // Event handlers
    void OnTextureCheckboxChanged(const FString& TextureId, bool bIsChecked);
    void OnTextureRowClicked(const FString& TextureId);

    // Data members
    TArray<TSharedPtr<FUITextureRow>> TextureListData;
    TSharedPtr<SListView<TSharedPtr<FUITextureRow>>> TextureListView;
};

// Delegate for texture row clicked
DECLARE_DELEGATE_OneParam(FOnTextureRowClicked, const FString&);
