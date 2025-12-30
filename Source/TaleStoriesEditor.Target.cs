using UnrealBuildTool;
using System.Collections.Generic;

public class TaleStoriesEditorTarget : TargetRules
{
	public TaleStoriesEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("TaleStories");
	}
}
