using UnrealBuildTool;

public class TaleStories : ModuleRules
{
	public TaleStories(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"ThirdPartyLibUE",
			"Sockets",
			"Networking",
			"HTTP",
			"Json", // Добавь это
			"JsonUtilities" // Добавь это
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[]
		{
			"TaleStories"
		});
	}
}