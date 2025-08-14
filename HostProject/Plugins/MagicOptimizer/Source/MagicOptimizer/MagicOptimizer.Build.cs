// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MagicOptimizer : ModuleRules
{
	public MagicOptimizer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"AssetRegistry",
				"EditorAssetLibrary",
				"PythonScriptPlugin",
				"PythonScriptPluginEditor",
				"EditorStyle",
				"PropertyEditor",
				"DetailCustomizations",
				"LevelEditor",
				"WorkspaceMenuStructure",
				"ContentBrowser",
				"AssetTools",
				"EditorSubsystem"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"AssetRegistry",
				"EditorAssetLibrary",
				"PythonScriptPlugin",
				"PythonScriptPluginEditor",
				"EditorStyle",
				"PropertyEditor",
				"DetailCustomizations",
				"LevelEditor",
				"WorkspaceMenuStructure",
				"ContentBrowser",
				"AssetTools",
				"EditorSubsystem",
				"Projects",
				"InputCore",
				"ApplicationCore",
				"RHI",
				"RenderCore",
				"Renderer",
				"MaterialEditor",
				"MeshUtilities",
				"MeshReductionInterface",
				"NaniteTools",
				"StaticMeshEditor",
				"TextureEditor",
				"MaterialEditorUtilities"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		// Add source files
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"OptimizerSettings",
				"PythonBridge",
				"OptimizerRun"
			}
			);
	}
}
