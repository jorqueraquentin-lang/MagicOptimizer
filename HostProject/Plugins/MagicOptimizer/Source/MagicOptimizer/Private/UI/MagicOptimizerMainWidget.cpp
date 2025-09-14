#include "UI/MagicOptimizerMainWidget.h"
#include "UI/MagicOptimizerUIDataManager.h"
#include "MagicOptimizerAuditManager.h"
#include "MagicOptimizerDiagnostics.h"
#include "Model/ProgressUpdate.h"
#include "Model/AssetAuditData.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "Components/Spacer.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMagicOptimizerMainWidget::UMagicOptimizerMainWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UIDataManager = nullptr;
	bIsInitialized = false;
}

TSharedRef<SWidget> UMagicOptimizerMainWidget::RebuildWidget()
{
	// Build *before* Super::RebuildWidget so the base class can pull the root
	BuildTreeIfNeeded();
	return Super::RebuildWidget();
}

void UMagicOptimizerMainWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Log sizes once the Slate widget exists
	if (ContentSizeBox)
	{
		const FVector2D Desired = ContentSizeBox->GetDesiredSize();
		MAGIC_LOG(General, FString::Printf(TEXT("MagicOptimizer: ContentSizeBox desired: %s"), *Desired.ToString()));
	}
	if (WidgetTree && WidgetTree->RootWidget)
	{
		MAGIC_LOG(General, FString::Printf(TEXT("MagicOptimizer: RootWidget desired: %s"), *WidgetTree->RootWidget->GetDesiredSize().ToString()));
	}
	
	// Manager should already be injected by the tab spawner at this point
	if (UIDataManager)
	{
		MAGIC_LOG(General, TEXT("NativeConstruct: UIDataManager is valid"));
		BindEvents(true);
	}
	else
	{
		MAGIC_LOG(Warning, TEXT("NativeConstruct: UIDataManager is null - events will not be bound"));
	}
}

void UMagicOptimizerMainWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	// Initialize the widget (manager injection happens later in tab spawner)
	InitializeWidget();
	
	MAGIC_LOG(General, TEXT("NativeOnInitialized completed - waiting for manager injection"));
}

void UMagicOptimizerMainWidget::NativeDestruct()
{
	// Unbind all events before shutdown
	BindEvents(false);
	
	if (UIDataManager)
	{
		UIDataManager->Shutdown();
		UIDataManager = nullptr;
	}
	
	Super::NativeDestruct();
}

void UMagicOptimizerMainWidget::BuildTreeIfNeeded()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}

	if (WidgetTree->RootWidget)
	{
		return; // already built
	}

	MAGIC_LOG(General, TEXT("BuildTreeIfNeeded: Creating comprehensive professional UI"));

	// 1) Root SizeBox with absolute overrides to guarantee non-zero desired size
	USizeBox* RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
	RootSizeBox->SetWidthOverride(1200.f);
	RootSizeBox->SetHeightOverride(800.f);
	RootSizeBox->SetMinDesiredWidth(1000.f);
	RootSizeBox->SetMinDesiredHeight(700.f);
	WidgetTree->RootWidget = RootSizeBox;

	// 2) Main border container
	RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RootBorder"));
	RootBorder->SetPadding(FMargin(0.f));
	RootBorder->SetBrushColor(FLinearColor(0.05f, 0.08f, 0.1f, 1.f)); // Dark professional background
	RootSizeBox->AddChild(RootBorder);

	// 3) Main vertical container
	MainVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainVBox"));
	RootBorder->SetContent(MainVBox);

	// Create the comprehensive UI layout
	CreateComprehensiveUILayout(MainVBox);

	MAGIC_LOG(General, TEXT("BuildTreeIfNeeded: Comprehensive UI created successfully"));
}

void UMagicOptimizerMainWidget::CreateComprehensiveUILayout(UVerticalBox* Parent)
{
	// === HEADER SECTION ===
	CreateModernHeader(Parent);
	
	// === MAIN CONTENT AREA ===
	UHorizontalBox* MainContentArea = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MainContentArea"));
	{
		UVerticalBoxSlot* VBoxSlot = Parent->AddChildToVerticalBox(MainContentArea);
		VBoxSlot->SetPadding(FMargin(0.f));
		VBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	// Left Rail (Project Scope & Asset Types)
	CreateLeftRail(MainContentArea);
	
	// Center Content (Tabs, Charts, Tables)
	CreateCenterContent(MainContentArea);
	
	// Right Rail (Quick Actions & Summary)
	CreateRightRail(MainContentArea);
}

void UMagicOptimizerMainWidget::CreateModernHeader(UVerticalBox* Parent)
{
	// Header background
	UBorder* HeaderBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HeaderBorder"));
	HeaderBorder->SetBrushColor(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f));
	HeaderBorder->SetPadding(FMargin(16, 12, 16, 12));
	
	UVerticalBox* HeaderContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HeaderContent"));
	HeaderBorder->SetContent(HeaderContent);
	
	// Top row: Title and settings
	UHorizontalBox* TopRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TopRow"));
	{
		UVerticalBoxSlot* VBoxSlot = HeaderContent->AddChildToVerticalBox(TopRow);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 8));
	}
	
	// Title
	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetText(FText::FromString(TEXT("MagicOptimizer")));
	TitleText->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 20));
	TitleText->SetColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f));
	{
		UHorizontalBoxSlot* HBoxSlot = TopRow->AddChildToHorizontalBox(TitleText);
		HBoxSlot->SetPadding(FMargin(0, 0, 16, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	// Preset dropdown
	UComboBoxString* PresetCombo = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("PresetCombo"));
	PresetCombo->AddOption(TEXT("Console Balanced"));
	PresetCombo->AddOption(TEXT("High Performance"));
	PresetCombo->AddOption(TEXT("Mobile Optimized"));
	PresetCombo->SetSelectedOption(TEXT("Console Balanced"));
	{
		UHorizontalBoxSlot* HBoxSlot = TopRow->AddChildToHorizontalBox(PresetCombo);
		HBoxSlot->SetPadding(FMargin(0, 0, 16, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	// Console target icons
	CreateConsoleTargetIcons(TopRow);
	
	// Scope info
	UTextBlock* ScopeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ScopeText"));
	ScopeText->SetText(FText::FromString(TEXT("Scope: /Game, 2 excludes")));
	ScopeText->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
	{
		UHorizontalBoxSlot* HBoxSlot = TopRow->AddChildToHorizontalBox(ScopeText);
		HBoxSlot->SetPadding(FMargin(0, 0, 16, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	// Change Cap input
	UEditableTextBox* ChangeCapInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("ChangeCapInput"));
	ChangeCapInput->SetHintText(FText::FromString(TEXT("Change Cap 800")));
	ChangeCapInput->SetText(FText::FromString(TEXT("800")));
	{
		UHorizontalBoxSlot* HBoxSlot = TopRow->AddChildToHorizontalBox(ChangeCapInput);
		HBoxSlot->SetPadding(FMargin(0, 0, 16, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	// Action buttons
	CreateProfessionalActionButtons(TopRow);
	
	// Add header to parent
	{
		UVerticalBoxSlot* VBoxSlot = Parent->AddChildToVerticalBox(HeaderBorder);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 8));
	}
}

void UMagicOptimizerMainWidget::CreateConsoleTargetIcons(UHorizontalBox* Parent)
{
	// Console target icons (PS5, Xbox, etc.)
	FString ConsoleTargets[] = { TEXT("PS5"), TEXT("Xbox"), TEXT("PC"), TEXT("Quest"), TEXT("Switch") };
	for (const FString& Console : ConsoleTargets)
	{
		UButton* ConsoleButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), *FString::Printf(TEXT("Console_%s"), *Console));
		ConsoleButton->SetBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f));
		{
			UHorizontalBoxSlot* HBoxSlot = Parent->AddChildToHorizontalBox(ConsoleButton);
			HBoxSlot->SetPadding(FMargin(0, 0, 4, 0));
			HBoxSlot->SetSize(ESlateSizeRule::Fill);
		}
		
		UTextBlock* ConsoleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("ConsoleText_%s"), *Console));
		ConsoleText->SetText(FText::FromString(Console));
		ConsoleText->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
		ConsoleButton->SetContent(ConsoleText);
	}
}

void UMagicOptimizerMainWidget::CreateProfessionalActionButtons(UHorizontalBox* Parent)
{
	// Run Scan button with play icon
	UButton* RunScanButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RunScanButton"));
	RunScanButton->SetBackgroundColor(FLinearColor(0.2f, 0.7f, 0.2f, 1.0f));
	{
		UHorizontalBoxSlot* HBoxSlot = Parent->AddChildToHorizontalBox(RunScanButton);
		HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UTextBlock* RunScanText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RunScanText"));
	RunScanText->SetText(FText::FromString(TEXT("Run Scan")));
	RunScanText->SetColorAndOpacity(FLinearColor::White);
	RunScanButton->SetContent(RunScanText);
	
	// Auto Fix button with wrench icon
	UButton* AutoFixButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("AutoFixButton"));
	AutoFixButton->SetBackgroundColor(FLinearColor(0.2f, 0.5f, 0.8f, 1.0f));
	{
		UHorizontalBoxSlot* HBoxSlot = Parent->AddChildToHorizontalBox(AutoFixButton);
		HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UTextBlock* AutoFixText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AutoFixText"));
	AutoFixText->SetText(FText::FromString(TEXT("Auto Fix")));
	AutoFixText->SetColorAndOpacity(FLinearColor::White);
	AutoFixButton->SetContent(AutoFixText);
	
	// Preview button with eye icon
	UButton* PreviewButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PreviewButton"));
	PreviewButton->SetBackgroundColor(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));
	{
		UHorizontalBoxSlot* HBoxSlot = Parent->AddChildToHorizontalBox(PreviewButton);
		HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UTextBlock* PreviewText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PreviewText"));
	PreviewText->SetText(FText::FromString(TEXT("Preview")));
	PreviewText->SetColorAndOpacity(FLinearColor::White);
	PreviewButton->SetContent(PreviewText);
	
	// Tasks button with clipboard icon
	UButton* TasksButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("TasksButton"));
	TasksButton->SetBackgroundColor(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));
	{
		UHorizontalBoxSlot* HBoxSlot = Parent->AddChildToHorizontalBox(TasksButton);
		HBoxSlot->SetPadding(FMargin(0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UTextBlock* TasksText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TasksText"));
	TasksText->SetText(FText::FromString(TEXT("Tasks")));
	TasksText->SetColorAndOpacity(FLinearColor::White);
	TasksButton->SetContent(TasksText);
}

void UMagicOptimizerMainWidget::CreateActionButtons(UHorizontalBox* Parent)
{
	// Run Scan button
	UButton* RunScanButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RunScanButton"));
	RunScanButton->SetBackgroundColor(FLinearColor(0.2f, 0.7f, 0.2f, 1.0f));
	{
		UHorizontalBoxSlot* HBoxSlot = Parent->AddChildToHorizontalBox(RunScanButton);
		HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UTextBlock* RunScanText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RunScanText"));
	RunScanText->SetText(FText::FromString(TEXT("Run Scan")));
	RunScanText->SetColorAndOpacity(FLinearColor::White);
	RunScanButton->SetContent(RunScanText);
	
	// Auto Fix button
	UButton* AutoFixButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("AutoFixButton"));
	AutoFixButton->SetBackgroundColor(FLinearColor(0.2f, 0.5f, 0.8f, 1.0f));
	{
		UHorizontalBoxSlot* HBoxSlot = Parent->AddChildToHorizontalBox(AutoFixButton);
		HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UTextBlock* AutoFixText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AutoFixText"));
	AutoFixText->SetText(FText::FromString(TEXT("Auto Fix")));
	AutoFixText->SetColorAndOpacity(FLinearColor::White);
	AutoFixButton->SetContent(AutoFixText);
	
	// Preview button
	UButton* PreviewButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PreviewButton"));
	PreviewButton->SetBackgroundColor(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));
	{
		UHorizontalBoxSlot* HBoxSlot = Parent->AddChildToHorizontalBox(PreviewButton);
		HBoxSlot->SetPadding(FMargin(0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UTextBlock* PreviewText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PreviewText"));
	PreviewText->SetText(FText::FromString(TEXT("Preview")));
	PreviewText->SetColorAndOpacity(FLinearColor::White);
	PreviewButton->SetContent(PreviewText);
}

void UMagicOptimizerMainWidget::CreatePresetAndTargetChips(UVerticalBox* Parent)
{
	UHorizontalBox* ChipsRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ChipsRow"));
	{
		UVerticalBoxSlot* VBoxSlot = Parent->AddChildToVerticalBox(ChipsRow);
		VBoxSlot->SetPadding(FMargin(0));
	}
	
	// Preset chip
	UBorder* PresetChip = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PresetChip"));
	PresetChip->SetBrushColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f));
	PresetChip->SetPadding(FMargin(8, 4, 8, 4));
	{
		UHorizontalBoxSlot* HBoxSlot = ChipsRow->AddChildToHorizontalBox(PresetChip);
		HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UTextBlock* PresetText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PresetText"));
	PresetText->SetText(FText::FromString(TEXT("Preset: Console Balanced")));
	PresetText->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
	PresetChip->SetContent(PresetText);
	
	// Target chips
	FString TargetPlatforms[] = { TEXT("PS5"), TEXT("Xbox"), TEXT("PC"), TEXT("Quest"), TEXT("Switch") };
	for (const FString& Platform : TargetPlatforms)
	{
		UBorder* TargetChip = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("TargetChip_%s"), *Platform));
		TargetChip->SetBrushColor(FLinearColor(0.2f, 0.3f, 0.5f, 1.0f));
		TargetChip->SetPadding(FMargin(6, 3, 6, 3));
		{
			UHorizontalBoxSlot* HBoxSlot = ChipsRow->AddChildToHorizontalBox(TargetChip);
			HBoxSlot->SetPadding(FMargin(0, 0, 4, 0));
			HBoxSlot->SetSize(ESlateSizeRule::Fill);
		}
		
		UTextBlock* TargetText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("TargetText_%s"), *Platform));
		TargetText->SetText(FText::FromString(Platform));
		TargetText->SetColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f));
		TargetChip->SetContent(TargetText);
	}
}

void UMagicOptimizerMainWidget::CreateCenterContent(UHorizontalBox* Parent)
{
	// Center content container
	UBorder* CenterBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CenterBorder"));
	CenterBorder->SetBrushColor(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f));
	CenterBorder->SetPadding(FMargin(16));
	
	UVerticalBox* CenterContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CenterContent"));
	CenterBorder->SetContent(CenterContent);
	
	// Tab interface
	CreateTabInterface(CenterContent);
	
	// Health overview cards
	CreateHealthOverviewCards(CenterContent);
	
	// Data tables and charts
	CreateDataVisualization(CenterContent);
	
	// Add to parent
	{
		UHorizontalBoxSlot* HBoxSlot = Parent->AddChildToHorizontalBox(CenterBorder);
		HBoxSlot->SetPadding(FMargin(8, 0, 8, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
}

void UMagicOptimizerMainWidget::CreateTabInterface(UVerticalBox* Parent)
{
	// Tab container
	UHorizontalBox* TabContainer = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TabContainer"));
	{
		UVerticalBoxSlot* VBoxSlot = Parent->AddChildToVerticalBox(TabContainer);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 16));
	}
	
	FString TabNames[] = { TEXT("Audit"), TEXT("Budgets"), TEXT("Recommend"), TEXT("Apply"), TEXT("Verify"), TEXT("Reports") };
	for (int32 i = 0; i < 6; ++i)
	{
		UButton* TabButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), *FString::Printf(TEXT("Tab_%d"), i));
		TabButton->SetBackgroundColor(i == 0 ? FLinearColor(0.2f, 0.4f, 0.8f, 1.0f) : FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
		{
			UHorizontalBoxSlot* HBoxSlot = TabContainer->AddChildToHorizontalBox(TabButton);
			HBoxSlot->SetPadding(FMargin(0, 0, 4, 0));
			HBoxSlot->SetSize(ESlateSizeRule::Fill);
		}
		
		UTextBlock* TabText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("TabText_%d"), i));
		TabText->SetText(FText::FromString(TabNames[i]));
		TabText->SetColorAndOpacity(FLinearColor::White);
		TabButton->SetContent(TabText);
	}
}

void UMagicOptimizerMainWidget::CreateHealthOverviewCards(UVerticalBox* Parent)
{
	// Health cards container
	UHorizontalBox* HealthCards = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HealthCards"));
	{
		UVerticalBoxSlot* VBoxSlot = Parent->AddChildToVerticalBox(HealthCards);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 16));
	}
	
	// Health Score card with line graph
	UBorder* HealthScoreCard = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HealthScoreCard"));
	HealthScoreCard->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
	HealthScoreCard->SetPadding(FMargin(16));
	{
		UHorizontalBoxSlot* HBoxSlot = HealthCards->AddChildToHorizontalBox(HealthScoreCard);
		HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UVerticalBox* HealthScoreContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HealthScoreContent"));
	HealthScoreCard->SetContent(HealthScoreContent);
	
	// Health score with large number
	UTextBlock* HealthScore = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthScore"));
	HealthScore->SetText(FText::FromString(TEXT("78/100")));
	HealthScore->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("LargeBoldFont").FontObject, 32));
	HealthScore->SetColorAndOpacity(FLinearColor(0.2f, 0.8f, 0.2f, 1.0f));
	{
		UVerticalBoxSlot* VBoxSlot = HealthScoreContent->AddChildToVerticalBox(HealthScore);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 8));
	}
	
	// Simple line graph representation (using text for now)
	UTextBlock* LineGraph = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LineGraph"));
	LineGraph->SetText(FText::FromString(TEXT("📈 75 → 78 → 80 → 78")));
	LineGraph->SetColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f));
	{
		UVerticalBoxSlot* VBoxSlot = HealthScoreContent->AddChildToVerticalBox(LineGraph);
		VBoxSlot->SetPadding(FMargin(0));
	}
	
	// Top Offenders card with detailed badges
	UBorder* OffendersCard = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OffendersCard"));
	OffendersCard->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
	OffendersCard->SetPadding(FMargin(16));
	{
		UHorizontalBoxSlot* HBoxSlot = HealthCards->AddChildToHorizontalBox(OffendersCard);
		HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UVerticalBox* OffendersContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OffendersContent"));
	OffendersCard->SetContent(OffendersContent);
	
	UTextBlock* OffendersTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OffendersTitle"));
	OffendersTitle->SetText(FText::FromString(TEXT("Top Offenders")));
	OffendersTitle->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14));
	OffendersTitle->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
	{
		UVerticalBoxSlot* VBoxSlot = OffendersContent->AddChildToVerticalBox(OffendersTitle);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 8));
	}
	
	// Detailed offenders with colored badges
	FString OffenderItems[] = { TEXT("Masks with sRGB on"), TEXT("Normals not BC5"), TEXT("Non VT assets flagged VT") };
	int32 OffenderCounts[] = { 45, 82, 16 };
	FLinearColor OffenderColors[] = { FLinearColor(0.9f, 0.3f, 0.3f, 1.0f), FLinearColor(0.3f, 0.5f, 0.9f, 1.0f), FLinearColor(0.2f, 0.3f, 0.6f, 1.0f) };
	
	for (int32 i = 0; i < 3; ++i)
	{
		UHorizontalBox* OffenderRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), *FString::Printf(TEXT("OffenderRow_%d"), i));
		{
			UVerticalBoxSlot* VBoxSlot = OffendersContent->AddChildToVerticalBox(OffenderRow);
			VBoxSlot->SetPadding(FMargin(0, 0, 0, 4));
		}
		
		UTextBlock* OffenderText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("OffenderText_%d"), i));
		OffenderText->SetText(FText::FromString(OffenderItems[i]));
		OffenderText->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
		{
			UHorizontalBoxSlot* HBoxSlot = OffenderRow->AddChildToHorizontalBox(OffenderText);
			HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
			HBoxSlot->SetSize(ESlateSizeRule::Fill);
		}
		
		// Badge with count
		UBorder* OffenderBadge = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("OffenderBadge_%d"), i));
		OffenderBadge->SetBrushColor(OffenderColors[i]);
		OffenderBadge->SetPadding(FMargin(6, 2, 6, 2));
		{
			UHorizontalBoxSlot* HBoxSlot = OffenderRow->AddChildToHorizontalBox(OffenderBadge);
			HBoxSlot->SetPadding(FMargin(0));
			HBoxSlot->SetSize(ESlateSizeRule::Fill);
		}
		
		UTextBlock* OffenderCount = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("OffenderCount_%d"), i));
		OffenderCount->SetText(FText::FromString(FString::Printf(TEXT("%d"), OffenderCounts[i])));
		OffenderCount->SetColorAndOpacity(FLinearColor::White);
		OffenderBadge->SetContent(OffenderCount);
	}
	
	// Summary card
	UBorder* SummaryCard = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SummaryCard"));
	SummaryCard->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
	SummaryCard->SetPadding(FMargin(16));
	{
		UHorizontalBoxSlot* HBoxSlot = HealthCards->AddChildToHorizontalBox(SummaryCard);
		HBoxSlot->SetPadding(FMargin(0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UVerticalBox* SummaryContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SummaryContent"));
	SummaryCard->SetContent(SummaryContent);
	
	UTextBlock* SummaryTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SummaryTitle"));
	SummaryTitle->SetText(FText::FromString(TEXT("Summary")));
	SummaryTitle->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14));
	SummaryTitle->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
	{
		UVerticalBoxSlot* VBoxSlot = SummaryContent->AddChildToVerticalBox(SummaryTitle);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 8));
	}
	
	UTextBlock* SummaryText1 = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SummaryText1"));
	SummaryText1->SetText(FText::FromString(TEXT("Preset: Console Balanced")));
	SummaryText1->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
	{
		UVerticalBoxSlot* VBoxSlot = SummaryContent->AddChildToVerticalBox(SummaryText1);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 4));
	}
	
	UTextBlock* SummaryText2 = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SummaryText2"));
	SummaryText2->SetText(FText::FromString(TEXT("Targets: PS5, Xbox")));
	SummaryText2->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
	{
		UVerticalBoxSlot* VBoxSlot = SummaryContent->AddChildToVerticalBox(SummaryText2);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 4));
	}
	
	UTextBlock* SummaryText3 = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SummaryText3"));
	SummaryText3->SetText(FText::FromString(TEXT("Scope: /Game 2 excludes")));
	SummaryText3->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
	{
		UVerticalBoxSlot* VBoxSlot = SummaryContent->AddChildToVerticalBox(SummaryText3);
		VBoxSlot->SetPadding(FMargin(0));
	}
}

void UMagicOptimizerMainWidget::CreateDataVisualization(UVerticalBox* Parent)
{
	// Data section title
	UTextBlock* DataTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DataTitle"));
	DataTitle->SetText(FText::FromString(TEXT("Data")));
	DataTitle->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 18));
	DataTitle->SetColorAndOpacity(FLinearColor::White);
	{
		UVerticalBoxSlot* VBoxSlot = Parent->AddChildToVerticalBox(DataTitle);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 16));
	}
	
	// Data visualization container
	UHorizontalBox* DataContainer = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("DataContainer"));
	{
		UVerticalBoxSlot* VBoxSlot = Parent->AddChildToVerticalBox(DataContainer);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 16));
	}
	
	// 3D Bar Chart with detailed data
	UBorder* BarChartCard = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BarChartCard"));
	BarChartCard->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
	BarChartCard->SetPadding(FMargin(16));
	{
		UHorizontalBoxSlot* HBoxSlot = DataContainer->AddChildToHorizontalBox(BarChartCard);
		HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UVerticalBox* BarChartContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BarChartContent"));
	BarChartCard->SetContent(BarChartContent);
	
	UTextBlock* BarChartTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BarChartTitle"));
	BarChartTitle->SetText(FText::FromString(TEXT("3D Bar Chart")));
	BarChartTitle->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14));
	BarChartTitle->SetColorAndOpacity(FLinearColor::White);
	{
		UVerticalBoxSlot* VBoxSlot = BarChartContent->AddChildToVerticalBox(BarChartTitle);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 8));
	}
	
	// 3D bar chart representation with ASCII art
	UTextBlock* BarChartVisual = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BarChartVisual"));
	BarChartVisual->SetText(FText::FromString(TEXT("📊 3D Bar Chart\n\n    ███\n  ███ ███\n█████ ███\n█████ ███\n█████ ███\n\nTextures  Materials")));
	BarChartVisual->SetColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f));
	BarChartVisual->SetJustification(ETextJustify::Center);
	{
		UVerticalBoxSlot* VBoxSlot = BarChartContent->AddChildToVerticalBox(BarChartVisual);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 8));
	}
	
	// Bar chart legend
	UTextBlock* BarChartLegend = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BarChartLegend"));
	BarChartLegend->SetText(FText::FromString(TEXT("Textures: 1,234\nMaterials: 567")));
	BarChartLegend->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
	BarChartLegend->SetJustification(ETextJustify::Center);
	{
		UVerticalBoxSlot* VBoxSlot = BarChartContent->AddChildToVerticalBox(BarChartLegend);
		VBoxSlot->SetPadding(FMargin(0));
	}
	
	// Donut Chart with detailed data
	UBorder* DonutChartCard = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DonutChartCard"));
	DonutChartCard->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
	DonutChartCard->SetPadding(FMargin(16));
	{
		UHorizontalBoxSlot* HBoxSlot = DataContainer->AddChildToHorizontalBox(DonutChartCard);
		HBoxSlot->SetPadding(FMargin(0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UVerticalBox* DonutChartContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DonutChartContent"));
	DonutChartCard->SetContent(DonutChartContent);
	
	UTextBlock* DonutChartTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DonutChartTitle"));
	DonutChartTitle->SetText(FText::FromString(TEXT("Donut Chart")));
	DonutChartTitle->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14));
	DonutChartTitle->SetColorAndOpacity(FLinearColor::White);
	{
		UVerticalBoxSlot* VBoxSlot = DonutChartContent->AddChildToVerticalBox(DonutChartTitle);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 8));
	}
	
	// Donut chart representation with ASCII art
	UTextBlock* DonutChartVisual = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DonutChartVisual"));
	DonutChartVisual->SetText(FText::FromString(TEXT("🍩 Donut Chart\n\n    ╭─────╮\n  ╱         ╲\n ╱     ●     ╲\n╱             ╲\n╲             ╱\n ╲     ●     ╱\n  ╲         ╱\n    ╰─────╯\n\nOptimized  Issues")));
	DonutChartVisual->SetColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f));
	DonutChartVisual->SetJustification(ETextJustify::Center);
	{
		UVerticalBoxSlot* VBoxSlot = DonutChartContent->AddChildToVerticalBox(DonutChartVisual);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 8));
	}
	
	// Donut chart legend with percentages
	UTextBlock* DonutChartLegend = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DonutChartLegend"));
	DonutChartLegend->SetText(FText::FromString(TEXT("Optimized: 78%\nIssues: 22%")));
	DonutChartLegend->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
	DonutChartLegend->SetJustification(ETextJustify::Center);
	{
		UVerticalBoxSlot* VBoxSlot = DonutChartContent->AddChildToVerticalBox(DonutChartLegend);
		VBoxSlot->SetPadding(FMargin(0));
	}
}

void UMagicOptimizerMainWidget::CreateAssetTable(UVerticalBox* Parent)
{
	// Table container
	UBorder* TableContainer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TableContainer"));
	TableContainer->SetBrushColor(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f));
	TableContainer->SetPadding(FMargin(8));
	
	UVerticalBox* TableContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TableContent"));
	TableContainer->SetContent(TableContent);
	
	// Table header
	UHorizontalBox* TableHeader = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TableHeader"));
	{
		UVerticalBoxSlot* VBoxSlot = TableContent->AddChildToVerticalBox(TableHeader);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 8));
	}
	
	FString HeaderColumns[] = { TEXT("Asset Path"), TEXT("Size"), TEXT("Status") };
	for (const FString& Column : HeaderColumns)
	{
		UTextBlock* HeaderText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("Header_%s"), *Column));
		HeaderText->SetText(FText::FromString(Column));
		HeaderText->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 12));
		HeaderText->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
		{
			UHorizontalBoxSlot* HBoxSlot = TableHeader->AddChildToHorizontalBox(HeaderText);
			HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
			HBoxSlot->SetSize(ESlateSizeRule::Fill);
		}
	}
	
	// Sample table rows
	FString SampleAssets[] = { 
		TEXT("/Game/Props/T_005 2kPending"),
		TEXT("/Game/Textures/UI_Button 1k"),
		TEXT("/Game/Materials/M_Default 512"),
		TEXT("/Game/Characters/Player_Texture 4k"),
		TEXT("/Game/Environment/Grass_Texture 2k")
	};
	
	for (int32 i = 0; i < 5; ++i)
	{
		UHorizontalBox* TableRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), *FString::Printf(TEXT("TableRow_%d"), i));
		{
			UVerticalBoxSlot* VBoxSlot = TableContent->AddChildToVerticalBox(TableRow);
			VBoxSlot->SetPadding(FMargin(0, 0, 0, 4));
		}
		
		// Asset path
		UTextBlock* AssetPath = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("AssetPath_%d"), i));
		AssetPath->SetText(FText::FromString(SampleAssets[i]));
		AssetPath->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
		{
			UHorizontalBoxSlot* HBoxSlot = TableRow->AddChildToHorizontalBox(AssetPath);
			HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
			HBoxSlot->SetSize(ESlateSizeRule::Fill);
		}
		
		// Size
		UTextBlock* AssetSize = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("AssetSize_%d"), i));
		AssetSize->SetText(FText::FromString(TEXT("2.4 MB")));
		AssetSize->SetColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f));
		{
			UHorizontalBoxSlot* HBoxSlot = TableRow->AddChildToHorizontalBox(AssetSize);
			HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
			HBoxSlot->SetSize(ESlateSizeRule::Fill);
		}
		
		// Status
		UTextBlock* AssetStatus = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("AssetStatus_%d"), i));
		AssetStatus->SetText(FText::FromString(TEXT("Needs Optimization")));
		AssetStatus->SetColorAndOpacity(FLinearColor(0.9f, 0.4f, 0.4f, 1.0f));
		{
			UHorizontalBoxSlot* HBoxSlot = TableRow->AddChildToHorizontalBox(AssetStatus);
			HBoxSlot->SetPadding(FMargin(0));
			HBoxSlot->SetSize(ESlateSizeRule::Fill);
		}
	}
	
	// Add table to parent
	{
		UVerticalBoxSlot* VBoxSlot = Parent->AddChildToVerticalBox(TableContainer);
		VBoxSlot->SetPadding(FMargin(0));
	}
}

void UMagicOptimizerMainWidget::CreateRightRail(UHorizontalBox* Parent)
{
	// Right rail container
	UBorder* RightRailBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RightRailBorder"));
	RightRailBorder->SetBrushColor(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f));
	RightRailBorder->SetPadding(FMargin(16));
	
	UVerticalBox* RightRailContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RightRailContent"));
	RightRailBorder->SetContent(RightRailContent);
	
	// Quick Actions section
	UTextBlock* QuickActionsTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("QuickActionsTitle"));
	QuickActionsTitle->SetText(FText::FromString(TEXT("Quick Actions")));
	QuickActionsTitle->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 16));
	QuickActionsTitle->SetColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f));
	{
		UVerticalBoxSlot* VBoxSlot = RightRailContent->AddChildToVerticalBox(QuickActionsTitle);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 12));
	}
	
	// Quick action buttons
	UButton* QuickFixButton1 = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("QuickFixButton1"));
	QuickFixButton1->SetBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f));
	{
		UVerticalBoxSlot* VBoxSlot = RightRailContent->AddChildToVerticalBox(QuickFixButton1);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 8));
	}
	
	UTextBlock* QuickFixText1 = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("QuickFixText1"));
	QuickFixText1->SetText(FText::FromString(TEXT("Set sRGB off for mask textures")));
	QuickFixText1->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
	QuickFixButton1->SetContent(QuickFixText1);
	
	UButton* PreviewPlanButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PreviewPlanButton"));
	PreviewPlanButton->SetBackgroundColor(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f));
	{
		UVerticalBoxSlot* VBoxSlot = RightRailContent->AddChildToVerticalBox(PreviewPlanButton);
		VBoxSlot->SetPadding(FMargin(0));
	}
	
	UTextBlock* PreviewPlanText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PreviewPlanText"));
	PreviewPlanText->SetText(FText::FromString(TEXT("Preview Plan")));
	PreviewPlanText->SetColorAndOpacity(FLinearColor::White);
	PreviewPlanButton->SetContent(PreviewPlanText);
	
	// Add to parent
	{
		UHorizontalBoxSlot* HBoxSlot = Parent->AddChildToHorizontalBox(RightRailBorder);
		HBoxSlot->SetPadding(FMargin(8, 0, 0, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
		// Note: UHorizontalBoxSlot doesn't have SetMaxSize, using fixed width via container
	}
}

void UMagicOptimizerMainWidget::CreateLeftRail(UHorizontalBox* Parent)
{
	// Left rail container
	UBorder* LeftRailBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LeftRailBorder"));
	LeftRailBorder->SetBrushColor(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f));
	LeftRailBorder->SetPadding(FMargin(16));
	
	UVerticalBox* LeftRailContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LeftRailContent"));
	LeftRailBorder->SetContent(LeftRailContent);
	
	// Project Scope section with expandable Game folder
	UTextBlock* ScopeTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ScopeTitle"));
	ScopeTitle->SetText(FText::FromString(TEXT("Project Scope")));
	ScopeTitle->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14));
	ScopeTitle->SetColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f));
	{
		UVerticalBoxSlot* VBoxSlot = LeftRailContent->AddChildToVerticalBox(ScopeTitle);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 8));
	}
	
	// Game folder with dropdown arrow
	UHorizontalBox* GameFolderRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("GameFolderRow"));
	{
		UVerticalBoxSlot* VBoxSlot = LeftRailContent->AddChildToVerticalBox(GameFolderRow);
		VBoxSlot->SetPadding(FMargin(0, 0, 0, 8));
	}
	
	UTextBlock* DropdownArrow = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DropdownArrow"));
	DropdownArrow->SetText(FText::FromString(TEXT("▼")));
	DropdownArrow->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
	{
		UHorizontalBoxSlot* HBoxSlot = GameFolderRow->AddChildToHorizontalBox(DropdownArrow);
		HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	UTextBlock* GameFolderText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GameFolderText"));
	GameFolderText->SetText(FText::FromString(TEXT("Game")));
	GameFolderText->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
	{
		UHorizontalBoxSlot* HBoxSlot = GameFolderRow->AddChildToHorizontalBox(GameFolderText);
		HBoxSlot->SetPadding(FMargin(0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
	
	// Project scope checkboxes (indented)
	FString ProjectScopeItems[] = { TEXT("Maps"), TEXT("Meshes"), TEXT("Textures"), TEXT("Materials"), TEXT("UI"), TEXT("Audio"), TEXT("Niagara") };
	bool ProjectScopeChecked[] = { true, true, true, true, true, false, false };
	
	for (int32 i = 0; i < 7; ++i)
	{
		UHorizontalBox* CheckboxRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), *FString::Printf(TEXT("ProjectScopeRow_%d"), i));
		{
			UVerticalBoxSlot* VBoxSlot = LeftRailContent->AddChildToVerticalBox(CheckboxRow);
			VBoxSlot->SetPadding(FMargin(16, 0, 0, 4)); // Indent
		}
		
		UCheckBox* ScopeCheckbox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), *FString::Printf(TEXT("ScopeCheckbox_%d"), i));
		ScopeCheckbox->SetIsChecked(ProjectScopeChecked[i]);
		{
			UHorizontalBoxSlot* HBoxSlot = CheckboxRow->AddChildToHorizontalBox(ScopeCheckbox);
			HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
			HBoxSlot->SetSize(ESlateSizeRule::Fill);
		}
		
		UTextBlock* ScopeLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("ScopeLabel_%d"), i));
		ScopeLabel->SetText(FText::FromString(ProjectScopeItems[i]));
		ScopeLabel->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11));
		ScopeLabel->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
		{
			UHorizontalBoxSlot* HBoxSlot = CheckboxRow->AddChildToHorizontalBox(ScopeLabel);
			HBoxSlot->SetPadding(FMargin(0));
			HBoxSlot->SetSize(ESlateSizeRule::Fill);
		}
	}
	
	// Asset Types section
	UTextBlock* AssetTypesTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AssetTypesTitle"));
	AssetTypesTitle->SetText(FText::FromString(TEXT("Asset Types")));
	AssetTypesTitle->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 12));
	AssetTypesTitle->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
	{
		UVerticalBoxSlot* VBoxSlot = LeftRailContent->AddChildToVerticalBox(AssetTypesTitle);
		VBoxSlot->SetPadding(FMargin(0, 16, 0, 8));
	}
	
	// Asset type checkboxes
	FString AssetTypes[] = { TEXT("Textures"), TEXT("Meshes"), TEXT("Materials"), TEXT("Blueprints"), TEXT("Animations"), TEXT("Audio"), TEXT("Niagara"), TEXT("Levels"), TEXT("UI") };
	bool AssetTypesChecked[] = { true, false, false, false, false, false, false, false, false };
	
	for (int32 i = 0; i < 9; ++i)
	{
		UHorizontalBox* CheckboxRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), *FString::Printf(TEXT("AssetTypeRow_%d"), i));
		{
			UVerticalBoxSlot* VBoxSlot = LeftRailContent->AddChildToVerticalBox(CheckboxRow);
			VBoxSlot->SetPadding(FMargin(0, 0, 0, 4));
		}
		
		UCheckBox* AssetCheckbox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), *FString::Printf(TEXT("AssetCheckbox_%d"), i));
		AssetCheckbox->SetIsChecked(AssetTypesChecked[i]);
		{
			UHorizontalBoxSlot* HBoxSlot = CheckboxRow->AddChildToHorizontalBox(AssetCheckbox);
			HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
			HBoxSlot->SetSize(ESlateSizeRule::Fill);
		}
		
		UTextBlock* AssetLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("AssetLabel_%d"), i));
		AssetLabel->SetText(FText::FromString(AssetTypes[i]));
		AssetLabel->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11));
		AssetLabel->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
		{
			UHorizontalBoxSlot* HBoxSlot = CheckboxRow->AddChildToHorizontalBox(AssetLabel);
			HBoxSlot->SetPadding(FMargin(0));
			HBoxSlot->SetSize(ESlateSizeRule::Fill);
		}
	}
	
	// Saved Views section
	UTextBlock* SavedViewsTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SavedViewsTitle"));
	SavedViewsTitle->SetText(FText::FromString(TEXT("Saved Views")));
	SavedViewsTitle->SetFont(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 12));
	SavedViewsTitle->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
	{
		UVerticalBoxSlot* VBoxSlot = LeftRailContent->AddChildToVerticalBox(SavedViewsTitle);
		VBoxSlot->SetPadding(FMargin(0, 16, 0, 8));
	}
	
	// Saved view buttons
	FString SavedViews[] = { TEXT("High Impact"), TEXT("UI Pass"), TEXT("Console Ready") };
	for (int32 i = 0; i < 3; ++i)
	{
		UButton* SavedViewButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), *FString::Printf(TEXT("SavedView_%d"), i));
		SavedViewButton->SetBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f));
		{
			UVerticalBoxSlot* VBoxSlot = LeftRailContent->AddChildToVerticalBox(SavedViewButton);
			VBoxSlot->SetPadding(FMargin(0, 0, 0, 4));
		}
		
		UTextBlock* SavedViewText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("SavedViewText_%d"), i));
		SavedViewText->SetText(FText::FromString(SavedViews[i]));
		SavedViewText->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
		SavedViewButton->SetContent(SavedViewText);
	}
	
	// Add to parent
	{
		UHorizontalBoxSlot* HBoxSlot = Parent->AddChildToHorizontalBox(LeftRailBorder);
		HBoxSlot->SetPadding(FMargin(0, 0, 8, 0));
		HBoxSlot->SetSize(ESlateSizeRule::Fill);
	}
}

void UMagicOptimizerMainWidget::CreateProfessionalLayout(UVerticalBox* Parent)
{
	// This method is kept for compatibility but now delegates to the comprehensive layout
	CreateComprehensiveUILayout(Parent);
}

void UMagicOptimizerMainWidget::InitializeWidget()
{
	if (bIsInitialized)
	{
		return;
	}

	MAGIC_LOG(General, TEXT("InitializeWidget: Starting widget initialization"));

	// Create the UIDataManager if it doesn't exist
	if (!UIDataManager)
	{
		UIDataManager = NewObject<UMagicOptimizerUIDataManager>(this);
		if (UIDataManager)
		{
			MAGIC_LOG(General, TEXT("InitializeWidget: Created UIDataManager"));
		}
		else
		{
			MAGIC_LOG(Error, TEXT("InitializeWidget: Failed to create UIDataManager"));
		}
	}

	bIsInitialized = true;
	MAGIC_LOG(General, TEXT("InitializeWidget: Widget initialization completed"));
}

void UMagicOptimizerMainWidget::BindEvents(bool bBind)
{
	if (!UIDataManager)
	{
		MAGIC_LOG(Warning, TEXT("BindEvents: UIDataManager is null, cannot bind events"));
		return;
	}

	MAGIC_LOG(General, FString::Printf(TEXT("BindEvents: %s events"), bBind ? TEXT("Binding") : TEXT("Unbinding")));

	// Bind/unbind events here when needed
	// Example: UIDataManager->OnDataChanged.AddDynamic(this, &UMagicOptimizerMainWidget::OnDataChanged);
}

void UMagicOptimizerMainWidget::SetDebugText(const FString& InText)
{
	if (DebugText)
	{
		DebugText->SetText(FText::FromString(InText));
		MAGIC_LOG(General, FString::Printf(TEXT("SetDebugText: %s"), *InText));
	}
	else
	{
		MAGIC_LOG(Warning, TEXT("SetDebugText: DebugText widget is null"));
	}
}

void UMagicOptimizerMainWidget::SetUIDataManager(UMagicOptimizerUIDataManager* InManager)
{
	UIDataManager = InManager;
	MAGIC_LOG(General, TEXT("SetUIDataManager: Manager set"));
}

void UMagicOptimizerMainWidget::UpdateUI()
{
	MAGIC_LOG(General, TEXT("UpdateUI: Updating UI"));
	// Update UI elements here when needed
}

void UMagicOptimizerMainWidget::OnStartAuditClicked()
{
	MAGIC_LOG(General, TEXT("OnStartAuditClicked: Starting audit"));
	// Start audit logic here
}

void UMagicOptimizerMainWidget::OnStopAuditClicked()
{
	MAGIC_LOG(General, TEXT("OnStopAuditClicked: Stopping audit"));
	// Stop audit logic here
}

void UMagicOptimizerMainWidget::OnGenerateReportClicked()
{
	MAGIC_LOG(General, TEXT("OnGenerateReportClicked: Generating report"));
	// Generate report logic here
}

void UMagicOptimizerMainWidget::OnProgressUpdated(const FProgressUpdate& ProgressUpdate)
{
	MAGIC_LOG(General, TEXT("OnProgressUpdated: Progress updated"));
	// Update progress UI here
}

void UMagicOptimizerMainWidget::OnAssetProcessed(const FAssetAuditData& AssetData)
{
	MAGIC_LOG(General, FString::Printf(TEXT("OnAssetProcessed: %s"), *AssetData.AssetPath));
	// Update asset list UI here
}

void UMagicOptimizerMainWidget::OnAuditCompleted()
{
	MAGIC_LOG(General, TEXT("OnAuditCompleted: Audit finished"));
	// Update UI for completed audit
}

void UMagicOptimizerMainWidget::OnAuditStarted()
{
	MAGIC_LOG(General, TEXT("OnAuditStarted: Audit started"));
	// Update UI for started audit
}