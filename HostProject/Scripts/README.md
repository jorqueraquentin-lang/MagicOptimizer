# 🔧 MagicOptimizer Build Scripts

This directory contains build and testing scripts for the MagicOptimizer plugin.

## 📋 **Available Scripts**

### **Primary Scripts**
- **`rebuild_plugin.ps1`** - Main plugin rebuild script (use this one)
- **`test_plugin.ps1`** - Basic plugin functionality test
- **`test_plugin_comprehensive.ps1`** - Comprehensive plugin testing
- **`final_test.ps1`** - Final validation test

### **Packaging Scripts**
- **`PackagePlugin.ps1`** - Package plugin for distribution
- **`PackagePlugin.bat`** - Batch version of packaging script

## 🚀 **Quick Start**

### **Rebuild Plugin**
```powershell
# From HostProject directory
.\Scripts\rebuild_plugin.ps1
```

### **Test Plugin**
```powershell
# From HostProject directory
.\Scripts\test_plugin.ps1
```

## 📁 **Archived Scripts**

The following scripts have been archived to `../../archive_plugin_code/scripts/`:
- `rebuild_plugin_fixed.ps1` - Fixed encoding issues (superseded)
- `build_plugin_only.ps1` - Plugin-only build (failed approach)
- `build_project_with_plugin.ps1` - Alternative build approach (superseded)

## ⚠️ **Important Notes**

- **Always run from HostProject directory** - Scripts expect to be in the project root
- **Use rebuild_plugin.ps1** - This is the only rebuild script you need
- **Restart Editor after rebuild** - Plugin changes require complete editor restart
- **Check for duplicates** - Script automatically prevents duplicate settings windows

## 🔧 **Troubleshooting**

### **Build Fails**
1. Check Unreal Engine 5.6 installation path
2. Verify project file is valid
3. Check for duplicate config files
4. Run cleanup script first

### **Plugin Not Loading**
1. Restart Unreal Editor completely
2. Check Project Settings → Plugins
3. Verify plugin is enabled
4. Check for compilation errors

### **Duplicate Settings Windows**
1. Run rebuild script to clean configs
2. Restart editor completely
3. Check for manual settings registration in code
4. Verify only plugin-level config exists

## 📚 **Documentation**

- [Plugin Development Guide](../../docs/DEV_README.md)
- [Duplicate Settings Prevention](../../Plugins/MagicOptimizer/DUPLICATE_SETTINGS_PREVENTION.md)
- [Development Log](../../docs/DEVELOPMENT_LOG.md)

