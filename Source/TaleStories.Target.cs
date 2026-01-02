using UnrealBuildTool;
using System.Collections.Generic;

public class TaleStoriesTarget : TargetRules
{
	public TaleStoriesTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("TaleStories");

		// игра без editor будет пока в режиме сервера отдельным таргетом собираться
		GlobalDefinitions.Add("SERVER_MODE");
		bOverrideBuildEnvironment = true;
		if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			AdditionalLinkerArguments += " -Wl,-dead_strip";
		}
	}
}