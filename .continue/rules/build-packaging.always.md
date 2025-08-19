
# Build and Packaging - Always Apply

## 🔨 **CRITICAL: Multi-DLL Plugin Architecture**

### **UE5 Plugin Reality**
UE5 plugins often have multiple DLLs that MUST be rebuilt together:

```cpp
// ❌ WRONG: Partial rebuilds cause "ghost" behavior
// Old code still runs, debugging becomes impossible

// ✅ CORRECT: Rebuild ALL plugin DLLs together
// Check build timestamps match across all plugin DLLs
```

### **Required Build Steps**
1. **Clean plugin Binaries and Intermediate** before building
2. **Rebuild ALL plugin modules** together
3. **Verify build timestamps match** across all plugin DLLs
4. **Check file sizes** to ensure complete rebuilds

## 🏗️ **Build Configuration**

### **Module Dependencies**
```cpp
// ✅ CORRECT: Runtime module dependencies
PublicDependencyModuleNames.AddRange({
    "Core",
    "CoreUObject", 
    "Engine"
});

// ✅ CORRECT: Editor module dependencies (Private only)
PrivateDependencyModuleNames.AddRange({
    "EditorStyle",
    "UnrealEd"
});
```

### **PCH Usage**
```cpp
// ✅ CORRECT: Use explicit or shared PCHs
PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

// ❌ WRONG: Default PCH usage
PCHUsage = PCHUsageMode.Default;
```

## 🚨 **CRITICAL: Development vs Distribution Builds**

### **Development Build (RECOMMENDED for Development)**
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

**Benefits of Development Build:**
- **Immediate testing** in editor environment
- **Faster iteration cycles** without packaging overhead
- **Direct access** to plugin in development location
- **Consistent with project history** and established patterns

### **Distribution Build (ONLY for Distribution)**
```bash
# ✅ CORRECT: Use packaging script for distribution only
Scripts/PackagePlugin.ps1 -EngineRoot "C:\Path\To\UE5.6" -Clean -NoHostPlatform
```

**When to Use Distribution Build:**
- ✅ **Creating plugin packages** for distribution
- ✅ **Testing packaged plugin** in fresh projects
- ✅ **Preparing releases** for marketplace or distribution
- ❌ **NOT for development** or testing changes

**Why NOT for Development:**
- ❌ **Isolates build** from development location
- ❌ **Plugin DLLs not generated** in original location
- ❌ **Local development environment** not ready for testing
- ❌ **Breaks established workflow** of direct development

## 🔄 **Direct Plugin Development Workflow**

### **Complete Development Build Process**
```bash
# Step 1: Project File Regeneration
1. Open HostProject.uproject in UE5.6
2. Choose "Yes" to regenerate project files
3. Wait for Visual Studio solution to generate

# Step 2: Direct Plugin Build
4. Open HostProject.sln in Visual Studio 2022
5. Build Solution (Build → Build Solution)
6. Target: Development Editor | Win64

# Step 3: Build Verification
7. Check for generated plugin DLLs:
   - HostProject/Plugins/MagicOptimizer/Binaries/Win64/
   - Should contain MagicOptimizer.dll and MagicOptimizerEditor.dll

# Step 4: Functionality Testing
8. Return to UE5.6 Editor
9. Look for 'Magic Optimizer' in Window menu
10. Open plugin panel to verify functionality
```

### **Development Build Success Indicators**
- ✅ **Project files regenerate** without errors
- ✅ **Visual Studio solution builds** successfully
- ✅ **Plugin DLLs generated** in correct location
- ✅ **Plugin loads in editor** without crashes
- ✅ **UI panel opens** and displays correctly

## 📦 **Packaging Requirements**

### **Canonical Entry Point**
```bash
# ✅ CORRECT: Use the official packaging script
Scripts/PackagePlugin.ps1

# ❌ WRONG: Manual packaging commands
RunUAT BuildPlugin -Plugin="..." -Package="..."
```

### **Windows Packaging**
```bash
# ✅ CORRECT: Avoid host rules scans
Scripts/PackagePlugin.ps1 -NoHostPlatform

# ❌ WRONG: Default packaging on Windows
Scripts/PackagePlugin.ps1
```

### **Pre-Packaging Checklist**
- ✅ **Clean Binaries/Intermediate** directories
- ✅ **Regenerate project files** if modules changed
- ✅ **Verify plugin descriptor** is correct
- ✅ **Check FilterPlugin.ini** excludes dev content
- ✅ **Confirm plugin works** in development build first

## 🔍 **Build Validation**

### **Post-Build Verification**
1. **Check all plugin DLLs** were rebuilt
2. **Verify build timestamps** match across modules
3. **Test plugin loads** in editor
4. **Check for UHT/UBT warnings** (treat as errors)

### **Common Build Issues**
```cpp
// ❌ SYMPTOM: "Plugin not found" errors
// ❌ CAUSE: Missing or incorrect .uplugin file
// ✅ SOLUTION: Verify plugin descriptor and module paths

// ❌ SYMPTOM: Linker errors
// ❌ CAUSE: Missing dependencies or incorrect module names
// ✅ SOLUTION: Check PublicDependencyModuleNames and PrivateDependencyModuleNames

// ❌ SYMPTOM: Plugin DLLs not generated in development location
// ❌ CAUSE: Using packaging script for development
// ✅ SOLUTION: Use direct development build workflow
```

## 📁 **Output Management**

### **Development Build Output Location**
```
HostProject/Plugins/MagicOptimizer/Binaries/Win64/  # Development location
├── MagicOptimizer.dll                              # Runtime module
└── MagicOptimizerEditor.dll                        # Editor module
```

### **Package Output Location**
```
HostProject/Artifacts/                    # Package output directory
├── MagicOptimizer-v1.0.0.zip            # Packaged plugin
├── MagicOptimector-v1.0.0/              # Extracted package
└── BuildLogs/                            # Build artifacts
```

### **Version Management**
- **Update CHANGELOG** before packaging
- **Bump Version fields** in plugin descriptor
- **Tag releases** in git after successful packaging
- **Store packaged output** under Artifacts directory

## ⚠️ **Build Pitfalls**

### **Ghost Code Trap**
```cpp
// ❌ SYMPTOM: Old code still runs after "fixes"
// ❌ CAUSE: Partial rebuilds, mismatched DLL timestamps
// ✅ SOLUTION: Rebuild ALL plugin DLLs together
```

### **Module Boundary Violations**
```cpp
// ❌ SYMPTOM: Editor code in runtime module
// ❌ CAUSE: Missing WITH_EDITOR guards
// ✅ SOLUTION: Use WITH_EDITOR guards for editor-only code
```

### **Package-Install Cycle Trap**
```cpp
// ❌ SYMPTOM: Plugin DLLs not in development location
// ❌ CAUSE: Using packaging script for development
// ✅ SOLUTION: Use direct development build workflow
// ❌ SYMPTOM: Local development environment not ready
// ❌ CAUSE: Plugin isolated in packaged location
// ✅ SOLUTION: Build directly in HostProject/Plugins/MagicOptimizer/
```

## 🎯 **Success Metrics**

### **Development Build Success**
- ✅ **All plugin DLLs rebuilt** together
- ✅ **Build timestamps match** across modules
- ✅ **No UHT/UBT warnings** (clean build)
- ✅ **Plugin loads successfully** in editor
- ✅ **UI panel opens** and displays correctly
- ✅ **All features functional** for testing

### **Distribution Build Success**
- ✅ **Packaging completes** without errors
- ✅ **Version fields updated** before packaging
- ✅ **Artifacts stored** in correct location
- ✅ **Packaged plugin works** in fresh projects

## 🔄 **Workflow Integration**

### **Development Session Workflow**
1. **Start Development**: Use direct plugin build workflow
2. **Make Changes**: Follow "baby steps and testing" approach
3. **Test Changes**: Test immediately in editor after each change
4. **Iterate**: Continue development cycle
5. **Package When Ready**: Use packaging script only for distribution

### **Integration with Other Rules**
- **Development Workflow**: Build process integrated into development cycle
- **Testing & CI**: Build validation included in testing process
- **Project Sanity Check**: Build status included in regular checks
- **UE5.6.1 Plugin Builder**: Build architecture aligned with module structure

description: Build and packaging rules that enforce proper build processes, development vs distribution build distinction, direct plugin development workflow, and build validation for the MagicOptimizer UE5 plugin project.
globs: ["**/*.cpp", "**/*.h", "**/*.uplugin", "**/*.uproject", "**/*.ps1", "**/*.bat"]
alwaysApply: true
---
---
