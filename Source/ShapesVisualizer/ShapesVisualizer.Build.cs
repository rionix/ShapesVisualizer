// Copyright (c) 2003-2022 rionix. All Rights Reserved.

using UnrealBuildTool;

public class ShapesVisualizer : ModuleRules
{
	public ShapesVisualizer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core" });
		PrivateDependencyModuleNames.AddRange(new string[] { "CoreUObject", "RenderCore", "Engine" });
	}
}
