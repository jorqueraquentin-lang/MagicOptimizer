# UE 5.6 C++ Plugins — Deep Addendum (Windows)

---

## Table of Contents
1. [What This Adds (Delta)](#what-this-adds-delta)
2. [Build System Essentials for Plugin Authors](#build-system-essentials-for-plugin-authors)
3. [UPlugin Fields That Affect Packaging & Runtime](#uplugin-fields-that-affect-packaging--runtime)
4. [Reflection Correctness & MinimalAPI](#reflection-correctness--minimalapi)
5. [Strings & Localization You Actually Need](#strings--localization-you-actually-need)
6. [Subsystems & Orderly Startup Hooks](#subsystems--orderly-startup-hooks)
7. [Console Variables (CVars) That Power Optimizers](#console-variables-cvars-that-power-optimizers)
8. [Multithreading: Pick the Right Primitive](#multithreading-pick-the-right-primitive)
9. [Render Thread Access & RDG](#render-thread-access--rdg)
10. [PSO Caching (PC DX12 Stutter Control)](#pso-caching-pc-dx12-stutter-control)
11. [Asset Loading & Data Footprint](#asset-loading--data-footprint)
12. [Config Hierarchy & Precedence](#config-hierarchy--precedence)
13. [Profiling & Automation (Headless-Friendly)](#profiling--automation-headless-friendly)
14. [Editor UX Hooks Your Plugin Likely Needs](#editor-ux-hooks-your-plugin-likely-needs)
15. [Minimal, Correct Patterns (Ready to Paste)](#minimal-correct-patterns-ready-to-paste)
16. [Common "Gotchas" Your Agents Should Pre-empt](#common-gotchas-your-agents-should-pre-empt)
17. [Source Map (Quick Jump)](#source-map-quick-jump)
18. [Final Checklists](#final-checklists)

---

## 1. What This Adds (Delta)

This addendum provides advanced knowledge beyond the basics:

- **UBT & module/target rules** that affect binary compatibility, build speed, & IWYU
- **UPlugin fields** that impact platform packaging, load order, & distribution
- **Reflection correctness** (specifiers, MinimalAPI/export), **text types** (FName/FString/FText), and **localization** do/don't
- **Subsystem patterns** (Engine/Editor/Game/LocalPlayer) for plugin startup/shutdown
- **Console variables** that back your optimizer (creation, binding, scalability interplay)
- **Multithreading & task graph** (UE::Tasks vs AsyncTask vs FRunnable/ThreadPools)
- **Render thread access & RDG**—how to add passes safely from a plugin
- **PSO caching** (stable/recorded/bundled caches) for stutter-free DX12 on PC
- **Asset Manager & Asset Registry** (soft refs, primary assets, async loads)
- **Config hierarchy & precedence** (Default\*.ini vs Saved/*.ini vs Base*.ini)
- **Profiling & tracing** (Insights/Trace/LLM/CSV) as automation-friendly CLI
- **Editor UX hooks** (ToolMenus, Detail/Property customizations, tabs, Slate widgets)

---

## 2. Build System Essentials for Plugin Authors

### 2.1 Modules (.build.cs) and Targets (.target.cs)

- **Modules** are defined via `ModuleRules` in `.build.cs` and control PCHs, dependencies, IWYU/unity choices, etc.
- **Targets** are `TargetRules` in `.target.cs` and select overall build shape (Editor/Game/Server/Program)
- [Epic Games Developers - Module Properties](https://dev.epicgames.com/documentation/en-us/unreal-engine/module-properties-in-unreal-engine?utm_source=chatgpt.com)

#### Skeleton (Runtime Module)

```csharp
// FooOpt/FooOpt.Build.cs
using UnrealBuildTool;

public class FooOpt : ModuleRules 
{
    public FooOpt(ReadOnlyTargetRules Target) : base(Target) 
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs; // required under IWYU
        bUseUnity = true;                                 // flip per-module if needed
        
        PublicDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine"
        });
        
        PrivateDependencyModuleNames.AddRange(new[] {
            "Projects", "DeveloperSettings"
        });
    }
}
```

> **Tip**: UBT options that affect cache/graph invalidation (e.g., toggling IWYU/unity) regenerate the UBT Makefile; `-gather` or re-generate project files can force it.
> [Epic Games Developers - Build Configuration](https://dev.epicgames.com/documentation/en-us/unreal-engine/build-configuration-for-unreal-engine?application_version=5.6&utm_source=chatgpt.com)

#### Target Switches You'll Actually Use (Editor Target)
Examples that frequently matter for tooling/optimizer plugins:

- **`DisableUnityBuildForModules`**, **`DisableMergingModuleAndGeneratedFilesInUnityFiles`**
- **`bShowIncludes`**, **`InternalPluginDependencies`**

These live on `TargetRules` and can be set per target.
[Epic Games Developers - Target Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-build-tool-target-reference?utm_source=chatgpt.com)

### 2.2 UBT Overview & Configs

- **UBT orchestrates compile/link for all targets/configs**
- **Read BuildConfiguration for user-configurable knobs**
- [Epic Games Developers - Unreal Build Tool](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-build-tool-in-unreal-engine?application_version=5.6&utm_source=chatgpt.com)

---

## 3. UPlugin Fields That Affect Packaging & Runtime

Key fields from `FPluginDescriptor` (the in-code descriptor for `.uplugin`):

- **`"SupportedTargetPlatforms"`**: Ensure only Win64 (and any others you truly support)
- **Mismatches here cause staging failures**
- [Epic Games Developers - SupportedTargetPlatforms](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Projects/FPluginDescriptor/SupportedTargetPlatforms?utm_source=chatgpt.com)
- [Stack Overflow - Platform Mismatch](https://stackoverflow.com/questions/78060237/interchangeeditor-uplugin-is-referenced-via-project-with-a-mismatched-supported?utm_source=chatgpt.com)

- **`"EnabledByDefault"`**, **`"CanContainContent"`**, **`"LoadingPhase"`** (e.g., `PreDefault`, `Default`, `PostEngineInit`)
- **Choose the earliest phase only when you must bind early**
- [Epic Games Developers - FPluginDescriptor](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Projects/FPluginDescriptor?utm_source=chatgpt.com)

#### Minimal Windows-Only Descriptor

```json
{
  "FileVersion": 3,
  "VersionName": "1.0.0",
  "FriendlyName": "Foo Optimizer",
  "SupportedTargetPlatforms": ["Win64"],
  "Modules": [
    { "Name": "FooOpt", "Type": "Runtime", "LoadingPhase": "Default" },
    { "Name": "FooOptEditor", "Type": "Editor" }
  ]
}
```

#### Packaging (RunUAT)

```powershell
& "Engine/Build/BatchFiles/RunUAT.bat" BuildPlugin `
  -plugin="D:\Src\FooOpt\FooOpt.uplugin" `
  -package="D:\Artifacts\FooOpt_Win64" `
  -targetplatform=Win64 -CrashReporter
```

**Use BuildPlugin to ship a self-contained zip; the result contains only what the consumer needs.**
[Epic Games Developers - Logging](https://dev.epicgames.com/documentation/en-us/unreal-engine/logging-in-unreal-engine?utm_source=chatgpt.com)

**General plugin dev/management overview** is here.
[Epic Games Developers - Plugins](https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine?utm_source=chatgpt.com)

---

## 4. Reflection Correctness (UCLASS/UFUNCTION/UPROPERTY) & MinimalAPI

- **UPROPERTY/UFUNCTION/UCLASS specifiers**: Always pick the minimal exposure required (e.g., `BlueprintCallable` only if needed)
- **Specifiers catalogues**: Properties, functions, classes, and metadata
- [Epic Games Developers - UProperties](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-uproperties?utm_source=chatgpt.com)

- **MinimalAPI**: Export only a thin ABI from your module; this reduces DLL surface and link churn
- **Use full API export (`FOOOPT_API`) for types you instantiate across modules**
- **Use `MinimalAPI` for types primarily referenced via UPROPERTY/Blueprint**
- [Epic Games Developers - Class Specifiers](https://dev.epicgames.com/documentation/en-us/unreal-engine/class-specifiers?application_version=5.6&utm_source=chatgpt.com)
- [DeepWiki - MinimalAPI](https://deepwiki.com/fjz13/UnrealSpecifiers/2.1-minimalapi-and-api-exposure?utm_source=chatgpt.com)
- [GitHub - MinimalAPI](https://github.com/fjz13/UnrealSpecifiers/blob/main/Doc/en/Specifier/UCLASS/UHT/MinimalAPI/MinimalAPI.md?utm_source=chatgpt.com)

#### Common Compile/Link Pitfalls Your Agents Should Check

- **Headers must respect IWYU**: Include what you use; keep generated include last in the header block
- **IWYU enforced in UE5 builds**
- [Epic Games Developers - Automation Spec](https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-spec-in-unreal-engine?utm_source=chatgpt.com)

- **If a `UCLASS` is not exported and instantiated from another module, you'll hit unresolved externals**
- **Either export its class or mark it `MinimalAPI` and provide exported factory functions**
- [GitHub - MinimalAPI](https://github.com/fjz13/UnrealSpecifiers/blob/main/Doc/en/Specifier/UCLASS/UHT/MinimalAPI/MinimalAPI.md?utm_source=chatgpt.com)

---

## 5. Strings & Localization You Actually Need in an Optimizer

- **Use `FName`** for identifiers/keys (fast compare, name table-backed)
- [Epic Games Developers - FName](https://dev.epicgames.com/documentation/en-us/unreal-engine/fname-in-unreal-engine?utm_source=chatgpt.com)

- **Use `FString`** for mutable text processing
- [Epic Games Developers - FString](https://dev.epicgames.com/documentation/en-us/unreal-engine/fstring-in-unreal-engine?utm_source=chatgpt.com)

- **Use `FText`** for user-facing/localized text only (UI, messages)
- **Prefer `LOCTEXT`/`NSLOCTEXT`**
- [Epic Games Developers - Text Localization](https://dev.epicgames.com/documentation/en-us/unreal-engine/text-localization-in-unreal-engine?utm_source=chatgpt.com)

**Changing culture via code/Blueprint is possible; avoid stomping user/editor settings with `SetCurrentCulture` in engine code.**
[Epic Games Developers - SetCurrentCulture](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Internationalization/FInternationalization/SetCurrentCulture?utm_source=chatgpt.com)

---

## 6. Subsystems & Orderly Startup Hooks

Pick the smallest scope that fits:

- **UEngineSubsystem/UEditorSubsystem/UGameInstanceSubsystem/ULocalPlayerSubsystem**
- **These provide predictable lifecycle & auto-instancing**
- **Register console variables, tick sources, or menu entries here instead of globals**
- [Epic Games Developers - ToolMenus](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Developer/ToolMenus?utm_source=chatgpt.com)

---

## 7. Console Variables (CVars) That Power Optimizers

- **Create static CVars using `TAutoConsoleVariable`** or bind to member state via `FAutoConsoleVariableRef`
- **This lets profiles & runtime tweaks flow through without polling**
- [Epic Games Developers - TAutoConsoleVariable](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/HAL/TAutoConsoleVariable?utm_source=chatgpt.com)

- **Wire to scalability and project settings sanely**
- **Know that scalability presets often override resolution/quality CVars** (e.g., `r.ScreenPercentage`)
- [Epic Games Developers - Scalability Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/scalability-reference-for-unreal-engine?utm_source=chatgpt.com)

#### Pattern

```cpp
// Globals in FooOpt module (FooOptCvars.cpp)
static int32 GFooBudgetMB = 512;
static FAutoConsoleVariableRef CVarFooBudgetMB(
    TEXT("foo.BudgetMB"), GFooBudgetMB,
    TEXT("Memory budget used by Foo optimizer in MB"), FConsoleVariableDelegate(),
    ECVF_Default);
```

**CVar reference & commands** lists for UE 5.6 here; C++ API for CVars here.
[Epic Games Developers - Console Variables Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-console-variables-reference?utm_source=chatgpt.com)

---

## 8. Multithreading: Pick the Right Primitive

- **UE::Tasks (preferred)**: Structured tasks with priorities; integrates with Insights' Task Graph
- **Great for granular work**
- [Epic Games Developers - UEngineSubsystem](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Subsystems/UEngineSubsystem?utm_source=chatgpt.com)

- **`AsyncTask(ENamedThreads::…)`**: Jump work to a named thread (e.g., back to GameThread)
- **Useful for thread-affinity fixes**
- [Epic Games Developers - ENamedThreads](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Async/ENamedThreads__Type?utm_source=chatgpt.com)

- **`FRunnable` & ThreadPools**: Explicit worker threads; only when you need full control (long-lived, dedicated IO, etc.)
- [Epic Games Developers - FRunnable](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/HAL/FRunnable?utm_source=chatgpt.com)

---

## 9. Render Thread Access & RDG (Render Dependency Graph)

- **Never touch RHI resources from the game thread**
- **Use `ENQUEUE_RENDER_COMMAND` or better, RDG (`FRDGBuilder` + `AddPass`) to stage work**
- **Lifetimes, barriers, and culling are handled by RDG when you declare pass parameters correctly**
- [Epic Games Developers - FRDGBuilder](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/RenderCore/FRDGBuilder?utm_source=chatgpt.com)

> **For compute/raster passes, build your graph on render thread, declare resources in pass params, then `Execute()`.**
> [Epic Games Developers - FRDGBuilder](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/RenderCore/FRDGBuilder?utm_source=chatgpt.com)

---

## 10. PSO Caching (PC DX12 Stutter Control)

For Windows DX12:

- **Prefer PSO Precaching/Stable Cache for common paths**
- **Generate Bundled PSO caches for shipping**
- **Ray tracing PSOs aren't bundled; compute PSOs are generated during cook** (configurable via CVars)
- [Epic Games Developers - Bundled PSO Caches](https://dev.epicgames.com/documentation/en-us/unreal-engine/manually-creating-bundled-pso-caches-in-unreal-engine?utm_source=chatgpt.com)

- **Know the two cache types (Recorded vs Stable), and that bundling focuses on Graphics PSOs**
- **Official guides and deep dives**
- [Epic Games Developers - PSO Caching](https://dev.epicgames.com/documentation/en-us/unreal-engine/optimizing-rendering-with-pso-caches-in-unreal-engine?utm_source=chatgpt.com)

---

## 11. Asset Loading & Data Footprint

- **Use Asset Registry (metadata only) to query/filter without loading packages**
- [Epic Games Developers - Asset Registry](https://dev.epicgames.com/documentation/en-us/unreal-engine/asset-registry-in-unreal-engine?application_version=5.6&utm_source=chatgpt.com)

- **Use UAssetManager to async load primary assets and keep handles for lifetime control**
- **Prefer soft references in content to decouple**
- [Epic Games Developers - UAssetManager](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Engine/UAssetManager?utm_source=chatgpt.com)

---

## 12. Config Hierarchy & Precedence (Avoid "It Worked in Editor")

**Read order**:
1. Engine *Base*.ini
2. Platform *Base*.ini  
3. Project **Default*.ini** ← **Ship project settings here**
4. User/profile **Saved/Config/*/*.ini** (highest precedence at runtime)

**Don't try to overwrite engine `Base*.ini`**
- [Epic Games Developers - Configuration Files](https://dev.epicgames.com/documentation/en-us/unreal-engine/configuration-files-in-unreal-engine?utm_source=chatgpt.com)
- [ikrima.dev - Config Files](https://ikrima.dev/ue4guide/application-os-generic-platform/config-files/?utm_source=chatgpt.com)

---

## 13. Profiling & Automation (Headless-Friendly)

- **Unreal Insights + Trace**: Start traces from the status bar (Editor) or headless with `-trace=<channels>`
- **Open `.utrace` files in the Session Browser**
- **Export CSV from CLI for CI bots**
- [Epic Games Developers - Unreal Insights](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine?utm_source=chatgpt.com)

**Useful docs**: Trace Quick Start, Task Graph Insights, Automated CSV exporters
- [Epic Games Developers - Trace Quick Start](https://dev.epicgames.com/documentation/en-us/unreal-engine/trace-quick-start-guide-in-unreal-engine?utm_source=chatgpt.com)

---

## 14. Editor UX Hooks Your Plugin Likely Needs

- **ToolMenus**: Add menu/toolbar items with C++ (Editor module StartupModule)
- [Epic Games Developers - Plugins](https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine?utm_source=chatgpt.com)

- **Details panel**: Per-type layout via `IDetailCustomization` or per-property via `IPropertyTypeCustomization`
- [Epic Games Developers - Details Panel Customizations](https://dev.epicgames.com/documentation/en-us/unreal-engine/details-panel-customizations-in-unreal-engine?utm_source=chatgpt.com)

- **Slate tabs**: Register a Nomad Tab spawner with `FGlobalTabmanager::RegisterNomadTabSpawner`, then invoke by TabId
- [Epic Games Developers - RegisterNomadTabSpawner](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Slate/Framework/Docking/FGlobalTabmanager/RegisterNomadTabSpawner?utm_source=chatgpt.com)

- **Custom Slate**: Extend `SCompoundWidget` (Include `Slate` & `SlateCore` in module deps)
- [Epic Games Developers - SCompoundWidget](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/SlateCore/Widgets/SCompoundWidget?utm_source=chatgpt.com)

---

## 15. Minimal, Correct Patterns (Ready to Paste)

### 15.1 Developer Settings + CVars Bridge

```cpp
UCLASS(Config=EditorPerProjectUserSettings, defaultconfig, MinimalAPI)
class UFooOptSettings : public UDeveloperSettings 
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Config, Category="Foo", meta=(ClampMin="64", ClampMax="8192"))
    int32 BudgetMB = 512;
};
```

```cpp
// On module startup: bind the editable setting to the CVar
static int32 GFooBudgetMB = GetDefault<UFooOptSettings>()->BudgetMB;
static FAutoConsoleVariableRef CVarFooBudgetMB(
    TEXT("foo.BudgetMB"), GFooBudgetMB,
    TEXT("..."), FConsoleVariableDelegate(), ECVF_Default);
```

> **`UDeveloperSettings` lives in Editor/Project Settings and persists to config; the CVar gives runtime override.**
> [Epic Games Developers - Metadata Specifiers](https://dev.epicgames.com/documentation/en-us/unreal-engine/metadata-specifiers-in-unreal-engine?utm_source=chatgpt.com)

### 15.2 UE::Tasks + Safe Hop to GameThread

```cpp
#include "Async/Async.h"
#include "Tasks/Task.h"

UE::Tasks::FTask LaunchHeavy() 
{
    return UE::Tasks::Launch(TEXT("FooBuildClusters"), [](){
        // work...
        AsyncTask(ENamedThreads::GameThread, [](){ /* touch UObjects here */ });
    });
}
```

[Epic Games Developers - UEngineSubsystem](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Subsystems/UEngineSubsystem?utm_source=chatgpt.com)

### 15.3 RDG "Hello Pass" Shape

```cpp
// In a render-thread context (e.g., via ENQUEUE_RENDER_COMMAND), or inside renderer hook:
FRDGBuilder GraphBuilder(RHICmdList);
{
    struct FParams : public FRenderGraphPassParameters {} *Params = GraphBuilder.AllocParameters<FParams>();
    GraphBuilder.AddPass(RDG_EVENT_NAME("FooOptPass"), Params, ERDGPassFlags::None,
        [](FRHICommandListImmediate& RHICmdList){ /* issue RHI work */ });
}
GraphBuilder.Execute();
```

[Epic Games Developers - FRDGBuilder](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/RenderCore/FRDGBuilder?utm_source=chatgpt.com)

### 15.4 PSO: From Run to Bundled Cache (DX12)

1. **Collect PSOs during play/tests → build stable cache**
2. **Bundle for shipping** (graphics PSOs; compute auto-generated at cook; RT not supported)

**Docs and caveats here.**
[Epic Games Developers - PSO Caching](https://dev.epicgames.com/documentation/en-us/unreal-engine/optimizing-rendering-with-pso-caches-in-unreal-engine?utm_source=chatgpt.com)

---

## 16. Common "Gotchas" Your Agents Should Pre-empt

- **Missing platform in `.uplugin`** causes staging failure or silent non-load on packaged builds
- **Verify `SupportedTargetPlatforms`**
- [Epic Games Developers - SupportedTargetPlatforms](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Projects/FPluginDescriptor/SupportedTargetPlatforms?utm_source=chatgpt.com)

- **Over-exporting (no `MinimalAPI`) explodes your ABI; under-exporting breaks instantiation across modules**
- **Check unresolved externals for `GetPrivateStaticClass`**
- [GitHub - MinimalAPI](https://github.com/fjz13/UnrealSpecifiers/blob/main/Doc/en/Specifier/UCLASS/UHT/MinimalAPI/MinimalAPI.md?utm_source=chatgpt.com)

- **Touching UObjects off GameThread—always marshal via `AsyncTask(GameThread)` or engine delegates**
- [Epic Games Developers - ENamedThreads](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Async/ENamedThreads__Type?utm_source=chatgpt.com)

- **RDG misuse (allocations/lifetimes) when not declaring pass parameters/resources → undefined behavior**
- **Let RDG manage lifetimes/barriers**
- [Epic Games Developers - FRDGBuilder](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/RenderCore/FRDGBuilder?utm_source=chatgpt.com)

- **Scalability vs CVar fights (e.g., `r.ScreenPercentage`)**—your plugin should either own it (write after preset) or observe changes
- [Epic Games Developers - Scalability Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/scalability-reference-for-unreal-engine?utm_source=chatgpt.com)

- **Config writes**: Ship **Default*.ini**; expect runtime/user overrides in **Saved/Config** to win
- **Don't edit `Base*.ini`**
- [Epic Games Developers - Configuration Files](https://dev.epicgames.com/documentation/en-us/unreal-engine/configuration-files-in-unreal-engine?utm_source=chatgpt.com)

---

## 17. Source Map (Quick Jump)

### Essential Documentation Links

- **Modules & Targets**: [Module Properties](https://dev.epicgames.com/documentation/en-us/unreal-engine/module-properties-in-unreal-engine?utm_source=chatgpt.com); [Target Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-build-tool-target-reference?utm_source=chatgpt.com); [UBT Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-build-tool-in-unreal-engine?application_version=5.6&utm_source=chatgpt.com)

- **Plugins & Packaging**: [Plugins Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine?utm_source=chatgpt.com); [FPluginDescriptor](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Projects/FPluginDescriptor?utm_source=chatgpt.com); [SupportedTargetPlatforms](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Projects/FPluginDescriptor/SupportedTargetPlatforms?utm_source=chatgpt.com); [BuildPlugin UAT](https://dev.epicgames.com/documentation/en-us/unreal-engine/logging-in-unreal-engine?utm_source=chatgpt.com)

- **Reflection**: [UProperties](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-uproperties?utm_source=chatgpt.com); [UFunctions](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-ufunctions?utm_source=chatgpt.com); [Class Specifiers](https://dev.epicgames.com/documentation/en-us/unreal-engine/class-specifiers?application_version=5.6&utm_source=chatgpt.com); [Metadata](https://dev.epicgames.com/documentation/en-us/unreal-engine/metadata-specifiers-in-unreal-engine?utm_source=chatgpt.com); [MinimalAPI Deep Dives](https://github.com/fjz13/UnrealSpecifiers/blob/main/Doc/en/Specifier/UCLASS/UHT/MinimalAPI/MinimalAPI.md?utm_source=chatgpt.com)

- **Strings & Localization**: [String Handling](https://dev.epicgames.com/documentation/en-us/unreal-engine/string-handling-in-unreal-engine?utm_source=chatgpt.com); [FText](https://dev.epicgames.com/documentation/en-us/unreal-engine/text-localization-in-unreal-engine?utm_source=chatgpt.com); [SetCurrentCulture](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Internationalization/FInternationalization/SetCurrentCulture?utm_source=chatgpt.com)

- **Subsystems**: [Subsystems in UE](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Developer/ToolMenus?utm_source=chatgpt.com)

- **CVars**: [Console Variables Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-console-variables-reference?utm_source=chatgpt.com); [FAutoConsoleVariableRef](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/HAL/FAutoConsoleVariableRef?utm_source=chatgpt.com); [TAutoConsoleVariable](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/HAL/TAutoConsoleVariable?utm_source=chatgpt.com); [Scalability](https://dev.epicgames.com/documentation/en-us/unreal-engine/scalability-reference-for-unreal-engine?utm_source=chatgpt.com)

- **Tasks/Threads**: [Tasks System](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Subsystems/UEngineSubsystem?utm_source=chatgpt.com); [ENamedThreads](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Async/ENamedThreads__Type?utm_source=chatgpt.com); [FRunnable/ThreadPool](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/HAL/FRunnable?utm_source=chatgpt.com)

- **RDG**: [FRDGBuilder](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/RenderCore/FRDGBuilder?utm_source=chatgpt.com); [RDG Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/RenderCore/FRDGBuilder?utm_source=chatgpt.com)

- **PSO Caches**: [Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/optimizing-rendering-with-pso-caches-in-unreal-engine?utm_source=chatgpt.com); [Bundled Caches (Limits)](https://dev.epicgames.com/documentation/en-us/unreal-engine/manually-creating-bundled-pso-caches-in-unreal-engine?utm_source=chatgpt.com)

- **Assets**: [Asset Registry](https://dev.epicgames.com/documentation/en-us/unreal-engine/asset-registry-in-unreal-engine?application_version=5.6&utm_source=chatgpt.com); [UAssetManager (Load/Preload)](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Engine/UAssetManager?utm_source=chatgpt.com)

- **Config**: [Configuration Files (Hierarchy)](https://dev.epicgames.com/documentation/en-us/unreal-engine/configuration-files-in-unreal-engine?utm_source=chatgpt.com)

- **Insights/Trace**: [Insights Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine?utm_source=chatgpt.com); [Trace Quick Start](https://dev.epicgames.com/documentation/en-us/unreal-engine/trace-quick-start-guide-in-unreal-engine?utm_source=chatgpt.com); [Session Browser](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine?utm_source=chatgpt.com); [Task Graph Insights](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine?utm_source=chatgpt.com)

- **Editor UX**: [ToolMenus](https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine?utm_source=chatgpt.com); [Detail/Property Customization](https://dev.epicgames.com/documentation/en-us/unreal-engine/details-panel-customizations-in-unreal-engine?utm_source=chatgpt.com); [Tabs](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Slate/Framework/Docking/FGlobalTabmanager/RegisterNomadTabSpawner?utm_source=chatgpt.com); [SCompoundWidget](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/SlateCore/Widgets/SCompoundWidget?utm_source=chatgpt.com)

---

## 18. Final Checklists (Copy into Your Repo)

### Build & Ship

- [ ] **`.uplugin` has `"SupportedTargetPlatforms": ["Win64"]` and correct `LoadingPhase`**
  [Epic Games Developers - SupportedTargetPlatforms](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Projects/FPluginDescriptor/SupportedTargetPlatforms?utm_source=chatgpt.com)

- [ ] **`FooOpt.Build.cs` uses `UseExplicitOrSharedPCHs`; minimal deps**
  [Epic Games Developers - Module Properties](https://dev.epicgames.com/documentation/en-us/unreal-engine/module-properties-in-unreal-engine?utm_source=chatgpt.com)

- [ ] **Package via `RunUAT BuildPlugin` (no stray source/build files in artifact)**
  [Epic Games Developers - Logging](https://dev.epicgames.com/documentation/en-us/unreal-engine/logging-in-unreal-engine?utm_source=chatgpt.com)

### Runtime Safety

- [ ] **All UObject work on GameThread (use `AsyncTask(GameThread)` hop)**
  [Epic Games Developers - ENamedThreads](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Async/ENamedThreads__Type?utm_source=chatgpt.com)

- [ ] **Render-thread work via RDG/`ENQUEUE_RENDER_COMMAND`**
  [Epic Games Developers - FRDGBuilder](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/RenderCore/FRDGBuilder?utm_source=chatgpt.com)

- [ ] **CVars created with `FAutoConsoleVariableRef`/`TAutoConsoleVariable`; documented in plugin readme**
  [Epic Games Developers - FAutoConsoleVariableRef](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/HAL/FAutoConsoleVariableRef?utm_source=chatgpt.com)

### Perf & Diagnostics

- [ ] **Insights traces captured with `-trace=default,task,memtag,gpu` during test runs; CSV exported in CI**
  [Epic Games Developers - Unreal Insights](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine?utm_source=chatgpt.com)

- [ ] **PSO stable/bundled caches generated before shipping**
  [Epic Games Developers - Bundled PSO Caches](https://dev.epicgames.com/documentation/en-us/unreal-engine/manually-creating-bundled-pso-caches-in-unreal-engine?utm_source=chatgpt.com)

---

## Summary

This Deep Addendum provides advanced knowledge for UE 5.6 C++ plugin development on Windows, covering build systems, reflection, multithreading, rendering, and performance optimization. It serves as a comprehensive reference for experienced developers building production-ready optimization tools.

---

*For the most up-to-date information, always consult the official Epic Games Developers documentation.*

