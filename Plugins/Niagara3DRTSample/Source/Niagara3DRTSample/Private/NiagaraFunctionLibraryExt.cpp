#include "NiagaraFunctionLibraryExt.h"
#include "NiagaraSystem.h"
#include "NiagaraDataInterfaceVolumeTextureExt.h"
#include "UObject/UObjectBaseUtility.h"
#include "Engine/VolumeTexture.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "NiagaraComponent.h"

DEFINE_LOG_CATEGORY(LogNiagaraExtension);


UNiagaraFunctionLibraryExt::UNiagaraFunctionLibraryExt(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UNiagaraFunctionLibraryExt::SetVolumeTextureObjectAsRT(UNiagaraComponent* NiagaraSystem, const FString& OverrideName, UTextureRenderTargetVolume* Texture)
{
	if (!NiagaraSystem)
	{
		UE_LOG(LogNiagaraExtension, Warning, TEXT("NiagaraSystem in \"SetVolumeTextureObject\" is NULL, OverrideName \"%s\" and Texture \"%s\", skipping."), *OverrideName, *GetFullNameSafe(Texture));
		return;
	}

	if (!Texture)
	{
		UE_LOG(LogNiagaraExtension, Warning, TEXT("Volume Texture in \"SetVolumeTextureObject\" is NULL, OverrideName \"%s\" and NiagaraSystem \"%s\", skipping."), *OverrideName, *GetFullNameSafe(NiagaraSystem));
		return;
	}

	const FNiagaraParameterStore& OverrideParameters = NiagaraSystem->GetOverrideParameters();

	FNiagaraVariable Variable(FNiagaraTypeDefinition(UNiagaraDataInterfaceVolumeTextureExt::StaticClass()), *OverrideName);

	int32 Index = OverrideParameters.IndexOf(Variable);
	if (Index == INDEX_NONE)
	{
		UE_LOG(LogNiagaraExtension, Warning, TEXT("Could not find index of variable \"%s\" in the OverrideParameters map of NiagaraSystem \"%s\"."), *OverrideName, *GetFullNameSafe(NiagaraSystem));
		return;
	}

	UNiagaraDataInterfaceVolumeTextureExt* TextureDI = Cast<UNiagaraDataInterfaceVolumeTextureExt>(OverrideParameters.GetDataInterface(Index));
	if (!TextureDI)
	{
		UE_LOG(LogNiagaraExtension, Warning, TEXT("Did not find a matching Volume Texture Data Interface variable named \"%s\" in the User variables of NiagaraSystem \"%s\" ."), *OverrideName, *GetFullNameSafe(NiagaraSystem));
		return;
	}

	// In the editor we must set the parameter before SetParameterOverride as it will duplicate the DataInterface
	TextureDI->SetTexture(Texture);
#if WITH_EDITOR
	NiagaraSystem->SetParameterOverride(Variable, FNiagaraVariant(TextureDI));
#endif
}