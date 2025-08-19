
# UE 5.6.1 Plugin Builder - Core Rules

## 🎯 **Scope and Boundaries**

**CRITICAL**: This rule applies to ALL MagicOptimizer plugin development work.

### **Target Environment**
- **Engine Version**: Unreal Engine 5.6.1 ONLY
- **Work Area**: Keep edits inside `HostProject/Plugins/MagicOptimizer/**`
- **Engine Source**: NEVER touch Engine source code
- **Single Source**: All code edits live under `HostProject/Plugins/MagicOptimizer/**`

### **Module Architecture**
- **MagicOptimizer**: Runtime module (Core, CoreUObject, Engine dependencies)
- **MagicOptimizerEditor**: Editor module (Editor dependencies in Private only)
- **Module Boundaries**: Keep headers minimal, prefer forward declarations

## 🏗️ **Repository Structure**

```
MagicOptimizer/                    # Repo root (docs/, tools/, CI only)
├── .cursor/rules/                # Project rules (this file)
├── docs/                         # Documentation
├── tools/                        # Development tools
├── Reports/                      # Generated reports
└── HostProject/                  # UE project
    └── Plugins/MagicOptimizer/   # SINGLE SOURCE OF TRUTH
        ├── Source/
        │   ├── MagicOptimizer/   # Runtime module
        │   └── MagicOptimizerEditor/ # Editor module
        ├── Content/Python/        # Plugin-shipped Python
        └── MagicOptimizer.uplugin # Plugin descriptor
```

**NEVER** keep a duplicate plugin at the repo root.

## 🔨 **Build Architecture - CRITICAL**

### **Multi-DLL Plugin Reality**
UE5 plugins often have multiple DLLs that MUST be rebuilt together:

```cpp
// ❌ WRONG: Partial rebuilds cause "ghost" behavior
// Old code still runs, debugging becomes impossible

// ✅ CORRECT: Rebuild ALL plugin DLLs together
// Check build timestamps match across all plugin DLLs
```

### **Development Build Process (PRIMARY WORKFLOW)**
```bash
# ✅ CORRECT: Direct plugin development workflow
1. Regenerate project files in UE5.6
2. Build plugin directly in HostProject/Plugins/MagicOptimizer/
3. Test immediately in editor
4. Iterate and test again
```

**When to Use Development Build:**
- ✅ **During active development** and feature implementation
- ✅ **When testing changes** and debugging
- ✅ **For immediate validation** of modifications
- ✅ **Following "baby steps and testing"** philosophy

### **Project File Regeneration Requirements**
```bash
# MANDATORY: Regenerate project files when:
1. Plugin modules added/removed/modified
2. .uplugin file changes
3. Build.cs dependencies modified
4. After major structural changes
5. When switching between development branches
```

**Regeneration Process:**
1. **Open HostProject.uproject** in UE5.6
2. **Choose "Yes"** to regenerate project files
3. **Wait for completion** - Visual Studio solution will be generated
4. **Verify solution** contains all plugin modules

### **Direct Plugin Build Steps**
```bash
# Complete Development Build Process:
1. Open HostProject.sln in Visual Studio 2022
2. Build Solution (Build → Build Solution)
3. Target: Development Editor | Win64
4. Verify plugin DLLs generated in:
   - HostProject/Plugins/MagicOptimizer/Binaries/Win64/
   - Should contain MagicOptimizer.dll and MagicOptimizerEditor.dll
5. Test plugin in UE5.6 Editor
```

### **Build Rules**
1. **Use direct development build** for all development work
2. **Regenerate project files** when descriptor or modules change
3. **Windows baseline**: Visual Studio 2022 + current Windows SDK
4. **Clean Binaries/Intermediate** before major rebuilds
5. **Verify all plugin DLLs** rebuilt together

### **Distribution Build (ONLY for Distribution)**
```bash
# Use packaging script ONLY for distribution:
Scripts/PackagePlugin.ps1 -EngineRoot "C:\Path\To\UE5.6" -Clean -NoHostPlatform
```

**When to Use Distribution Build:**
- ✅ **Creating plugin packages** for distribution
- ✅ **Testing packaged plugin** in fresh projects
- ✅ **Preparing releases** for marketplace or distribution
- ❌ **NOT for development** or testing changes

## 📝 **Coding Standards**

### **API Surface**
```cpp
// ✅ CORRECT: Use proper API macros
MAGICOPTIMIZER_API void MyFunction();

// ❌ WRONG: Missing API macro
void MyFunction(); // Won't export properly
```

### **Naming Conventions**
- **Existing code style takes precedence** over new conventions
- **Follow established patterns** in the codebase
- **Preserve working behavior** and module boundaries

## 🧪 **Testing and Validation - CRITICAL**

*Note: Detailed testing and CI requirements are defined in `testing-ci.always.mdc` to avoid duplication.*

**NEVER** keep duplicate plugin code at repo root.

## 📦 **Python and Assets**

*Note: Detailed Python packaging and asset classification rules are defined in `python-assets.always.mdc` to avoid duplication.*

## 🔄 **Development Workflow**

*Note: Detailed development workflow is defined in `development-workflow.always.mdc` to avoid duplication.*

**NEVER** keep duplicate plugin code at repo root.

## ⚠️ **Common Pitfalls**

*Note: Detailed build pitfalls and common issues are defined in `build-packaging.always.mdc` to avoid duplication.*

### **Development Build Pitfalls**
```cpp
// ❌ SYMPTOM: Plugin DLLs not generated in development location
// ❌ CAUSE: Using packaging script for development
// ✅ SOLUTION: Use direct development build workflow

// ❌ SYMPTOM: Project files not regenerated after module changes
// ❌ CAUSE: Skipping project file regeneration
// ✅ SOLUTION: Always regenerate when modules change

// ❌ SYMPTOM: Plugin not appearing in editor after build
// ❌ CAUSE: Build failed or DLLs not in correct location
// ✅ SOLUTION: Verify build success and DLL location
```

## 🎯 **Success Metrics**

- ✅ **All plugin DLLs rebuilt** together
- ✅ **Build timestamps match** across modules
- ✅ **Module boundaries preserved**
- ✅ **Existing code style maintained**
- ✅ **Non-destructive change principles** followed
- ✅ **UE5.6.1 constraints** respected
- ✅ **Development build workflow** used for development
- ✅ **Project files regenerated** when needed
- ✅ **Plugin loads in editor** after build

## 🛡️ **Non-Destructive Change Policy - CRITICAL**

### **Core Principles**
- **Preserve behavior**: Outputs and user experience must stay the same unless explicitly approved
- **Minimal change**: Prefer the smallest, local fix near the error site
- **No feature loss**: Never remove or deactivate features to resolve build errors
- **No "light" variants**: Do not create reduced versions or temporary forks to "get a green build"

### **UE-Specific Guardrails**
- **Do not modify** PublicDependencyModuleNames or PrivateDependencyModuleNames to avoid compiling real deps
- **Do not change** .uplugin Modules, Type, LoadingPhase, or EnabledByDefault to skip loading
- **Do not move code** from MagicOptimizerEditor into MagicOptimizer to dodge editor-only build errors
- **Do not touch Engine source** or global DefaultEngine.ini to mask issues
- **Keep editor-only dependencies** inside the editor module

### **Destructive Operations Forbidden**
- ❌ Deleting or commenting out classes, functions, or modules to bypass errors
- ❌ Disabling code paths, features, delegates, or UI that previously worked
- ❌ Changing public APIs or data schemas without an approved migration
- ❌ Replacing implementations with hardcoded stubs that alter behavior
- ❌ Creating "lite" or "safe" copies instead of fixing the real issue

### **Allowed Moves (Preferred)**
- ✅ Correct include paths, types, macros, build settings that fix compilation
- ✅ Add narrowly scoped null checks or guards that preserve prior behavior
- ✅ Fix lifecycle or threading issues without changing public contracts
- ✅ Gate new code with compile or runtime flags that default to current behavior

## 🔄 **Build Workflow Integration**

### **Development Session Build Flow**
1. **Start Session**: Ensure clean development environment
2. **Regenerate Files**: Regenerate project files if needed
3. **Make Changes**: Implement features following "baby steps and testing"
4. **Build Plugin**: Use direct development build workflow
5. **Test Changes**: Test immediately in editor
6. **Iterate**: Continue development cycle
7. **Package When Ready**: Use packaging script only for distribution

### **Integration with Other Rules**
- **Build & Packaging**: Development build workflow integrated
- **Development Workflow**: Build process included in development cycle
- **Testing & CI**: Build validation included in testing process
- **Project Sanity Check**: Build status included in regular checks

description: UE 5.6.1 plugin builder rules that enforce proper plugin architecture, development build processes, project file regeneration, canonical development location, and non-destructive change principles for the MagicOptimizer UE5 plugin project.
globs: ["**/*.cpp", "**/*.h", "**/*.md", "**/*.py", "**/*.uplugin", "**/*.uproject"]
alwaysApply: true
---

