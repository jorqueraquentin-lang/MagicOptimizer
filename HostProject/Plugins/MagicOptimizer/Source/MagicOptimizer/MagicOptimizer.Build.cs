using UnrealBuildTool;

public class MagicOptimizer : ModuleRules
{
    public MagicOptimizer(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        // Core UE5.6 modules - only add what we absolutely need
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject", 
                "Engine"
            }
        );
        
        // Editor-only modules (this is an editor plugin)
        if (Target.bBuildEditor)
        {
            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "UnrealEd",
                    "AssetRegistry",
                    "DeveloperSettings",
                    "Settings",
                    "Slate",
                    "SlateCore",
                    "EditorStyle",
                    "EditorWidgets",
                    "ToolMenus",
                    "LevelEditor",
                    "UMG",
                    "UMGEditor",
                    "BlueprintGraph",
                    "KismetCompiler",
                    "UnrealEd",
                    "PropertyEditor",
                    "DetailCustomizations",
                    "EditorWidgets"
                }
            );
        }
        
        // Keep it minimal for now - we'll add more as needed
    }
}
