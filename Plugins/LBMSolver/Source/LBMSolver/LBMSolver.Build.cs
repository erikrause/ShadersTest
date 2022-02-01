// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class LBMSolver : ModuleRules
{
	private string poject_root_path
	{
		get { return Path.Combine(ModuleDirectory, "../.."); }
	}

	public LBMSolver(ReadOnlyTargetRules Target) : base(Target)
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
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Renderer",
				"RenderCore",
				"RHI",
				"Projects"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		// CUDA dependencies:
		string custom_cuda_lib_include = "libs/CUDASolver/include";
		string custom_cuda_lib_lib = "libs/CUDASolver/lib";

		PublicIncludePaths.Add(Path.Combine(poject_root_path, custom_cuda_lib_include));
		PublicAdditionalLibraries.Add(Path.Combine(poject_root_path, custom_cuda_lib_lib, "LBMSolver.lib"));

		string cuda_path = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.5";
		string cuda_include = "include";
		string cuda_lib = "lib/x64";

		PublicIncludePaths.Add(Path.Combine(cuda_path, cuda_include));

		//PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "cudart.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "cudart_static.lib"));
	}
}
