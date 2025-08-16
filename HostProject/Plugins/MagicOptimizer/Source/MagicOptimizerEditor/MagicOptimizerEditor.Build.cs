using UnrealBuildTool;
public class MagicOptimizerEditor : ModuleRules
{
    public MagicOptimizerEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = true;
        if (!Target.bBuildEditor)
        {
            throw new System.Exception("MagicOptimizerEditor requires bBuildEditor");
        }
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "MagicOptimizer", // runtime module for public headers like OptimizerSettings
            "UnrealEd",
            "Slate",
            "SlateCore",
            "AssetRegistry",
            "ContentBrowser",
            "EditorSubsystem",
            "Projects",
            "LevelEditor",
            "EditorStyle",
            "Settings",
            "ApplicationCore",
            "InputCore"
        });
    }
}


