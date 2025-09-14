#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Slate.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Styling/AppStyle.h"
#include "MagicOptimizerLeftRail.generated.h"

// Forward declare delegates to avoid circular dependencies
DECLARE_DELEGATE_OneParam(FOnScopeChanged, const FString&);

// Forward declarations
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FProjectScopeItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Id;

    UPROPERTY(BlueprintReadOnly)
    FString Label;

    UPROPERTY(BlueprintReadOnly)
    bool bIsExpanded;

    UPROPERTY(BlueprintReadOnly)
    bool bIsChecked;

    // Remove UPROPERTY to prevent recursive struct definition via arrays
    // This is for internal tree structure and doesn't need Blueprint exposure
    TArray<FProjectScopeItem> Children;

    FProjectScopeItem()
        : Id(TEXT(""))
        , Label(TEXT(""))
        , bIsExpanded(true)
        , bIsChecked(true)
    {}

    FProjectScopeItem(const FString& InId, const FString& InLabel, bool bInExpanded = true, bool bInChecked = true)
        : Id(InId)
        , Label(InLabel)
        , bIsExpanded(bInExpanded)
        , bIsChecked(bInChecked)
    {}
};

USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FAssetTypeItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Id;

    UPROPERTY(BlueprintReadOnly)
    FString Label;

    UPROPERTY(BlueprintReadOnly)
    bool bIsChecked;

    FAssetTypeItem()
        : Id(TEXT(""))
        , Label(TEXT(""))
        , bIsChecked(false)
    {}

    FAssetTypeItem(const FString& InId, const FString& InLabel, bool bInChecked = false)
        : Id(InId)
        , Label(InLabel)
        , bIsChecked(bInChecked)
    {}
};

USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FSavedViewItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Id;

    UPROPERTY(BlueprintReadOnly)
    FString Label;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    FSavedViewItem()
        : Id(TEXT(""))
        , Label(TEXT(""))
        , Description(TEXT(""))
    {}

    FSavedViewItem(const FString& InId, const FString& InLabel, const FString& InDescription = TEXT(""))
        : Id(InId)
        , Label(InLabel)
        , Description(InDescription)
    {}
};

/**
 * Left rail widget for MagicOptimizer that contains:
 * - Project Scope tree with checkboxes
 * - Asset Types checkboxes
 * - Saved Views buttons
 */
class MAGICOPTIMIZER_API SMagicOptimizerLeftRail : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerLeftRail) {}
    
    SLATE_EVENT(FOnScopeChanged, OnScopeChanged)
    
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    // Events
    FOnScopeChanged OnScopeChanged;

    // Data
    TArray<FProjectScopeItem> ProjectScopeItems;
    TArray<FAssetTypeItem> AssetTypeItems;
    TArray<FSavedViewItem> SavedViewItems;

    // Widget creation
    TSharedRef<SWidget> MakeProjectScopeSection();
    TSharedRef<SWidget> MakeAssetTypesSection();
    TSharedRef<SWidget> MakeSavedViewsSection();
    TSharedRef<SWidget> MakeProjectScopeTree();
    TSharedRef<SWidget> MakeProjectScopeItem(const FProjectScopeItem& Item, int32 Depth = 0);
    TSharedRef<SWidget> MakeAssetTypeCheckbox(const FAssetTypeItem& Item);
    TSharedRef<SWidget> MakeSavedViewButton(const FSavedViewItem& Item);

    // Event handlers
    void OnProjectScopeItemToggled(const FString& ItemId, bool bIsChecked);
    void OnAssetTypeToggled(const FString& ItemId, bool bIsChecked);
    void OnSavedViewClicked(const FString& ItemId);
    void ToggleProjectScopeItem(const FString& ItemId);

    // Data initialization
    void InitializeData();
    void UpdateScopeSummary();
    FString GenerateScopeSummary() const;
};
