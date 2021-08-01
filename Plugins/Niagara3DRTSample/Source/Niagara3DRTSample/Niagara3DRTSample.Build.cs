// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Niagara3DRTSample : ModuleRules
{
	public Niagara3DRTSample(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"Niagara/Private"
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Niagara",
				"VectorVM",
				"RHI",
				"RenderCore"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"NiagaraCore",
				"NiagaraShader",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);



		///

		        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "NiagaraCore",
                "NiagaraShader",
                "Core",
                "Engine",
                "TimeManagement",
                "TraceLog",
                "Renderer",
                "JsonUtilities",
				"Landscape",
				"Json",
				"AudioPlatformConfiguration",
				"SignalProcessing",
				"ApplicationCore",
				"DeveloperSettings"
			}
        );


        PublicDependencyModuleNames.AddRange(
            new string[] {
                "NiagaraCore",
                "NiagaraShader",
                "MovieScene",
				"MovieSceneTracks",
				"CoreUObject",
                "VectorVM",
                "RHI",
                "NiagaraVertexFactories",
                "RenderCore",
                "IntelISPC",
            }
        );


        PrivateIncludePaths.AddRange(
            new string[] {
                "Niagara/Private",
            })
        ;

        // If we're compiling with the engine, then add Core's engine dependencies
        if (Target.bCompileAgainstEngine == true)
        {
            if (!Target.bBuildRequiresCookedData)
            {
                DynamicallyLoadedModuleNames.AddRange(new string[] { "DerivedDataCache" });
            }
        }


		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[] {
				"TargetPlatform",
				"UnrealEd",
				"SlateCore",
				"Slate"
			});
		}
	}
}
