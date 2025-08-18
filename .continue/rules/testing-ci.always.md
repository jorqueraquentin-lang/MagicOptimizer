
# Testing and CI - Always Apply

## 🧪 **MANDATORY: Use Built-in CI System**

### **ALWAYS Prefer Automation Over Manual Testing**
```bash
# ✅ CORRECT: Use automated testing
tools/run_ci.ps1 -Phase Audit -FailOnError -MinRows 1

# ❌ WRONG: Manual testing only
# Manual testing is error-prone and inconsistent
```

### **CI System Benefits**
- **Consistent results** across different environments
- **Comprehensive logging** with before/after states
- **Regression testing** after implementing fixes
- **Eliminates human error** in testing
- **Build validation** integrated into testing process

## 🔨 **MANDATORY: Build Validation**

### **Build Validation Required After Every Change**
```bash
# Every code change must include build validation:
1. Build plugin using direct development build workflow
2. Verify plugin loads in editor
3. Test affected functionality
4. Document build status in development log
```

### **Build Validation Steps**
```bash
# Complete build validation process:
1. Regenerate project files in UE5.6 (if needed)
2. Build plugin directly in HostProject/Plugins/MagicOptimizer/
3. Verify plugin DLLs generated in correct location
4. Test plugin loads in editor without crashes
5. Verify UI panel opens and displays correctly
6. Test affected functionality works as expected
7. Check for UHT/UBT warnings (treat as errors)
```

### **Build Validation Success Indicators**
- ✅ **Plugin DLLs generated** in `HostProject/Plugins/MagicOptimizer/Binaries/Win64/`
- ✅ **Plugin loads in editor** without crashes
- ✅ **UI panel opens** and displays correctly
- ✅ **All features functional** for testing
- ✅ **No UHT/UBT warnings** (clean build)
- ✅ **Build timestamps match** across all plugin DLLs

## 📊 **MANDATORY: Log Analysis After Every Change**

### **Immediate Log Check Required**
After ANY fix or change:
1. **Check generated log files** immediately
2. **Compare before/after log states** to verify the fix worked
3. **Look for error patterns** that might indicate new issues
4. **Document the log analysis** in responses
5. **Logs contain the truth** about what's happening
6. **Check build logs** for compilation warnings or errors

### **Log Analysis Commands**
```bash
# Check CI logs
Get-Content "Reports/ci_*.log" | Select-String "ERROR\|WARNING\|FAILED"

# Compare before/after states
Get-ChildItem "Reports/" | Sort-Object LastWriteTime -Descending | Select-Object -First 5

# Check build logs for warnings
Get-Content "HostProject/Saved/Logs/*.log" | Select-String "Warning\|Error\|Failed"
```

## 🔍 **Testing Validation Steps**

### **Before Testing**
- ✅ **All plugin DLLs rebuilt** together
- ✅ **Build timestamps match** across modules
- ✅ **Project files regenerated** if needed
- ✅ **Plugin loads in editor** without crashes
- ✅ **Build validation completed** successfully

### **During Testing**
- ✅ **Use CI system** when possible
- ✅ **Monitor log output** in real-time
- ✅ **Verify build status** maintained throughout testing
- ✅ **Check plugin functionality** in editor

### **After Testing**
- ✅ **Check generated logs** immediately
- ✅ **Analyze error patterns** if any
- ✅ **Update development log** with results
- ✅ **Verify fix actually worked** through logs
- ✅ **Confirm build status** still valid
- ✅ **Verify plugin still loads** in editor

## ⚠️ **Common Testing Mistakes**

### **Ghost Code Trap**
```cpp
// ❌ SYMPTOM: Old code still runs after "fixes"
// ❌ CAUSE: Partial rebuilds, mismatched DLL timestamps
// ✅ SOLUTION: Rebuild ALL plugin DLLs together
```

### **Log Ignorance**
```cpp
// ❌ SYMPTOM: "Fix didn't work" without log analysis
// ❌ CAUSE: Not checking logs after changes
// ✅ SOLUTION: ALWAYS check logs immediately after changes
```

### **Manual Testing Only**
```cpp
// ❌ SYMPTOM: Inconsistent test results
// ❌ CAUSE: Relying only on manual testing
// ✅ SOLUTION: Use CI system for consistent, reproducible results
```

### **Build Validation Skipped**
```cpp
// ❌ SYMPTOM: Plugin not working after changes
// ❌ CAUSE: Skipping build validation after code changes
// ✅ SOLUTION: ALWAYS validate build after every change
```

### **Packaging Used for Development**
```cpp
// ❌ SYMPTOM: Plugin DLLs not in development location
// ❌ CAUSE: Using packaging script for development testing
// ✅ SOLUTION: Use direct development build workflow for development
```

## 🎯 **Success Metrics**

- ✅ **CI system used** for testing when possible
- ✅ **Logs analyzed** after every change
- ✅ **Before/after states** compared and documented
- ✅ **Error patterns** identified and addressed
- ✅ **Development log updated** with test results
- ✅ **Fix effectiveness** verified through log analysis
- ✅ **Build validation completed** after every change
- ✅ **Plugin loads in editor** after all changes
- ✅ **Build status maintained** throughout testing

## 🛡️ **Behavior Preservation Verification - CRITICAL**

### **Validation Checklist**
- ✅ **Clean rebuild** of all plugin modules without warnings from UHT or UBT
- ✅ **Feature reproduces** the same behavior as before the fix
- ✅ **No features disabled**, hidden, or removed
- ✅ **CI or scripted test run** passes and logs are attached
- ✅ **Packaging still succeeds** with the plugin enabled
- ✅ **Build validation passes** after every change
- ✅ **Plugin loads in editor** consistently

### **Behavior Verification Steps**
1. **Exercise affected features** end-to-end to confirm behavior is unchanged
2. **Compare outputs** before and after the fix
3. **Verify UI elements** still function as expected
4. **Check performance** characteristics remain similar
5. **Validate data integrity** and consistency
6. **Verify build status** maintained throughout testing
7. **Confirm plugin functionality** in editor

### **Regression Testing Requirements**
- **Run comprehensive tests** that cover the affected functionality
- **Test edge cases** and error conditions
- **Verify integration points** with other systems
- **Check backward compatibility** if APIs were modified
- **Validate build process** still works correctly
- **Verify plugin loading** in editor

### **Evidence Collection**
- **Screenshots** of before/after states
- **Log files** showing successful execution
- **Performance metrics** if applicable
- **Test results** from CI system
- **User acceptance** verification if UI was modified
- **Build validation results** after every change
- **Plugin loading verification** in editor

## 🔄 **Build Testing Integration**

### **Development Build Testing**
```bash
# Test development build workflow:
1. Make code changes
2. Use direct development build workflow
3. Verify plugin loads in editor
4. Test affected functionality
5. Document build status in development log
```

### **CI Build Validation**
```bash
# CI should validate build process:
1. Build plugin using direct development workflow
2. Verify plugin loads in headless editor
3. Test core functionality
4. Collect build artifacts and logs
5. Report build status and any issues
```

### **Build Failure Handling**
```bash
# When build fails:
1. Check compilation errors in logs
2. Verify module dependencies are correct
3. Check for syntax errors or missing includes
4. Regenerate project files if needed
5. Attempt rebuild with clean environment
6. Document build failure in development log
```

## 📋 **Testing Workflow Integration**

### **Complete Testing Workflow**
1. **Code Changes**: Implement features following "baby steps and testing"
2. **Build Validation**: Use direct development build workflow
3. **Editor Testing**: Verify plugin loads and functions in editor
4. **CI Testing**: Run automated tests using CI system
5. **Log Analysis**: Check all logs for errors or warnings
6. **Documentation**: Update development log with results
7. **Iteration**: Continue development cycle

### **Integration with Other Rules**
- **Build & Packaging**: Build validation integrated into testing
- **Development Workflow**: Testing includes build verification
- **UE5.6.1 Plugin Builder**: Testing validates build architecture
- **Project Sanity Check**: Testing results included in regular checks

description: Testing and CI rules that enforce automated testing, comprehensive log analysis, build validation, development build testing, and behavior preservation verification for the MagicOptimizer UE5 plugin project.
globs: ["**/*.cpp", "**/*.h", "**/*.py", "**/*.ps1", "**/*.md"]
alwaysApply: true
---
