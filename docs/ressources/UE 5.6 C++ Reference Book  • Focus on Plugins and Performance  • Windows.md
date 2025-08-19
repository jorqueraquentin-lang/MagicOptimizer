# UE 5.6 C++ Reference Book
## Focus on Plugins and Performance • Windows

---

## Table of Contents
1. [Orientation for 5.6 and Plugin Work](#orientation-for-56-and-plugin-work)
2. [Core Language Bindings and Primitives](#core-language-bindings-and-primitives)
3. [Reflection and Metadata](#reflection-and-metadata)
4. [Object Lifetime, GC, and Smart Pointers](#object-lifetime-gc-and-smart-pointers)
5. [Modules, Plugins, Settings](#modules-plugins-settings)
6. [Files, Paths, and Processes on Windows](#files-paths-and-processes-on-windows)
7. [Engine Framework Essentials for Plugins](#engine-framework-essentials-for-plugins)
8. [Asset Discovery and Management](#asset-discovery-and-management)
9. [Editor Extension Points for Tooling](#editor-extension-points-for-tooling)
10. [UI Stacks](#ui-stacks)
11. [Rendering Hooks for Optimization](#rendering-hooks-for-optimization)
12. [Input Systems](#input-systems)
13. [AI, Navigation, Gameplay Tags and Tasks](#ai-navigation-gameplay-tags-and-tasks)
14. [Physics, Niagara, Animation](#physics-niagara-animation)
15. [Networking and Replication](#networking-and-replication)
16. [Console Commands and CVars](#console-commands-and-cvars)
17. [Windows Platform Specifics](#windows-platform-specifics)
18. [Quick Recipes for Optimization Plugins](#quick-recipes-for-optimization-plugins)
19. [Appendix A: Common Engine Types](#appendix-a--common-engine-types)
20. [Appendix B: Snippet Pack for AI Agents](#appendix-b--snippet-pack-for-ai-agents)

---

## 1. Orientation for 5.6 and Plugin Work

### API Home and Index
- **Use this to resolve names and headers** (Epic Games Developers)
- **Modules and .build.cs drive what you can include and what symbols are exported**
- **Add dependencies here, not in your IDE** (Epic Games Developers)

### Target Types for Windows Builds
- **Game, Client, Server, Editor, Program**
- **Set in .target.cs via TargetRules** (Epic Games Developers)

### Module API Specifiers
- **Control symbol visibility across DLLs**
- **Examples: ENGINE_API, CORE_API, YOURMODULE_API**
- **Your plugin needs these on any public class or function used by other modules** (Epic Games Developers)

#### Example Build Configuration
```cpp
// YourModule.Build.cs
PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });

// Export from your DLL
class YOURMODULE_API FMyOptimizer { /* ... */ };
```

---

## 2. Core Language Bindings and Primitives

### Containers, Strings, Names
- **FString**: Dynamic string
- **FName**: Atomized comparison
- **FText**: Localized text
- **FName storage and behavior are documented here** (Epic Games Developers)
- **TArray, TMap, TSet**: Live in Core, prefer reserve and Emplace for performance

### Math Types
- **FVector, FRotator, FTransform**: Typedefs of double precision math templates in 5.6
- **Good to know for numeric stability** (Epic Games Developers)
- **Useful helpers**: UKismetMathLibrary::MakeTransform, NormalizedDeltaRotator (Epic Games Developers)

#### Math Examples
```cpp
const FVector Dir = FRotator(0, YawDeg, 0).Vector(); // unit direction
FTransform T = UKismetMathLibrary::MakeTransform(Loc, Rot, Scale);
```

### Async, Tasks, Threading
- **UE::Tasks**: Modern job system for gameplay code
- **Prefer it over raw TaskGraph** (Epic Games Developers)
- **Async helpers**: EAsyncExecution, TFuture, TPromise, UE::Tasks::Launch (Epic Games Developers)

#### Task Example
```cpp
// Lightweight async work
auto Task = UE::Tasks::Launch(UE_SOURCE_LOCATION, [] { Heavy(); });
Task.Wait();
```

---

## 3. Reflection and Metadata

### UCLASS, USTRUCT, UFUNCTION, UPROPERTY
- **Enable reflection, GC tracking, and Blueprint exposure**
- **Overview and specifier lists here** (Epic Games Developers)
- **UClass and UFunction are the reflected types at runtime** (Epic Games Developers)

### Delegates
- **Use DECLARE_DELEGATE and friends**
- **Support UFUNCTION style specifiers via UDELEGATE** (Epic Games Developers)

#### Reflection Example
```cpp
UCLASS(BlueprintType)
class YOURMODULE_API UMyConfig : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Config) int32 BudgetMb = 256;
    UFUNCTION(BlueprintCallable) void Apply();
};
```

---

## 4. Object Lifetime, GC, and Smart Pointers

### UPROPERTY Usage
- **Use on UObject fields for GC reachability**
- **Store non-UObject with C++ smart pointers**

### Smart Pointer Types
- **TWeakObjectPtr**: Non-owning references to UObjects
- **TSoftObjectPtr**: Lazy asset references with LoadSynchronous (Epic Games Developers)
- **TSharedPtr, TUniquePtr**: For Slate and non-UObject lifetimes (Epic Games Developers)

#### Smart Pointer Examples
```cpp
UPROPERTY() UTexture2D* Tracked = nullptr;              // GC aware
TWeakObjectPtr<UTexture2D> Maybe;                       // weak
TSoftObjectPtr<UTexture2D> Lazy;                        // path, can load on demand
auto* Tex = Lazy.LoadSynchronous();                     // blocking load
```

---

## 5. Modules, Plugins, Settings

### Module Interface
- **IModuleInterface for startup and shutdown**
- **IMPLEMENT_MODULE in your module cpp** (Epic Games Developers)

### Plugin Management
- **IPlugin and descriptor access via IPluginManager**
- **For runtime checks or content paths** (Epic Games Developers)

### Settings Module
- **ISettingsModule registers an Editor settings page for your plugin** (Epic Games Developers)

### Authoring Plugins Overview
- **What they can extend**: runtime, editor, menus, file types (Epic Games Developers)

#### Module Example
```cpp
class FYourModule final : public IModuleInterface
{
    void StartupModule() override { /* register menus, cvars */ }
    void ShutdownModule() override { /* unregister */ }
};
IMPLEMENT_MODULE(FYourModule, YourModule)
```

---

## 6. Files, Paths, and Processes on Windows

### Path Management
- **FPaths helpers**: For project and engine dirs, splitting and combining paths (Epic Games Developers)
- **IFileManager and IPlatformFile**: For IO, directory traversal, copy, create writer (Epic Games Developers)

### Process Control
- **FGenericPlatformProcess**: For launching processes, URLs, and capturing output
- **Great for tool integration** (Epic Games Developers)
- **FGenericPlatformMisc**: For environment variables and exit codes (Epic Games Developers)

#### Process Example
```cpp
FString Out, Err; int32 Code=0;
FGenericPlatformProcess::ExecProcess(*Exe, *Args, &Code, &Out, &Err);
```

---

## 7. Engine Framework Essentials for Plugins

### World and Gameplay Classes
- **UWorld, AActor, UActorComponent, APawn, ACharacter, PlayerController, GameMode, GameState, GameInstance, ULevel and streaming, FTimerManager**
- **Use UGameplayStatics for global helpers** (Epic Games Developers)
- **AActor::GetTransform and other accessors** (Epic Games Developers)

### Subsystems for Clean Hooks
- **Use EngineSubsystem, EditorSubsystem, LocalPlayerSubsystem, WorldSubsystem**
- **Attach your plugin to engine life cycle without subclassing gameplay classes**
- **Index and examples live in the API home** (Epic Games Developers)

---

## 8. Asset Discovery and Management

### Asset Registry Module
- **FAssetRegistryModule exposes IAssetRegistry**
- **Find assets without loading**
- **Use EnumerateAssets and filters** (Epic Games Developers)

### Asset Registry State
- **FAssetRegistryState and version provide cooked cache support** (Epic Games Developers)

### Asset Manager
- **UAssetManager centralizes primary asset types and loads by type**
- **Use for large scale streaming and optimization** (Epic Games Developers)

#### Asset Registry Example
```cpp
IAssetRegistry& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
FARFilter Filter; 
Filter.ClassPaths.Add(UTexture2D::StaticClass()->GetClassPathName());
AR.EnumerateAssets(Filter, [&](const FAssetData& A){ /* ... */ return true; });
```

---

## 9. Editor Extension Points for Tooling

### ToolMenus System
- **UToolMenus, UToolMenu, FToolMenuSection, FToolMenuEntry**
- **Owner scoping and extension helpers** (Epic Games Developers)

#### Menu Extension Example
```cpp
UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window")
    ->AddSubMenu(FToolMenuOwner("YourPlugin"), "WindowLayout", "YourOptim", 
                 FText::FromString("Optimizer"), FText::GetEmpty());
```

---

## 10. UI Stacks

### Slate for Editor Windows and Custom Widgets
- **Framework overview, application entry, window management**
- **FSlateApplication::AddWindow and Get** (Epic Games Developers)

### UMG for Runtime UI
- **UUserWidget lifecycle**: Create, PreConstruct, SynchronizeProperties
- **WidgetBlueprintLibrary helpers** (Epic Games Developers)

---

## 11. Rendering Hooks for Optimization

### Render Thread Work
- **EnqueueRenderCommand and EnqueueUniqueRenderCommand**
- **For safe cross thread updates** (Epic Games Developers)

### Render Dependency Graph (RDG)
- **FRDGBuilder, CreateTexture, RegisterExternalTexture, QueueTextureExtraction, Execute**
- **Modern way to author GPU work efficiently** (Epic Games Developers)

### Materials
- **UMaterialInterface and UMaterialInstanceDynamic::Create**
- **For dynamic parameter changes** (Epic Games Developers)

### Threaded Rendering
- **Overview for performance patterns** (Epic Games Developers)

#### RDG Example
```cpp
// Minimal RDG pass
FRDGBuilder Graph(RHICmdList);
FRDGTextureRef Out = Graph.CreateTexture(Desc, TEXT("YourRT"));
Graph.AddPass(RDG_EVENT_NAME("YourPass"), Params,
    ERDGPassFlags::Compute, [=](FRHICommandListImmediate& RHICmd){ /* RHI calls */});
Graph.Execute();
```

---

## 12. Input Systems

### Enhanced Input
- **Enhanced Input subsystems and components**
- **For deterministic mappings and replayable actions**
- **Use UEnhancedInputLocalPlayerSubsystem for context management**

---

## 13. AI, Navigation, Gameplay Tags and Tasks

### AI and Navigation
- **UNavigationSystemV1, AIController, UBlackboardComponent, Behavior Trees**
- **Live under AIModule and NavigationSystem**
- **Use these for agent pathing** (Epic Games Developers)

### Gameplay Systems
- **GameplayTags and GameplayTasks**
- **For lightweight state and background actions** (Epic Games Developers)

---

## 14. Physics, Niagara, Animation

### Physics
- **Chaos data structures and helpers appear across Chaos namespaces**
- **For example transform facades** (Epic Games Developers)

### Niagara
- **UNiagaraFunctionLibrary for spawning systems programmatically** (Epic Developer Community Forums)

---

## 15. Networking and Replication

### Replication
- **Use UPROPERTY(Replicated) and GetLifetimeReplicatedProps overrides on AActor**
- **Replicated multicast UFUNCTION works with NetMulticast specifier**
- **High level replication API is part of Engine and CoreUObject** (Epic Games Developers)

---

## 16. Console Commands and CVars

### Console Commands
- **Console commands guide and usage patterns** (Epic Games Developers)

### Console Variables
- **Console variables and flags reference with categories and hot reload behavior** (Epic Games Developers)

#### CVar Registration Example
```cpp
// Register cvars
static TAutoConsoleVariable<int32> CVarYourBudget(
    TEXT("r.YourPlugin.BudgetMb"), 256, TEXT("Memory budget in megabytes"));

// Bind exec
static FAutoConsoleCommand Cmd(TEXT("YourPlugin.Dump"),
    TEXT("Dump state"), FConsoleCommandDelegate::CreateStatic(&Dump));
```

---

## 17. Windows Platform Specifics

### SDK Management
- **Managing SDKs and devices from Editor Platforms dropdown**
- **Handy for CI set up** (Epic Games Developers)

### System Information
- **CPU info and environment access helpers** (Epic Games Developers)

---

## 18. Quick Recipes for Optimization Plugins

### A. Create an Editor Menu, Settings Panel, and CVars

#### Module Startup Example
```cpp
// Module startup
void FYourModule::StartupModule()
{
    // Settings page
    if (ISettingsModule* Settings = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        Settings->RegisterSettings("Project", "Plugins", "YourOptimizer",
            FText::FromString("Your Optimizer"),
            FText::FromString("Budget and capture settings"),
            GetMutableDefault<UMyConfig>());
    }

    // ToolMenus
    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateLambda([]{
            auto* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
            Menu->AddSubMenu(FToolMenuOwner("YourPlugin"), "WindowLayout", "YourOptim",
                FText::FromString("Optimizer"), FText::FromString("Budget tools"));
        }));
}
```

**References**: Settings and ToolMenus (Epic Games Developers)

### B. Scan Assets Without Loading and Produce a Size Report

```cpp
IAssetRegistry& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
FARFilter F; 
F.ClassPaths.Add(UTexture2D::StaticClass()->GetClassPathName());
AR.EnumerateAssets(F, [&](const FAssetData& A){
    // A.GetFullName(), A.PackageName, A.GetTagValue("TextureGroup", ...)
    return true;
});
```

**Uses**: FAssetRegistryModule and enumerate API (Epic Games Developers)

### C. Kick Work to Render Thread Safely

```cpp
ENQUEUE_RENDER_COMMAND(UpdateStats)(
    [](FRHICommandListImmediate& RHICmd){ /* query stats, GPU timing */ });
```

**Reference**: Render thread enqueue helpers (Epic Games Developers)

### D. Build a Quick RDG Texture Test

```cpp
FRDGBuilder Graph(RHICmdList);
FRDGTextureRef RT = Graph.CreateTexture(Desc, TEXT("TestRT"));
Graph.Execute();
```

**Reference**: RDG builder usage (Epic Games Developers)

### E. Open a Report Folder or External Tool on Windows

```cpp
FString Folder = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Reports"));
IFileManager::Get().MakeDirectory(*Folder, /*Tree*/ true);
FGenericPlatformProcess::LaunchURL(*Folder, nullptr, nullptr);
```

**References**: Paths, IO, and URL launch (Epic Games Developers)

---

## Appendix A: Common Engine Types

| Area | Representative API |
|------|-------------------|
| **Core math** | FVector, FRotator, FTransform |
| **Engine helpers** | UGameplayStatics, AActor accessors |
| **Materials** | UMaterialInterface, UMaterialInstanceDynamic::Create |
| **Slate runtime** | FSlateApplication methods AddWindow, ProcessMouseButtonDownEvent |
| **UMG runtime** | UUserWidget CreateWidgetInstance, SynchronizeProperties, WidgetBlueprintLibrary |
| **Asset registry** | FAssetRegistryModule, FAssetRegistryState |
| **Tasks and async** | UE::Tasks::Launch, EAsyncExecution, TFuture |
| **File system** | FPaths, IFileManager, IPlatformFile |
| **Process control** | ExecProcess, CreateProc, LaunchURL |
| **Menus** | UToolMenus, UToolMenu, FToolMenuEntry |
| **RDG** | FRDGBuilder core methods |

---

## Appendix B: Snippet Pack for AI Agents

### Minimal Class with Reflection and Blueprint Exposure

```cpp
UCLASS(BlueprintType, Blueprintable)
class YOURMODULE_API UOptimizerSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Config) int32 CaptureFrames = 60;
    UFUNCTION(BlueprintCallable) void StartCapture();
};
```

### Find Widgets and Operate Safely

```cpp
TArray<UUserWidget*> Widgets;
UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GWorld, Widgets, UYourWidget::StaticClass(), false);
```

**Reference**: (Epic Games Developers)

### Create Dynamic Material Instance

```cpp
UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
MID->SetScalarParameterValue("Budget", 0.5f);
```

**Reference**: (Epic Games Developers)

### Core Containers & Strings (Fast, Safe, Alloc-Aware)

#### Arrays/Maps/Sets
- **TArray<T>**: Dynamic array. Prefer Reserve, AddUninitialized, Emplace, RemoveAtSwap to minimize copies. Watch pointer invalidation on growth.
- **TMap<Key,Value> / TMultiMap**: Hash maps. Great general-purpose lookups; consider TSortedMap for cache-friendly, small N scenarios.
- **TSet<T>**: Hashed set; Array() materializes to contiguous storage when needed.

#### Strings
- **FString**: Mutable text; conversions, search, split/join; heavy if overused in hot paths.
- **FName**: Interned, case-insensitive identifier; cheap comparisons; ideal for tags, lookups, event names.
- **FText**: Localized, user-facing; avoid in tight loops; convert via FText::FromString sparingly.

### Reflection Essentials (UHT) You'll Actually Use

#### UPROPERTY (Serialization, GC, Editor & Blueprint Exposure)
- **Common specifiers**: EditAnywhere, BlueprintReadOnly/Write, Transient, Config, VisibleAnywhere, Instanced, Replicated, ReplicatedUsing=OnRep…
- **Useful meta**: ClampMin/Max, DisplayName, EditCondition, Bitmask/BitmaskEnum, ExposeOnSpawn.

#### UFUNCTION (Expose to BP, Editor Buttons, RPC)
- **Blueprint**: BlueprintCallable, BlueprintPure, BlueprintImplementableEvent, BlueprintNativeEvent, CallInEditor.
- **Networking**: Server, Client, NetMulticast, Reliable / Unreliable.
- **Console exec**: Exec.
- **Latent**: Latent, LatentInfo.

#### Pattern (RPC + Property Notify)
```cpp
UCLASS()
class UPerfController : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(ReplicatedUsing=OnRep_Mode)
    int32 Mode = 0;

    UFUNCTION(Server, Reliable) void ServerSetMode(int32 InMode);
    UFUNCTION() void OnRep_Mode();

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override {
        DOREPLIFETIME(UPerfController, Mode);
    }
};
```

**Reference**: RPC/function specifiers and replication hook signatures.

### Delegates (Events Without Tight Coupling)

- **Unicast**: TDelegate<Signature> — bind one target, ExecuteIfBound.
- **Multicast**: TMulticastDelegate — broadcast to many; no return value.
- **Dynamic (UObject/serialized)**: DECLARE_DYNAMIC_* macros for BP exposure.
- **Handles**: FDelegateHandle for safe unbinds.

#### Pattern
```cpp
DECLARE_MULTICAST_DELEGATE_OneParam(FOnScanDone, int32 /*AssetsScanned*/);

class FScanner {
public:
    FOnScanDone OnScanDone;  // native multicast
    void Run() { /*...*/ OnScanDone.Broadcast(Count); }
};
```

**Reference**: Delegate types/macros overview.

### Subsystems (Where to Park Plugin Logic)

Use these for lifecycle-managed singletons:

- **UEngineSubsystem (engine-wide)**: Great for global throttles/toggles.
- **UGameInstanceSubsystem (per game instance)**: Runtime manager per session.
- **UWorldSubsystem (per world)**: World-scoped collectors/profilers.
- **UEditorSubsystem (editor only)**: Menu hooks, batch tools.

### Asset Registry & Asset Manager (Fast Discovery, No Loads)

#### Asset Registry
- **Entrypoints**: FAssetRegistryModule → IAssetRegistry&.
- **Bread-and-butter queries**: GetAssetsByClass, GetAssetsByPath, EnumerateAssets, EnumerateAllAssets, GetDependencies/GetReferencers.
- **Events/maintenance**: OnAssetUpdatedOnDisk, AssetCreated, Serialize, AppendState, SearchAllAssets.

#### Pattern
```cpp
IAssetRegistry& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
FARFilter Filter; 
Filter.ClassNames.Add(UStaticMesh::StaticClass()->GetFName());
AR.EnumerateAssets(Filter, [](const FAssetData& A){ /* index, score, tag query */ return true; });
```

#### Asset Manager (Primary Assets, Bundles)
- **Use Primary Asset Types/Ids to model "content families," async load by type/bundle.**
- **Drive cook filters and late-load groups for performance.**

### Tasks & Async (Don't Block the Game Thread)

- **AsyncTask(ENamedThreads::Type, Lambda)**: Fire work on task graph: lightweight, no join.
- **FAsyncTask<TaskType>**: Thread-pool job object with explicit StartBackgroundTask() / EnsureCompletion().
- **UE::Tasks::Launch(...)**: Modern DAG-style jobs, composable dependencies; better scaling than classic TaskGraph.

#### Pattern
```cpp
// fire-and-forget sample collection
AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]{ CollectStats(); });

// cancellable pooled work (heavy indexing)
class FIndexTask { public: void DoWork(){ BuildIndex(); } };
FAsyncTask<FIndexTask> Task; 
Task.StartBackgroundTask(); 
/* later */ Task.EnsureCompletion();
```

### RDG (Render Dependency Graph) for GPU Tasks

- **Orchestrate GPU passes via FRDGBuilder**
- **Add passes with AddPass, create resources with CreateTexture**
- **Bridge existing RHI targets via RegisterExternalTexture**
- **Read back via AddReadbackTexturePass**
- **RDG handles lifetimes, barriers, async-compute overlap**
- **You supply pass parameter structs and lambdas**

#### Minimal RDG Pass
```cpp
RDG_EVENT_SCOPE(GraphBuilder, "PerfHeatmap");
FRDGTextureRef Heat = GraphBuilder.CreateTexture(
    FRDGTextureDesc::Create2D(Extent, PF_R8_UINT, FClearValueBinding::Black, TexCreate_ShaderResource|TexCreate_UAV),
    TEXT("Perf.Heatmap"));

GraphBuilder.AddPass(
    RDG_EVENT_NAME("WriteHeat"),
    ERDGPassFlags::Compute,
    [Heat](FRHICommandListImmediate& RHICmdList)
    {
        // Dispatch compute to fill Heat UAV (pseudo code)
    });
```

**Reference**: Overview and key APIs.

**Note**: Screen-pass helpers (FScreenPassTexture, FScreenPassRenderTarget) are convenient for post steps.

### Console Variables & Commands (Runtime Tuning, Toggles)

- **Global references**: Console Variables Reference and Console Commands Reference (audit what exists before inventing new).
- **C++**: Register config-backed controls (IConsoleManager, FAutoConsoleVariableRef, TAutoConsoleVariable). Great for plugin feature flags and thresholds. See "Console Variables C++" for patterns.
- **Editor UX**: Console Variables Editor to build presets for QA/perf sweeps.

### Build & Packaging (Windows)

- **Plugin basics & structure**: Author .uplugin, split modules (Runtime/Developer/Editor), mark dependencies.
- **Package your plugin**: RunUAT BuildPlugin -plugin="...\Your.uplugin" -package="...\Out" -TargetPlatforms=Win64. Use for Marketplace/fab deliverables too.
- **Modules overview**: How to structure/link modules cleanly.
- **Turnkey & SDKs**: Manage platform SDKs via RunUAT.bat -Mode=... Turnkey CLI.
- **3rd-party libs**: Linking/static vs dynamic, staging rules.

### Editor UI Hooks You'll Likely Need (Menus + Details)

#### ToolMenus (Menus/Toolbar)
- **Extend or register menus**
- **Add entries bound to FUICommandInfo or lambdas**
- **UToolMenus::ExtendMenu, UToolMenu::AddMenuEntry, FToolMenuEntry::InitMenuEntry**

#### Details Panel Customizations
- **Implement IDetailCustomization::CustomizeDetails**
- **Group, hide, or replace property widgets**
- **Register in your module's startup**

#### Pattern (Register)
```cpp
FPropertyEditorModule& PEM = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
PEM.RegisterCustomClassLayout("YourSettings", 
    FOnGetDetailCustomizationInstance::CreateStatic(&FYourSettingsCustomization::MakeInstance));
```

**Reference**: Quickstart & API.

### Networking Bits You'll Touch (For Remote Profiling/Toggles)

- **RPC specifiers live on UFUNCTION** (Server/Client/NetMulticast, Reliable/Unreliable).
- **Replicate state via GetLifetimeReplicatedProps** (override on AActor/UObject where appropriate).
- **Property replication guide** (conditions, custom rules).

### Profiling Stack (Make the Plugin Data-Driven)

- **Unreal Insights**: Trace events → Trace Server → Insights UI; saves .utrace, supports headless export; use for CPU/GPU/Tasks/Memory analysis.
- **Task Graph Insights**: Visualize task dependencies/timing (great when you parallelize scans/builds).

### Scalable Rendering Controls (Frequent Optimization Levers)

- **r.ScreenPercentage, TSR controls, Dynamic Resolution modes**
- **Anchor your plugin's presets around these**
- **Or expose custom CVars that set bundles of them**

---

## Handy Code Crib (Drop-In)

### Register Plugin CVars
```cpp
static TAutoConsoleVariable<float> CVarPerfBudget(
    TEXT("perf.BudgetMs"),
    3.0f,
    TEXT("Frame time budget for plugin heuristics in ms."),
    ECVF_Default);
```

**Reference**: C++ CVar pattern.

### Enumerate Meshes Fast (No Load)
```cpp
IAssetRegistry& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
AR.GetAssetsByClass(UStaticMesh::StaticClass()->GetFName(), OutMeshAssets);
```

**Reference**: Registry entrypoints & queries.

### Fire Background Work
```cpp
AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]{ Analyze(); });
```

**Reference**: TaskGraph convenience.

### Add Editor Menu Button
```cpp
UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]{
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
    FToolMenuSection& Sec = Menu->AddSection("Perf");
    Sec.AddMenuEntry("OpenPerfPanel", LOCTEXT("Perf","Open Perf Panel"), {}, FSlateIcon(),
                     FUIAction(FExecuteAction::CreateStatic(&FYourModule::OpenPerfTab)));
}));
```

**Reference**: ToolMenus API.

## References

All references point to Epic Games Developers documentation unless otherwise noted. For the most up-to-date information, always consult the official UE 5.6 documentation.

---

## UE 5.6 C++ Plugin Power-Guide (Windows)

### 1. UPROPERTY/UFUNCTION Specifiers That Actually Matter (With Safe Defaults)

#### UPROPERTY Buckets You'll Use Constantly

- **Visibility & Blueprint**: VisibleAnywhere, EditAnywhere, BlueprintReadOnly, BlueprintReadWrite
- **Use Category & DisplayName to keep the editor clean**
- **See declaration & specifier syntax and examples** (Epic Games Developers)

- **Config-driven**: Config on members + UCLASS(config=Game) auto-fills from INI
- **Override via hierarchy (Base→Default→User)**
- **Arrays support +, ., -, ! operators for append/dup/remove/clear** (Epic Games Developers)

- **Instancing & GC**: Instanced (own the subobject lifetime), Transient (don't serialize), DuplicateTransient (skip during duplication), TextExportTransient (skip text export)
- **See GC overview for UObject rules** (GitHub)

- **Soft vs hard references**: Prefer TSoftObjectPtr<T> in content-facing structs to decouple loads/cooking; resolve at runtime
- **API reference**: TSoftObjectPtr (Epic Games Developers)

- **Networking**: Replicated, ReplicatedUsing=OnRep_X, optional conditions in GetLifetimeReplicatedProps via DOREPLIFETIME_*
- **Core how-to and API anchors** (Epic Games Developers)

- **Editor UX**: Metadata like ClampMin/Max, UIMin/Max, EditCondition, AdvancedDisplay—keeps details panel sane
- **Specifier catalog**: Start at UPROPERTY page; it links out (Epic Games Developers)

#### UFUNCTION Buckets

- **Blueprint exposure**: BlueprintCallable, BlueprintPure, BlueprintImplementableEvent, BlueprintNativeEvent
- **RPC**: Server, Client, NetMulticast (+ Reliable sparingly)
- **Exec & editor**: Exec for console; CallInEditor for tooling
- **Official specifier reference** (complete lists & constraints) (Epic Games Developers)

#### Pitfall (Replication)
- **Marking a property Replicated isn't enough—you must register it in GetLifetimeReplicatedProps**
- **Engine warns and it won't replicate** (Epic Games Developers, Epic Developer Community Forums)

---

### 2. Subsystems: Pick the Right Lifecycle & Where to Put Work

#### Quick Matrix

- **UEngineSubsystem**: Starts once per engine; great for global CVars, persistent services
- **UEditorSubsystem**: Editor-only automation, menus, validation; loads with editor
- **API anchor** (Epic Games Developers)
- **UGameInstanceSubsystem**: Per game session; owns online/SaveGame managers
- **UWorldSubsystem**: Per UWorld; scene-specific services (spawners, queries)
- **ULocalPlayerSubsystem**: Per player; input/contextual per-user state

**Docs + patterns and when each is created**: Programming Subsystems (Epic Games Developers)

#### Skeleton You Can Paste (Engine-Level Service with Config & CVar)

```cpp
UCLASS(Config=Engine)
class UMyOptimizationService : public UEngineSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& C) override;
    virtual void Deinitialize() override;
private:
    UPROPERTY(Config) int32 BudgetMs = 2;
};
```

**Use this to centralize plugin-wide settings and hot-reload-safe CVars.**

---

### 3. Console Variables & Commands from C++ (Robust Pattern)

- **Prefer TAutoConsoleVariable<T> for thread-aware getters** (GetValueOnGameThread/RenderThread)
- **Register dynamic CVars/commands through IConsoleManager** (and avoid callbacks unless necessary; use sinks or polling)
- **Understand priority sources** (Scalability, DeviceProfile, cmdline, console, etc.) to avoid "my CVar won't change"
- **Full examples & flags** (Epic Games Developers)

---

### 4. Render Dependency Graph (RDG) Cookbook for Plugins

#### Why RDG
- **Correct lifetimes, barriers, and async rules handled for you**
- **Build passes with FRDGBuilder and Execute()** (Epic Games Developers)

#### Core Idioms

- **Import an existing texture** (e.g., UTextureRenderTarget2D) into RDG:
  ```cpp
  GraphBuilder.RegisterExternalTexture(PooledRenderTarget)
  // or
  RegisterExternalTexture(GraphBuilder, FRHITexture*, Name, Flags)
  ```
  (Epic Games Developers)

- **Add a compute/raster pass**:
  ```cpp
  GraphBuilder.AddPass(
      FRDGEventName(TEXT("MyPass")), 
      ERDGPassFlags::Compute, 
      [=](FRHICommandListImmediate& RHICmdList){ /* RHI work */ }
  );
  ```
  (Epic Games Developers)

- **GPU→CPU readback**:
  ```cpp
  AddReadbackTexturePass(
      GraphBuilder, 
      TEXT("Readback"), 
      RDGTex, 
      [&](FRHIGPUTextureReadback* Readback){ ... }
  );
  ```
  (Epic Games Developers)

#### Gotchas the AI Should Enforce

- **You cannot touch created RDG resources outside their passes—lifetimes aren't guaranteed** (Remark on AddPass page) (Epic Games Developers)
- **When bridging editor surfaces/materials and RDG, import/export via pooled targets**
- **Forum recipe shows the RegisterExternalTexture → copy flow** (Epic Developer Community Forums)
- **Further**: RDG concepts & shader-param coupling overview (Epic Games Developers)

---

### 5. Tasks & Threading (Safe Patterns for Gameplay/Tools)

- **Modern tasks**: UE::Tasks::Launch & TTask for lightweight async
- **Use for CPU work that doesn't touch UObjects off-thread**
- **API anchors** (Epic Games Developers)

- **Legacy/utility**: AsyncTask(ENamedThreads::GameThread, ...) to marshal results back
- **Thread enums listed in ENamedThreads::Type** (Epic Games Developers)

- **Profiling tasks**: Record -trace=default,task and inspect in Insights "Task Graph" lanes (Epic Games Developers)

---

### 6. Memory Profiling & Tagging (LLM + Insights) for Optimization Plugins

- **Turn it on**: Run with -LLM or record traces with -trace=memtag
- **LLM data now shows in Memory Insights** (Epic Games Developers)

- **Tag your allocations**: Wrap scopes with LLM_SCOPE(ELLMTag::YourTag) (or project tags)
- **Your plugin's memory shows up under a named bucket**
- **Docs recommend LLM_SCOPE; see Developer Guide to Tracing** (Epic Games Developers)

- **Check quickly**: stat llm / stat llmfull in the console
- **Commands reference** (Epic Games Developers)

---

### 7. Insights Headless Workflows (So You Can Automate in CI)

#### Record
- **Editor/game**: 
  ```bash
  UnrealEditor.exe <uproject> -game -trace=default,task,memtag,gpu -NoVSync -log
  ```
- **Trace quick start & tool pages** (Epic Games Developers)

#### Inspect/Export
- **Open traces in Unreal Insights → use Timing, Tasks, Memory Insights**
- **Feature docs** (Epic Games Developers)

---

### 8. PSO (Pipeline State) Precache & Bundled Caches (Kill Shader Hitching)

- **Runtime PSO precaching compiles needed PSOs up front** (esp. D3D12/Vulkan/Metal)
- **Enable & tune; component precaching happens post-load for UPrimitiveComponent** (Epic Games Developers)

- **Bundled caches**: Build & ship prebuilt PSO cache files to further minimize runtime compilation
- **Official process is documented** (Epic Games Developers)

- **Context/limits**: Epic overview on how precaching evolved; still has coverage gaps (plan for fallbacks) (Unreal Engine)

---

### 9. UObject Pointer Types, Containers, and GC Rules That Avoid Crashes

- **Prefer TObjectPtr<T> for hard references tracked by the engine**
- **TWeakObjectPtr<T> for non-owning, GC-safe handles**
- **TSoftObjectPtr<T> for asset refs that can be lazily loaded**
- **API anchors** (Epic Games Developers)

- **Containers**: See TArray/TMap/TSet docs for semantics/perf
- **If storing UObjects and the container itself isn't a UPROPERTY, consider FGCObject::AddReferencedObjects to keep them rooted** (Epic Games Developers)

- **Replication & subobjects**: UObjects can replicate as actor/comp subobjects; follow the documented path (Epic Games Developers)

---

### 10. Build Automation for Plugins (UAT & BuildGraph)

- **One-shot plugin build** (zips, filters UE headers correctly):
  ```bash
  RunUAT.bat BuildPlugin -Plugin="<Path>\YourPlugin.uplugin" -Package="<OutDir>" -TargetPlatforms=Win64
  ```
- **Tooling page** (Epic Games Developers)

- **BuildGraph for CI pipelines and custom steps**
- **Extend with tasks or UAT BuildCommands when you outgrow simple switches** (Epic Games Developers)

- **Module knobs**: Public/private include paths, dependencies, whitelist platforms
- **Learn the module properties that control how your code compiles/links** (Epic Games Developers)

---

### 11. Config System & Core Redirects (Rename-Proof Your APIs)

- **INI hierarchy and array operators matter when you ship defaults vs per-user overrides**
- **See syntax & load order table** (Epic Games Developers)

- **Core Redirects**: When you rename classes/properties/functions, add redirects so old assets keep loading
- **Know about asset redirectors for content moves** (Epic Games Developers)

---

### 12. Replication: The Minimum Template Your Agents Should Stamp Out

```cpp
void AMyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const
{
    Super::GetLifetimeReplicatedProps(Out);
    DOREPLIFETIME(AMyActor, Health); // or DOREPLIFETIME_CONDITION(...)
}
```

- **Mark UPROPERTY(Replicated) or ReplicatedUsing=OnRep_Health**
- **Implement OnRep_Health(), and include UnrealNetwork.h**
- **Authoritative flow & pitfalls** (Epic Games Developers, Epic Developer Community Forums)

---

### 13. Windows Toolchain Pointers (UE 5.6)

- **Visual Studio 2022 is the supported IDE**
- **Follow Epic's VS setup guidance for C++/Win10+ SDK integration**
- **Rider and VS Code also supported** (Epic Games Developers)

---

### 14. Ready-to-Use Checklists for an Optimization Plugin

#### Startup (EngineSubsystem)
- **Register CVars** (TAutoConsoleVariable) with help text + ECVF_Scalability
- **Read defaults from [/Script/YourPlugin.YourSettings] in DefaultEngine.ini**
- **Start trace if r.YourPlugin.Trace=1** (append -trace=default,task,memtag when launching) (Epic Games Developers)

#### During Work
- **Run CPU tasks via UE::Tasks::Launch**
- **Marshal to game thread with AsyncTask(ENamedThreads::GameThread, ...)** (Epic Games Developers)
- **GPU algorithms via RDG passes only**
- **Import/export textures with RegisterExternalTexture, use AddPass, and readbacks with AddReadbackTexturePass** (Epic Games Developers)
- **Tag heavy allocs with LLM_SCOPE(...) so memory shows under your plugin in Memory Insights** (Epic Games Developers)

#### Shipping
- **Enable PSO precache; if stutters remain, generate bundled PSO cache and ship it** (Epic Games Developers)
- **Lock down redirects if you renamed any public types** (Epic Games Developers)

---

### 15. Fast Snippets Your Agents Can Pattern-Match

#### CVar with Change Detection (Sink)
```cpp
static TAutoConsoleVariable<int32> CVarBudget(
    TEXT("r.MyPlugin.BudgetMs"), 2, TEXT("Frame budget in ms"), ECVF_Scalability);
static void OnCVarChanged(){ /* reconfigure */ }
static FAutoConsoleVariableSink GMySink(
    FConsoleCommandDelegate::CreateStatic(&OnCVarChanged));
```

**Why sink > callback; priority semantics** (Epic Games Developers)

#### RDG Compute Pass Skeleton
```cpp
RDG_EVENT_SCOPE(GraphBuilder, "MyCompute");
FMyPassParameters* Params = GraphBuilder.AllocParameters<FMyPassParameters>();
// Bind SRVs/UAVs on Params...
GraphBuilder.AddPass(
    RDG_EVENT_NAME("MyCompute"),
    Params,
    ERDGPassFlags::Compute,
    [Params](FRHICommandListImmediate& RHICmdList){ /* Dispatch */ });
```

**AddPass contract and lifetime cautions** (Epic Games Developers)

#### LLM Tag
```cpp
#include "ProfilingDebugging/MemoryTrace.h"
void FMySystem::DoWork(){ 
    LLM_SCOPE(ELLMTag::Audio); /* or your custom tag */ 
    /* ... */ 
}
```

**LLM overview + Memory Insights panel** (Epic Games Developers)

---

### 16. Where to Browse When You Need the Raw API

- **Complete UE 5.6 C++ API index** (canonical entry point) (Epic Games Developers)
- **Console Variables & Commands** (C++ mechanics + references) (Epic Games Developers)
- **RDG API surfaces** (FRDGBuilder, AddPass, RegisterExternalTexture, readbacks) (Epic Games Developers)
- **Subsystems** (concept + editor subsystem API) (Epic Games Developers)
- **Insights** (Timing/Tasks/Memory) & Tracing (CLI and UI) (Epic Games Developers)
- **PSO precache/bundled cache docs** (Epic Games Developers)
- **Replication for actors, components, and UObjects** (plus GetLifetimeReplicatedProps) (Epic Games Developers)
- **Config files & hierarchy** (syntax, arrays, load order) (Epic Games Developers)