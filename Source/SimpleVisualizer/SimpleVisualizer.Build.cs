// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SimpleVisualizer : ModuleRules
{
	public SimpleVisualizer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[] { "Core" });
		PrivateDependencyModuleNames.AddRange(new string[] { "CoreUObject", "RenderCore", "Engine" });
	}
}
