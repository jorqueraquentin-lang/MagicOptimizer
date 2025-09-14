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
                    "SlateRHIRenderer",
                    "EditorStyle",
                    "EditorWidgets",
                    "ToolMenus",
                    "LevelEditor",
                    "UMG",
                    "UMGEditor",
                    "PropertyEditor",
                    "WorkspaceMenuStructure",
                    "ContentBrowser",
                    "MainFrame",
                    "AppFramework",
                    "ToolWidgets",
                    "DesktopPlatform"
                }
            );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "InputCore",
                    "RenderCore",
                    "RHI",
                    "Json",
                    "HTTP",
                    "Projects",
                    "TargetPlatform",
                    "ImageWrapper",
                    "Serialization",
                    "TextureCompressor",
                    "MessageLog",
                    "Engine"
                }
            );
        }
        
        // Keep it minimal for now - we'll add more as needed
    }
}
