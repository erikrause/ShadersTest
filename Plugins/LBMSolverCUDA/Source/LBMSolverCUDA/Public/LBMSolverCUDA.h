#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FLBMSolverCUDAModule : public IModuleInterface		// CHECK: нужен ли LBMSOLVER_API?
{
public:
	static inline FLBMSolverCUDAModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FLBMSolverCUDAModule>("LBMSolverCUDA");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("LBMSolverCUDA");
	}

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
