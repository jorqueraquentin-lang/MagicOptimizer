#include "UI/MagicOptimizerRecommendView.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerRecommendView"

void SMagicOptimizerRecommendView::Construct(const FArguments& InArgs)
{
    // Bind attributes
    TextureData = InArgs._TextureData;
    SelectedTextureIds = InArgs._SelectedTextureIds;

    // Bind events
    OnPreview = InArgs._OnPreview;
    OnAutoFix = InArgs._OnAutoFix;

    // Initialize data
    UpdateRecommendations();

    ChildSlot
    [
        SNew(SScrollBox)
        +SScrollBox::Slot()
        [
            SNew(SVerticalBox)
            
            // Playbook section (conditional)
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 24)
            [
                MakePlaybookSection()
            ]
            
            // Summary section
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 24)
            [
                MakeSummarySection()
            ]
            
            // Proposed changes section
            +SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                MakeProposedChangesSection()
            ]
        ]
    ];
}

void SMagicOptimizerRecommendView::RefreshData(const TArray<FUITextureRow>& NewTextureData)
{
    UpdateRecommendations();
}

TSharedRef<SWidget> SMagicOptimizerRecommendView::MakePlaybookSection()
{
    // Only show playbook if there are recommendations
    if (PlaybookRecommendations.Num() == 0)
    {
        return SNullWidget::NullWidget;
    }

    TSharedRef<SVerticalBox> PlaybookBox = SNew(SVerticalBox);

    // Add each playbook recommendation
    for (const FPlaybookRecommendation& Recommendation : PlaybookRecommendations)
    {
        PlaybookBox->AddSlot()
        .AutoHeight()
        .Padding(0, 0, 0, 8)
        [
            MakePlaybookCard(Recommendation)
        ];
    }

    return PlaybookBox;
}

TSharedRef<SWidget> SMagicOptimizerRecommendView::MakePlaybookCard(const FPlaybookRecommendation& Recommendation)
{
    FLinearColor BorderColor = FLinearColor(0.8f, 0.6f, 0.2f, 0.4f); // Warning color like React mock

    return SNew(SBorder)
    .Padding(FMargin(20))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
    .BorderBackgroundColor(BorderColor)
    [
        SNew(SVerticalBox)
        
        // Title
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 12)
        [
            SNew(STextBlock)
            .Text(FText::FromString(FString::Printf(TEXT("Playbook: %s"), *Recommendation.Title)))
            .Font(FAppStyle::GetFontStyle("NormalFontBold"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        // Description and actions
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            
            // Description
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .VAlign(VAlign_Top)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Recommendation.Description))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                .AutoWrapText(true)
            ]
            
            // Action buttons
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Top)
            .Padding(16, 0, 0, 0)
            [
                SNew(SHorizontalBox)
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 8, 0)
                [
                    SNew(SButton)
                    .Text(LOCTEXT("OpenBudgets", "Open Budgets"))
                    .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                    .ContentPadding(FMargin(12, 6, 12, 6))
                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 8, 0)
                [
                    SNew(SButton)
                    .Text(LOCTEXT("Filter4k", "Filter 4k"))
                    .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                    .ContentPadding(FMargin(12, 6, 12, 6))
                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(LOCTEXT("PreviewPlan", "Preview Plan"))
                    .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                    .ContentPadding(FMargin(12, 6, 12, 6))
                    .OnClicked_Lambda([this]() { OnPreviewPlan(); return FReply::Handled(); })
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerRecommendView::MakeSummarySection()
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
            .Text(LOCTEXT("SummaryTitle", "Summary"))
            .Font(FAppStyle::GetFontStyle("NormalFontBold"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        // Summary cards
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            
            // Estimated disk saved
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .Padding(0, 0, 24, 0)
            [
                MakeSummaryCard(
                    TEXT("Estimated disk saved"),
                    FString::Printf(TEXT("%.0f MB"), OptimizationSummary.TotalDiskSaving),
                    TEXT("")
                )
            ]
            
            // Estimated memory saved
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .Padding(0, 0, 24, 0)
            [
                MakeSummaryCard(
                    TEXT("Estimated memory saved"),
                    FString::Printf(TEXT("%.0f MB"), OptimizationSummary.TotalMemorySaving),
                    TEXT("")
                )
            ]
            
            // Risk notes
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                MakeSummaryCard(
                    TEXT("Risk notes"),
                    TEXT(""),
                    OptimizationSummary.RiskNotes
                )
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerRecommendView::MakeSummaryCard(const FString& Title, const FString& Value, const FString& Subtitle)
{
    return SNew(SVerticalBox)
    
    // Title
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 8)
    [
        SNew(STextBlock)
        .Text(FText::FromString(Title))
        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 12)
        .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
    ]
    
    // Value (if provided)
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, !Subtitle.IsEmpty() ? 4 : 0)
    [
        !Value.IsEmpty() ?
        StaticCastSharedRef<SWidget>(
            SNew(STextBlock)
            .Text(FText::FromString(Value))
            .Font(FAppStyle::GetFontStyle("BoldFont"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 24)
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ) :
        SNullWidget::NullWidget
    ]
    
    // Subtitle (if provided)
    +SVerticalBox::Slot()
    .AutoHeight()
    [
        !Subtitle.IsEmpty() ?
        StaticCastSharedRef<SWidget>(
            SNew(STextBlock)
            .Text(FText::FromString(Subtitle))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("Regular").FontObject, 11))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            .AutoWrapText(true)
        ) :
        SNullWidget::NullWidget
    ];
}

TSharedRef<SWidget> SMagicOptimizerRecommendView::MakeProposedChangesSection()
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
            .Text(LOCTEXT("ProposedChangesTitle", "Proposed changes"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        // Changes list
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 16)
        [
            [this]() -> TSharedRef<SWidget>
            {
                TSharedRef<SVerticalBox> ChangesBox = SNew(SVerticalBox);
                
                for (int32 i = 0; i < ProposedChanges.Num(); ++i)
                {
                    ChangesBox->AddSlot()
                    .AutoHeight()
                    .Padding(0, 0, 0, i < ProposedChanges.Num() - 1 ? 12 : 0)
                    [
                        MakeProposedChangeRow(ProposedChanges[i])
                    ];
                }
                
                return ChangesBox;
            }()
        ]
        
        // Action buttons
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            MakeActionButtons()
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerRecommendView::MakeProposedChangeRow(const FProposedChange& Change)
{
    return SNew(SBorder)
    .Padding(FMargin(16, 12, 16, 12))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
    [
        SNew(SHorizontalBox)
        
        // Change description
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Change.Label))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
            .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
        ]
        
        // Item count badge
        +SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(16, 0, 0, 0)
        [
            SNew(SBorder)
            .Padding(FMargin(8, 4, 8, 4))
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .ColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
            .BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
            [
                SNew(STextBlock)
                .Text(FText::AsNumber(Change.AffectedTextures.Num()))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .ColorAndOpacity(FLinearColor::White)
            ]
        ]
        
        // Show details button
        +SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(16, 0, 0, 0)
        [
            SNew(SButton)
            .Text(LOCTEXT("ShowDetails", "Show details"))
            .ButtonStyle(FAppStyle::Get(), "SimpleButton")
            .ContentPadding(FMargin(12, 6, 12, 6))
            .OnClicked_Lambda([this, Change]()
            {
                OnShowDetails(Change.Id);
                return FReply::Handled();
            })
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerRecommendView::MakeActionButtons()
{
    return SNew(SHorizontalBox)
    
    // Preview Plan button
    +SHorizontalBox::Slot()
    .AutoWidth()
    .Padding(0, 0, 12, 0)
    [
        SNew(SButton)
        .Text(LOCTEXT("PreviewPlanBtn", "👁 Preview Plan"))
        .ButtonStyle(FAppStyle::Get(), "PrimaryButton")
        .ContentPadding(FMargin(16, 12, 16, 12))
        .ButtonColorAndOpacity(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f))
        .OnClicked_Lambda([this]() { OnPreviewPlan(); return FReply::Handled(); })
    ]
    
    // Apply button  
    +SHorizontalBox::Slot()
    .AutoWidth()
    .Padding(0, 0, 12, 0)
    [
        SNew(SButton)
        .Text(LOCTEXT("Apply", "✨ Apply"))
        .ButtonStyle(FAppStyle::Get(), "Button")
        .ContentPadding(FMargin(16, 12, 16, 12))
        .ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
        .OnClicked_Lambda([this]() { OnApplyChanges(); return FReply::Handled(); })
    ]
    
    // Copy commandlet button
    +SHorizontalBox::Slot()
    .AutoWidth()
    [
        SNew(SButton)
        .Text(LOCTEXT("CopyCommandlet", "📁 Copy commandlet"))
        .ButtonStyle(FAppStyle::Get(), "Button")
        .ContentPadding(FMargin(16, 12, 16, 12))
        .ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
        .OnClicked_Lambda([this]() { OnCopyCommandlet(); return FReply::Handled(); })
    ];
}

void SMagicOptimizerRecommendView::UpdateRecommendations()
{
    PlaybookRecommendations = AnalyzePlaybook();
    ProposedChanges = BuildProposedChanges();
    OptimizationSummary = CalculateOptimizationSummary();
}

TArray<FPlaybookRecommendation> SMagicOptimizerRecommendView::AnalyzePlaybook() const
{
    TArray<FPlaybookRecommendation> Recommendations;
    
    // Analyze texture data for patterns
    const TArray<FUITextureRow>& Textures = TextureData.Get();
    
    // Count 4k textures
    int32 BigTextureCount = 0;
    for (const FUITextureRow& Texture : Textures)
    {
        if (Texture.Size == TEXT("4k"))
        {
            BigTextureCount++;
        }
    }
    
    // Add streaming pool risk if too many 4k textures
    if (BigTextureCount > 20)
    {
        TArray<FString> Actions = {TEXT("Open Budgets"), TEXT("Filter 4k"), TEXT("Preview Plan")};
        Recommendations.Add(FPlaybookRecommendation(
            TEXT("Streaming Pool Risk"),
            TEXT("Large number of 4k textures detected. Consider lowering MaxTextureSize or increasing LODBias for World/Props groups."),
            Actions,
            TEXT("warning")
        ));
    }
    
    return Recommendations;
}

TArray<FProposedChange> SMagicOptimizerRecommendView::BuildProposedChanges() const
{
    TArray<FProposedChange> Changes;
    
    const TArray<FUITextureRow> SelectedTextures = GetSelectedTextures();
    const TArray<FUITextureRow>& SourceTextures = SelectedTextures.Num() > 0 ? SelectedTextures : TextureData.Get();
    
    // Find masks with sRGB on
    TArray<FUITextureRow> MasksWithSRGB;
    for (const FUITextureRow& Texture : SourceTextures)
    {
        if (Texture.Compression == TEXT("Masks") && Texture.bSRGB)
        {
            MasksWithSRGB.Add(Texture);
        }
    }
    
    if (MasksWithSRGB.Num() > 0)
    {
        Changes.Add(FProposedChange(
            TEXT("srgb_off"),
            FString::Printf(TEXT("Set sRGB off for %d mask textures"), MasksWithSRGB.Num()),
            MasksWithSRGB,
            MasksWithSRGB.Num() * 2.0f,
            MasksWithSRGB.Num() * 0.5f
        ));
    }
    
    // Find normals not BC5
    TArray<FUITextureRow> NormalsNotBC5;
    for (const FUITextureRow& Texture : SourceTextures)
    {
        if (Texture.Compression == TEXT("Normal"))
        {
            NormalsNotBC5.Add(Texture);
        }
    }
    
    if (NormalsNotBC5.Num() > 0)
    {
        Changes.Add(FProposedChange(
            TEXT("normals_bc5"),
            FString::Printf(TEXT("Convert %d normals to BC5"), NormalsNotBC5.Num()),
            NormalsNotBC5,
            NormalsNotBC5.Num() * 1.5f,
            NormalsNotBC5.Num() * 1.2f
        ));
    }
    
    // Find VT assets that shouldn't be VT
    TArray<FUITextureRow> NonVTAssetsWithVT;
    for (const FUITextureRow& Texture : SourceTextures)
    {
        if (Texture.bVT && Texture.Group != TEXT("World"))
        {
            NonVTAssetsWithVT.Add(Texture);
        }
    }
    
    if (NonVTAssetsWithVT.Num() > 0)
    {
        Changes.Add(FProposedChange(
            TEXT("disable_vt"),
            FString::Printf(TEXT("Disable VT on %d non VT assets"), NonVTAssetsWithVT.Num()),
            NonVTAssetsWithVT,
            NonVTAssetsWithVT.Num() * 0.8f,
            NonVTAssetsWithVT.Num() * 0.3f
        ));
    }
    
    return Changes;
}

FOptimizationSummary SMagicOptimizerRecommendView::CalculateOptimizationSummary() const
{
    FOptimizationSummary Summary;
    
    for (const FProposedChange& Change : ProposedChanges)
    {
        Summary.TotalDiskSaving += Change.EstimatedDiskSaving;
        Summary.TotalMemorySaving += Change.EstimatedMemorySaving;
        Summary.TotalChanges += Change.AffectedTextures.Num();
    }
    
    Summary.RiskNotes = TEXT("Check UI assets for crispness before commit");
    return Summary;
}

TArray<FUITextureRow> SMagicOptimizerRecommendView::GetSelectedTextures() const
{
    TArray<FUITextureRow> Selected;
    const TArray<FString>& SelectedIds = SelectedTextureIds.Get();
    const TArray<FUITextureRow>& AllTextures = TextureData.Get();
    
    for (const FString& Id : SelectedIds)
    {
        for (const FUITextureRow& Texture : AllTextures)
        {
            if (Texture.Id == Id)
            {
                Selected.Add(Texture);
                break;
            }
        }
    }
    
    return Selected;
}

void SMagicOptimizerRecommendView::OnPreviewPlan()
{
    OnPreview.ExecuteIfBound();
}

void SMagicOptimizerRecommendView::OnApplyChanges()
{
    OnAutoFix.ExecuteIfBound();
}

void SMagicOptimizerRecommendView::OnCopyCommandlet()
{
    FString Commandlet = TEXT("UE4Editor-Cmd.exe Project.uproject -run=MagicOptimizer -mode=apply -preset=\"Console Balanced\" -scope=\"/Game\" -report");
    FPlatformMisc::ClipboardCopy(*Commandlet);
    UE_LOG(LogTemp, Log, TEXT("Commandlet copied to clipboard: %s"), *Commandlet);
}

void SMagicOptimizerRecommendView::OnShowDetails(const FString& ChangeId)
{
    UE_LOG(LogTemp, Log, TEXT("Show details for change: %s"), *ChangeId);
}

#undef LOCTEXT_NAMESPACE
