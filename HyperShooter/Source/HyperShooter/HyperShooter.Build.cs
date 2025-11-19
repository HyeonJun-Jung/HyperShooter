// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HyperShooter : ModuleRules
{
	public HyperShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", 
			"EnhancedInput", "Niagara",
            "GameplayTags",
            "UMG", "Slate", "SlateCore",
			"OnlineSubsystem", "OnlineSubsystemUtils",});

        PrivateIncludePaths.Add("HyperShooter");
    }
}
