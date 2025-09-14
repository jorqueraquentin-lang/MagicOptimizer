#include "UI/MagicOptimizerAuditView.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSeparator.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerAuditView"

void SMagicOptimizerAuditView::Construct(const FArguments& InArgs)
{
    // Bind attributes
    TextureData = InArgs._TextureData;
    SelectedTextureIds = InArgs._SelectedTextureIds;
    SearchText = InArgs._SearchText;

    // Bind events
    OnSearchTextChangedDelegate = InArgs._OnSearchTextChanged;
    OnTextureSelectionChangedDelegate = InArgs._OnTextureSelectionChanged;

    // Initialize data
    UpdateFilteredTextures();
    HealthData = CalculateHealthData();

    ChildSlot
    [
        SNew(SVerticalBox)
        
        // Health cards
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 16)
        [
            MakeHealthCards()
        ]
        
        // Texture table
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            MakeTextureTable()
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerAuditView::MakeHealthCards()
{
    return SNew(SHorizontalBox)
    
    // Health Score card
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .Padding(0, 0, 8, 0)
    [
        MakeHealthScoreCard()
    ]
    
    // Top Offenders card
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .Padding(0, 0, 8, 0)
    [
        MakeTopOffendersCard()
    ]
    
    // Summary card
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    [
        MakeSummaryCard()
    ];
}

TSharedRef<SWidget> SMagicOptimizerAuditView::MakeHealthScoreCard()
{
    return SNew(SBorder)
    .Padding(FMargin(16))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    [
        SNew(SVerticalBox)
        
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 12)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("HealthTitle", "Health"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 12)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(STextBlock)
                .Text_Lambda([this]()
                {
                    return FText::AsNumber(HealthData.Score);
                })
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("BoldFont").FontObject, 48))
                .ColorAndOpacity(FLinearColor(0.2f, 0.8f, 0.2f, 1.0f)) // Green color for health score
                .ShadowOffset(FVector2D(1.0f, 1.0f))
                .ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.3f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(6, 0, 0, 0)
            .VAlign(VAlign_Bottom)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("HealthMax", "/100"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFont").FontObject, 20))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
        ]
        
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 0)
        [
            SNew(SBox)
            .HeightOverride(96)
            [
                MakeHealthTrendChart()
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerAuditView::MakeHealthTrendChart()
{
    return SNew(SBorder)
    .Padding(FMargin(0))
    .BorderImage(FAppStyle::GetBrush("NoBorder"))
    .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
    [
        SNew(SVerticalBox)
        
        // Chart area with grid lines
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SNew(SBorder)
            .Padding(FMargin(8, 8, 8, 8))
            .BorderImage(FAppStyle::GetBrush("NoBorder"))
            .ColorAndOpacity(FLinearColor(0.05f, 0.05f, 0.05f, 1.0f))
            [
                SNew(SVerticalBox)
                
                // Grid lines (simplified representation)
                +SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    SNew(SHorizontalBox)
                    
                    // Y-axis labels
                    +SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(0, 0, 8, 0)
                    [
                        SNew(SVerticalBox)
                        +SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("Y100", "100"))
                            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 8))
                            .ColorAndOpacity(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f))
                        ]
                        +SVerticalBox::Slot()
                        .FillHeight(1.0f)
                        [
                            SNew(SSeparator)
                            .Orientation(Orient_Horizontal)
                            .ColorAndOpacity(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f))
                        ]
                        +SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("Y50", "50"))
                            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 8))
                            .ColorAndOpacity(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f))
                        ]
                        +SVerticalBox::Slot()
                        .FillHeight(1.0f)
                        [
                            SNew(SSeparator)
                            .Orientation(Orient_Horizontal)
                            .ColorAndOpacity(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f))
                        ]
                        +SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("Y0", "0"))
                            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 8))
                            .ColorAndOpacity(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f))
                        ]
                    ]
                    
                    // Chart area
                    +SHorizontalBox::Slot()
                    .FillWidth(1.0f)
                    [
                        SNew(SBorder)
                        .Padding(FMargin(0))
                        .BorderImage(FAppStyle::GetBrush("NoBorder"))
                        .ColorAndOpacity(FLinearColor(0.02f, 0.02f, 0.02f, 1.0f))
                        [
                            SNew(SVerticalBox)
                            
                            // Chart line representation (simplified)
                            +SVerticalBox::Slot()
                            .FillHeight(1.0f)
                            [
                                SNew(SHorizontalBox)
                                
                                // Mock chart line using text
                                +SHorizontalBox::Slot()
                                .FillWidth(1.0f)
                                [
                                    SNew(STextBlock)
                                    .Text(LOCTEXT("ChartLine", "📈 Health Trend: 78 → 82 → 79 → 85 → 78"))
                                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
                                    .ColorAndOpacity(FLinearColor(0.2f, 0.8f, 0.2f, 1.0f))
                                    .Justification(ETextJustify::Center)
                                    // VAlign not needed for STextBlock in UE5.6
                                ]
                            ]
                        ]
                    ]
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerAuditView::MakeTopOffendersCard()
{
    return SNew(SBorder)
    .Padding(FMargin(16))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    [
        SNew(SVerticalBox)
        
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 12)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("TopOffendersTitle", "Top Offenders"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 8)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("Offender1", "Masks with sRGB on"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SBorder)
                .Padding(FMargin(8, 4, 8, 4))
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                .ColorAndOpacity(FLinearColor(0.8f, 0.2f, 0.2f, 1.0f))
                .BorderBackgroundColor(FLinearColor(0.8f, 0.2f, 0.2f, 1.0f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Count1", "45"))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
        ]
        
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 8)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("Offender2", "Normals not BC5"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SBorder)
                .Padding(FMargin(8, 4, 8, 4))
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
                .BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Count2", "82"))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
        ]
        
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("Offender3", "Non VT assets flagged VT"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SBorder)
                .Padding(FMargin(8, 4, 8, 4))
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                .ColorAndOpacity(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f))
                .BorderBackgroundColor(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Count3", "16"))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerAuditView::MakeSummaryCard()
{
    return SNew(SBorder)
    .Padding(FMargin(16))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    [
        SNew(SVerticalBox)
        
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 12)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("SummaryTitle", "Summary"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("NormalFontBold").FontObject, 14))
            .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
        ]
        
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 8)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0, 0, 8, 0)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("PresetLabel", "Preset"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SBorder)
                .Padding(FMargin(6, 2, 6, 2))
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                .ColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
                .BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("PresetValue", "Console Balanced"))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
        ]
        
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 8)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0, 0, 8, 0)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("TargetsLabel", "Targets"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0, 0, 4, 0)
            [
                SNew(SBorder)
                .Padding(FMargin(6, 2, 6, 2))
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                .ColorAndOpacity(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f))
                .BorderBackgroundColor(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Target1", "PS5"))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SBorder)
                .Padding(FMargin(6, 2, 6, 2))
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                .ColorAndOpacity(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f))
                .BorderBackgroundColor(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Target2", "Xbox"))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
        ]
        
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0, 0, 8, 0)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("ScopeLabel", "Scope"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 12))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0, 0, 4, 0)
            [
                SNew(SBorder)
                .Padding(FMargin(6, 2, 6, 2))
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                .ColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
                .BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("ScopeValue", "/Game"))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SBorder)
                .Padding(FMargin(6, 2, 6, 2))
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                .ColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
                .BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("ExcludesValue", "2 excludes"))
                    .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerAuditView::MakeTextureTable()
{
    return SNew(SBorder)
    .Padding(FMargin(0))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
    .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
    [
        SNew(SVerticalBox)
        
        // Search and filter
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(16, 16, 16, 8)
        [
            MakeSearchAndFilter()
        ]
        
        // Proper texture table with virtualization
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SAssignNew(TextureTableWidget, SMagicOptimizerTextureTable)
            .TextureData(FilteredTextures)
            .SelectedTextureIds(SelectedTextureIds)
            .SeverityFilter(SeverityFilter)
            .OnTextureSelectionChanged(this, &SMagicOptimizerAuditView::OnTextureSelectionChanged)
            .OnTextureRowClicked(this, &SMagicOptimizerAuditView::OnTextureRowClicked)
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerAuditView::MakeSearchAndFilter()
{
    return SNew(SHorizontalBox)
    
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .Padding(0, 0, 12, 0)
    [
        SNew(SSearchBox)
        .HintText(LOCTEXT("SearchHint", "Search textures..."))
        .OnTextChanged_Lambda([this](const FText& NewText)
        {
            OnSearchTextChanged(NewText);
        })
        .Style(FAppStyle::Get(), "SearchBox")
    ]
    
    +SHorizontalBox::Slot()
    .AutoWidth()
    [
        SNew(SComboButton)
        .ButtonContent()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 4, 0)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("FilterIcon", "🔍"))
                .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("SeverityFilter", "Severity"))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
        ]
        .ButtonStyle(FAppStyle::Get(), "SimpleButton")
        .ContentPadding(FMargin(12, 6))
        .ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
        .MenuContent()
        [
            SNew(SVerticalBox)
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(4)
            [
                SNew(SButton)
                .Text(LOCTEXT("All", "All"))
                .OnClicked_Lambda([this]()
                {
                    OnSeverityFilterChanged(TEXT(""));
                    return FReply::Handled();
                })
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(4)
            [
                SNew(SButton)
                .Text(LOCTEXT("MustFix", "Must fix"))
                .OnClicked_Lambda([this]()
                {
                    OnSeverityFilterChanged(TEXT("red"));
                    return FReply::Handled();
                })
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(4)
            [
                SNew(SButton)
                .Text(LOCTEXT("ShouldFix", "Should fix"))
                .OnClicked_Lambda([this]()
                {
                    OnSeverityFilterChanged(TEXT("amber"));
                    return FReply::Handled();
                })
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerAuditView::MakeTextureList()
{
    return SNew(SScrollBox)
    +SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            // Table header
            SNew(SBorder)
            .Padding(FMargin(12, 8, 12, 8))
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .ColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
            [
                SNew(SHorizontalBox)
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 12, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Select", ""))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SHorizontalBox::Slot()
                .FillWidth(1.0f)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Path", "Path"))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 12, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Size", "Size"))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 12, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Group", "Group"))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 12, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Compression", "Compression"))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 12, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("sRGB", "sRGB"))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 12, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Mips", "Mips"))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 12, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("VT", "VT"))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 12, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("LOD", "LOD"))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 12, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Refs", "Refs"))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 12, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Issues", "Issues"))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Actions", "Actions"))
                    .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                    .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                ]
            ]
        ]
        // Texture rows - show first 10 for now
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            MakeTextureRows()
        ]
        
        // Footer with stats
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 8, 0, 0)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(STextBlock)
                .Text_Lambda([this]()
                {
                    return FText::Format(LOCTEXT("RowCount", "{0} rows • {1} selected"), 
                        FText::AsNumber(FilteredTextures.Num()),
                        FText::AsNumber(SelectedTextureIds.Get().Num()));
                })
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(STextBlock)
                .Text(LOCTEXT("ScanningStatus", "Scanning Textures… 6124 of 16214"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerAuditView::MakeTextureRows()
{
    TSharedRef<SVerticalBox> RowsBox = SNew(SVerticalBox);
    
    // Show first 10 textures for now
    const int32 MaxRows = FMath::Min(10, FilteredTextures.Num());
    for (int32 i = 0; i < MaxRows; ++i)
    {
        RowsBox->AddSlot()
        .AutoHeight()
        .Padding(0, 1, 0, 0)
        [
            MakeTextureRow(FilteredTextures[i])
        ];
    }
    
    return RowsBox;
}

TSharedRef<SWidget> SMagicOptimizerAuditView::MakeTextureRow(const FUITextureRow& Texture)
{
    const bool bIsSelected = SelectedTextureIds.Get().Contains(Texture.Id);
    
    return SNew(SBorder)
    .Padding(FMargin(12, 6, 12, 6))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(bIsSelected ? FLinearColor(0.15f, 0.15f, 0.15f, 1.0f) : FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
    [
        SNew(SHorizontalBox)
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SCheckBox)
            .IsChecked_Lambda([this, Texture]()
            {
                return SelectedTextureIds.Get().Contains(Texture.Id) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
            })
            .OnCheckStateChanged_Lambda([this, Texture](ECheckBoxState NewState)
            {
                OnTextureCheckboxChanged(Texture.Id, NewState == ECheckBoxState::Checked);
            })
        ]
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Texture.Path))
            .Font(FAppStyle::GetFontStyle("MonoFont"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
            .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Texture.Size))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Texture.Group))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Texture.Compression))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(STextBlock)
            .Text(Texture.bSRGB ? LOCTEXT("On", "On") : LOCTEXT("Off", "Off"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
            .ColorAndOpacity(Texture.bSRGB ? FLinearColor(0.2f, 0.8f, 0.2f, 1.0f) : FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(STextBlock)
            .Text(Texture.bMips ? LOCTEXT("Yes", "Yes") : LOCTEXT("No", "No"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(STextBlock)
            .Text(Texture.bVT ? LOCTEXT("VT", "VT") : LOCTEXT("Dash", "—"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
            .ColorAndOpacity(Texture.bVT ? FLinearColor(0.2f, 0.8f, 0.2f, 1.0f) : FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(STextBlock)
            .Text(FText::AsNumber(Texture.LOD))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(STextBlock)
            .Text(FText::AsNumber(Texture.Referencers))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            MakeIssueBadge(Texture.Issues)
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0, 0, 4, 0)
            [
                SNew(SButton)
                .Text(LOCTEXT("Preview", "Preview"))
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .ContentPadding(FMargin(8, 4, 8, 4))
                .OnClicked_Lambda([this, Texture]()
                {
                    OnTextureRowClicked(Texture.Id);
                    return FReply::Handled();
                })
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0, 0, 4, 0)
            [
                SNew(SButton)
                .Text(LOCTEXT("Open", "Open"))
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .ContentPadding(FMargin(8, 4, 8, 4))
                .OnClicked_Lambda([this, Texture]()
                {
                    // TODO: Open asset in content browser
                    return FReply::Handled();
                })
            ]
            +SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                .Text(LOCTEXT("Audit", "Audit"))
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .ContentPadding(FMargin(8, 4, 8, 4))
                .OnClicked_Lambda([this, Texture]()
                {
                    // TODO: Open in asset audit
                    return FReply::Handled();
                })
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerAuditView::MakeIssueBadge(int32 IssueCount)
{
    FLinearColor BadgeColor;
    FText BadgeText;
    
    if (IssueCount >= 2)
    {
        BadgeColor = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);
        BadgeText = LOCTEXT("MustFix", "Must fix");
    }
    else if (IssueCount == 1)
    {
        BadgeColor = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
        BadgeText = LOCTEXT("ShouldFix", "Should fix");
    }
    else
    {
        BadgeColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);
        BadgeText = LOCTEXT("Info", "Info");
    }
    
    return SNew(SBorder)
    .Padding(FMargin(6, 2, 6, 2))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(BadgeColor)
    .BorderBackgroundColor(BadgeColor)
    [
        SNew(STextBlock)
        .Text(BadgeText)
        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
        .Font(FAppStyle::GetFontStyle("NormalFontBold"))
        .ColorAndOpacity(FLinearColor::White)
    ];
}

void SMagicOptimizerAuditView::RefreshData(const TArray<FUITextureRow>& NewTextureData)
{
    UpdateFilteredTextures();
    HealthData = CalculateHealthData();
}

void SMagicOptimizerAuditView::UpdateFilteredTextures()
{
    FilteredTextures = GetFilteredTextures();
}

FHealthData SMagicOptimizerAuditView::CalculateHealthData() const
{
    FHealthData Health;
    Health.Score = 78;
    
    // Generate mock history data
    for (int32 i = 0; i < 20; ++i)
    {
        Health.History.Add(FMath::RandRange(60, 90));
    }
    
    Health.TopOffenders = {
        TEXT("Masks with sRGB on"),
        TEXT("Normals not BC5"),
        TEXT("Non VT assets flagged VT")
    };
    
    Health.OffenderCounts = {45, 82, 16};
    
    return Health;
}

TArray<FUITextureRow> SMagicOptimizerAuditView::GetFilteredTextures() const
{
    TArray<FUITextureRow> Filtered = TextureData.Get();
    
    // Apply search filter
    if (!SearchText.Get().IsEmpty())
    {
        const FString SearchLower = SearchText.Get().ToLower();
        Filtered = Filtered.FilterByPredicate([SearchLower](const FUITextureRow& Texture)
        {
            return Texture.Path.ToLower().Contains(SearchLower) ||
                   Texture.Group.ToLower().Contains(SearchLower) ||
                   Texture.Compression.ToLower().Contains(SearchLower);
        });
    }
    
    // Apply severity filter
    if (!SeverityFilter.IsEmpty())
    {
        if (SeverityFilter == TEXT("red"))
        {
            Filtered = Filtered.FilterByPredicate([](const FUITextureRow& Texture)
            {
                return Texture.Issues >= 2;
            });
        }
        else if (SeverityFilter == TEXT("amber"))
        {
            Filtered = Filtered.FilterByPredicate([](const FUITextureRow& Texture)
            {
                return Texture.Issues == 1;
            });
        }
        else if (SeverityFilter == TEXT("blue"))
        {
            Filtered = Filtered.FilterByPredicate([](const FUITextureRow& Texture)
            {
                return Texture.Issues == 0;
            });
        }
    }
    
    return Filtered;
}

void SMagicOptimizerAuditView::OnSearchTextChanged(const FText& NewText)
{
    OnSearchTextChangedDelegate.ExecuteIfBound(NewText);
    UpdateFilteredTextures();
    
    // Refresh texture table widget with new filtered data
    if (TextureTableWidget.IsValid())
    {
        TextureTableWidget->RefreshData(FilteredTextures);
    }
}

void SMagicOptimizerAuditView::OnSeverityFilterChanged(const FString& NewFilter)
{
    SeverityFilter = NewFilter;
    UpdateFilteredTextures();
    
    // Refresh texture table widget with new filtered data
    if (TextureTableWidget.IsValid())
    {
        TextureTableWidget->RefreshData(FilteredTextures);
    }
}

void SMagicOptimizerAuditView::OnTextureSelectionChanged(const TArray<FString>& NewSelection)
{
    OnTextureSelectionChangedDelegate.ExecuteIfBound(NewSelection);
}

void SMagicOptimizerAuditView::OnTextureRowClicked(const FString& TextureId)
{
    // Find the texture data
    for (const FUITextureRow& Texture : FilteredTextures)
    {
        if (Texture.Id == TextureId)
        {
            // Update selection to include this texture
            TArray<FString> NewSelection = {TextureId};
            OnTextureSelectionChangedDelegate.ExecuteIfBound(NewSelection);
            
            // Trigger preview (which will open the slide-over)
            // This will be handled by the main panel
            break;
        }
    }
}

void SMagicOptimizerAuditView::OnTextureCheckboxChanged(const FString& TextureId, bool bIsChecked)
{
    TArray<FString> NewSelection = SelectedTextureIds.Get();
    
    if (bIsChecked)
    {
        NewSelection.AddUnique(TextureId);
    }
    else
    {
        NewSelection.Remove(TextureId);
    }
    
    OnTextureSelectionChangedDelegate.ExecuteIfBound(NewSelection);
}

// =====================================================
// SMagicOptimizerTextureTable Implementation
// =====================================================

void SMagicOptimizerTextureTable::Construct(const FArguments& InArgs)
{
    // Bind attributes and events
    TextureData = InArgs._TextureData;
    SelectedTextureIds = InArgs._SelectedTextureIds;
    SeverityFilter = InArgs._SeverityFilter;
    OnTextureSelectionChangedDelegate = InArgs._OnTextureSelectionChanged;
    // OnTextureRowClicked delegate not implemented in this class

    // Initialize texture list data
    RefreshData(TextureData.Get());

    ChildSlot
    [
        SNew(SVerticalBox)
        
        // Table header
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            MakeTableHeader()
        ]
        
        // Table body with list view
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            MakeTableBody()
        ]
        
        // Footer with stats
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(12, 8, 12, 8)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(STextBlock)
                .Text_Lambda([this]()
                {
                    return FText::Format(LOCTEXT("TableRowCount", "{0} textures • {1} selected"), 
                        FText::AsNumber(TextureListData.Num()),
                        FText::AsNumber(SelectedTextureIds.Get().Num()));
                })
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerTextureTable::MakeTableHeader()
{
    return SNew(SBorder)
    .Padding(FMargin(12, 8, 12, 8))
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
    [
        SNew(SHorizontalBox)
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(24)
            [
                SNew(SCheckBox)
                .IsChecked_Lambda([this]()
                {
                    const TArray<FString>& Selected = SelectedTextureIds.Get();
                    if (Selected.Num() == 0) return ECheckBoxState::Unchecked;
                    if (Selected.Num() == TextureListData.Num()) return ECheckBoxState::Checked;
                    return ECheckBoxState::Undetermined;
                })
                .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
                {
                    TArray<FString> NewSelection;
                    if (NewState == ECheckBoxState::Checked)
                    {
                        // Select all
                        for (const TSharedPtr<FUITextureRow>& Texture : TextureListData)
                        {
                            if (Texture.IsValid())
                            {
                                NewSelection.Add(Texture->Id);
                            }
                        }
                    }
                    // If unchecked or undetermined, clear selection (NewSelection stays empty)
                    OnTextureSelectionChangedDelegate.ExecuteIfBound(NewSelection);
                })
            ]
        ]
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("Path", "Path"))
            .Font(FAppStyle::GetFontStyle("NormalFontBold"))
            .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(60)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("Size", "Size"))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(80)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("Group", "Group"))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(90)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("Compression", "Compression"))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(40)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("sRGB", "sRGB"))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(40)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("Mips", "Mips"))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(30)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("VT", "VT"))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(30)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("LOD", "LOD"))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(40)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("Refs", "Refs"))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(50)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("Issues", "Issues"))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SBox)
            .WidthOverride(80)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("Actions", "Actions"))
                .Font(FAppStyle::GetFontStyle("NormalFontBold"))
                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerTextureTable::MakeTableBody()
{
    return SAssignNew(TextureListView, SListView<TSharedPtr<FUITextureRow>>)
        .ListItemsSource(&TextureListData)
        .OnGenerateRow_Lambda([this](TSharedPtr<FUITextureRow> Texture, const TSharedRef<STableViewBase>& OwnerTable) -> TSharedRef<ITableRow>
        {
            return SNew(STableRow<TSharedPtr<FUITextureRow>>, OwnerTable)
            .Style(FAppStyle::Get(), "TableView.Row")
            [
                MakeTextureRow(*Texture)
            ];
        })
        .ItemHeight(32)
        .SelectionMode(ESelectionMode::Multi)
        .OnSelectionChanged_Lambda([this](TSharedPtr<FUITextureRow> Texture, ESelectInfo::Type SelectInfo)
        {
            if (SelectInfo != ESelectInfo::Direct)
            {
                // Update selection based on list view selection
                TArray<TSharedPtr<FUITextureRow>> SelectedItems = TextureListView->GetSelectedItems();
                TArray<FString> NewSelection;
                for (const TSharedPtr<FUITextureRow>& Item : SelectedItems)
                {
                    if (Item.IsValid())
                    {
                        NewSelection.Add(Item->Id);
                    }
                }
                OnTextureSelectionChangedDelegate.ExecuteIfBound(NewSelection);
            }
        })
        .OnMouseButtonClick_Lambda([this](TSharedPtr<FUITextureRow> Texture)
        {
            if (Texture.IsValid())
            {
                // OnTextureRowClicked delegate not implemented for this class
            }
            // Note: OnMouseButtonClick expects void return, not FReply
        });
}

TSharedRef<SWidget> SMagicOptimizerTextureTable::MakeTextureRow(const FUITextureRow& Texture)
{
    const bool bIsSelected = SelectedTextureIds.Get().Contains(Texture.Id);
    
    return SNew(SBorder)
    .Padding(FMargin(12, 4, 12, 4))
    .BorderImage(FAppStyle::GetBrush("NoBorder"))
    .ColorAndOpacity(bIsSelected ? FLinearColor(0.15f, 0.15f, 0.15f, 1.0f) : FLinearColor::Transparent)
    [
        SNew(SHorizontalBox)
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(24)
            [
                SNew(SCheckBox)
                .IsChecked(bIsSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
                .OnCheckStateChanged_Lambda([this, Texture](ECheckBoxState NewState)
                {
                    OnTextureCheckboxChanged(Texture.Id, NewState == ECheckBoxState::Checked);
                })
            ]
        ]
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        [
            SNew(SButton)
            .ButtonStyle(FAppStyle::Get(), "NoBorder")
            .ContentPadding(FMargin(0))
            .OnClicked_Lambda([this, Texture]()
            {
                // OnTextureRowClicked delegate not implemented for this class
                return FReply::Handled();
            })
            [
                SNew(STextBlock)
                .Text(FText::FromString(Texture.Path))
                .Font(FAppStyle::GetFontStyle("MonoFont"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
                .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(60)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Texture.Size))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(80)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Texture.Group))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
                .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(90)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Texture.Compression))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
                .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(40)
            [
                SNew(STextBlock)
                .Text(Texture.bSRGB ? LOCTEXT("On", "On") : LOCTEXT("Off", "Off"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(Texture.bSRGB ? FLinearColor(0.2f, 0.8f, 0.2f, 1.0f) : FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(40)
            [
                SNew(STextBlock)
                .Text(Texture.bMips ? LOCTEXT("Yes", "Yes") : LOCTEXT("No", "No"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(30)
            [
                SNew(STextBlock)
                .Text(Texture.bVT ? LOCTEXT("Yes", "Yes") : LOCTEXT("No", "No"))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(30)
            [
                SNew(STextBlock)
                .Text(FText::AsNumber(Texture.LOD))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(40)
            [
                SNew(STextBlock)
                .Text(FText::AsNumber(Texture.Referencers))
                .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
                .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 12, 0)
        [
            SNew(SBox)
            .WidthOverride(50)
            [
                MakeIssueBadge(Texture.Issues)
            ]
        ]
        +SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SBox)
            .WidthOverride(80)
            [
                SNew(SButton)
                .Text(LOCTEXT("Preview", "Preview"))
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .ContentPadding(FMargin(8, 2, 8, 2))
                .OnClicked_Lambda([this, Texture]()
                {
                    // OnTextureRowClicked delegate not implemented for this class
                    return FReply::Handled();
                })
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerTextureTable::MakeIssueBadge(int32 IssueCount)
{
    if (IssueCount == 0)
    {
        return SNew(STextBlock)
            .Text(LOCTEXT("NoIssues", "-"))
            .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 11))
            .ColorAndOpacity(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));
    }
    
    FLinearColor BadgeColor = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f); // Red
    if (IssueCount == 1)
    {
        BadgeColor = FLinearColor(0.8f, 0.6f, 0.2f, 1.0f); // Amber
    }
    
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
    .ColorAndOpacity(BadgeColor)
    .Padding(FMargin(6, 2, 6, 2))
    [
        SNew(STextBlock)
        .Text(FText::AsNumber(IssueCount))
        .Font(FSlateFontInfo(FAppStyle::GetFontStyle("SmallFont").FontObject, 10))
        .ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
    ];
}

void SMagicOptimizerTextureTable::RefreshData(const TArray<FUITextureRow>& NewTextureData)
{
    // Update texture list data
    TextureListData.Empty();
    
    const FString CurrentFilter = SeverityFilter.Get();
    
    for (const FUITextureRow& Texture : NewTextureData)
    {
        // Apply severity filter
        bool bPassesFilter = true;
        if (!CurrentFilter.IsEmpty())
        {
            if (CurrentFilter == TEXT("red") && Texture.Issues < 2)
            {
                bPassesFilter = false;
            }
            else if (CurrentFilter == TEXT("amber") && Texture.Issues != 1)
            {
                bPassesFilter = false;
            }
        }
        
        if (bPassesFilter)
        {
            TextureListData.Add(MakeShared<FUITextureRow>(Texture));
        }
    }
    
    // Update list view
    if (TextureListView.IsValid())
    {
        TextureListView->RequestListRefresh();
        
        // Update selection in list view
        const TArray<FString>& CurrentSelection = SelectedTextureIds.Get();
        TArray<TSharedPtr<FUITextureRow>> SelectedItems;
        
        for (const TSharedPtr<FUITextureRow>& Item : TextureListData)
        {
            if (Item.IsValid() && CurrentSelection.Contains(Item->Id))
            {
                SelectedItems.Add(Item);
            }
        }
        
        TextureListView->ClearSelection();
        for (const auto& SelectedItem : SelectedItems)
        {
            TextureListView->SetItemSelection(SelectedItem, true);
        }
    }
}

void SMagicOptimizerTextureTable::OnTextureCheckboxChanged(const FString& TextureId, bool bIsChecked)
{
    TArray<FString> NewSelection = SelectedTextureIds.Get();
    
    if (bIsChecked)
    {
        NewSelection.AddUnique(TextureId);
    }
    else
    {
        NewSelection.Remove(TextureId);
    }
    
    OnTextureSelectionChangedDelegate.ExecuteIfBound(NewSelection);
}

void SMagicOptimizerTextureTable::OnTextureRowClicked(const FString& TextureId)
{
    // OnTextureRowClicked delegate not implemented for this class
}

#undef LOCTEXT_NAMESPACE