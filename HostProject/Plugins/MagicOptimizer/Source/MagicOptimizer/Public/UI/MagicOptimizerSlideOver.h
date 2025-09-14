#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "UI/UIDataTypes.h"

// Forward declarations - these are now defined in UIDataTypes.h

class MAGICOPTIMIZER_API SMagicOptimizerSlideOver : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMagicOptimizerSlideOver) {}
    SLATE_ATTRIBUTE(bool, IsOpen)
    SLATE_ATTRIBUTE(FUITextureRow, SelectedTexture)
    SLATE_ATTRIBUTE(TArray<FString>, TargetPlatforms)
    SLATE_EVENT(FOnCloseDrawer, OnClose)
    SLATE_EVENT(FOnApplyToAsset, OnApplyToAsset)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetIsOpen(bool bInIsOpen);
    void SetSelectedTexture(const FUITextureRow& InTexture);

private:
    // Attributes
    TSlateAttribute<bool> IsOpen;
    TSlateAttribute<FUITextureRow> SelectedTexture;
    TSlateAttribute<TArray<FString>> TargetPlatforms;
    
    // Events
    FOnCloseDrawer OnClose;
    FOnApplyToAsset OnApplyToAsset;

    // Widget creation
    TSharedRef<SWidget> MakeSlideOverContent();
    TSharedRef<SWidget> MakeHeader();
    TSharedRef<SWidget> MakeBeforeAfterSection();
    TSharedRef<SWidget> MakeControlsSection();
    TSharedRef<SWidget> MakeCookedFormatsSection();
    TSharedRef<SWidget> MakeEstimatesSection();
    TSharedRef<SWidget> MakeActionsSection();

    // Event handlers
    void OnMaxTextureSizeChanged(const FText& NewText);
    void OnLODBiasChanged(const FText& NewText);
    void OnCompressionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
    void OnApplyToThisAsset();

    // Data management
    FString GetCookedFormat(const FString& Compression, const FString& Target) const;
    TArray<FCookedFormat> GetCookedFormats() const;
    FString GetEstimatedSavings() const;

    // Widget references
    TSharedPtr<SEditableTextBox> MaxTextureSizeTextBox;
    TSharedPtr<SEditableTextBox> LODBiasTextBox;
    TSharedPtr<SComboBox<TSharedPtr<FString>>> CompressionComboBox;

    // Current settings (what-if values)
    FTextureSettings CurrentSettings;
    
    // Compression options
    TArray<TSharedPtr<FString>> CompressionOptions;
};
