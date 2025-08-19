# File Safety and Verification - Always Apply

## 🎯 **Scope and Purpose**

This rule enforces file protection practices, verification requirements, and safety measures for MagicOptimizer development.

## 🛡️ **File Purpose Verification**

### **Mandatory Verification Requirements**
- **NEVER assume** a file's purpose without checking its context
- **Always examine** file contents before deleting or modifying
- **Verify file relationships** and dependencies
- **Check file history** and usage patterns
- **Understand file role** in the project architecture

### **Before Any File Operation**
1. **Read file contents** to understand purpose
2. **Check file references** and dependencies
3. **Verify file location** in project structure
4. **Review file history** and recent changes
5. **Confirm operation safety** with team if uncertain

## 📁 **File Protection Requirements**

### **DEVELOPMENT_LOG.md Protection**
- **Append-only file**: Never modify existing content
- **Only add new entries** at the end
- **Preserve all historical** information
- **Backup before any** modifications
- **Version control** all changes

### **Critical File Categories**
- **Configuration files**: `.ini`, `.uproject`, `.uplugin`
- **Source code**: `.cpp`, `.h`, `.cs`
- **Documentation**: `.md`, `.txt`
- **Build scripts**: `.ps1`, `.bat`
- **Project files**: `.sln`, `.vcxproj`

## 🔍 **File Analysis Process**

### **Context Investigation Steps**
1. **File location analysis** - where is it in the project?
2. **Dependency mapping** - what depends on this file?
3. **Usage pattern analysis** - how is it used?
4. **History review** - what changes have been made?
5. **Purpose verification** - what is its role?

### **Safety Assessment Questions**
- **Is this file referenced** by other parts of the system?
- **Does it contain critical** configuration or data?
- **Is it part of the build** or deployment process?
- **Would removing it break** functionality?
- **Is there a backup** or version control?

## 🚫 **Forbidden Operations**

### **What NOT to Do**
- ❌ **Don't delete files** without understanding their purpose
- ❌ **Don't modify files** without checking context
- ❌ **Don't assume file roles** based on names alone
- ❌ **Don't skip verification** steps
- ❌ **Don't ignore file dependencies**

### **What to ALWAYS Do**
- ✅ **Verify file purpose** before any operation
- ✅ **Check file context** and relationships
- ✅ **Backup critical files** before changes
- ✅ **Document file operations** in development log
- ✅ **Test system functionality** after file changes

## 🔄 **Safe File Operations**

### **Modification Process**
1. **Create backup** of original file
2. **Make minimal changes** to achieve goal
3. **Test functionality** after changes
4. **Update documentation** to reflect changes
5. **Commit changes** with clear description

### **Deletion Process**
1. **Verify file is not needed** by any system component
2. **Check for references** in other files
3. **Create backup** before deletion
4. **Test system** after removal
5. **Document deletion** in development log

## 📊 **File Safety Metrics**

### **Success Indicators**
- ✅ **No critical files** accidentally deleted or modified
- ✅ **All file operations** properly verified
- ✅ **System functionality** preserved after changes
- ✅ **File dependencies** properly understood
- ✅ **Backups created** for all critical operations

### **Risk Mitigation**
- **Regular backups** of critical files
- **Version control** for all changes
- **Documentation** of file purposes
- **Team communication** about file changes
- **Testing procedures** after file operations

## 🎯 **Success Metrics**

- ✅ **All file operations** properly verified
- ✅ **No accidental file** deletions or corruptions
- ✅ **System functionality** preserved after changes
- ✅ **File purposes clearly** understood
- ✅ **Backups and version control** maintained

description: File safety rules that enforce verification practices, protection measures, and safe file operations for MagicOptimizer development.
globs: ["**/*", "!**/Binaries/**", "!**/Intermediate/**", "!**/DerivedDataCache/**"]
alwaysApply: true
---
