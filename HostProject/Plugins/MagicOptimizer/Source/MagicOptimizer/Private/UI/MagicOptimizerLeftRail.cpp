#include "UI/MagicOptimizerLeftRail.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SButton.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "SMagicOptimizerLeftRail"

void SMagicOptimizerLeftRail::Construct(const FArguments& InArgs)
{
    // Bind events
    OnScopeChanged = InArgs._OnScopeChanged;

    // Initialize data
    InitializeData();

    ChildSlot
    [
        SNew(SBorder)
        .Padding(FMargin(12, 16, 12, 16))
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .ColorAndOpacity(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
        .BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.12f, 1.0f))
        [
            SNew(SScrollBox)
            +SScrollBox::Slot()
            [
                SNew(SVerticalBox)
                
                // Project Scope section
                +SVerticalBox::Slot()
                .AutoHeight()
                [
                    MakeProjectScopeSection()
                ]
                
                // Asset Types section
                +SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 16, 0, 0)
                [
                    MakeAssetTypesSection()
                ]
                
                // Saved Views section
                +SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 16, 0, 0)
                [
                    MakeSavedViewsSection()
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerLeftRail::MakeProjectScopeSection()
{
    return SNew(SVerticalBox)
    
    +SVerticalBox::Slot()
    .AutoHeight()
    [
        SNew(STextBlock)
        .Text(LOCTEXT("ProjectScopeTitle", "Project Scope"))
        .Font(FAppStyle::GetFontStyle("NormalFontBold"))
        .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
    ]
    
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 8, 0, 0)
    [
        MakeProjectScopeTree()
    ];
}

TSharedRef<SWidget> SMagicOptimizerLeftRail::MakeAssetTypesSection()
{
    return SNew(SVerticalBox)
    
    +SVerticalBox::Slot()
    .AutoHeight()
    [
        SNew(STextBlock)
        .Text(LOCTEXT("AssetTypesTitle", "Asset Types"))
        .Font(FAppStyle::GetFontStyle("NormalFontBold"))
        .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
    ]
    
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 8, 0, 0)
    [
        SNew(SVerticalBox)
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            MakeAssetTypeCheckbox(AssetTypeItems[0]) // Textures
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeAssetTypeCheckbox(AssetTypeItems[1]) // Meshes
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeAssetTypeCheckbox(AssetTypeItems[2]) // Materials
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeAssetTypeCheckbox(AssetTypeItems[3]) // Blueprints
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeAssetTypeCheckbox(AssetTypeItems[4]) // Animations
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeAssetTypeCheckbox(AssetTypeItems[5]) // Audio
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeAssetTypeCheckbox(AssetTypeItems[6]) // Niagara
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeAssetTypeCheckbox(AssetTypeItems[7]) // Levels
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeAssetTypeCheckbox(AssetTypeItems[8]) // UI
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerLeftRail::MakeSavedViewsSection()
{
    return SNew(SVerticalBox)
    
    +SVerticalBox::Slot()
    .AutoHeight()
    [
        SNew(STextBlock)
        .Text(LOCTEXT("SavedViewsTitle", "Saved Views"))
        .Font(FAppStyle::GetFontStyle("NormalFontBold"))
        .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
    ]
    
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 8, 0, 0)
    [
        SNew(SVerticalBox)
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            MakeSavedViewButton(SavedViewItems[0]) // High Impact
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeSavedViewButton(SavedViewItems[1]) // UI Pass
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeSavedViewButton(SavedViewItems[2]) // Console Ready
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerLeftRail::MakeProjectScopeTree()
{
    return SNew(SVerticalBox)
    +SVerticalBox::Slot()
    .AutoHeight()
    [
        MakeProjectScopeItem(ProjectScopeItems[0]) // Game folder
    ];
}

TSharedRef<SWidget> SMagicOptimizerLeftRail::MakeProjectScopeItem(const FProjectScopeItem& Item, int32 Depth)
{
    // Base case: if no children, just return a simple checkbox
    if (Item.Children.Num() == 0 || Depth > 2)
    {
        return SNew(SHorizontalBox)
        +SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SCheckBox)
            .IsChecked(true)
            .OnCheckStateChanged_Lambda([this, ItemId = Item.Id](ECheckBoxState NewState)
            {
                OnProjectScopeItemToggled(ItemId, NewState == ECheckBoxState::Checked);
            })
        ]
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .VAlign(VAlign_Center)
        .Padding(8, 0, 0, 0)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Item.Label))
            .Font(FAppStyle::GetFontStyle("NormalFont"))
        ];
    }

    return SNew(SVerticalBox)
    
    // Main item row
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 4)
    [
        SNew(SHorizontalBox)
        
        // Expand/collapse button (if has children)
        +SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 8, 0)
        [
            SNew(SButton)
            .ButtonStyle(FAppStyle::Get(), "NoBorder")
            .ContentPadding(FMargin(4, 2))
            .Visibility_Lambda([&Item]()
            {
                return Item.Children.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed;
            })
            .OnClicked_Lambda([this, &Item]()
            {
                // Toggle expansion
                ToggleProjectScopeItem(Item.Id);
                return FReply::Handled();
            })
            [
                SNew(STextBlock)
                .Text(Item.bIsExpanded ? FText::FromString(TEXT("▼")) : FText::FromString(TEXT("▶")))
                .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
            ]
        ]
        
        // Checkbox
        +SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 8, 0)
        [
            SNew(SCheckBox)
            .IsChecked_Lambda([&Item]()
            {
                return Item.bIsChecked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
            })
            .OnCheckStateChanged_Lambda([this, &Item](ECheckBoxState NewState)
            {
                OnProjectScopeItemToggled(Item.Id, NewState == ECheckBoxState::Checked);
            })
        ]
        
        // Label
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Item.Label))
            .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
        ]
    ]
    
    // Children (if expanded)
    +SVerticalBox::Slot()
    .AutoHeight()
    .Padding(16, 0, 0, 0)
    [
        SNew(SBox)
        .Visibility_Lambda([&Item]()
        {
            return Item.bIsExpanded && Item.Children.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed;
        })
        [
        SNew(SVerticalBox)
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            MakeProjectScopeItem(Item.Children[0], Depth + 1) // Maps
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeProjectScopeItem(Item.Children[1], Depth + 1) // Meshes
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeProjectScopeItem(Item.Children[2], Depth + 1) // Textures
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeProjectScopeItem(Item.Children[3], Depth + 1) // Materials
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeProjectScopeItem(Item.Children[4], Depth + 1) // UI
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeProjectScopeItem(Item.Children[5], Depth + 1) // Audio
        ]
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4, 0, 0)
        [
            MakeProjectScopeItem(Item.Children[6], Depth + 1) // Niagara
        ]
        ]
    ];
}

TSharedRef<SWidget> SMagicOptimizerLeftRail::MakeAssetTypeCheckbox(const FAssetTypeItem& Item)
{
    return SNew(SHorizontalBox)
    
    +SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    .Padding(0, 0, 8, 0)
    [
        SNew(SCheckBox)
        .IsChecked_Lambda([&Item]()
        {
            return Item.bIsChecked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
        })
        .OnCheckStateChanged_Lambda([this, &Item](ECheckBoxState NewState)
        {
            OnAssetTypeToggled(Item.Id, NewState == ECheckBoxState::Checked);
        })
    ]
    
    +SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .VAlign(VAlign_Center)
    [
        SNew(STextBlock)
        .Text(FText::FromString(Item.Label))
        .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
    ];
}

TSharedRef<SWidget> SMagicOptimizerLeftRail::MakeSavedViewButton(const FSavedViewItem& Item)
{
    return SNew(SButton)
    .Text(FText::FromString(Item.Label))
    .OnClicked_Lambda([this, &Item]()
    {
        OnSavedViewClicked(Item.Id);
        return FReply::Handled();
    })
    .ButtonStyle(FAppStyle::Get(), "SimpleButton")
    .ContentPadding(FMargin(8, 4))
    .ButtonColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
    .HAlign(HAlign_Left);
}

void SMagicOptimizerLeftRail::InitializeData()
{
    // Initialize project scope items using simpler approach
    FProjectScopeItem GameItem;
    GameItem.Id = TEXT("game");
    GameItem.Label = TEXT("Game");
    GameItem.bIsExpanded = true;
    GameItem.bIsChecked = true;
    
    // Add children
    FProjectScopeItem MapsItem; MapsItem.Id = TEXT("maps"); MapsItem.Label = TEXT("Maps");
    FProjectScopeItem MeshesItem; MeshesItem.Id = TEXT("meshes"); MeshesItem.Label = TEXT("Meshes");
    FProjectScopeItem TexturesItem; TexturesItem.Id = TEXT("textures"); TexturesItem.Label = TEXT("Textures");
    FProjectScopeItem MaterialsItem; MaterialsItem.Id = TEXT("materials"); MaterialsItem.Label = TEXT("Materials");
    FProjectScopeItem UIItem; UIItem.Id = TEXT("ui"); UIItem.Label = TEXT("UI");
    FProjectScopeItem AudioItem; AudioItem.Id = TEXT("audio"); AudioItem.Label = TEXT("Audio");
    FProjectScopeItem NiagaraItem; NiagaraItem.Id = TEXT("niagara"); NiagaraItem.Label = TEXT("Niagara");
    
    GameItem.Children = {MapsItem, MeshesItem, TexturesItem, MaterialsItem, UIItem, AudioItem, NiagaraItem};
    ProjectScopeItems = {GameItem};

    // Initialize asset type items
    AssetTypeItems = {
        FAssetTypeItem(TEXT("textures"), TEXT("Textures"), true),
        FAssetTypeItem(TEXT("meshes"), TEXT("Meshes"), false),
        FAssetTypeItem(TEXT("materials"), TEXT("Materials"), false),
        FAssetTypeItem(TEXT("blueprints"), TEXT("Blueprints"), false),
        FAssetTypeItem(TEXT("animations"), TEXT("Animations"), false),
        FAssetTypeItem(TEXT("audio"), TEXT("Audio"), false),
        FAssetTypeItem(TEXT("niagara"), TEXT("Niagara"), false),
        FAssetTypeItem(TEXT("levels"), TEXT("Levels"), false),
        FAssetTypeItem(TEXT("ui"), TEXT("UI"), false)
    };

    // Initialize saved view items
    SavedViewItems = {
        FSavedViewItem(TEXT("high_impact"), TEXT("High Impact"), TEXT("Focus on high-impact optimizations")),
        FSavedViewItem(TEXT("ui_pass"), TEXT("UI Pass"), TEXT("UI-specific optimizations")),
        FSavedViewItem(TEXT("console_ready"), TEXT("Console Ready"), TEXT("Console-optimized settings"))
    };
}

void SMagicOptimizerLeftRail::OnProjectScopeItemToggled(const FString& ItemId, bool bIsChecked)
{
    // Update the item state
    for (FProjectScopeItem& Item : ProjectScopeItems)
    {
        if (Item.Id == ItemId)
        {
            Item.bIsChecked = bIsChecked;
            break;
        }
        
        // Check children
        for (FProjectScopeItem& Child : Item.Children)
        {
            if (Child.Id == ItemId)
            {
                Child.bIsChecked = bIsChecked;
                break;
            }
        }
    }
    
    UpdateScopeSummary();
}

void SMagicOptimizerLeftRail::OnAssetTypeToggled(const FString& ItemId, bool bIsChecked)
{
    // Update the item state
    for (FAssetTypeItem& Item : AssetTypeItems)
    {
        if (Item.Id == ItemId)
        {
            Item.bIsChecked = bIsChecked;
            break;
        }
    }
    
    UpdateScopeSummary();
}

void SMagicOptimizerLeftRail::OnSavedViewClicked(const FString& ItemId)
{
    // Apply saved view settings
    if (ItemId == TEXT("high_impact"))
    {
        // Set high impact settings
        for (FAssetTypeItem& Item : AssetTypeItems)
        {
            Item.bIsChecked = (Item.Id == TEXT("textures") || Item.Id == TEXT("meshes"));
        }
    }
    else if (ItemId == TEXT("ui_pass"))
    {
        // Set UI pass settings
        for (FAssetTypeItem& Item : AssetTypeItems)
        {
            Item.bIsChecked = (Item.Id == TEXT("textures") || Item.Id == TEXT("ui"));
        }
    }
    else if (ItemId == TEXT("console_ready"))
    {
        // Set console ready settings
        for (FAssetTypeItem& Item : AssetTypeItems)
        {
            Item.bIsChecked = (Item.Id == TEXT("textures") || Item.Id == TEXT("meshes") || Item.Id == TEXT("materials"));
        }
    }
    
    UpdateScopeSummary();
}

void SMagicOptimizerLeftRail::ToggleProjectScopeItem(const FString& ItemId)
{
    // Toggle expansion state
    for (FProjectScopeItem& Item : ProjectScopeItems)
    {
        if (Item.Id == ItemId)
        {
            Item.bIsExpanded = !Item.bIsExpanded;
            break;
        }
        
        // Check children
        for (FProjectScopeItem& Child : Item.Children)
        {
            if (Child.Id == ItemId)
            {
                Child.bIsExpanded = !Child.bIsExpanded;
                break;
            }
        }
    }
}

void SMagicOptimizerLeftRail::UpdateScopeSummary()
{
    FString NewScope = GenerateScopeSummary();
    OnScopeChanged.ExecuteIfBound(NewScope);
}

FString SMagicOptimizerLeftRail::GenerateScopeSummary() const
{
    int32 CheckedCount = 0;
    int32 TotalCount = 0;
    
    // Count project scope items
    for (const FProjectScopeItem& Item : ProjectScopeItems)
    {
        if (Item.bIsChecked)
        {
            CheckedCount++;
        }
        TotalCount++;
        
        for (const FProjectScopeItem& Child : Item.Children)
        {
            if (Child.bIsChecked)
            {
                CheckedCount++;
            }
            TotalCount++;
        }
    }
    
    // Count asset types
    for (const FAssetTypeItem& Item : AssetTypeItems)
    {
        if (Item.bIsChecked)
        {
            CheckedCount++;
        }
        TotalCount++;
    }
    
    int32 ExcludeCount = TotalCount - CheckedCount;
    return FString::Printf(TEXT("/Game, %d excludes"), ExcludeCount);
}

#undef LOCTEXT_NAMESPACE
