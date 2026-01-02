using UnrealBuildTool;
using System;
using System.IO;
using System.Diagnostics;

public class ThirdPartyLibUE : ModuleRules
{
	public ThirdPartyLibUE(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

		string Root = Path.Combine(ModuleDirectory, "../../ThirdParty");

		string Config;
		switch (Target.Configuration)
		{
			case UnrealTargetConfiguration.Debug:
				Config = "Debug";
				break;
			default:
				Config = "Release";
				break;
		}

		PublicIncludePaths.Add(Path.Combine(Root, "include"));

		string PlatformFolder = "";
		string LibName = "";

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PlatformFolder = "Win64";
			LibName = "LibUE.dll";
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PlatformFolder = "Mac";
			LibName = "libLibUE.dylib";

			// 3. Сообщаем Unreal, что мы знаем о конфликтах OpenSSL.
			// Мы принудительно разрешаем линковщику использовать наш OpenSSL 3.0 из LibUE.a
			// bOverrideBuildEnvironment = true;
			// PublicDefinitions.Add("WITH_OPENSSL=1");
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			PlatformFolder = "Linux";
			LibName = "libLibUE.so";
		}

		string Script = Target.Platform == UnrealTargetPlatform.Win64
			? Path.Combine(Root, "scripts/build_lib_ue.bat")
			: Path.Combine(Root, "scripts/build_lib_ue.sh");

		if (File.Exists(Script))
		{
			Process proc = new Process();
			proc.StartInfo.FileName = Script;
			proc.StartInfo.WorkingDirectory = Root;
			proc.StartInfo.UseShellExecute = false;
			proc.StartInfo.CreateNoWindow = true;

			proc.Start();
			proc.WaitForExit();
			if (proc.ExitCode != 0)
			{
				throw new Exception($"Failed to build LibUE via {Script}");
			}
		}

		string LibPath = Path.Combine(Root, "build", PlatformFolder, Config, LibName);
		string LibDir = Path.Combine(Root, "build", "Mac", Config);

		PublicAdditionalLibraries.Add(
			LibPath
		);

		PublicRuntimeLibraryPaths.Add(LibDir);

		RuntimeDependencies.Add("$(BinaryOutputDir)/" + LibName, LibPath);
		
		// PublicDelayLoadDLLs.Add(LibName);
	}
}