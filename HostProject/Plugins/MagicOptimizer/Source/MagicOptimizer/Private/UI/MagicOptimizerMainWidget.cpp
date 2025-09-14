#include "UI/MagicOptimizerMainWidget.h"
#include "UI/MagicOptimizerUIDataManager.h"
#include "MagicOptimizerAuditManager.h"
#include "MagicOptimizerDiagnostics.h"
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
#include "Engine/World.h"
#include "TimerManager.h"

UMagicOptimizerMainWidget::UMagicOptimizerMainWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UIDataManager = nullptr;
	bIsInitialized = false;
}

void UMagicOptimizerMainWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Guard against multiple tree builds
	if (!WidgetTree || !WidgetTree->RootWidget)
	{
		// Create the widget hierarchy entirely in C++
		CreateWidgetHierarchy();
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

void UMagicOptimizerMainWidget::CreateWidgetHierarchy()
{
	MAGIC_LOG(General, TEXT("Creating UMG Widget Hierarchy"));
	
	// Create a SizeBox as the root widget with ABSOLUTE dimensions (not minimum)
	USizeBox* RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
	RootSizeBox->SetWidthOverride(800.0f);   // Set ABSOLUTE width - not minimum!
	RootSizeBox->SetHeightOverride(600.0f);  // Set ABSOLUTE height - not minimum!
	WidgetTree->RootWidget = RootSizeBox;
	
	// Create main vertical container with visual debug background
	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RootBorder"));
	RootBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.2f, 0.8f)); // Semi-transparent dark background
	RootBorder->SetPadding(FMargin(10));
	RootSizeBox->SetContent(RootBorder);
	
	UVerticalBox* MainVerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainVerticalBox"));
	RootBorder->SetContent(MainVerticalBox);
	
	// Ensure the widget fills the available space
	MainVerticalBox->SetVisibility(ESlateVisibility::Visible);
	
	// Add a prominent banner to verify UMG is working
	UTextBlock* Banner = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Banner"));
	Banner->SetText(FText::FromString(TEXT("MagicOptimizer")));
	Banner->SetFont(FAppStyle::Get().GetFontStyle("Bold"));
	Banner->SetColorAndOpacity(FLinearColor::White);
	MainVerticalBox->AddChild(Banner);
	
	// Add a simple test to verify UMG is working
	UTextBlock* TestText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TestText"));
	TestText->SetText(FText::FromString("UMG Widget - WORKING!"));
	TestText->SetColorAndOpacity(FLinearColor::Green);
	MainVerticalBox->AddChild(TestText);
	MAGIC_LOG(General, TEXT("Added test text to widget"));
	
	// Add a debug info text that will always be visible
	UTextBlock* DebugText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DebugText"));
	DebugText->SetText(FText::FromString("Debug: Widget hierarchy created successfully"));
	DebugText->SetColorAndOpacity(FLinearColor(0.0f, 1.0f, 1.0f, 1.0f)); // Cyan color
	MainVerticalBox->AddChild(DebugText);
	MAGIC_LOG(General, TEXT("Added debug text to widget"));
	
	// Add a large, obvious test element
	UTextBlock* LargeTest = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LargeTest"));
	LargeTest->SetText(FText::FromString("LARGE TEST - THIS SHOULD BE VISIBLE"));
	LargeTest->SetColorAndOpacity(FLinearColor::Red);
	LargeTest->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 24));
	MainVerticalBox->AddChild(LargeTest);
	MAGIC_LOG(General, TEXT("Added large test text to widget"));
	
	// === HEADER SECTION ===
	CreateHeaderSection(MainVerticalBox);
	
	// === HEALTH OVERVIEW SECTION ===
	CreateHealthOverviewSection(MainVerticalBox);
	
	// === CONTROL BUTTONS SECTION ===
	CreateControlButtonsSection(MainVerticalBox);
	
	// === PROGRESS SECTION ===
	CreateProgressSection(MainVerticalBox);
	
	// === ASSET TABLE SECTION ===
	CreateAssetTableSection(MainVerticalBox);
	
	MAGIC_LOG(General, TEXT("UMG Widget Hierarchy created successfully"));
	
	// Force layout update to ensure proper sizing
	RootSizeBox->ForceLayoutPrepass();
	MAGIC_LOG(General, TEXT("Forced layout prepass on root SizeBox"));
	
	// Log the actual size after setting overrides
	FVector2D WidgetSize = RootSizeBox->GetDesiredSize();
	MAGIC_LOG(General, FString::Printf(TEXT("RootSizeBox desired size after override: %.2f x %.2f"), WidgetSize.X, WidgetSize.Y));
	
	// Force a layout update to ensure the widget is properly rendered
	if (MainVerticalBox)
	{
		MainVerticalBox->ForceLayoutPrepass();
		UE_LOG(LogTemp, Log, TEXT("MagicOptimizer: Forced layout prepass on main widget"));
	}
}

void UMagicOptimizerMainWidget::CreateHeaderSection(UVerticalBox* Parent)
{
	// Header container
	UBorder* HeaderBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HeaderBorder"));
	HeaderBorder->SetPadding(FMargin(10, 10, 10, 10));
	HeaderBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.2f, 1.0f));
	
	UHorizontalBox* HeaderHorizontal = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HeaderHorizontal"));
	HeaderBorder->SetContent(HeaderHorizontal);
	
	// Title text
	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetText(FText::FromString("Magic Optimizer - Asset Audit System"));
	TitleText->SetColorAndOpacity(FLinearColor::White);
	TitleText->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 18));
	HeaderHorizontal->AddChild(TitleText);
	
	// Spacer using proper USpacer widget
	USpacer* HeaderSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("HeaderSpacer"));
	HeaderHorizontal->AddChild(HeaderSpacer);
	
	// Status text
	StatusTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
	StatusTextBlock->SetText(FText::FromString("Ready"));
	StatusTextBlock->SetColorAndOpacity(FLinearColor::Green);
	StatusTextBlock->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 12));
	HeaderHorizontal->AddChild(StatusTextBlock);
	
	Parent->AddChild(HeaderBorder);
}

void UMagicOptimizerMainWidget::CreateHealthOverviewSection(UVerticalBox* Parent)
{
	// Health overview container
	UBorder* HealthBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HealthBorder"));
	HealthBorder->SetPadding(FMargin(10, 10, 10, 10));
	HealthBorder->SetBrushColor(FLinearColor(0.05f, 0.15f, 0.05f, 1.0f));
	
	UVerticalBox* HealthVertical = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HealthVertical"));
	HealthBorder->SetContent(HealthVertical);
	
	// Health title
	UTextBlock* HealthTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthTitle"));
	HealthTitle->SetText(FText::FromString("Project Health Overview"));
	HealthTitle->SetColorAndOpacity(FLinearColor::White);
	HealthTitle->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 14));
	HealthVertical->AddChild(HealthTitle);
	
	// Health metrics horizontal container
	UHorizontalBox* HealthMetrics = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HealthMetrics"));
	HealthVertical->AddChild(HealthMetrics);
	
	// Health Score
	UVerticalBox* HealthScoreBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HealthScoreBox"));
	HealthMetrics->AddChild(HealthScoreBox);
	
	UTextBlock* HealthScoreLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthScoreLabel"));
	HealthScoreLabel->SetText(FText::FromString("Health Score"));
	HealthScoreLabel->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
	HealthScoreBox->AddChild(HealthScoreLabel);
	
	HealthScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthScoreText"));
	HealthScoreText->SetText(FText::FromString("85%"));
	HealthScoreText->SetColorAndOpacity(FLinearColor::Green);
	HealthScoreText->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 16));
	HealthScoreBox->AddChild(HealthScoreText);
	
	// Critical Issues
	UVerticalBox* CriticalIssuesBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CriticalIssuesBox"));
	HealthMetrics->AddChild(CriticalIssuesBox);
	
	UTextBlock* CriticalIssuesLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CriticalIssuesLabel"));
	CriticalIssuesLabel->SetText(FText::FromString("Critical Issues"));
	CriticalIssuesLabel->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
	CriticalIssuesBox->AddChild(CriticalIssuesLabel);
	
	CriticalIssuesText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CriticalIssuesText"));
	CriticalIssuesText->SetText(FText::FromString("3"));
	CriticalIssuesText->SetColorAndOpacity(FLinearColor::Red);
	CriticalIssuesText->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 16));
	CriticalIssuesBox->AddChild(CriticalIssuesText);
	
	// Assets Scanned
	UVerticalBox* AssetsScannedBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("AssetsScannedBox"));
	HealthMetrics->AddChild(AssetsScannedBox);
	
	UTextBlock* AssetsScannedLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AssetsScannedLabel"));
	AssetsScannedLabel->SetText(FText::FromString("Assets Scanned"));
	AssetsScannedLabel->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
	AssetsScannedBox->AddChild(AssetsScannedLabel);
	
	AssetsScannedText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AssetsScannedText"));
	AssetsScannedText->SetText(FText::FromString("0"));
	AssetsScannedText->SetColorAndOpacity(FLinearColor::Yellow);
	AssetsScannedText->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 16));
	AssetsScannedBox->AddChild(AssetsScannedText);
	
	Parent->AddChild(HealthBorder);
}

void UMagicOptimizerMainWidget::CreateControlButtonsSection(UVerticalBox* Parent)
{
	// Control buttons container
	UBorder* ControlsBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ControlsBorder"));
	ControlsBorder->SetPadding(FMargin(10, 10, 10, 10));
	ControlsBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
	
	UHorizontalBox* ControlsHorizontal = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ControlsHorizontal"));
	ControlsBorder->SetContent(ControlsHorizontal);
	
	// Start Audit Button
	StartAuditButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StartAuditButton"));
	UTextBlock* StartAuditText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StartAuditText"));
	StartAuditText->SetText(FText::FromString("Start Asset Audit"));
	StartAuditButton->SetContent(StartAuditText);
	StartAuditButton->SetBackgroundColor(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f));
	ControlsHorizontal->AddChild(StartAuditButton);
	
	// Stop Audit Button
	StopAuditButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StopAuditButton"));
	UTextBlock* StopAuditText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StopAuditText"));
	StopAuditText->SetText(FText::FromString("Stop Audit"));
	StopAuditButton->SetContent(StopAuditText);
	StopAuditButton->SetBackgroundColor(FLinearColor(0.6f, 0.2f, 0.2f, 1.0f));
	StopAuditButton->SetVisibility(ESlateVisibility::Collapsed);
	ControlsHorizontal->AddChild(StopAuditButton);
	
	// Spacer
	USpacer* ButtonSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("ButtonSpacer"));
	ControlsHorizontal->AddChild(ButtonSpacer);
	
	// Export Report Button
	UButton* ExportButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ExportButton"));
	UTextBlock* ExportText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ExportText"));
	ExportText->SetText(FText::FromString("Export Report"));
	ExportButton->SetContent(ExportText);
	ExportButton->SetBackgroundColor(FLinearColor(0.2f, 0.2f, 0.6f, 1.0f));
	ControlsHorizontal->AddChild(ExportButton);
	
	Parent->AddChild(ControlsBorder);
}

void UMagicOptimizerMainWidget::CreateProgressSection(UVerticalBox* Parent)
{
	// Progress container
	UBorder* ProgressBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ProgressBorder"));
	ProgressBorder->SetPadding(FMargin(10, 10, 10, 10));
	ProgressBorder->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.15f, 1.0f));
	
	UVerticalBox* ProgressVertical = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ProgressVertical"));
	ProgressBorder->SetContent(ProgressVertical);
	
	// Progress title
	UTextBlock* ProgressTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ProgressTitle"));
	ProgressTitle->SetText(FText::FromString("Audit Progress"));
	ProgressTitle->SetColorAndOpacity(FLinearColor::White);
	ProgressTitle->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 14));
	ProgressVertical->AddChild(ProgressTitle);
	
	// Progress bar
	ProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("ProgressBar"));
	ProgressBar->SetPercent(0.0f);
	ProgressBar->SetFillColorAndOpacity(FLinearColor(0.2f, 0.8f, 0.2f, 1.0f));
	ProgressVertical->AddChild(ProgressBar);
	
	// Progress text
	ProgressText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ProgressText"));
	ProgressText->SetText(FText::FromString("Ready to start audit"));
	ProgressText->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
	ProgressVertical->AddChild(ProgressText);
	
	Parent->AddChild(ProgressBorder);
}

void UMagicOptimizerMainWidget::CreateAssetTableSection(UVerticalBox* Parent)
{
	// Asset table container
	UBorder* TableBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TableBorder"));
	TableBorder->SetPadding(FMargin(10, 10, 10, 10));
	TableBorder->SetBrushColor(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f));
	
	UVerticalBox* TableVertical = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TableVertical"));
	TableBorder->SetContent(TableVertical);
	
	// Table title
	UTextBlock* TableTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TableTitle"));
	TableTitle->SetText(FText::FromString("Asset Analysis Results"));
	TableTitle->SetColorAndOpacity(FLinearColor::White);
	TableTitle->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 14));
	TableVertical->AddChild(TableTitle);
	
	// Scrollable asset list
	UScrollBox* AssetScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("AssetScrollBox"));
	TableVertical->AddChild(AssetScrollBox);
	
	// Add some sample asset entries
	for (int32 i = 0; i < 5; i++)
	{
		CreateAssetEntry(AssetScrollBox, FString::Printf(TEXT("Sample Asset %d"), i + 1));
	}
	
	Parent->AddChild(TableBorder);
}

void UMagicOptimizerMainWidget::CreateAssetEntry(UScrollBox* Parent, const FString& AssetName)
{
	UBorder* AssetBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("AssetBorder"));
	AssetBorder->SetPadding(FMargin(5, 5, 5, 5));
	AssetBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
	
	UHorizontalBox* AssetHorizontal = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("AssetHorizontal"));
	AssetBorder->SetContent(AssetHorizontal);
	
	// Asset name
	UTextBlock* AssetNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AssetNameText"));
	AssetNameText->SetText(FText::FromString(AssetName));
	AssetNameText->SetColorAndOpacity(FLinearColor::White);
	AssetHorizontal->AddChild(AssetNameText);
	
	// Spacer
	USpacer* AssetSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("AssetSpacer"));
	AssetHorizontal->AddChild(AssetSpacer);
	
	// Asset status
	UTextBlock* AssetStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AssetStatusText"));
	AssetStatusText->SetText(FText::FromString("Pending"));
	AssetStatusText->SetColorAndOpacity(FLinearColor::Yellow);
	AssetHorizontal->AddChild(AssetStatusText);
	
	Parent->AddChild(AssetBorder);
}

void UMagicOptimizerMainWidget::InitializeWidget()
{
	if (bIsInitialized)
	{
		return;
	}

	MAGIC_LOG(General, TEXT("Initializing Magic Optimizer Main Widget"));

	// NOTE: UIDataManager is now injected from outside, not created here
	// This prevents lifecycle ordering issues

	// Initial UI update (will use placeholder data if manager is null)
	UpdateUI();

	bIsInitialized = true;
	MAGIC_LOG(General, TEXT("Magic Optimizer Main Widget initialized"));
}

void UMagicOptimizerMainWidget::UpdateUI()
{
	if (!UIDataManager)
	{
		return;
	}

	UpdateHealthScore();
	UpdateProgressBar();
	UpdateStatusText();
	UpdateAssetList();
	UpdateMemoryInfo();
	UpdateButtonStates();
}

void UMagicOptimizerMainWidget::OnStartAuditClicked()
{
	MAGIC_LOG(UI, TEXT("Start Audit button clicked"));

	if (!UIDataManager)
	{
		return;
	}

	// Start asset audit
	FAuditConfig Config;
	Config.TargetPlatform = TEXT("WindowsEditor");
	Config.QualityLevel = EQualityLevel::High;
	Config.bDryRunMode = true;
	Config.bGenerateReports = true;
	Config.IncludeAssetTypes.Add(EAssetType::Texture);
	Config.IncludeAssetTypes.Add(EAssetType::Mesh);
	Config.IncludeAssetTypes.Add(EAssetType::Material);
	Config.IncludeAssetTypes.Add(EAssetType::Blueprint);

	if (UMagicOptimizerAuditManager::StartAssetAudit(Config))
	{
		MAGIC_LOG(AssetProcessing, TEXT("Asset audit started successfully"));
		OnAuditStarted();
	}
	else
	{
		MAGIC_LOG_ERROR(TEXT("Failed to start asset audit"), TEXT("StartAuditButton"));
	}
}

void UMagicOptimizerMainWidget::OnStopAuditClicked()
{
	MAGIC_LOG(UI, TEXT("Stop Audit button clicked"));

	UMagicOptimizerAuditManager::StopAssetAudit();
	UpdateButtonStates();
}

void UMagicOptimizerMainWidget::OnGenerateReportClicked()
{
	MAGIC_LOG(UI, TEXT("Generate Report button clicked"));

	// Generate diagnostic report
	FMagicOptimizerDiagnostics::GenerateDiagnosticReport();
}

void UMagicOptimizerMainWidget::OnProgressUpdated(const FProgressUpdate& ProgressUpdate)
{
	UpdateUI();
}

void UMagicOptimizerMainWidget::OnAssetProcessed(const FAssetAuditData& AssetData)
{
	UpdateUI();
}

void UMagicOptimizerMainWidget::OnAuditCompleted()
{
	MAGIC_LOG(General, TEXT("Audit completed - updating UI"));
	UpdateUI();
}

void UMagicOptimizerMainWidget::OnAuditStarted()
{
	MAGIC_LOG(General, TEXT("Audit started - updating UI"));
	UpdateUI();
}

void UMagicOptimizerMainWidget::SetUIDataManager(UMagicOptimizerUIDataManager* InMgr)
{
	UIDataManager = InMgr;
	MAGIC_LOG(General, TEXT("UIDataManager injected into widget"));
}

UMagicOptimizerUIDataManager* UMagicOptimizerMainWidget::GetUIDataManagerChecked() const
{
	check(UIDataManager);
	return UIDataManager;
}

void UMagicOptimizerMainWidget::BindEvents(bool bBind)
{
	if (!UIDataManager)
	{
		MAGIC_LOG(Warning, TEXT("Cannot bind events - UIDataManager is null"));
		return;
	}

	MAGIC_LOG(General, FString::Printf(TEXT("Binding events: %s"), bBind ? TEXT("true") : TEXT("false")));

	// Bind/unbind data manager events using AddUniqueDynamic to prevent duplicates
	if (bBind)
	{
		UIDataManager->OnProgressUpdated.AddUniqueDynamic(this, &UMagicOptimizerMainWidget::OnProgressUpdated);
		UIDataManager->OnAssetProcessed.AddUniqueDynamic(this, &UMagicOptimizerMainWidget::OnAssetProcessed);
		UIDataManager->OnAuditCompleted.AddUniqueDynamic(this, &UMagicOptimizerMainWidget::OnAuditCompleted);
		UIDataManager->OnAuditStarted.AddUniqueDynamic(this, &UMagicOptimizerMainWidget::OnAuditStarted);
	}
	else
	{
		UIDataManager->OnProgressUpdated.RemoveDynamic(this, &UMagicOptimizerMainWidget::OnProgressUpdated);
		UIDataManager->OnAssetProcessed.RemoveDynamic(this, &UMagicOptimizerMainWidget::OnAssetProcessed);
		UIDataManager->OnAuditCompleted.RemoveDynamic(this, &UMagicOptimizerMainWidget::OnAuditCompleted);
		UIDataManager->OnAuditStarted.RemoveDynamic(this, &UMagicOptimizerMainWidget::OnAuditStarted);
	}

	// Bind/unbind button events using AddUniqueDynamic
	if (StartAuditButton)
	{
		if (bBind)
		{
			StartAuditButton->OnClicked.AddUniqueDynamic(this, &UMagicOptimizerMainWidget::OnStartAuditClicked);
		}
		else
		{
			StartAuditButton->OnClicked.RemoveDynamic(this, &UMagicOptimizerMainWidget::OnStartAuditClicked);
		}
	}

	if (StopAuditButton)
	{
		if (bBind)
		{
			StopAuditButton->OnClicked.AddUniqueDynamic(this, &UMagicOptimizerMainWidget::OnStopAuditClicked);
		}
		else
		{
			StopAuditButton->OnClicked.RemoveDynamic(this, &UMagicOptimizerMainWidget::OnStopAuditClicked);
		}
	}

	MAGIC_LOG(General, TEXT("Bound events with valid UIDataManager"));
	
	// Debug: Force widget to be visible and log widget state
	if (WidgetTree && WidgetTree->RootWidget)
	{
		WidgetTree->RootWidget->SetVisibility(ESlateVisibility::Visible);
		MAGIC_LOG(General, TEXT("Widget tree root set to visible"));
		
		// Log widget dimensions
		FVector2D DesiredSize = WidgetTree->RootWidget->GetDesiredSize();
		MAGIC_LOG(General, FString::Printf(TEXT("Widget desired size: %f x %f"), DesiredSize.X, DesiredSize.Y));
		
		// Force another layout update to ensure sizing is applied
		WidgetTree->RootWidget->ForceLayoutPrepass();
		FVector2D UpdatedSize = WidgetTree->RootWidget->GetDesiredSize();
		MAGIC_LOG(General, FString::Printf(TEXT("Widget updated size: %f x %f"), UpdatedSize.X, UpdatedSize.Y));
	}
	else
	{
		MAGIC_LOG(Error, TEXT("Widget tree or root widget is null after binding"));
	}
}

void UMagicOptimizerMainWidget::UpdateHealthScore()
{
	if (HealthScoreText && UIDataManager)
	{
		int32 HealthScore = UIDataManager->GetHealthScore();
		FString HealthText = FString::Printf(TEXT("%d"), HealthScore);
		HealthScoreText->SetText(FText::FromString(HealthText));

		// Set color based on health score
		FLinearColor HealthColor;
		if (HealthScore >= 80)
		{
			HealthColor = FLinearColor::Green;
		}
		else if (HealthScore >= 60)
		{
			HealthColor = FLinearColor::Yellow;
		}
		else
		{
			HealthColor = FLinearColor::Red;
		}
		HealthScoreText->SetColorAndOpacity(HealthColor);
	}
}

void UMagicOptimizerMainWidget::UpdateProgressBar()
{
	if (ProgressBar && UIDataManager)
	{
		int32 ProgressPercent = UIDataManager->GetProgressPercentage();
		ProgressBar->SetPercent(ProgressPercent / 100.0f);
	}

	if (ProgressText && UIDataManager)
	{
		int32 ProgressPercent = UIDataManager->GetProgressPercentage();
		FString ProgressString = FString::Printf(TEXT("%d%%"), ProgressPercent);
		ProgressText->SetText(FText::FromString(ProgressString));
	}
}

void UMagicOptimizerMainWidget::UpdateStatusText()
{
	if (StatusTextBlock && UIDataManager)
	{
		FString StatusMessage = UIDataManager->GetCurrentStatusMessage();
		StatusTextBlock->SetText(FText::FromString(StatusMessage));
	}
}

void UMagicOptimizerMainWidget::UpdateAssetList()
{
	if (CriticalIssuesText && UIDataManager)
	{
		int32 CriticalIssues = UIDataManager->GetCriticalIssuesCount();
		FString IssuesText = FString::Printf(TEXT("%d"), CriticalIssues);
		CriticalIssuesText->SetText(FText::FromString(IssuesText));
	}

	if (AssetsScannedText && UIDataManager)
	{
		int32 AssetsScanned = UIDataManager->GetTotalAssetsScanned();
		FString AssetsText = FString::Printf(TEXT("%d"), AssetsScanned);
		AssetsScannedText->SetText(FText::FromString(AssetsText));
	}
}

void UMagicOptimizerMainWidget::UpdateMemoryInfo()
{
	// Memory info widgets not implemented in current version
	// Will be added in future iterations
}

void UMagicOptimizerMainWidget::UpdateButtonStates()
{
	if (!UIDataManager)
	{
		return;
	}

	bool bIsAuditRunning = UIDataManager->IsAuditRunning();

	if (StartAuditButton)
	{
		StartAuditButton->SetIsEnabled(!bIsAuditRunning);
	}

	if (StopAuditButton)
	{
		StopAuditButton->SetIsEnabled(bIsAuditRunning);
	}
}
