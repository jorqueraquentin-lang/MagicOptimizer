# MagicOptimizer Plugin for Unreal Engine 5.6

A comprehensive asset optimization plugin for Unreal Engine 5.6 that provides runtime optimization services, editor integration, and performance monitoring.

## Features

### 🚀 **Runtime Optimization Engine**
- **Engine Subsystem Architecture**: Clean, engine-level optimization services
- **Asynchronous Processing**: UE5.6 Task system for non-blocking optimization
- **Multiple Optimization Types**: Textures, Meshes, Materials, and Full passes
- **Performance Tracking**: Integrated with Unreal Insights for profiling

### 🎮 **Console Variables (CVars)**
All optimization features can be controlled via console commands:

```bash
# Enable/disable the optimizer
magicopt.Enabled 1

# Set maximum changes per pass
magicopt.MaxChanges 100

# Set timeout for optimization operations
magicopt.Timeout 300.0

# Enable verbose logging
magicopt.Verbose 1

# Enable dry run mode (don't apply changes)
magicopt.DryRun 1

# Enable performance tracking
magicopt.PerformanceTracking 1
```

### 🎯 **Console Commands**
Quick access to optimization functions:

```bash
# Run optimization passes
magicopt.Run              # Full optimization
magicopt.Textures         # Texture optimization only
magicopt.Meshes           # Mesh optimization only
magicopt.Materials        # Material optimization only

# Status and information
magicopt.Status           # Show current status
magicopt.CVars            # Show all CVars
```

### 🖥️ **Editor Integration**
- **Modern ToolMenus System**: UE5.6 best practices for editor extensibility
- **Main Menu Integration**: Window → Magic Optimizer menu
- **Toolbar Integration**: Quick access buttons in the main toolbar
- **Dedicated Panel**: Custom tab for detailed optimization control

### 📊 **Performance Monitoring**
- **Unreal Insights Integration**: Custom stat groups for optimization metrics
- **Real-time Statistics**: Track assets processed, optimized, and memory saved
- **Performance Counters**: Cycle counters for each optimization type
- **Error Tracking**: Monitor optimization errors and warnings

## Installation

1. **Copy the plugin** to your project's `Plugins/` folder
2. **Enable the plugin** in Project Settings → Plugins
3. **Restart the editor** to load the plugin
4. **Access via menu**: Window → Magic Optimizer

## Usage

### **Editor Interface**
1. **Open the panel**: Window → Magic Optimizer
2. **Configure settings**: Use the settings panel to configure optimization parameters
3. **Run optimizations**: Use the toolbar buttons or menu items to start optimization passes

### **Runtime Control**
1. **Console commands**: Use the console commands listed above
2. **Blueprint integration**: Call optimization functions from Blueprints
3. **C++ integration**: Access the subsystem directly in code

### **Performance Monitoring**
1. **View stats**: Use `stat MagicOptimizer` in console
2. **Unreal Insights**: Connect to your project for detailed profiling
3. **Custom metrics**: Monitor specific optimization types and performance

## Architecture

### **Module Structure**
```
MagicOptimizer/
├── MagicOptimizer/           # Runtime module
│   ├── MagicOptimizerSubsystem.h/cpp    # Engine subsystem
│   ├── MagicOptimizerCVars.h/cpp        # Console variables
│   ├── MagicOptimizerStats.h            # Performance tracking
│   └── OptimizerSettings.h/cpp          # Configuration
└── MagicOptimizerEditor/     # Editor module
    ├── MagicOptimizerEditor.h/cpp       # Editor integration
    └── SOptimizerPanel.h/cpp            # Custom UI panel
```

### **Key Components**
- **`UMagicOptimizerSubsystem`**: Engine-level optimization service
- **`MagicOptimizerCVars`**: Runtime configuration via console variables
- **`MagicOptimizerStats`**: Performance monitoring and Unreal Insights integration
- **`FMagicOptimizerCommands`**: Modern ToolMenus command system

## Configuration

### **Project Settings**
Access via Project Settings → Plugins → Magic Optimizer:
- **Categories**: Select which asset types to optimize
- **Target Profile**: Choose optimization profile (PC, Console, Mobile, VR)
- **Run Mode**: Audit, Recommend, Apply, or Verify
- **Safety Settings**: Dry run, backups, maximum changes
- **Auto-reporting**: Configure automatic reporting features

### **Runtime Configuration**
Use CVars for dynamic configuration:
- **magicopt.Enabled**: Enable/disable the optimizer
- **magicopt.MaxChanges**: Limit changes per optimization pass
- **magicopt.Timeout**: Set operation timeout
- **magicopt.DryRun**: Test without applying changes

## Performance Considerations

### **Optimization Workflow**
1. **Start with dry run**: Use `magicopt.DryRun 1` to test
2. **Monitor performance**: Use `stat MagicOptimizer` and Unreal Insights
3. **Adjust settings**: Modify CVars based on performance requirements
4. **Production use**: Disable verbose logging and performance tracking in production

### **Memory Management**
- **Asset processing**: Uses UE5.6 Task system for efficient background processing
- **Memory tracking**: Monitors memory savings from optimizations
- **Garbage collection**: Properly manages temporary objects and references

## Troubleshooting

### **Common Issues**
1. **Plugin not loading**: Check that PythonScriptPlugin is enabled
2. **Optimization not running**: Verify `magicopt.Enabled` is set to 1
3. **Performance issues**: Check `magicopt.PerformanceTracking` and monitor with Insights
4. **Console commands not working**: Ensure the runtime module is loaded

### **Debug Information**
- **Verbose logging**: Enable with `magicopt.Verbose 1`
- **Status check**: Use `magicopt.Status` to see current state
- **CVar display**: Use `magicopt.CVars` to see all settings

## Development

### **Building from Source**
1. **Clone the repository** to your project's Plugins folder
2. **Generate project files** using Unreal's project generator
3. **Build the plugin** using your IDE or Unreal's build system
4. **Test in editor** and packaged builds

### **Extending the Plugin**
- **Add new optimization types**: Extend the subsystem with new methods
- **Custom CVars**: Add new console variables for configuration
- **Performance metrics**: Add new stat counters for monitoring
- **Editor integration**: Extend the ToolMenus system with new commands

## Support

- **Documentation**: Check the plugin's built-in help and tooltips
- **Console commands**: Use the built-in help system (`magicopt.CVars`, `magicopt.Status`)
- **Performance issues**: Use Unreal Insights and the built-in stat system
- **Bug reports**: Check the console for error messages and warnings

## License

Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

---

*This plugin follows UE5.6 best practices and provides a robust foundation for asset optimization in Unreal Engine projects.*
