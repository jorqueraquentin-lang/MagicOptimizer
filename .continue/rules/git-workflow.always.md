# Git Workflow Management - Always Apply

## 🎯 **Scope and Purpose**

This rule enforces Git branch management, commit standards, and workflow practices for MagicOptimizer development.

## 🌿 **Branch Management**

### **Default Development Branch**
- **Primary branch**: `V0.01` (as per user preference)
- **All development work** should continue on this branch by default
- **Feature branches** should branch from and merge back to `V0.01`
- **Release branches** should be created from `V0.01` for version tags

### **Branch Naming Convention**
```
feature/description-of-feature
bugfix/issue-description
hotfix/critical-fix-description
release/v1.0.0
```

## 📝 **Commit Standards**

### **Commit Message Format**
```
type(scope): <short description>

<detailed description if needed>

<footer with breaking changes, fixes, etc.>
```

### **Commit Types**
- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes
- **style**: Code style changes (formatting, etc.)
- **refactor**: Code refactoring
- **test**: Adding or updating tests
- **chore**: Maintenance tasks

### **Commit Template**
```
feat/fix: <one line summary>

Root cause:
Change:
Why non destructive:
Tests/validation:
Affected modules:
```

## 🔄 **Workflow Process**

### **Development Workflow**
1. **Start from V0.01** branch
2. **Create feature branch** for significant changes
3. **Make incremental commits** with clear messages
4. **Test thoroughly** before merging
5. **Merge back to V0.01** when complete

### **Before Committing**
- ✅ **Code compiles** without errors
- ✅ **Tests pass** locally
- ✅ **Development log updated** with changes
- ✅ **No temporary files** or debug code included

### **After Committing**
- ✅ **Push to remote** repository
- ✅ **Verify CI passes** if applicable
- ✅ **Update development log** with commit hash
- ✅ **Notify team** of significant changes

## 🚫 **Git Anti-Patterns**

### **What NOT to Do**
- ❌ **Don't commit directly** to main/master branches
- ❌ **Don't force push** to shared branches
- ❌ **Don't commit large binary files** (use LFS if needed)
- ❌ **Don't commit incomplete work** or broken code
- ❌ **Don't rewrite history** of shared branches

### **What to ALWAYS Do**
- ✅ **Use descriptive commit messages**
- ✅ **Test before committing**
- ✅ **Update development log**
- ✅ **Follow branch naming conventions**
- ✅ **Keep commits atomic** and focused

## 🎯 **Success Metrics**

- ✅ **All work continues** on V0.01 branch by default
- ✅ **Clear commit history** with descriptive messages
- ✅ **Feature branches** properly managed
- ✅ **No broken code** committed to shared branches
- ✅ **Development log** updated with commit references

description: Git workflow rules that enforce branch management, commit standards, and development practices for MagicOptimizer development.
globs: ["**/*.md", "**/*.ps1", "**/*.bat", "**/*.gitignore", "**/*.gitattributes"]
alwaysApply: true
---
