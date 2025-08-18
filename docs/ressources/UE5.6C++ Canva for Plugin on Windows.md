---

# UE5.6 C++ Canvas for Plugin Authors on Windows

## 1. Foundations Your Agents Must Internalize

### Toolchain and IDE
- **Visual Studio 2022** is the supported default on Windows for UE5 series
- Rider and VS Code are also supported
- Install the C++ workload and Windows SDK
- [Epic Games Developers - Hardware and Software Specifications](https://dev.epicgames.com/documentation/en-us/unreal-engine/hardware-and-software-specifications-for-unreal-engine)

### Two-Stage Compile
- Every compile runs Unreal Header Tool (UHT) first to parse reflection macros and generate glue
- Then Unreal Build Tool (UBT) compiles C++
- If UHT fails, the build fails regardless of your C++ syntax
- [Epic Games Developers - Unreal Header Tool](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-header-tool-for-unreal-engine)

### Learning Resources
- Use Epic's 5.6 C++ section and API browser as the primary source of truth
- [Epic Games Developers - C++ Programming](https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-with-cplusplus-in-unreal-engine?application_version=5.6)
- [Microsoft Learn - VS Tools for Unreal](https://learn.microsoft.com/en-us/visualstudio/gamedev/unreal/get-started/vs-tools-unreal-uproject)

---

## 2. Minimal Plugin Anatomy for 5.6

### Folder Layout

```
MyOptimizationPlugin/
├── MyOptimizationPlugin.uplugin
├── Source/
│   ├── MyOptimizationPlugin/           # Runtime module
│   │   ├── MyOptimizationPlugin.Build.cs
│   │   ├── Public/
│   │   └── Private/
│   └── MyOptimizationPluginEditor/     # Editor module
│       ├── MyOptimizationPluginEditor.Build.cs
│       └── Private/
├── Resources/                           # Icons, Slate styles
└── Config/                              # Optional defaults for settings or CVars
```

### Descriptor File

**`MyOptimizationPlugin.uplugin`**

```json
{
  "FileVersion": 3,
  "VersionName": "1.0.0",
  "FriendlyName": "My Optimization Plugin",
  "Description": "Runtime optimizations and editor tooling",
  "Category": "Performance",
  "Modules": [
    { "Name": "MyOptimizationPlugin", "Type": "Runtime", "LoadingPhase": "Default" },
    { "Name": "MyOptimizationPluginEditor", "Type": "Editor", "LoadingPhase": "Default" }
  ]
}
```

Descriptor and module loading semantics are defined by the Projects module and descriptor classes, which also expose loading phases used by the plugin browser and module manager.

---

## 3. Build Rules That Never Bite You Later

### Runtime Module Build.cs

```csharp
// Source/MyOptimizationPlugin/MyOptimizationPlugin.Build.cs
using UnrealBuildTool;

public class MyOptimizationPlugin : ModuleRules
{
    public MyOptimizationPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new[] {
            "Projects"           // Plugin manager, descriptors
        });
    }
}
```

### Editor Module Build.cs

```csharp
// Source/MyOptimizationPluginEditor/MyOptimizationPluginEditor.Build.cs
using UnrealBuildTool;

public class MyOptimizationPluginEditor : ModuleRules
{
    public MyOptimizationPluginEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine",
            "Slate", "SlateCore",
            "ToolMenus",            // Modern editor menus
            "UnrealEd",             // Editor framework
            "Projects"
        });
    }
}
```

Modules and Build.cs responsibilities, plus Target rules, are codified in Epic docs. Use these patterns so UBT resolves include and link order correctly.

---

## 4. Module Entry Points That Are Safe and Deterministic

### Runtime Module

```cpp
// Source/MyOptimizationPlugin/Private/MyOptimizationPluginModule.cpp
#include "Modules/ModuleManager.h"
#include "MyOptimizationPluginLog.h"

class FMyOptimizationPluginModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        UE_LOG(LogMyOpt, Log, TEXT("Optimization runtime started"));
        // Register CVars, console commands, stat groups here
    }

    virtual void ShutdownModule() override
    {
        // Unregister everything you registered in StartupModule
    }
};

IMPLEMENT_MODULE(FMyOptimizationPluginModule, MyOptimizationPlugin)
```

### Log Category

```cpp
// Source/MyOptimizationPlugin/Public/MyOptimizationPluginLog.h
#pragma once
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMyOpt, Log, All);
```

```cpp
// Source/MyOptimizationPlugin/Private/MyOptimizationPluginLog.cpp
#include "MyOptimizationPluginLog.h"
DEFINE_LOG_CATEGORY(LogMyOpt);
```

`IModuleInterface` is the required surface. Always maintain symmetry between register and unregister operations. Use a dedicated log category.

---

## 5. Editor Menus and Commands in the Modern ToolMenus System

Add a toolbar button and a menu entry that triggers your optimization pass in the editor.

```cpp
// Source/MyOptimizationPluginEditor/Private/MyOptEditorModule.cpp
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandList.h"

class FMyOptCommands final : public TCommands<FMyOptCommands>
{
public:
    FMyOptCommands()
    : TCommands<FMyOptCommands>(
        TEXT("MyOpt"), NSLOCTEXT("MyOpt", "MyOpt", "My Opt"),
        NAME_None, TEXT("EditorStyle"))
    {}

    virtual void RegisterCommands() override
    {
        UI_COMMAND(RunPass, "Run Optimization", "Run analysis and apply fixes", EUserInterfaceActionType::Button, FInputChord());
    }

    TSharedPtr<FUICommandInfo> RunPass;
};

class FMyOptimizationPluginEditorModule final : public IModuleInterface
{
    TSharedPtr<FUICommandList> CommandList;

    virtual void StartupModule() override
    {
        FMyOptCommands::Register();
        CommandList = MakeShared<FUICommandList>();
        CommandList->MapAction(
            FMyOptCommands::Get().RunPass,
            FExecuteAction::CreateRaw(this, &FMyOptimizationPluginEditorModule::ExecuteRunPass)
        );

        UToolMenus::RegisterStartupCallback(
            FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMyOptimizationPluginEditorModule::RegisterMenus));
    }

    virtual void ShutdownModule() override
    {
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
        FMyOptCommands::Unregister();
    }

    void RegisterMenus()
    {
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
        FToolMenuSection& Section = Menu->AddSection("MyOpt", FText::FromString("Optimization"));
        Section.AddMenuEntry(FMyOptCommands::Get().RunPass);
    }

    void ExecuteRunPass()
    {
        // Do your optimization pass here
    }
};

IMPLEMENT_MODULE(FMyOptimizationPluginEditorModule, MyOptimizationPluginEditor)
```

ToolMenus and UI command mapping are the current best practice for editor extensibility in the 5.x series.

---

## 6. Reflection and Metadata Your Agents Must Use Correctly

### Required Macros
- Use `UCLASS`, `USTRUCT`, `UENUM`, `UINTERFACE`, `UFUNCTION`, `UPROPERTY`
- Generated headers must be included at the end of your header includes

### Typical UHT Pitfalls
1. **Missing generated header include** - Always include `"ClassName.generated.h"` last
2. **Code before includes** - Don't put code before includes that defines types UHT expects
3. **Templates with UPROPERTY** - UHT has no serializer for complex templates
4. **Missing Blueprint exposure** - Forgetting `BlueprintType` or `Blueprintable` when you need editor exposure

Epic's programming overview and gameplay class guides detail these requirements.

---

## 7. Strings and Names for Low-Cost Hot Paths

Choose the right type per use case:

- **`FName`** for IDs and keys - Global table backed, very cheap to compare
- **`FString`** for mutable text
- **`FText`** for localized user-facing text

Epic documents these choices clearly in their string handling guide.

---

## 8. Console Variables and Commands Your Plugin Will Expose

For fast iteration and profiling toggles, register CVars and commands in `StartupModule`, and unregister in `ShutdownModule`.

```cpp
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarMyOpt_Enabled(
    TEXT("myopt.Enabled"),
    1,
    TEXT("Enable the optimization subsystem at runtime"),
    ECVF_Default);

static FAutoConsoleCommand CmdMyOptRun(
    TEXT("myopt.Run"),
    TEXT("Execute the optimization pass now"),
    FConsoleCommandDelegate::CreateLambda([]()
    {
        // Call into your runtime subsystem or service
    }));
```

Use the Console Manager API and auto-registering types for commands. List and manage through the Console Variables Editor.

---

## 9. Subsystems for Clean Lifetimes

Prefer subsystems to singletons:

- **Runtime control**: Use `UEngineSubsystem` or `UGameInstanceSubsystem`
- **Editor-only state**: Use an editor module or `UEditorSubsystem`

```cpp
// Runtime optimization service
UCLASS()
class UMyOptEngineSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void RunOptimizationPass();
};
```

Subsystems give you automatic lifetime hooks scoped to engine, world, game instance, or editor.

---

## 10. Tasks and Concurrency That Do Not Stall the Game Thread

Your optimization work should be asynchronous. For 5.6, prefer the Tasks System under `UE::Tasks` for structured jobs, or use `Async` helpers for simple fire-and-forget work.

### Simple Launch on Background Thread, Then Marshal to Game Thread

```cpp
#include "Tasks/Task.h"
#include "Async/Async.h"

void UMyOptEngineSubsystem::RunOptimizationPass()
{
    UE::Tasks::Launch(UE_SOURCE_LOCATION, [this]()
    {
        // Heavy scanning work - no touching UObjects without proper marshaling
        FOptResult Result = DoHeavyScan();

        AsyncTask(ENamedThreads::GameThread, [this, Result]()
        {
            ApplyResultOnGameThread(Result);
        });
    });
}
```

### Important Warnings

- **Do not touch or create UObjects off the game thread**
- Use `FTSTicker` or latent actions if you must poll regularly

For asset streaming, use Asset Manager async loads with soft references, which are designed for non-blocking behavior.

For deeper profiling of tasks and work stealing, use Unreal Insights Task Graph view.

---

## 11. Logging and Stats You Will Actually Use

### Logging

```cpp
UE_LOG(LogMyOpt, Display, TEXT("Found %d actors to process"), Count);
```

### Add a Stat Group for Quick Counters

```cpp
// MyOptStats.h
DECLARE_STATS_GROUP(TEXT("MyOpt"), STATGROUP_MyOpt, STATCAT_Advanced);
DECLARE_DWORD_COUNTER_STAT(TEXT("Actors Scanned"), STAT_MyOptActors, STATGROUP_MyOpt);
```

```cpp
// Usage
INC_DWORD_STAT_BY(STAT_MyOptActors, Actors.Num());
```

Use your own log category and stat group so Insights can filter your data.

---

## 12. Project Settings Surface for Users, Backed by Config and CVars

Expose a neat Project Settings page using `ISettingsModule` and a `UDeveloperSettings` object. Bind to CVars when you want live toggles.

```cpp
// Settings object
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="My Optimization"))
class UMyOptSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Config, Category="General")
    bool bEnabledByDefault = true;
};
```

Register in your editor module startup. The config system saves to `DefaultEngine.ini` or `DefaultGame.ini` depending on the category.

---

## 13. Performance Patterns for an Optimization Plugin

### Key Principles
- Prefer soft object references and Asset Manager queues for data-heavy passes
- Replace per-tick work with event-driven hooks, or schedule with `FTSTicker` and generous intervals
- Avoid allocations in hot loops - use `reserve` on `TArray`, reuse small buffers, prefer `FName` for keys
- Wrap expensive operations with scoped `QUICK_SCOPE_CYCLE_COUNTER` and verify in Insights under Stat and Task views
- Provide CVars for every heavy feature so users can toggle and profile quickly: `myopt.Enabled`, `myopt.Verbose`, `myopt.MaxPerFrame`

Epic provides the command line, console variables, and Insights references you will need.

---

## 14. Windows Packaging and Distribution of Your Plugin

### Local Test Build of the Plugin

Run from `Engine/Build/BatchFiles` or via the editor's commandlet runner:

```batch
RunUAT.bat BuildPlugin ^
 -Plugin="D:\Src\MyProject\Plugins\MyOptimizationPlugin\MyOptimizationPlugin.uplugin" ^
 -Package="D:\Packages\MyOptimizationPlugin"
```

This produces a redistributable folder with Binaries, Intermediate cleaned, and a cooked layout suited for Marketplace or manual distribution.

### Third-Party Library Inclusion

Place third-party code under `Source/ThirdParty/<LibName>` with a dedicated `<LibName>.Build.cs` that sets:
- `PublicIncludePaths`
- `PublicAdditionalLibraries`
- `RuntimeDependencies` for staged DLLs

Prefer static where licensing allows, and ensure architecture-correct libs for Win64. Epic's third-party guide outlines patterns and staging rules.

---

## 15. Stable Cross-Version Code for 5.6

If your agents must straddle minor versions, gate on engine version macros from `Version.h`, or query `FEngineVersion`. Keep conditionals shallow.

```cpp
#include "Misc/EngineVersion.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
// 5.6+ only path
#endif
```

Engine version components are exposed in `FEngineVersion` and macros for major, minor, patch are provided.

---

## 16. Clean Example Features Your Plugin Can Ship Today

### A. Scene Scanning Optimization Pass

```cpp
void UMyOptEngineSubsystem::RunOptimizationPass()
{
    UE::Tasks::Launch(UE_SOURCE_LOCATION, [this]()
    {
        TArray<TWeakObjectPtr<AActor>> Candidates;

        // Gather on game thread only
        AsyncTask(ENamedThreads::GameThread, [this, Candidates=MoveTemp(Candidates)]() mutable
        {
            UWorld* World = GetWorld();
            for (TActorIterator<AActor> It(World); It; ++It)
            {
                if (It->IsHidden() || !It->IsRootComponentMovable())
                {
                    Candidates.Add(*It);
                }
            }

            // Kick background evaluation
            UE::Tasks::Launch(UE_SOURCE_LOCATION, [Candidates=MoveTemp(Candidates)]()
            {
                // Evaluate and collect changes
                FOptResult Result = Evaluate(Candidates);

                AsyncTask(ENamedThreads::GameThread, [Result]()
                {
                    ApplyResultOnGameThread(Result);
                    UE_LOG(LogMyOpt, Display, TEXT("Optimization applied"));
                });
            });
        });
    });
}
```

Uses Tasks for heavy work and marshals back to the game thread before touching actors.

### B. Editor Menu That Runs the Pass and Shows Counts
Already provided in Section 5. Your command calls the subsystem method.

### C. CVars to Throttle Per-Frame Work
Already provided in Section 8. Expose `myopt.MaxPerFrame` and read it inside your pass to limit work.

---

## 17. Common Pitfalls and How Your Agents Avoid Them

### Critical Issues to Avoid

1. **Building editor-only features into runtime modules or shipping targets**
   - Fix by splitting into an editor module and marking module types correctly

2. **Touching UObjects or components from worker threads**
   - Always marshal back to the game thread for any UObject ownership or mutation
   - Use Tasks plus `AsyncTask` to switch threads

3. **Missing generated include at the end of headers**
   - Every reflected header must end with `#include "ClassName.generated.h"`
   - That line must come last among includes

4. **Overusing FString on hot paths instead of FName**
   - Prefer FName for IDs and map keys
   - Use FString only for mutable text or user output

5. **Dirty shutdown because you never unregistered commands or CVars**
   - Unregister in `ShutdownModule` and avoid static lifetime surprises

6. **Packaging fails because third-party DLLs are not staged**
   - Add `RuntimeDependencies.Add` in your third-party Build.cs so files are copied for shipping

---

## 18. Fast Reference Links Your Agents Will Reuse

### Essential Documentation
- [Programming Overview and API Browser for 5.6](https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-with-cplusplus-in-unreal-engine?application_version=5.6)
- [Modules and Build Rules Guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules)
- [Editor Menus and Commands APIs](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Developer/ToolMenus/UToolMenus)
- [Console Variables and Commands](https://dev.epicgames.com/documentation/en-us/unreal-engine/console-variables-cplusplus-in-unreal-engine)
- [Tasks System Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/tasks-systems-in-unreal-engine)
- [UHT and Reflection Basics](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-header-tool-for-unreal-engine)
- [Windows Setup and Requirements](https://dev.epicgames.com/documentation/en-us/unreal-engine/hardware-and-software-specifications-for-unreal-engine)
- [Build Plugin with UAT](https://dev.epicgames.com/community/learning/tutorials/qz93/unreal-engine-building-plugins)

---

## 19. Style and Conventions That Keep Your Codebase Durable

### Coding Standards
- Follow Epic's C++ standard for naming, formatting, and safety checks
- This will save you from many subtle build and runtime issues
- [Epic C++ Coding Standard](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine)

### Asset Naming
- Use asset and content naming guidance from Epic or established community guides
- Keep user projects organized with consistent conventions
- [Recommended Asset Naming Conventions](https://dev.epicgames.com/documentation/en-us/unreal-engine/recommended-asset-naming-conventions-in-unreal-engine-projects)

---

## 20. What to Do Next

### Implementation Steps
1. **Scaffold the plugin** with the exact files above
2. **Wire the editor command** and subsystem
3. **Add CVars** and a Project Settings object
4. **Profile with Insights** and expose stat counters
5. **Package with `RunUAT.bat BuildPlugin`** and test on a blank project

---

*This document provides a comprehensive guide for developing UE5.6 C++ optimization plugins on Windows, following Epic's best practices and avoiding common pitfalls.*
