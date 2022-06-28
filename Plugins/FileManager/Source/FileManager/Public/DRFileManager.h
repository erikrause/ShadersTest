#pragma once

#include "CoreMinimal.h"
#include "DigitalRock.h"
#include "DRFileManager.generated.h"

UCLASS(BlueprintType)
class FILEMANAGER_API UDRFileManager : public UObject
{
	GENERATED_BODY()

public:

	UDRFileManager();

	UFUNCTION(BlueprintCallable)
		static UDigitalRock* GetDigitalRockAmaretto(FString path);

	UFUNCTION(BlueprintCallable)
		static UDigitalRock* GetDigitalRock(FString path, FIntVector size);

protected:

	TArray<uint8> _getByteArray(FString path);
	static FString _getPorousFolder();
};
