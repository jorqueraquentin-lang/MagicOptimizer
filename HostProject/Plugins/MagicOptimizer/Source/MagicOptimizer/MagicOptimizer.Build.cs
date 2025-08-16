// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

using UnrealBuildTool;

public class MagicOptimizer : ModuleRules
{
	public MagicOptimizer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = true;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "DeveloperSettings" });
		PrivateDependencyModuleNames.AddRange(new string[] { "Json", "JsonUtilities", "Projects" });
		// Do not add editor dependencies here
	}
}
