// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

using UnrealBuildTool;

public class UnrealExDNK : ModuleRules
{
	public UnrealExDNK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"UMG",
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
				"Slate",
				"SlateCore",
                "HTTP",
            }
			);


        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("SourceControl");
        }

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
