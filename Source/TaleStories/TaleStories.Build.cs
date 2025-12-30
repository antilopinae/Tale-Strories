using UnrealBuildTool;

public class TaleStories : ModuleRules
{
	public TaleStories(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"ThirdPartyLibUE"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[] { });
		
		PublicIncludePaths.AddRange(new string[] {
			"TaleStories"
		});
	}
}