#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/ScrollBox.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/GridPanel.h"
#include "Components/WidgetSwitcher.h"
#include "UI/MagicOptimizerUIDataManager.h"
#include "Model/ProgressUpdate.h"
#include "Model/AssetAuditData.h"
#include "MagicOptimizerMainWidget.generated.h"

/**
 * Main UI Widget for Magic Optimizer Plugin
 * Provides a clean, data-driven interface connected to real audit data
 */
UCLASS(BlueprintType, Blueprintable)
class MAGICOPTIMIZER_API UMagicOptimizerMainWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMagicOptimizerMainWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	// Note: UI Components are now created dynamically in CreateWidgetHierarchy()
	// and stored in the private member variables below

public:
	// Initialize the widget
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	void InitializeWidget();

	// Public update helpers (optional)
	UFUNCTION(BlueprintCallable, Category="MagicOptimizer|UI")
	void SetDebugText(const FString& InText);

	// Event binding management
	void BindEvents(bool bBind);

	// Manager injection and access
	UFUNCTION(BlueprintCallable)
	void SetUIDataManager(UMagicOptimizerUIDataManager* InMgr);

	// Optional helper for safe access
	UMagicOptimizerUIDataManager* GetUIDataManagerChecked() const;

	// Update UI with current data
	UFUNCTION(BlueprintCallable, Category = "Magic Optimizer|UI")
	void UpdateUI();

	// Event handlers
	UFUNCTION()
	void OnStartAuditClicked();

	UFUNCTION()
	void OnStopAuditClicked();

	UFUNCTION()
	void OnGenerateReportClicked();

	// Data manager events
	UFUNCTION()
	void OnProgressUpdated(const FProgressUpdate& ProgressUpdate);

	UFUNCTION()
	void OnAssetProcessed(const FAssetAuditData& AssetData);

	UFUNCTION()
	void OnAuditCompleted();

	UFUNCTION()
	void OnAuditStarted();

private:
	// Internal state
	// UI Data Manager - Use TObjectPtr for better GC management
	UPROPERTY()
	TObjectPtr<UMagicOptimizerUIDataManager> UIDataManager;

	bool bIsInitialized;

	// Widget creation functions
	void BuildTreeIfNeeded();
	void CreateComprehensiveUILayout(class UVerticalBox* Parent);
	void CreateModernHeader(class UVerticalBox* Parent);
	void CreateConsoleTargetIcons(class UHorizontalBox* Parent);
	void CreateProfessionalActionButtons(class UHorizontalBox* Parent);
	void CreateActionButtons(class UHorizontalBox* Parent);
	void CreatePresetAndTargetChips(class UVerticalBox* Parent);
	void CreateCenterContent(class UHorizontalBox* Parent);
	void CreateTabInterface(class UVerticalBox* Parent);
	void CreateHealthOverviewCards(class UVerticalBox* Parent);
	void CreateDataVisualization(class UVerticalBox* Parent);
	void CreateAssetTable(class UVerticalBox* Parent);
	void CreateRightRail(class UHorizontalBox* Parent);
	void CreateLeftRail(class UHorizontalBox* Parent);
	void CreateProfessionalLayout(class UVerticalBox* Parent);
	void CreateControlButtonsSection(class UVerticalBox* Parent);
	void CreateProgressSection(class UVerticalBox* Parent);
	void CreateAssetTableSection(class UVerticalBox* Parent);
	void CreateAssetEntry(class UScrollBox* Parent, const FString& AssetName);

	// Helper functions
	void SetupEventHandlers();
	void UpdateHealthScore();
	void UpdateProgressBar();
	void UpdateStatusText();
	void UpdateAssetList();
	void UpdateMemoryInfo();
	void UpdateButtonStates();

	// Main widget tree components (Transient for proper GC)
	UPROPERTY(Transient)
	class UCanvasPanel* RootCanvas = nullptr;

	UPROPERTY(Transient)
	class USizeBox* ContentSizeBox = nullptr;

	UPROPERTY(Transient)
	class UBorder* RootBorder = nullptr;

	UPROPERTY(Transient)
	class UVerticalBox* MainVBox = nullptr;

	UPROPERTY(Transient)
	class UTextBlock* BannerText = nullptr;

	UPROPERTY(Transient)
	class UTextBlock* TestText = nullptr;

	UPROPERTY(Transient)
	class UTextBlock* DebugText = nullptr;

	UPROPERTY(Transient)
	class UTextBlock* LargeTest = nullptr;

	// UI Component pointers for dynamic widgets
	UPROPERTY()
	class UTextBlock* StatusTextBlock;

	UPROPERTY()
	class UTextBlock* HealthScoreText;

	UPROPERTY()
	class UTextBlock* CriticalIssuesText;

	UPROPERTY()
	class UTextBlock* AssetsScannedText;

	UPROPERTY()
	class UProgressBar* ProgressBar;

	UPROPERTY()
	class UTextBlock* ProgressText;

	UPROPERTY()
	class UButton* StartAuditButton;

	UPROPERTY()
	class UButton* StopAuditButton;
};
