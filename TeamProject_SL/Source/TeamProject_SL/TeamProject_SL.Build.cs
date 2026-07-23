// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TeamProject_SL : ModuleRules
{
    public TeamProject_SL(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
        "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "AIModule",
            "StateTreeModule",
            "GameplayStateTreeModule",
            "UMG",
            "Slate",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "Niagara"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        PublicIncludePaths.AddRange(new string[] {
            "TeamProject_SL",
            "TeamProject_SL/Variant_Platforming",
            "TeamProject_SL/Variant_Platforming/Animation",
            "TeamProject_SL/Variant_Combat",
            "TeamProject_SL/Variant_Combat/AI",
            "TeamProject_SL/Variant_Combat/Animation",
            "TeamProject_SL/Variant_Combat/Gameplay",
            "TeamProject_SL/Variant_Combat/Interfaces",
            "TeamProject_SL/Variant_Combat/UI",
            "TeamProject_SL/Variant_SideScrolling",
            "TeamProject_SL/Variant_SideScrolling/AI",
            "TeamProject_SL/Variant_SideScrolling/Gameplay",
            "TeamProject_SL/Variant_SideScrolling/Interfaces",
            "TeamProject_SL/Variant_SideScrolling/UI"
        });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
