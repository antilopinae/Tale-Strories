using UnrealBuildTool;
using System;  
using System.IO;
using System.Diagnostics;

public class ThirdPartyLibUE : ModuleRules
{
    public ThirdPartyLibUE(ReadOnlyTargetRules Target) : base(Target)
    {
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
            LibName = "LibUE.lib";
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PlatformFolder = "Mac";
            LibName = "libLibUE.a";
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PlatformFolder = "Linux";
            LibName = "libLibUE.a";
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

        PublicAdditionalLibraries.Add(
            Path.Combine(Root, "build", PlatformFolder, Config, LibName)
        );
    }
}