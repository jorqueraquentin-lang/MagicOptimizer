# MagicOptimizer C++ Implementation

This directory contains the C++ implementation of the MagicOptimizer plugin for Unreal Engine 5.6.

## File Structure

### Headers (`Public/`)
- **`MagicOptimizer.h`** - Main module interface
- **`OptimizerSettings.h`** - Configuration and settings management
- **`PythonBridge.h`** - Python integration interface
- **`SOptimizerPanel.h`** - UI panel interface
- **`OptimizerRun.h`** - Optimization workflow execution

### Implementation (`Private/`)
- **`MagicOptimizer.cpp`** - Main module implementation
- **`OptimizerSettings.cpp`** - Settings implementation
- **`PythonBridge.cpp`** - Python bridge implementation
- **`SOptimizerPanel.cpp`** - UI panel implementation
- **`OptimizerRun.cpp`** - Optimization run implementation

## Core Components

### 1. MagicOptimizer Module
- Main plugin module that handles initialization and shutdown
- Registers the UI panel and menu extensions
- Manages the plugin lifecycle

### 2. OptimizerSettings
- Extends `UDeveloperSettings` for persistent configuration
- Manages optimization profiles, categories, and safety settings
- Provides preset management functionality

### 3. PythonBridge
- Handles communication between C++ and Python backend
- Manages Python script execution and result parsing
- Provides error handling and validation

### 4. SOptimizerPanel
- Slate-based UI panel for the plugin
- Provides configuration controls and optimization workflow
- Integrates with the Python backend through PythonBridge

### 5. OptimizerRun
- Manages individual optimization workflow executions
- Provides progress tracking and cancellation support
- Handles asynchronous execution and event broadcasting

## Key Features

### UI Integration
- **Expandable Sections**: Configuration, Asset Categories, Scope, Safety, Output
- **Profile Selection**: 9 built-in optimization profiles
- **Category Filtering**: Textures, Meshes, Materials, Levels, Runtime, Audio
- **Safety Controls**: Dry-run, max-changes, backup creation
- **Action Buttons**: Audit, Recommend, Apply, Verify

### Python Integration
- **Script Execution**: Runs Python optimization scripts
- **Parameter Passing**: Configures optimization parameters
- **Result Handling**: Processes optimization results
- **Error Management**: Handles Python execution errors

### Configuration Management
- **Persistent Settings**: Saves configuration to INI files
- **Profile Management**: Built-in and custom optimization profiles
- **Category Masks**: Bitmask-based category selection
- **Safety Defaults**: Safe optimization defaults

## Usage

### Building the Plugin
1. Ensure all dependencies are available in your UE5.6 project
2. Build the plugin using your project's build system
3. The plugin will appear in the Editor under Window > Magic Optimizer

### Configuration
1. Open the Magic Optimizer panel
2. Select your target optimization profile
3. Choose asset categories to optimize
4. Configure scope and safety settings
5. Run optimization phases as needed

### Python Backend
1. Ensure Python is installed and accessible
2. Place Python scripts in `Content/Python/magic_optimizer/`
3. The plugin will automatically detect and use available scripts

## Dependencies

### Required Modules
- **Core**: Basic UE functionality
- **Slate/SlateCore**: UI framework
- **UnrealEd**: Editor integration
- **AssetRegistry**: Asset management
- **PythonScriptPlugin**: Python integration

### Optional Modules
- **MaterialEditor**: Material optimization support
- **MeshUtilities**: Mesh optimization support
- **TextureEditor**: Texture optimization support

## Safety Features

- **Dry Run Mode**: Preview changes without applying them
- **Max Changes Limit**: Prevent excessive modifications
- **Backup Creation**: Automatic backup before modifications
- **Category Filtering**: Selective optimization by asset type
- **Progress Tracking**: Real-time optimization progress

## Future Enhancements

- **Asset Preview**: Preview optimization results
- **Batch Processing**: Process multiple asset types simultaneously
- **Custom Presets**: User-defined optimization profiles
- **Performance Metrics**: Detailed optimization statistics
- **Integration APIs**: External tool integration support

## Troubleshooting

### Common Issues
1. **Python Not Found**: Ensure Python is in system PATH
2. **Build Errors**: Check module dependencies in Build.cs
3. **UI Not Showing**: Verify plugin is enabled in project settings
4. **Script Execution Failed**: Check Python script paths and permissions

### Debug Information
- Enable Python logging in plugin settings
- Check Output Log for detailed error messages
- Verify Python script execution in console

## Contributing

When adding new features:
1. Follow the existing code structure
2. Add proper error handling
3. Include configuration options
4. Update documentation
5. Test with various asset types
