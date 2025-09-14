#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "UI/MagicOptimizerMainPanel.h"
#include "MagicOptimizerScopeSheet.generated.h"

/**
 * Texture group settings for profiles
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FTextureGroupSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString GroupName;

    UPROPERTY(BlueprintReadOnly)
    int32 MaxTextureSize;

    UPROPERTY(BlueprintReadOnly)
    int32 LODBias;

    FTextureGroupSettings()
        : GroupName(TEXT(""))
        , MaxTextureSize(4096)
        , LODBias(0)
    {}

    FTextureGroupSettings(const FString& InGroupName, int32 InMaxTextureSize, int32 InLODBias)
        : GroupName(InGroupName)
        , MaxTextureSize(InMaxTextureSize)
        , LODBias(InLODBias)
    {}
};

/**
 * Folder include/exclude settings
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FFolderSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString FolderName;

    UPROPERTY(BlueprintReadOnly)
    bool bIncluded;

    FFolderSettings()
        : FolderName(TEXT(""))
        , bIncluded(true)
    {}

    FFolderSettings(const FString& InFolderName, bool bInIncluded)
        : FolderName(InFolderName)
        , bIncluded(bInIncluded)
    {}
};

DECLARE_DELEGATE_OneParam(FOnScopeApplied, const FString&);

/**
 * Scope & Profiles sheet widget for MagicOptimizer that provides:
 * - Include/Exclude folder configuration
 * - TextureGroup profile settings
 * - INI file preview
 * - Apply functionality
 */
class MAGICOPTIMIZER_API SMagicOptimizerScopeSheet : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerScopeSheet) {}
    
    SLATE_ATTRIBUTE(bool, IsOpen)
    SLATE_EVENT(FSimpleDelegate, OnClose)
    SLATE_EVENT(FOnScopeApplied, OnApply)
    
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetIsOpen(bool bInIsOpen);

private:
    // Attributes
    TAttribute<bool> IsOpen;
    FSimpleDelegate OnClose;
    FOnScopeApplied OnApply;

    // Data
    TArray<FFolderSettings> FolderSettings;
    TArray<FString> ExcludePaths;
    TArray<FTextureGroupSettings> TextureGroups;
    bool bApplyOnlyClassified = true;
    int32 ActiveTab = 0; // 0 = Include/Exclude, 1 = Profiles & Groups

    // Widget creation
    TSharedRef<SWidget> MakeScopeSheet();
    TSharedRef<SWidget> MakeTabButtons();
    TSharedRef<SWidget> MakeTabButton(const FString& Label, int32 TabIndex);
    TSharedRef<SWidget> MakeTabContent();
    TSharedRef<SWidget> MakeIncludeExcludeTab();
    TSharedRef<SWidget> MakeIncludeSection();
    TSharedRef<SWidget> MakeExcludeSection();
    TSharedRef<SWidget> MakeProfilesTab();
    TSharedRef<SWidget> MakeTextureGroupCard(int32 GroupIndex);
    TSharedRef<SWidget> MakeINIPreview();

    // Event handlers
    void OnTabChanged(int32 NewTabIndex);
    void OnFolderToggled(const FString& FolderName);
    void OnExcludePathChanged(int32 Index, const FText& NewText);
    void OnAddExcludePath();
    void OnRemoveExcludePath(int32 Index);
    void OnApplyOnlyClassifiedChanged(ECheckBoxState NewState);
    void OnTextureGroupChanged(int32 GroupIndex, const FString& Property, const FText& NewValue);
    void OnApplyChanges();

    // Data management
    void InitializeDefaultData();
    FString GenerateINIContent() const;
    FString GenerateScopeSummary() const;
};
