#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class LBMSOLVER_API FLBMSolverModule : public IModuleInterface		// CHECK: для чего LBMSOLVER_API?
{
public:
	static inline FLBMSolverModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FLBMSolverModule>("LBMSolver");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("LBMSolver");
	}

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
