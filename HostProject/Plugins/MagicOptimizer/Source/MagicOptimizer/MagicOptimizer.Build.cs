// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

using UnrealBuildTool;

public class MagicOptimizer : ModuleRules
{
	public MagicOptimizer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = true;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"DeveloperSettings"
			// UEngineSubsystem is part of the Engine module
		});
		
		PrivateDependencyModuleNames.AddRange(new string[] { 
			"Json", 
			"JsonUtilities", 
			"Projects"
			// UE::Tasks and AsyncTask are part of the Core module
		});
		
		// Do not add editor dependencies here
	}
}
