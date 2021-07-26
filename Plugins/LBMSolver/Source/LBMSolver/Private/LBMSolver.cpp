// Copyright Epic Games, Inc. All Rights Reserved.

#include "LBMSolver.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FLBMSolverModule"

void FLBMSolverModule::StartupModule()
{
	// Maps virtual shader source directory to actual shaders directory on disk.
	static FString baseDir = IPluginManager::Get().FindPlugin(TEXT("LBMSolver"))->GetBaseDir();
	FString ShaderDirectory = FPaths::Combine(baseDir, TEXT("Shaders/Private"));
	AddShaderSourceDirectoryMapping("/CustomShaders", ShaderDirectory);
}

void FLBMSolverModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLBMSolverModule, LBMSolver)