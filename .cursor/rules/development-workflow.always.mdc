
# Development Workflow - Always Apply

## 🔄 **MANDATORY: Incremental Development**

### **One Change at a Time**
```cpp
// ✅ CORRECT: Make one focused change, test, then continue
void MyFunction() {
    // Add one feature
    // Test it works
    // Add next feature
    // Test again
}

// ❌ WRONG: Multiple changes without testing
void MyFunction() {
    // Add feature 1
    // Add feature 2  
    // Add feature 3
    // Hope it all works together
}
```

### **Development Cycle**
1. **Plan** the next small step
2. **Implement** the change
3. **Build** the plugin using direct development build workflow
4. **Test** using CI system when possible
5. **Verify** the result works
6. **Document** in development log
7. **Perform sanity check** (see project-sanity-check.always.mdc rule)
8. **Repeat** for next step

## 🔨 **MANDATORY: Build Process Integration**

### **Build Process in Development Cycle**
```bash
# Every development change requires build verification:
1. Make code changes
2. Build plugin using direct development build workflow
3. Verify plugin loads in editor
4. Test affected functionality
5. Continue with next change
```

### **Direct Development Build Workflow**
```bash
# Use this workflow for ALL development work:
1. Regenerate project files in UE5.6 (if needed)
2. Build plugin directly in HostProject/Plugins/MagicOptimizer/
3. Test immediately in editor
4. Iterate and test again
```

**When to Regenerate Project Files:**
- ✅ **Plugin modules added/removed/modified**
- ✅ **.uplugin file changes**
- ✅ **Build.cs dependencies modified**
- ✅ **After major structural changes**
- ✅ **When switching between development branches**

### **Build Verification Steps**
```bash
# After every build, verify:
1. Plugin DLLs generated in correct location
2. Plugin loads in editor without crashes
3. UI panel opens and displays correctly
4. All features functional for testing
5. No UHT/UBT warnings (treat as errors)
```

## 📝 **MANDATORY: Development Log Updates**

### **After Every Task**
- **Append timestamped entry** to `docs/DEVELOPMENT_LOG.md`
- **Document what was implemented**
- **Record any issues encountered**
- **Note next steps**
- **Include file paths and line numbers** for significant changes
- **Include build status** and verification results

### **Log Entry Format**
```markdown
### 2025-08-17 HH:MM — TASK DESCRIPTION

**STATUS**: ✅ COMPLETED / 🔄 IN PROGRESS / ❌ FAILED

**SUMMARY**: Brief description of what was accomplished

**BUILD STATUS**: ✅ SUCCESS / ❌ FAILED / ⚠️ WARNINGS

**FILES MODIFIED**:
- `path/to/file.cpp` - Description of changes
- `path/to/file.h` - Description of changes

**BUILD VERIFICATION**:
- Plugin loads in editor: ✅/❌
- UI panel opens: ✅/❌
- Features functional: ✅/❌

**NEXT STEPS**: What should be done next
```

## 🏗️ **Repository Structure Rules**

*Note: Repository structure is defined in `ue561-plugin-builder.mdc` to avoid duplication.*

**NEVER** keep duplicate plugin code at repo root.

## 🔧 **Module Boundary Rules**

*Note: Detailed module boundary rules and WITH_EDITOR guards are defined in `ue561-plugin-builder.mdc` to avoid duplication.*

## 📚 **Documentation Requirements**

### **Code Documentation**
- **Header files**: Document public API with clear descriptions
- **Complex functions**: Add inline comments explaining logic
- **API changes**: Update header documentation immediately

### **Process Documentation**
- **Development log**: Updated after every task
- **README files**: Keep current and accurate
- **API documentation**: Maintain for public interfaces
- **Build documentation**: Include build status and verification results

## 🚫 **Development Restrictions**

### **What NOT to Do**
- ❌ **Don't touch Engine source** code
- ❌ **Don't change engine version** (UE 5.6.1 only)
- ❌ **Don't add third-party dependencies** without asking
- ❌ **Don't violate module boundaries**
- ❌ **Don't skip testing** after changes
- ❌ **Don't forget to update** development log
- ❌ **Don't use packaging script** for development
- ❌ **Don't skip build verification** after changes

### **What to ALWAYS Do**
- ✅ **Follow existing naming conventions**
- ✅ **Preserve working behavior**
- ✅ **Test after every change**
- ✅ **Update development log**
- ✅ **Use CI system** when possible
- ✅ **Check logs** after testing
- ✅ **Use direct development build** workflow
- ✅ **Verify build success** in editor
- ✅ **Test affected functionality** after build

## 🎯 **Success Metrics**

- ✅ **One change at a time** implemented and tested
- ✅ **Development log updated** after every task
- ✅ **Module boundaries preserved**
- ✅ **Existing code style maintained**
- ✅ **Working behavior preserved**
- ✅ **All changes tested** before continuing
- ✅ **Documentation kept current**
- ✅ **Build process integrated** into development cycle
- ✅ **Build verification completed** after every change
- ✅ **Direct development build workflow** used consistently

## 🚪 **Approval Gate System - CRITICAL**

### **When Approval is Required**
- **API changes**: Any modification to public interfaces or data schemas
- **File layout changes**: Moving, renaming, or restructuring files
- **Module dependencies**: Changes to Build.cs dependencies or .uplugin configuration
- **Behavior changes**: Any modification that affects user experience or output
- **Build process changes**: Modifications to build workflow or build configuration

### **Approval Request Format**
```
APPROVAL NEEDED: <summary>
Impact: <API/file layout/module/behavior/build>
Why not destructive: <reason>
Proposed diff scope: <files>
Rollback: <git ref or plan>
Proceed? yes/no
```

### **Required Response Format**
**Before editing files, reply with:**
```
Plan:
Impact:
Alternatives considered:
Why this is non destructive:
Validation plan:
Build verification plan:
```

**After implementation, reply with:**
```
Diff summary:
Behavior verification:
Build + test evidence:
Build verification results:
Packaging check:
```

## 📋 **Change Workflow Requirements**

### **Required Steps for Any Fix**
1. **Plan**: Produce a plan with root cause, minimal fix, alternatives rejected, risk assessment
2. **Approval**: Ask for confirmation when any API, file layout, module config, or build process might change
3. **Implement**: Apply the smallest fix. Do not edit unrelated files
4. **Build**: Use direct development build workflow to compile plugin
5. **Verify**: Verify plugin loads in editor and affected features work
6. **Validate**: Rebuild all plugin DLLs together, run tests, exercise affected features
7. **Report**: Output change report with what was broken, what changed, why non-destructive, validation results

### **Build Integration Requirements**
```bash
# Every change must include:
1. Code implementation
2. Plugin build using direct development workflow
3. Build verification in editor
4. Functionality testing
5. Build status documentation in development log
```

### **Commit Template**
```
feat/fix: <one line summary preserving behavior>

Root cause:
Change:
Why non destructive:
Build verification:
Tests/validation:
Affected modules:
```

## 🔄 **Build Workflow Integration**

### **Development Session Build Flow**
1. **Start Session**: Ensure clean development environment
2. **Regenerate Files**: Regenerate project files if needed
3. **Make Changes**: Implement features following "baby steps and testing"
4. **Build Plugin**: Use direct development build workflow
5. **Verify Build**: Check plugin loads in editor
6. **Test Changes**: Test affected functionality
7. **Iterate**: Continue development cycle
8. **Package When Ready**: Use packaging script only for distribution

### **Integration with Other Rules**
- **Build & Packaging**: Development build workflow integrated
- **UE5.6.1 Plugin Builder**: Build process included in development cycle
- **Testing & CI**: Build validation included in testing process
- **Project Sanity Check**: Build status included in regular checks

description: Development workflow rules that enforce incremental development, build process integration, proper logging, module boundary compliance, and approval gate requirements for the MagicOptimizer UE5 plugin project.
globs: ["**/*.cpp", "**/*.h", "**/*.md", "**/*.py"]
alwaysApply: true
---
