MagicOptimizer Plugin - Ultimate Project Guide
Project Overview
MagicOptimizer is a comprehensive asset optimization plugin for Unreal Engine 5.6 designed to automatically optimize textures, meshes, materials, and runtime settings with safety gates, dry-run capabilities, and preset-based configurations.

Core Technical Architecture
Primary Technology Stack
Language: C++ for core engine integration, Python for analysis/optimization logic
Engine: Unreal Engine 5.6 (with support for UE 5.4+)
Integration Points:
UEditorPerProjectUserSettings for configuration
IPythonScriptPlugin for Python bridge
IMeshReduction for mesh optimization
NaniteTools for Nanite management
Asset Registry and EditorAssetLibrary for scanning
Material Expression graph traversal
Core Subsystems
TextureDoctor (Python-heavy)
MeshDoctor (C++ with Python integration)
MaterialDoctor (C++/Python material graph scanning)
RuntimeOptimizer (C++)
WorldPartitionManager (C++)
AudioOptimizer (C++)
PackagingManager (C++)
History & Revert System
Collection Management
Project Structure & Core Architecture
File Structure
HostProject/
├── Plugins/
│   └── MagicOptimizer/
│       ├── MagicOptimizer.uplugin
│       ├── Resources/
│       │   └── Icon128.png
│       ├── Source/
│       │   └── MagicOptimizer/
│       │       ├── Public/
│       │       │   ├── MagicOptimizer.h
│       │       │   ├── SOptimizerPanel.h
│       │       │   ├── OptimizerSettings.h
│       │       │   ├── OptimizerRun.h
│       │       │   └── PythonBridge.h
│       │       └── Private/
│       │           ├── MagicOptimizerModule.cpp
│       │           ├── SOptimizerPanel.cpp
│       │           ├── OptimizerSettings.cpp
│       │           ├── OptimizerRun.cpp
│       │           └── PythonBridge.cpp
│       └── Content/
│           └── Python/
│               └── magic_optimizer/
│                   ├── __init__.py
│                   ├── entry.py
│                   ├── io_csv.py
│                   ├── utils.py
│                   ├── textures/
│                   │   ├── audit.py
│                   │   ├── recommend.py
│                   │   ├── apply.py
│                   │   └── verify.py
│                   ├── meshes/
│                   │   ├── audit.py
│                   │   ├── recommend.py
│                   │   ├── apply.py
│                   │   └── verify.py
│                   └── presets/
│                       ├── PC_High.json
│                       ├── Console.json
│                       ├── Mobile_AR.json
│                       ├── VR.json
│                       ├── VirtualProduction.json
│                       ├── Authoring_Safe.json
│                       └── Cinematic.json
Component Diagrams & Class Hierarchies
Component Diagram
Core Systems: TextureDoctor, MeshDoctor, MaterialDoctor, RuntimeOptimizer, WorldPartitionManager, AudioOptimizer, PackagingManager
Integration Points: IPythonScriptPlugin, UEditorPerProjectUserSettings, IMeshReduction, NaniteTools, AssetRegistry, EditorAssetLibrary, Material Expression Graph Traversal

Class Hierarchies
MagicOptimizerModule: Main module class
SOptimizerPanel: UI panel for the optimizer
OptimizerSettings: Configuration settings for optimization
PythonBridge: Bridge to run Python scripts
OptimizerRun: Orchestration of the optimization phases
Data Flow Diagrams
User interacts with SOptimizerPanel
Settings are read from OptimizerSettings
OptimizerRun orchestrates audit, recommend, apply, and verify phases
PythonBridge runs Python scripts for each phase
Results are reported back to the UI
Key Features Implementation
Texture Optimization System
Compression Class Assignment: Filename heuristics with material sampler inference
sRGB Flag Correction: Same heuristics + inference as compression system
Mipmap Generation Policy: Group policy enforcement with UI/LUT force no mips logic
LOD Group Enforcement: Classification system for UI/Normal/LUT assets
Virtual Texture Discipline: Path hints + RESPECT_VT_ON_LANDSCAPES flag support
Never Stream for UI/LUTs: Path and tag hint detection (/UI/, sprite, icon, font, lut)
LOD Bias Lower Bound: Name hints for detail/small textures with preset flag enforcement
Size Caps: MaxSizeColor/Normal/Mask per preset configuration with advice vs enforce toggle
Memory Impact Estimation: Format × resolution × mips calculation table lookup
Mesh Optimization System
Nanite Enable/Disable: Preset target logic with triangle count thresholds and blacklist handling
LOD Generation: Strategy: GroupFirst enforcement with triangle ceilings and screen size curve application
LOD Group Enforcement: Project LODGroup lookup via UStaticMesh::LODGroup
Lightmap UVs: Built-in charting with overlap detection and min/max resolution controls
Collision Simplification: BodySetup primitives count analysis with simple convex creation logic
Optional Actor Merging: Editor utility call integration with source asset preservation
Material Optimization System
DX/GL Normal Convention Audit: Texture name and sampler type scanning with green channel assumption comparison
Static Switch Cleanup: Graph analysis in C++ with Python traversal support and backup saving
Sampler Consolidation: Hash sampler states for duplicate detection with report-first approach
Packed Map Hints: Texture name cross-checking with material pins for BC5/BC4 opportunity detection
Runtime & Performance Systems
Scalability Bucket Configuration: Preset target configuration with triangle count thresholds
Device Profile Switching: Target platform support with device-specific optimizations
Shadow Map/VSM Management: Toggle capabilities with quality adjustment per preset
Post-process Quality Adjustment: Resolution/quality level management
Streaming and Async Loading Controls: Texture streaming pool size hints and async loading configuration
Safety & Governance Systems
Core Safety Mechanisms
Dry-run by Default: dry_run=true configuration
Max-changes Caps: Per-preset defaults with enforcement system
Revert Snapshots: History tracking in Saved/Optimizor/History/*.json
Source-control Checkout: Automation for version control systems
Editor Closure: Automatic editor closure before changes
Verification After Write: Retry post_edit_change functionality
Preset Management
Create all 9 curated presets:

PC Ultra - High-end optimizations
PC Balanced - Balanced performance vs quality
Console Optimized - Console-specific optimizations
Mobile Low - Mobile device constraints
Mobile Ultra-Lite - Maximum mobile optimization
VR - Virtual reality specific optimizations
Cinematic - High-quality cinematic workflows
UI Crisp - UI asset optimization focus
Archviz High-Fidelity - Architecture visualization optimizations
Each preset includes:

Specific apply_only gates
Max-changes settings
Preset-specific advice bundles
User Interface Requirements
Core Interface Components
Multi-tab Interface: Textures | Meshes | Materials | Runtime | Reports
Preset Selector: Visual badges and tooltips for preset selection
Scope Controls: Dry Run, Max Changes, Selection/Paths options
Asset-specific Tabs: Domain-specific optimization controls
Explain Change Popover: Per-asset change explanation functionality
Quick Actions: One-click Collections creation and report folder opening
Summary Display: Sticky summary counts display
Preset Badges: Footer with preset badges and delta tooltips
Implementation Phases (26 Weeks)
Phase 0: Foundation & Setup (Weeks 1-2)
Complete folder structure creation (Content/Python/magic_optimizer/)
OptimizerSettings class implementation
Python bridge setup with IPythonScriptPlugin
SOptimizerPanel UI framework establishment
Core subsystem configuration
Phase 1: Texture Optimization System (Weeks 3-5)
Texture analysis engine implementation
Compression decision engine for BC5/BC4
Material inference system for texture categorization
LODGroup precedence logic with policy enforcement
Virtual texture handling with landscape exceptions
Dry-run mode with verification after write
Collections system for changed/skipped assets
Revert snapshot functionality
Comprehensive reporting system (TXT, CSV, JSON)
Hot list generation
Phase 2: Mesh Optimization System (Weeks 6-8)
Mesh analysis engine integration
IMeshReduction pipeline with fallback to Simplygon
LOD group enforcement system with triangle percent calculation
Auto-LOD count algorithm based on triangle count and screensize curve
Nanite enabling/disabling by preset logic
Lightmap UV generation within min/max res budget
Collision creation with simple convex detection
Merge by cluster option for prop sets
Triangle count reporting per LOD and Nanite stats
Mesh-specific safety checks and verification
Phase 3: Material Optimization System (Weeks 9-10)
Material analysis engine implementation
DX/GL normal convention detection system
Packed-map hints recognition system
Unused static switch detection and cleanup
Redundant texture sampler identification
Material graph scanning capabilities
Automatic static switch cleanup in dry-run mode
Texture sampler consolidation suggestions
Mixed DX/GL normal convention flagging
Material-specific safety checks and recommendations
Phase 4: Runtime & Performance Systems (Weeks 11-12)
Scalability bucket configuration per preset
Device profile selection system with target platform support
Resolution/quality level management
Shadow map/VSM toggling capabilities
Post-process quality adjustment per preset
Texture streaming pool size hints
Fast Geometry Streaming for large worlds
Async loading time limits and IO batch sizes
World partition enforcement with sensible cell sizing
HLOD generation suggestions
Phase 5: UI/UX Implementation (Weeks 13-14)
Tab interface design
Quality-of-life features implementation
Visual indicators system
Preset selector enhancement
Scope controls optimization
Asset-specific tab layouts
Phase 6: Preset Management & Testing (Weeks 15-17)
Preset implementation across all systems
Testing framework setup
QA validation procedures
Regression testing on sample projects
Performance benchmarking
Phase 7: Final Polish & Delivery (Weeks 18-20)
Build automation and packaging
Documentation and user guides
Performance optimization
Final testing and validation
Release preparation
Core Implementation Files
OptimizerSettings.h
UENUM()
enum class EOptimizerCategory : uint8
{
    Textures,
    Meshes,
    Materials,
    Levels
};

UCLASS()
class MAGICOPTIMIZER_API UOptimizerSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, Category = "General")
    bool bUseSelection;
    
    UPROPERTY(Config, EditAnywhere, Category = "General")
    FString IncludePathsCsv;
    
    UPROPERTY(Config, EditAnywhere, Category = "General")
    FString ExcludePathsCsv;
    
    UPROPERTY(Config, EditAnywhere, Category = "General")
    FString TargetProfile;
    
    UPROPERTY(Config, EditAnywhere, Category = "General")
    uint8 CategoryMask;
    
    UPROPERTY(Config, EditAnywhere, Category = "General")
    bool bConservativeMode;
    
    UPROPERTY(Config, EditAnywhere, Category = "General")
    TArray<FString> ApplyOnlyList;
};
Python Integration
entry.py
import os
import json
from pathlib import Path

# Import category modules
from . import io_csv
from .utils import resolve_enum
from .textures import audit as textures_audit, recommend as textures_recommend, apply as textures_apply, verify as textures_verify
from .meshes import audit as meshes_audit, recommend as meshes_recommend, apply as meshes_apply, verify as meshes_verify

def run(config_path, phase, category):
    with open(config_path, 'r') as f:
        config = json.load(f)
    
    output_dir = config['paths']['output_dir']
    os.makedirs(output_dir, exist_ok=True)

    if category == "Textures":
        if phase == "audit":
            return textures_audit.run(config, output_dir)
        elif phase == "recommend":
            return textures_recommend.run(config, output_dir)
        elif phase == "apply":
            return textures_apply.run(config, output_dir)
        elif phase == "verify":
            return textures_verify.run(config, output_dir)
    elif category == "Meshes":
        if phase == "audit":
            return meshes_audit.run(config, output_dir)
        elif phase == "recommend":
            return meshes_recommend.run(config, output_dir)
        elif phase == "apply":
            return meshes_apply.run(config, output_dir)
        elif phase == "verify":
            return meshes_verify.run(config, output_dir)

    return {
        "phase": phase,
        "category": category,
        "scanned": 0,
        "changed": 0,
        "skipped": 0,
        "errors": 0,
        "artifacts": {},
        "summary": {}
    }
io_csv.py
import csv
import os
from pathlib import Path

def read_csv(file_path, encoding='utf-8'):
    if not os.path.exists(file_path):
        return []
    
    data = []
    try:
        with open(file_path, 'r', newline='', encoding=encoding) as csvfile:
            reader = csv.DictReader(csvfile)
            for row in reader:
                data.append(row)
    except Exception as e:
        print(f"Error reading CSV {file_path}: {e}")
        return []
    
    return data

def write_csv(file_path, fieldnames, rows, encoding='utf-8'):
    os.makedirs(os.path.dirname(file_path), exist_ok=True)
    
    try:
        with open(file_path, 'w', newline='', encoding=encoding) as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(rows)
    except Exception as e:
        print(f"Error writing CSV {file_path}: {e}")

def validate_csv_header(file_path, expected_fields):
    if not os.path.exists(file_path):
        return False
    
    try:
        with open(file_path, 'r', newline='', encoding='utf-8') as csvfile:
            reader = csv.reader(csvfile)
            header = next(reader, [])
            return set(header) >= set(expected_fields)
    except Exception:
        return False

def get_csv_field_value(row, field_name, default=None):
    return row.get(field_name, default)
utils.py
import os
from pathlib import Path

def resolve_enum(enum_cls, name, default=None):
    key = str(name).lower().replace("_","")
    for attr in dir(enum_cls):
        if not attr.startswith("__"):
            if attr.lower().replace("_","")==key:
                return getattr(enum_cls, attr)
    return default

def get_project_root():
    return os.getcwd()

def ensure_directory_exists(path):
    os.makedirs(path, exist_ok=True)

def format_path_for_unreal(path):
    formatted = path.replace('\\', '/')
    return formatted

def get_file_size(file_path):
    try:
        return os.path.getsize(file_path)
    except:
        return 0

def is_valid_asset_path(path):
    return path and path.startswith('/Game/') and not path.endswith('/')
SOptimizerPanel Implementation
SOptimizerPanel.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SOptimizerPanel : public SCompoundWidget
{
    SLATE_BEGIN_ARGS(SOptimizerPanel) {}
    SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs);

private:
    FReply OnAuditClicked();
    FReply OnApplyClicked();

    ECheckBoxState IsTexturesChecked() const;
    void OnTexturesChanged(ECheckBoxState NewState);

    ECheckBoxState IsMeshesChecked() const;
    void OnMeshesChanged(ECheckBoxState NewState);

    ECheckBoxState IsMaterialsChecked() const;
    void OnMaterialsChanged(ECheckBoxState NewState);

    ECheckBoxState IsLevelsChecked() const;
    void OnLevelsChanged(ECheckBoxState NewState);

    ECheckBoxState IsUseSelectionChecked() const;
    void OnUseSelectionChanged(ECheckBoxState NewState);

    FString GetIncludePaths() const;
    void OnIncludePathsChanged(const FText& NewText);

    FString GetExcludePaths() const;
    void OnExcludePathsChanged(const FText& NewText);

    TSharedRef<SWidget> OnGenerateProfileComboItem(FString Item) const;
    void OnProfileSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
    FText GetCurrentProfileText() const;

    TSharedRef<SWidget> OnGenerateRunModeComboItem(FString Item) const;
    void OnRunModeSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
    FText GetCurrentRunModeText() const;

private:
    UOptimizerSettings* OptimizerSettings;
    TArray<FString> TargetProfiles;
    TArray<FString> RunModes;
};
SOptimizerPanel.cpp
#include "SOptimizerPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SVerticalBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Framework/Notifications/SNotificationList.h"
#include "EditorStyleSet.h"
#include "OptimizerSettings.h"

void SOptimizerPanel::Construct(const FArguments& InArgs)
{
    OptimizerSettings = UOptimizerSettings::Get();

    // Initialize profiles
    TargetProfiles.Add(TEXT("PC_High"));
    TargetProfiles.Add(TEXT("Console"));
    TargetProfiles.Add(TEXT("Mobile_AR"));
    TargetProfiles.Add(TEXT("VR"));
    TargetProfiles.Add(TEXT("VirtualProduction"));
    TargetProfiles.Add(TEXT("Cinematic"));
    TargetProfiles.Add(TEXT("Authoring_Safe"));

    RunModes.Add(TEXT("Audit"));
    RunModes.Add(TEXT("Apply"));

    ChildSlot
    [
        SNew(SVerticalBox)

        // Header row
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.0f)
        [
            SNew(SGridPanel)
            .FillColumn(1, 1.0f)

            + SGridPanel::Slot(0, 0)
            .Padding(2.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Target Profile")))
            ]
            
            + SGridPanel::Slot(1, 0)
            .Padding(2.0f)
            [
                SNew(SComboBox<FString>)
                .OptionsSource(&TargetProfiles)
                .OnGenerateWidget(this, &SOptimizerPanel::OnGenerateProfileComboItem)
                .OnSelectionChanged(this, &SOptimizerPanel::OnProfileSelected)
                .Content()
                [
                    SNew(STextBlock)
                    .Text(this, &SOptimizerPanel::GetCurrentProfileText)
                ]
            ]

            + SGridPanel::Slot(0, 1)
            .Padding(2.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Categories")))
            ]
            
            + SGridPanel::Slot(1, 1)
            .Padding(2.0f)
            [
                SNew(SUniformGridPanel)
                .SlotPadding(4.0f)

                + SUniformGridPanel::Slot(0, 0)
                [
                    SNew(SCheckBox)
                    ..IsChecked(this, &SOptimizerPanel::IsTexturesChecked)
                    .OnCheckStateChanged(this, &SOptimizerPanel::OnTexturesChanged)
                    .Content()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Textures")))
                    ]
                ]

                + SUniformGridPanel::Slot(1, 0)
                [
                    SNew(SCheckBox)
                    ..IsChecked(this, &SOptimizerPanel::IsMeshesChecked)
                    .OnCheckStateChanged(this, &SOptimizerPanel::OnMeshesChanged)
                    .Content()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Meshes")))
                    ]
                ]

                + SUniformGridPanel::Slot(0, 1)
                [
                    SNew(SCheckBox)
                    ..IsChecked(this, &SOptimizerPanel::IsMaterialsChecked)
                    .OnCheckStateChanged(this, &SOptimizerPanel::OnMaterialsChanged)
                    .Content()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Materials")))
                    ]
                ]

                + SUniformGridPanel::Slot(1, 1)
                [
                    SNew(SCheckBox)
                    ..IsChecked(this, &SOptimizerPanel::IsLevelsChecked)
                    .OnCheckStateChanged(this, &SOptimizerPanel::OnLevelsChanged)
                    .Content()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Levels")))
                    ]
                ]
            ]

            + SGridPanel::Slot(0, 2)
            .Padding(2.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Scope")))
            ]
            
            + SGridPanel::Slot(1, 2)
            .Padding(2.0f)
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(SCheckBox)
                    ..IsChecked(this, &SOptimizerPanel::IsUseSelectionChecked)
                    .OnCheckStateChanged(this, &SOptimizerPanel::OnUseSelectionChanged)
                    .Content()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Use Selection")))
                    ]
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(SEditableTextBox)
                    .Text(this, &SOptimizerPanel::GetIncludePaths)
                    .OnTextChanged(this, &SOptimizerPanel::OnIncludePathsChanged)
                    .HintText(FText::FromString(TEXT("Include paths (comma separated)")))
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(SEditableTextBox)
                    .Text(this, &SOptimizerPanel::GetExcludePaths)
                    .OnTextChanged(this, &SOptimizerPanel::OnExcludePathsChanged)
                    .HintText(FText::FromString(TEXT("Exclude paths (comma separated)")))
                ]
            ]

            + SGridPanel::Slot(0, 3)
            .Padding(2.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Run Mode")))
            ]
            
            + SGridPanel::Slot(1, 3)
            .Padding(2.0f)
            [
                SNew(SComboBox<FString>)
                .OptionsSource(&RunModes)
                .OnGenerateWidget(this, &SOptimizerPanel::OnGenerateRunModeComboItem)
                .OnSelectionChanged(this, &SOptimizerPanel::OnRunModeSelected)
                .Content()
                [
                    SNew(STextBlock)
                    .Text(this, &SOptimizerPanel::GetCurrentRunModeText)
                ]
            ]
        ]

        // Main content area
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(4.0f)
        [
            SNew(SScrollBox)

            + SScrollBox::Slot()
            .Padding(4.0f)
            [
                SNew(SVerticalBox)

                // Textures accordion
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Textures")))
                ]

                // Meshes accordion
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Meshes")))
                ]

                // Materials accordion
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Materials")))
                ]

                // Levels accordion
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Levels")))
                ]
            ]
        ]

        // Footer buttons
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.0f)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            .HAlign(HAlign_Left)
            .Padding(4.0f)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Run Audit")))
                .OnClicked(this, &SOptimizerPanel::OnAuditClicked)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .HAlign(HAlign_Left)
            .Padding(4.0f)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Apply")))
                .OnClicked(this, &SOptimizerPanel::OnApplyClicked)
            ]

            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .HAlign(HAlign_Right)
            .Padding(4.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Last run: 0 assets changed")))
            ]
        ]
    ];
}

FReply SOptimizerPanel::OnAuditClicked()
{
    // This would call the Python audit function
    FString Phase = TEXT("audit");
    TArray<FString> Categories;
    if (IsTexturesChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Textures"));
    }
    if (IsMeshesChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Meshes"));
    }
    if (IsMaterialsChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Materials"));
    }
    if (IsLevelsChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Levels"));
    }

    UOptimizerRun* OptimizerRun = NewObject<UOptimizerRun>();
    OptimizerRun->Run(Phase, Categories);

    FNotificationInfo Info(FText::FromString(TEXT("Audit started...")));
    Info.bUseSuccessFailIcons = true;
    Info.ExpireDuration = 3.0f;
    FSlateNotificationManager::Get().AddNotification(Info);
    
    return FReply::Handled();
}

FReply SOptimizerPanel::OnApplyClicked()
{
    // This would call the Python apply function
    FString Phase = TEXT("apply");
    TArray<FString> Categories;
    if (IsTexturesChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Textures"));
    }
    if (IsMeshesChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Meshes"));
    }
    if (IsMaterialsChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Materials"));
    }
    if (IsLevelsChecked() == ECheckBoxState::Checked)
    {
        Categories.Add(TEXT("Levels"));
    }

    UOptimizerRun* OptimizerRun = NewObject<UOptimizerRun>();
    OptimizerRun->Run(Phase, Categories);

    FNotificationInfo Info(FText::FromString(TEXT("Apply started...")));
    Info.bUseSuccessFailIcons = true;
    Info.ExpireDuration = 3.0f;
    FSlateNotificationManager::Get().AddNotification(Info);
    
    return FReply::Handled();
}

ECheckBoxState SOptimizerPanel::IsTexturesChecked() const
{
    return (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Textures) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnTexturesChanged(ECheckBoxState NewState)
{
    if (NewState == ECheckBoxState::Checked)
    {
        OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Textures;
    }
    else
    {
        OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Textures);
    }
}

ECheckBoxState SOptimizerPanel::IsMeshesChecked() const
{
    return (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Meshes) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnMeshesChanged(ECheckBoxState NewState)
{
    if (NewState == ECheckBoxState::Checked)
    {
        OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Meshes;
    }
    else
    {
        OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Meshes);
    }
}

ECheckBoxState SOptimizerPanel::IsMaterialsChecked() const
{
    return (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Materials) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnMaterialsChanged(ECheckBoxState NewState)
{
    if (NewState == ECheckBoxState::Checked)
    {
        OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Materials;
    }
    else
    {
        OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Materials);
    }
}

ECheckBoxState SOptimizerPanel::IsLevelsChecked() const
{
    return (OptimizerSettings->CategoryMask & (uint8)EOptimizerCategory::Levels) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnLevelsChanged(ECheckBoxState NewState)
{
    if (NewState == ECheckBoxState::Checked)
    {
        OptimizerSettings->CategoryMask |= (uint8)EOptimizerCategory::Levels;
    }
    else
    {
        OptimizerSettings->CategoryMask &= ~((uint8)EOptimizerCategory::Levels);
    }
}

ECheckBoxState SOptimizerPanel::IsUseSelectionChecked() const
{
    return OptimizerSettings->bUseSelection ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOptimizerPanel::OnUseSelectionChanged(ECheckBoxState NewState)
{
    OptimizerSettings->bUseSelection = (NewState == ECheckBoxState::Checked);
}

FString SOptimizerPanel::GetIncludePaths() const
{
    return OptimizerSettings->IncludePathsCsv;
}

void SOptimizerPanel::OnIncludePathsChanged(const FText& NewText)
{
    OptimizerSettings->IncludePathsCsv = NewText.ToString();
}

FString SOptimizerPanel::GetExcludePaths() const
{
    return OptimizerSettings->ExcludePathsCsv;
}

void SOptimizerPanel::OnExcludePathsChanged(const FText& NewText)
{
    OptimizerSettings->ExcludePathsCsv = NewText.ToString();
}

TSharedRef<SWidget> SOptimizerPanel::OnGenerateProfileComboItem(FString Item) const
{
    return SNew(STextBlock)
    .Text(FText::FromString(Item));
}

void SOptimizerPanel::OnProfileSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
    if (SelectedItem.IsValid())
    {
        OptimizerSettings->TargetProfile = *SelectedItem;
    }
}

FText SOptimizerPanel::GetCurrentProfileText() const
{
    return FText::FromString(OptimizerSettings->TargetProfile);
}

TSharedRef<SWidget> SOptimizerPanel::OnGenerateRunModeComboItem(FString Item) const
{
    return SNew(STextBlock)
    .Text(FText::FromString(Item));
}

void SOptimizerPanel::OnRunModeSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
    if (SelectedItem.IsValid())
    {
        // Currently not used but can be added for future enhancements
    }
}

FText SOptimizerPanel::GetCurrentRunModeText() const
{
    return FText::FromString(TEXT("Audit")); // Default to Audit mode
}
Testing Strategy
Unit Tests
Texture Analysis: Test texture compression, LOD settings, and virtual texture handling
Mesh Analysis: Test LOD generation, Nanite toggling, and collision simplification
Material Analysis: Test normal map conversions, static switch cleanup, and sampler consolidation
Runtime Settings: Test scalability buckets, shadow maps, post-processing quality adjustments
Integration Tests
End-to-end Optimization: Run full optimization cycles for all categories
Python Bridge Stability: Ensure the bridge handles different Python scripts and data
UI Responsiveness: Validate that UI elements update correctly during operations
Safety Mechanisms: Test dry-run modes, max-changes caps, and revert snapshots
Performance Benchmarks
Optimization Speed: Measure time taken for audit, recommend, apply, and verify phases
Memory Usage: Monitor memory consumption during optimization processes
Regression Testing: Ensure that optimizations do not introduce regressions in asset quality or performance
Performance Optimization
Incremental Scans: Implement incremental scans to reduce processing time for unchanged assets
Batch Processing: Use batch operations for large datasets to improve performance
Threading and Asynchronous Loading: Leverage Unreal Engine's threading capabilities for parallel processing
Safety Implementation
Error Handling: Robust error handling with proper logging for all critical operations
Verification Systems: Implement verification systems to ensure correctness after optimizations
Revert Mechanisms: Provide full revert snapshots and history tracking to undo changes if necessary
Conclusion
This detailed implementation plan provides a comprehensive roadmap for developing the "MagicOptimizer" plugin. Each phase focuses on specific tasks, ensuring that all critical components are covered. By following this plan, you can build a robust and efficient asset optimization tool for Unreal Engine 5.6 that will significantly improve productivity for game developers by automating tedious optimization tasks while maintaining safety through comprehensive testing and verification mechanisms.

The plugin architecture supports extensibility for future features and maintains backward compatibility with existing projects, making it an essential tool for any serious Unreal Engine development workflow.

