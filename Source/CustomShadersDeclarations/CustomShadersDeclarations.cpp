// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomShadersDeclarations.h"

IMPLEMENT_GAME_MODULE(FCustomShadersDeclarationsModule, CustomShadersDeclarations);

void FCustomShadersDeclarationsModule::StartupModule()
{
	// Maps virtual shader source directory to actual shaders directory on disk.
	FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders/Private"));
	AddShaderSourceDirectoryMapping("/CustomShaders", ShaderDirectory);
}

void FCustomShadersDeclarationsModule::ShutdownModule()
{
}
