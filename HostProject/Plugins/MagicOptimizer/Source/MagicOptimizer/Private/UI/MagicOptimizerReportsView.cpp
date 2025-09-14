#include "UI/MagicOptimizerReportsView.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSeparator.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerReportsView"

void SMagicOptimizerReportsView::Construct(const FArguments& InArgs)
{
    InitializeMockData();

    ChildSlot
    [
        SNew(SHorizontalBox)
        
        // Run History section (left)
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(320)
            [
                MakeRunHistorySection()
            ]
        ]
        
        // Artifacts section (right)
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        [
            MakeArtifactsSection()
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerReportsView::MakeRunHistorySection()
{
    return SNew(SBorder)
    .Padding(FMargin(20))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    [
        SNew(SVerticalBox)
        
        // Title
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 16)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("RunHistoryTitle", "Run History"))
            .Font(FAppStyle::GetFontStyle("NormalFontBold"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        // Run list
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SNew(SListView<TSharedPtr<FReportRun>>)
            .ListItemsSource(&RunHistory)
            .OnGenerateRow(this, &SMagicOptimizerReportsView::OnGenerateRunRow)
            .OnSelectionChanged(this, &SMagicOptimizerReportsView::OnRunSelectionChanged)
            .SelectionMode(ESelectionMode::Single)
            .ScrollbarVisibility(EVisibility::Collapsed)
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerReportsView::MakeArtifactsSection()
{
    return SNew(SBorder)
    .Padding(FMargin(20))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    [
        SNew(SVerticalBox)
        
        // Title
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 16)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("ArtifactsTitle", "Artifacts"))
            .Font(FAppStyle::GetFontStyle("NormalFontBold"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        // Artifact buttons
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 16)
        [
            SNew(SVerticalBox)
            
            // CSV download
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 8)
            [
                MakeArtifactButton(TEXT("CSV"), TEXT("Download texture data as CSV for analysis"))
            ]
            
            // JSON download
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 8)
            [
                MakeArtifactButton(TEXT("JSON"), TEXT("Download structured JSON data for automation"))
            ]
            
            // HTML report
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 8)
            [
                MakeArtifactButton(TEXT("HTML"), TEXT("Download formatted HTML report for sharing"))
            ]
        ]
        
        // Separator
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 16, 0, 16)
        [
            SNew(SSeparator)
            .ColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
        ]
        
        // Share link button
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SButton)
            .Text(LOCTEXT("CopyShareLink", "🔗 Copy Share Link"))
            .ButtonStyle(FAppStyle::Get(), "Button")
            .ContentPadding(FMargin(16, 12, 16, 12))
            .OnClicked_Lambda([this]()
            {
                OnCopyShareLink();
                return FReply::Handled();
            })
        ]
    ];
}

TSharedRef<ITableRow> SMagicOptimizerReportsView::OnGenerateRunRow(TSharedPtr<FReportRun> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FReportRun>>, OwnerTable)
    [
        SNew(SBorder)
        .Padding(FMargin(12, 8, 12, 8))
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
        .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
        [
            SNew(SVerticalBox)
            
            // Date
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 4)
            [
                SNew(STextBlock)
                .Text(FText::FromString(InItem->Date))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
            ]
            
            // Details
            +SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(STextBlock)
                .Text(FText::FromString(FString::Printf(TEXT("%s • %s"), *InItem->Preset, *InItem->Notes)))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
                .AutoWrapText(true)
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerReportsView::MakeArtifactButton(const FString& Format, const FString& Description)
{
    return SNew(SVerticalBox)
    
    // Main button
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 4)
    [
        SNew(SButton)
        .Text(FText::FromString(FString::Printf(TEXT("📁 %s"), *Format)))
        .ButtonStyle(FAppStyle::Get(), "SimpleButton")
        .ContentPadding(FMargin(16, 12, 16, 12))
        .OnClicked_Lambda([this, Format]()
        {
            OnDownloadArtifact(Format);
            return FReply::Handled();
        })
        .ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
        .BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f))
    ]
    
    // Description
    +SVerticalBox::Slot()
    .AutoHeight()
    [
        SNew(STextBlock)
        .Text(FText::FromString(Description))
        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
        .ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f))
        .AutoWrapText(true)
    ];
}

void SMagicOptimizerReportsView::OnRunSelectionChanged(TSharedPtr<FReportRun> SelectedItem, ESelectInfo::Type SelectInfo)
{
    SelectedRun = SelectedItem;
}

void SMagicOptimizerReportsView::OnDownloadArtifact(const FString& Format)
{
    // TODO: Implement actual artifact download
    UE_LOG(LogTemp, Log, TEXT("Download artifact: %s"), *Format);
    
    if (!SelectedRun.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("No run selected for artifact download"));
        return;
    }
    
    // In real implementation, this would:
    // 1. Find the artifact file for the selected run
    // 2. Open file dialog for save location
    // 3. Copy/export the artifact file
    
    FString Message = FString::Printf(TEXT("Downloaded %s artifact for run %s"), *Format, *SelectedRun->Id);
    UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
}

void SMagicOptimizerReportsView::OnCopyShareLink()
{
    // TODO: Implement share link generation and clipboard copy
    if (!SelectedRun.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("No run selected for sharing"));
        return;
    }
    
    // Generate shareable link (mock)
    FString ShareLink = FString::Printf(TEXT("https://reports.magicoptimizer.com/run/%s"), *SelectedRun->Id);
    
    // Copy to clipboard
    FPlatformMisc::ClipboardCopy(*ShareLink);
    
    UE_LOG(LogTemp, Log, TEXT("Share link copied to clipboard: %s"), *ShareLink);
}

void SMagicOptimizerReportsView::InitializeMockData()
{
    // Initialize mock data matching the React mock
    TArray<FString> ArtifactTypes = {TEXT("CSV"), TEXT("JSON"), TEXT("HTML")};
    
    RunHistory.Add(MakeShareable(new FReportRun(
        TEXT("run_01"),
        TEXT("2025-09-10 14:12"),
        TEXT("Console Balanced"),
        TEXT("First scan, no writes"),
        ArtifactTypes
    )));
    
    RunHistory.Add(MakeShareable(new FReportRun(
        TEXT("run_02"),
        TEXT("2025-09-11 09:18"),
        TEXT("Console Balanced"),
        TEXT("Applied textures quick fixes"),
        ArtifactTypes
    )));
    
    RunHistory.Add(MakeShareable(new FReportRun(
        TEXT("run_03"),
        TEXT("2025-09-12 17:46"),
        TEXT("VR Crisp UI"),
        TEXT("UI pass"),
        ArtifactTypes
    )));
    
    // Select the first run by default
    if (RunHistory.Num() > 0)
    {
        SelectedRun = RunHistory[0];
    }
}

#undef LOCTEXT_NAMESPACE
