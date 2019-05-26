// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LanParty : ModuleRules
{
    public LanParty(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PrivatePCHHeaderFile = "Public/LanParty.h";

        PublicIncludePaths.AddRange(
            new string[] { }
            );
                
        
        PrivateIncludePaths.AddRange(
            new string[] { }
            );
            
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
            );
            
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Sockets", // required for using IMessageBus and friends
            }
            );
        
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            { }
            );
    }
}
