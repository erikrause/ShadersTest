// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "SplinesStartArea.generated.h"

/**
 * 
 */
UINTERFACE(MinimalAPI)
class USplinesStartArea : public UInterface
{
	GENERATED_BODY()
};


class SHADERSTEST_API ISplinesStartArea
{
	GENERATED_IINTERFACE_BODY()
	
public:

	virtual TArray<FVector> GetSplinesStartLocations(FIntVector resolution);
};
