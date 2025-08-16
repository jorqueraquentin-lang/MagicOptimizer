// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "ViewModels/TextureTableViewModel.h"

// Forward declarations
class STextureAuditSection;
class STextureRecommendSection;
class SButton;
class SCheckBox;
class SEditableTextBox;
class SComboBox;
class SExpandableArea;
class SGridPanel;
class STextBlock;

/**
 * Dedicated tab widget for texture optimization functionality
 * Provides a clean, focused interface for texture-related operations
 */
class STexturesTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STexturesTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~STexturesTab();

	// Set the ViewModel for this tab
	void SetViewModel(TSharedPtr<FTextureTableViewModel> InViewModel);

	// Get the ViewModel
	TSharedPtr<FTextureTableViewModel> GetViewModel() const { return TextureTableViewModel; }

protected:
	// ViewModel reference
	TSharedPtr<FTextureTableViewModel> TextureTableViewModel;

	// UI Components
	TSharedPtr<STextureAuditSection> TextureAuditSection;
	TSharedPtr<STextureRecommendSection> TextureRecommendSection;

	// Settings controls
	TSharedPtr<SCheckBox> DryRunCheckBox;
	TSharedPtr<SCheckBox> CreateBackupsCheckBox;
	TSharedPtr<SEditableTextBox> MaxChangesTextBox;
	TSharedPtr<SEditableTextBox> IncludePathsTextBox;
	TSharedPtr<SEditableTextBox> ExcludePathsTextBox;
	TSharedPtr<SEditableTextBox> OutputDirectoryTextBox;

	// Action buttons
	TSharedPtr<SButton> AuditButton;
	TSharedPtr<SButton> RecommendButton;
	TSharedPtr<SButton> ApplyButton;
	TSharedPtr<SButton> VerifyButton;

	// Profile selection
	TArray<TSharedPtr<FString>> TargetProfiles;
	FString CurrentProfile;

	// UI Event Handlers
	FReply OnAuditClicked();
	FReply OnRecommendClicked();
	FReply OnApplyClicked();
	FReply OnVerifyClicked();
	void OnProfileSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	FText GetCurrentProfileText() const;

	// Settings handlers
	void OnDryRunChanged(ECheckBoxState NewState);
	ECheckBoxState IsDryRunChecked() const;
	void OnCreateBackupsChanged(ECheckBoxState NewState);
	ECheckBoxState IsCreateBackupsChecked() const;
	void OnMaxChangesChanged(const FText& NewText);
	FText GetMaxChanges() const;
	void OnIncludePathsChanged(const FText& NewText);
	FText GetIncludePaths() const;
	void OnExcludePathsChanged(const FText& NewText);
	FText GetExcludePaths() const;
	void OnOutputDirectoryChanged(const FText& NewText);
	FText GetOutputDirectory() const;

	// Helper methods
	void InitializeUI();
	void LoadSettings();
	void SaveSettings();
	void ShowNotification(const FString& Message, bool bIsSuccess = true);
};
