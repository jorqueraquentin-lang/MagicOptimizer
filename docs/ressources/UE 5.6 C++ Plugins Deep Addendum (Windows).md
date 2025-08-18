Got it. I dug through UE 5.6’s official docs and high-signal community material, then compared it to the last guide. Below is a **tightly engineered addendum**—a new doc that fills the biggest coverage gaps for building **Windows C++ plugins** (esp. optimization tooling) in **UE 5.6**. It’s organized so your AI agents can mine it safely: each section states invariants/pitfalls, then gives minimal, correct code/INI/CMD patterns with sources.

---

# UE 5.6 C++ Plugins — Deep Addendum (Windows)

## 0) What this adds (delta)

* **UBT & module/target rules that affect binary compatibility, build speed, & IWYU.**
* **.uplugin** fields that impact platform packaging, load order, & distribution.
* **Reflection correctness** (specifiers, MinimalAPI/export), **text types** (FName/FString/FText), and **localization** do/don’t.
* **Subsystem patterns** (Engine/Editor/Game/LocalPlayer) for plugin startup/shutdown.
* **Console variables** that back your optimizer (creation, binding, scalability interplay).
* **Multithreading & task graph** (UE::Tasks vs AsyncTask vs FRunnable/ThreadPools).
* **Render thread access & RDG**—how to add passes safely from a plugin.
* **PSO caching** (stable/recorded/bundled caches) for stutter-free DX12 on PC.
* **Asset Manager & Asset Registry** (soft refs, primary assets, async loads).
* **Config hierarchy & precedence** (Default\*.ini vs Saved/*.ini vs Base*.ini).
* **Profiling & tracing** (Insights/Trace/LLM/CSV) as automation-friendly CLI.
* **Editor UX hooks** (ToolMenus, Detail/Property customizations, tabs, Slate widgets).

---

## 1) Build system essentials for plugin authors

### 1.1 Modules (.build.cs) and Targets (.target.cs)

* **Modules** are defined via `ModuleRules` in `.build.cs` and control PCHs, dependencies, IWYU/unity choices, etc. **Targets** are `TargetRules` in `.target.cs` and select overall build shape (Editor/Game/Server/Program). ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/module-properties-in-unreal-engine?utm_source=chatgpt.com))

**Skeleton (Runtime module)**

```csharp
// FooOpt/FooOpt.Build.cs
using UnrealBuildTool;
public class FooOpt : ModuleRules {
  public FooOpt(ReadOnlyTargetRules Target) : base(Target) {
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

> Tip: UBT options that affect cache/graph invalidation (e.g., toggling IWYU/unity) regenerate the UBT Makefile; `-gather` or re-generate project files can force it. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/build-configuration-for-unreal-engine?application_version=5.6&utm_source=chatgpt.com))

**Target switches you’ll actually use (Editor target)**
Examples that frequently matter for tooling/optimizer plugins:

* `DisableUnityBuildForModules`, `DisableMergingModuleAndGeneratedFilesInUnityFiles`
* `bShowIncludes`, `InternalPluginDependencies`
  These live on `TargetRules` and can be set per target. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-build-tool-target-reference?utm_source=chatgpt.com))

### 1.2 UBT overview & configs

* UBT orchestrates compile/link for all targets/configs; read **BuildConfiguration** for user-configurable knobs. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-build-tool-in-unreal-engine?application_version=5.6&utm_source=chatgpt.com))

---

## 2) .uplugin fields that affect packaging & runtime

Key fields from `FPluginDescriptor` (the in-code descriptor for `.uplugin`):

* `"SupportedTargetPlatforms"`: ensure only Win64 (and any others you truly support). Mismatches here cause staging failures. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Projects/FPluginDescriptor/SupportedTargetPlatforms?utm_source=chatgpt.com), [Stack Overflow](https://stackoverflow.com/questions/78060237/interchangeeditor-uplugin-is-referenced-via-project-with-a-mismatched-supported?utm_source=chatgpt.com))
* `"EnabledByDefault"`, `"CanContainContent"`, `"LoadingPhase"` (e.g., `PreDefault`, `Default`, `PostEngineInit`)—choose the earliest phase *only* when you must bind early. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Projects/FPluginDescriptor?utm_source=chatgpt.com))

**Minimal Windows-only descriptor**

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

**Packaging (RunUAT)**

```powershell
& "Engine/Build/BatchFiles/RunUAT.bat" BuildPlugin `
  -plugin="D:\Src\FooOpt\FooOpt.uplugin" `
  -package="D:\Artifacts\FooOpt_Win64" `
  -targetplatform=Win64 -CrashReporter
```

Use **BuildPlugin** to ship a self-contained zip; the result contains only what the consumer needs. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/logging-in-unreal-engine?utm_source=chatgpt.com))

General plugin dev/management overview is here. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine?utm_source=chatgpt.com))

---

## 3) Reflection correctness (UCLASS/UFUNCTION/UPROPERTY) & MinimalAPI

* **UPROPERTY/ UFUNCTION/ UCLASS specifiers**: always pick the minimal exposure required (e.g., `BlueprintCallable` only if needed). Specifiers catalogues: properties, functions, classes, and metadata. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-uproperties?utm_source=chatgpt.com))
* **MinimalAPI**: export only a thin ABI from your module; this reduces DLL surface and link churn. Use full API export (`FOOOPT_API`) for types you instantiate across modules; use `MinimalAPI` for types primarily referenced via UPROPERTY/Blueprint. (Deep dives: minimal API & examples.) ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/class-specifiers?application_version=5.6&utm_source=chatgpt.com), [DeepWiki](https://deepwiki.com/fjz13/UnrealSpecifiers/2.1-minimalapi-and-api-exposure?utm_source=chatgpt.com), [GitHub](https://github.com/fjz13/UnrealSpecifiers/blob/main/Doc/en/Specifier/UCLASS/UHT/MinimalAPI/MinimalAPI.md?utm_source=chatgpt.com))

**Common compile/link pitfalls your agents should check**

* Headers must respect IWYU; include what you use; keep generated include last in the header block. (IWYU enforced in UE5 builds.) ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-spec-in-unreal-engine?utm_source=chatgpt.com))
* If a `UCLASS` is not exported and **instantiated** from another module, you’ll hit unresolved externals—either export its class or mark it `MinimalAPI` and provide exported factory functions. ([GitHub](https://github.com/fjz13/UnrealSpecifiers/blob/main/Doc/en/Specifier/UCLASS/UHT/MinimalAPI/MinimalAPI.md?utm_source=chatgpt.com))

---

## 4) Strings & localization you actually need in an optimizer

* **Use `FName`** for identifiers/keys (fast compare, name table-backed). ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/fname-in-unreal-engine?utm_source=chatgpt.com))
* **Use `FString`** for mutable text processing. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/fstring-in-unreal-engine?utm_source=chatgpt.com))
* **Use `FText`** for user-facing/localized text only (UI, messages). Prefer `LOCTEXT`/`NSLOCTEXT`. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/text-localization-in-unreal-engine?utm_source=chatgpt.com))

Changing culture via code/Blueprint is possible; avoid stomping user/editor settings with `SetCurrentCulture` in engine code. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Internationalization/FInternationalization/SetCurrentCulture?utm_source=chatgpt.com))

---

## 5) Subsystems & orderly startup hooks

Pick the smallest scope that fits:

* **UEngineSubsystem/ UEditorSubsystem/ UGameInstanceSubsystem/ ULocalPlayerSubsystem**. These provide predictable lifecycle & auto-instancing. Register console variables, tick sources, or menu entries here instead of globals. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Developer/ToolMenus?utm_source=chatgpt.com))

---

## 6) Console variables (CVars) that power optimizers

* Create **static** CVars using `TAutoConsoleVariable` or bind to member state via `FAutoConsoleVariableRef`. This lets profiles & runtime tweaks flow through without polling. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/HAL/TAutoConsoleVariable?utm_source=chatgpt.com))
* Wire to scalability and project settings sanely—know that scalability presets often override resolution/quality CVars (e.g., `r.ScreenPercentage`). ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/scalability-reference-for-unreal-engine?utm_source=chatgpt.com))

**Pattern**

```cpp
// Globals in FooOpt module (FooOptCvars.cpp)
static int32 GFooBudgetMB = 512;
static FAutoConsoleVariableRef CVarFooBudgetMB(
  TEXT("foo.BudgetMB"), GFooBudgetMB,
  TEXT("Memory budget used by Foo optimizer in MB"), FConsoleVariableDelegate(),
  ECVF_Default);
```

**CVar reference & commands** lists for UE 5.6 here; C++ API for CVars here. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-console-variables-reference?utm_source=chatgpt.com))

---

## 7) Multithreading: pick the right primitive

* **UE::Tasks** (preferred): structured tasks with priorities; integrates with Insights’ Task Graph. Great for granular work. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Subsystems/UEngineSubsystem?utm_source=chatgpt.com))
* **`AsyncTask(ENamedThreads::…)`**: jump work to a named thread (e.g., back to GameThread). Useful for thread-affinity fixes. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Async/ENamedThreads__Type?utm_source=chatgpt.com))
* **`FRunnable` & ThreadPools**: explicit worker threads; only when you need full control (long-lived, dedicated IO, etc.). ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/HAL/FRunnable?utm_source=chatgpt.com))

---

## 8) Render thread access & RDG (Render Dependency Graph)

* Never touch RHI resources from the game thread. Use `ENQUEUE_RENDER_COMMAND` or better, **RDG** (`FRDGBuilder` + `AddPass`) to stage work. **Lifetimes, barriers, and culling are handled by RDG** when you declare pass parameters correctly. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/RenderCore/FRDGBuilder?utm_source=chatgpt.com))

> For compute/raster passes, build your graph on render thread, declare resources in pass params, then `Execute()`. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/RenderCore/FRDGBuilder?utm_source=chatgpt.com))

---

## 9) PSO caching (PC DX12 stutter control)

For Windows DX12:

* Prefer **PSO Precaching/Stable Cache** for common paths; generate **Bundled PSO** caches for shipping. Ray tracing PSOs aren’t bundled; compute PSOs are generated during cook (configurable via CVars). ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/manually-creating-bundled-pso-caches-in-unreal-engine?utm_source=chatgpt.com))
* Know the two cache types (Recorded vs Stable), and that bundling focuses on **Graphics PSOs**. Official guides and deep dives: ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/optimizing-rendering-with-pso-caches-in-unreal-engine?utm_source=chatgpt.com))

---

## 10) Asset loading & data footprint

* Use **Asset Registry** (metadata only) to query/filter without loading packages. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/asset-registry-in-unreal-engine?application_version=5.6&utm_source=chatgpt.com))
* Use **UAssetManager** to **async load** primary assets and **keep handles** for lifetime control; prefer soft references in content to decouple. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Engine/UAssetManager?utm_source=chatgpt.com))

---

## 11) Config hierarchy & precedence (avoid “it worked in Editor”)

* **Read order**: engine *Base*.ini → platform *Base*.ini → project **Default*.ini*\* → user/profile **Saved/Config/*/*.ini** (highest precedence at runtime). Ship project settings in **Default*.ini*\*; don’t try to overwrite engine `Base*.ini`. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/configuration-files-in-unreal-engine?utm_source=chatgpt.com), [ikrima.dev](https://ikrima.dev/ue4guide/application-os-generic-platform/config-files/?utm_source=chatgpt.com))

---

## 12) Profiling & automation (headless-friendly)

* **Unreal Insights** + **Trace**: start traces from the status bar (Editor) or headless with `-trace=<channels>`; open `.utrace` files in the **Session Browser**; export CSV from CLI for CI bots. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine?utm_source=chatgpt.com))
* Useful docs: **Trace Quick Start**, **Task Graph Insights**, **Automated CSV exporters**. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/trace-quick-start-guide-in-unreal-engine?utm_source=chatgpt.com))

---

## 13) Editor UX hooks your plugin likely needs

* **ToolMenus**: add menu/toolbar items with C++ (Editor module StartupModule). ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine?utm_source=chatgpt.com))
* **Details panel**: per-type layout via `IDetailCustomization` or per-property via `IPropertyTypeCustomization`. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/details-panel-customizations-in-unreal-engine?utm_source=chatgpt.com))
* **Slate tabs**: register a **Nomad Tab** spawner with `FGlobalTabmanager::RegisterNomadTabSpawner`, then invoke by TabId. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Slate/Framework/Docking/FGlobalTabmanager/RegisterNomadTabSpawner?utm_source=chatgpt.com))
* **Custom Slate**: extend `SCompoundWidget`. (Include `Slate` & `SlateCore` in module deps.) ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/SlateCore/Widgets/SCompoundWidget?utm_source=chatgpt.com))

---

## 14) Minimal, correct patterns (ready to paste)

### 14.1 Developer Settings + CVars bridge

```cpp
UCLASS(Config=EditorPerProjectUserSettings, defaultconfig, MinimalAPI)
class UFooOptSettings : public UDeveloperSettings {
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

> `UDeveloperSettings` lives in Editor/Project Settings and persists to config; the CVar gives runtime override. (Specifiers & CVars references above.) ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/metadata-specifiers-in-unreal-engine?utm_source=chatgpt.com))

### 14.2 UE::Tasks + safe hop to GameThread

```cpp
#include "Async/Async.h"
#include "Tasks/Task.h"

UE::Tasks::FTask LaunchHeavy() {
  return UE::Tasks::Launch(TEXT("FooBuildClusters"), [](){
    // work...
    AsyncTask(ENamedThreads::GameThread, [](){ /* touch UObjects here */ });
  });
}
```

([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Subsystems/UEngineSubsystem?utm_source=chatgpt.com))

### 14.3 RDG “hello pass” shape

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

([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/RenderCore/FRDGBuilder?utm_source=chatgpt.com))

### 14.4 PSO: from run to bundled cache (DX12)

1. Collect PSOs during play/tests → build **stable** cache
2. Bundle for shipping (graphics PSOs; compute auto-generated at cook; RT not supported)
   Docs and caveats here. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/optimizing-rendering-with-pso-caches-in-unreal-engine?utm_source=chatgpt.com))

---

## 15) Common “gotchas” your agents should pre-empt

* **Missing platform in `.uplugin`** causes staging failure or silent non-load on packaged builds. Verify `SupportedTargetPlatforms`. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Projects/FPluginDescriptor/SupportedTargetPlatforms?utm_source=chatgpt.com))
* **Over-exporting** (no `MinimalAPI`) explodes your ABI; **under-exporting** breaks instantiation across modules. Check unresolved externals for `GetPrivateStaticClass`. ([GitHub](https://github.com/fjz13/UnrealSpecifiers/blob/main/Doc/en/Specifier/UCLASS/UHT/MinimalAPI/MinimalAPI.md?utm_source=chatgpt.com))
* **Touching UObjects off GameThread**—always marshal via `AsyncTask(GameThread)` or engine delegates. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Async/ENamedThreads__Type?utm_source=chatgpt.com))
* **RDG misuse** (allocations/lifetimes) when not declaring pass parameters/resources → undefined behavior; let RDG manage lifetimes/barriers. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/RenderCore/FRDGBuilder?utm_source=chatgpt.com))
* **Scalability vs CVar fights** (e.g., `r.ScreenPercentage`)—your plugin should either own it (write after preset) or observe changes. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/scalability-reference-for-unreal-engine?utm_source=chatgpt.com))
* **Config writes**: ship **Default*.ini*\*; expect runtime/user overrides in **Saved/Config** to win. Don’t edit `Base*.ini`. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/configuration-files-in-unreal-engine?utm_source=chatgpt.com))

---

## 16) Source map (quick jump)

* **Modules & Targets**: Module Properties; Target Reference; UBT overview. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/module-properties-in-unreal-engine?utm_source=chatgpt.com))
* **Plugins & Packaging**: Plugins overview; FPluginDescriptor; SupportedTargetPlatforms; BuildPlugin UAT. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine?utm_source=chatgpt.com))
* **Reflection**: UPROPERTY; UFUNCTION; Class Specifiers; Metadata; MinimalAPI deep dives. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-uproperties?utm_source=chatgpt.com), [GitHub](https://github.com/fjz13/UnrealSpecifiers/blob/main/Doc/en/Specifier/UCLASS/UHT/MinimalAPI/MinimalAPI.md?utm_source=chatgpt.com))
* **Strings & Localization**: String Handling; FText; SetCurrentCulture. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/string-handling-in-unreal-engine?utm_source=chatgpt.com))
* **Subsystems**: Subsystems in UE. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Developer/ToolMenus?utm_source=chatgpt.com))
* **CVars**: Console Variables Reference; FAutoConsoleVariableRef; TAutoConsoleVariable; Scalability. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-console-variables-reference?utm_source=chatgpt.com))
* **Tasks/Threads**: Tasks System; ENamedThreads; FRunnable/ThreadPool. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Subsystems/UEngineSubsystem?utm_source=chatgpt.com))
* **RDG**: FRDGBuilder; RDG overview. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/RenderCore/FRDGBuilder?utm_source=chatgpt.com))
* **PSO caches**: Overview; Bundled caches (limits). ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/optimizing-rendering-with-pso-caches-in-unreal-engine?utm_source=chatgpt.com))
* **Assets**: Asset Registry; UAssetManager (Load/Preload). ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/asset-registry-in-unreal-engine?application_version=5.6&utm_source=chatgpt.com))
* **Config**: Configuration Files (hierarchy). ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/configuration-files-in-unreal-engine?utm_source=chatgpt.com))
* **Insights/Trace**: Insights overview; Trace Quick Start; Session Browser; Task Graph Insights. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine?utm_source=chatgpt.com))
* **Editor UX**: ToolMenus; Detail/Property customization; Tabs; SCompoundWidget. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine?utm_source=chatgpt.com))

---

## 17) Final checklists (copy into your repo)

**Build & Ship**

* [ ] `.uplugin` has `"SupportedTargetPlatforms": ["Win64"]` and correct `LoadingPhase`. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Projects/FPluginDescriptor/SupportedTargetPlatforms?utm_source=chatgpt.com))
* [ ] `FooOpt.Build.cs` uses `UseExplicitOrSharedPCHs`; minimal deps. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/module-properties-in-unreal-engine?utm_source=chatgpt.com))
* [ ] Package via `RunUAT BuildPlugin` (no stray source/build files in artifact). ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/logging-in-unreal-engine?utm_source=chatgpt.com))

**Runtime safety**

* [ ] All UObject work on GameThread (use `AsyncTask(GameThread)` hop). ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Async/ENamedThreads__Type?utm_source=chatgpt.com))
* [ ] Render-thread work via RDG/`ENQUEUE_RENDER_COMMAND`. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/RenderCore/FRDGBuilder?utm_source=chatgpt.com))
* [ ] CVars created with `FAutoConsoleVariableRef`/`TAutoConsoleVariable`; documented in plugin readme. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/HAL/FAutoConsoleVariableRef?utm_source=chatgpt.com))

**Perf & Diagnostics**

* [ ] Insights traces captured with `-trace=default,task,memtag,gpu` during test runs; CSV exported in CI. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine?utm_source=chatgpt.com))
* [ ] PSO stable/bundled caches generated before shipping. ([Epic Games Developers](https://dev.epicgames.com/documentation/en-us/unreal-engine/manually-creating-bundled-pso-caches-in-unreal-engine?utm_source=chatgpt.com))

---

If you want, I can now fold this addendum **into the original guide** as a single, linear doc (with a clean ToC), or produce **code-first templates** (Runtime + Editor modules, subsystem, ToolMenus, CVar bridge, RDG pass, Insights/Trace batch) ready to drop into your plugin.
