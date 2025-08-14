// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MagicOptimizer : ModuleRules
{
	public MagicOptimizer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// No additional include paths needed for this module
		
		// No additional private include paths needed for this module
			
		
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
				"PythonScriptPlugin",
				"EditorStyle",
				"PropertyEditor",
				"DetailCustomizations",
				"LevelEditor",
				"WorkspaceMenuStructure",
				"ContentBrowser",
				"AssetTools",
				"EditorSubsystem",
				"DeveloperSettings"
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
				"PythonScriptPlugin",
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
				"StaticMeshEditor",
				"TextureEditor",
				"AppFramework"
			}
			);
		
		
		// No dynamically loaded modules needed for this module

		// These are internal classes, not external modules
		// They will be compiled as part of this module
	}
}
