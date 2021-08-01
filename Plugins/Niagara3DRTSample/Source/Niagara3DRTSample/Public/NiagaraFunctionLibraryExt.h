#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "NiagaraFunctionLibraryExt.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogNiagaraExtension, Log, Verbose);

UCLASS()
class NIAGARA3DRTSAMPLE_API UNiagaraFunctionLibraryExt : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	//UNiagaraFunctionLibraryExt();
	/** Overrides the Volume Texture as Render Target for a Niagara Volume Texture Data Interface User Parameter.*/
	UFUNCTION(BlueprintCallable, Category = Niagara)
	static void SetVolumeTextureObjectAsRT(UNiagaraComponent* NiagaraSystem, const FString& OverrideName, UTextureRenderTargetVolume* Texture);
};