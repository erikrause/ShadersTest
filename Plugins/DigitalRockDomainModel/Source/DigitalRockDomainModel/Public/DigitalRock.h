#pragma once

#include "CoreMinimal.h"
#include "DigitalRock.generated.h"

/**
 * 
 */
UCLASS()
class DIGITALROCKDOMAINMODEL_API UDigitalRock : public UObject
{
	GENERATED_BODY()
	
public:
	
	void Init(TArray<uint8> byteArray, FIntVector size);

	float* GetPorousAsFloat();

	int* GetPorousAsInt();

	FIntVector GetSize();

protected:

	TArray<uint8> _byteArray;
	FIntVector _size;
};
