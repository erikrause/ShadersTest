#include "NiagaraDataInterfaceVolumeTextureExt.h"
#include "NiagaraShader.h"
#include "ShaderParameterUtils.h"
#include "NiagaraCustomVersion.h"
#include "Engine/VolumeTexture.h"
#include "Engine/TextureRenderTargetVolume.h"


#define LOCTEXT_NAMESPACE "UNiagaraDataInterfaceVolumeTextureExt"

const FName UNiagaraDataInterfaceVolumeTextureExt::SampleVolumeTextureName(TEXT("SampleVolumeTexture"));
const FName UNiagaraDataInterfaceVolumeTextureExt::TextureDimsName(TEXT("TextureDimensions3D"));
const FString UNiagaraDataInterfaceVolumeTextureExt::TextureName(TEXT("Texture_"));
const FString UNiagaraDataInterfaceVolumeTextureExt::SamplerName(TEXT("Sampler_"));
const FString UNiagaraDataInterfaceVolumeTextureExt::DimensionsBaseName(TEXT("Dimensions_"));

UNiagaraDataInterfaceVolumeTextureExt::UNiagaraDataInterfaceVolumeTextureExt(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
	, Texture(nullptr)
{
	Proxy.Reset(new FNiagaraDataInterfaceProxyVolumeTextureExt());
	MarkRenderDataDirty();
}

void UNiagaraDataInterfaceVolumeTextureExt::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), true, false, false);
	}

	MarkRenderDataDirty();
}

void UNiagaraDataInterfaceVolumeTextureExt::PostLoad()
{
	Super::PostLoad();

	// Not safe since the UTexture might not have yet PostLoad() called and so UpdateResource() called.
	// This will affect whether the SamplerStateRHI will be available or not.
	MarkRenderDataDirty();
}

#if WITH_EDITOR

void UNiagaraDataInterfaceVolumeTextureExt::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	MarkRenderDataDirty();
}

#endif

bool UNiagaraDataInterfaceVolumeTextureExt::CopyToInternal(UNiagaraDataInterface* Destination) const
{
	if (!Super::CopyToInternal(Destination))
	{
		return false;
	}
	UNiagaraDataInterfaceVolumeTextureExt* DestinationTexture = CastChecked<UNiagaraDataInterfaceVolumeTextureExt>(Destination);
	DestinationTexture->Texture = Texture;
	DestinationTexture->MarkRenderDataDirty();

	return true;
}

bool UNiagaraDataInterfaceVolumeTextureExt::Equals(const UNiagaraDataInterface* Other) const
{
	if (!Super::Equals(Other))
	{
		return false;
	}
	const UNiagaraDataInterfaceVolumeTextureExt* OtherTexture = CastChecked<const UNiagaraDataInterfaceVolumeTextureExt>(Other);
	return OtherTexture->Texture == Texture;
}

void UNiagaraDataInterfaceVolumeTextureExt::GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions)
{
	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = SampleVolumeTextureName;
		Sig.bMemberFunction = true;
		Sig.bRequiresContext = false;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Texture")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("UVW")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("MipLevel")));
		Sig.SetDescription(LOCTEXT("TextureSampleVolumeTextureDesc", "Sample the specified mip level of the input 3d texture at the specified UVW coordinates. The UVW origin (0, 0, 0) is in the bottom left hand corner of the volume."));
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec4Def(), TEXT("Value")));
		//Sig.Owner = *GetName();

		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = TextureDimsName;
		Sig.bMemberFunction = true;
		Sig.bRequiresContext = false;
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Texture")));
		Sig.SetDescription(LOCTEXT("TextureDimsDesc", "Get the dimensions of mip 0 of the texture."));
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Dimensions3D")));
		//Sig.Owner = *GetName();

		OutFunctions.Add(Sig);
	}
}

DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceVolumeTextureExt, SampleVolumeTexture)
void UNiagaraDataInterfaceVolumeTextureExt::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc)
{
	if (BindingInfo.Name == SampleVolumeTextureName)
	{
		check(BindingInfo.GetNumInputs() == 4 && BindingInfo.GetNumOutputs() == 4);
		NDI_FUNC_BINDER(UNiagaraDataInterfaceVolumeTextureExt, SampleVolumeTexture)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == TextureDimsName)
	{
		check(BindingInfo.GetNumInputs() == 0 && BindingInfo.GetNumOutputs() == 3);
		OutFunc = FVMExternalFunction::CreateUObject(this, &UNiagaraDataInterfaceVolumeTextureExt::GetTextureDimensions);
	}
}

void UNiagaraDataInterfaceVolumeTextureExt::GetTextureDimensions(FVectorVMContext& Context)
{
	VectorVM::FExternalFuncRegisterHandler<float> OutWidth(Context);
	VectorVM::FExternalFuncRegisterHandler<float> OutHeight(Context);
	VectorVM::FExternalFuncRegisterHandler<float> OutDepth(Context);

	if (Texture == nullptr)
	{
		for (int32 i = 0; i < Context.NumInstances; ++i)
		{
			*OutWidth.GetDestAndAdvance() = 0.0f;
			*OutHeight.GetDestAndAdvance() = 0.0f;
			*OutDepth.GetDestAndAdvance() = 0.0f;
		}
	}
	else if (UVolumeTexture* volumeTexture = Cast<UVolumeTexture>(Texture))
	{
		float Width = volumeTexture->GetSizeX();
		float Height = volumeTexture->GetSizeY();
		float Depth = volumeTexture->GetSizeZ();
		for (int32 i = 0; i < Context.NumInstances; ++i)
		{
			*OutWidth.GetDestAndAdvance() = Width;
			*OutHeight.GetDestAndAdvance() = Height;
			*OutDepth.GetDestAndAdvance() = Depth;
		}
	}
	else if (UTextureRenderTargetVolume* volumeRT = Cast<UTextureRenderTargetVolume>(Texture))
	{
		float Width = volumeRT->SizeX;
		float Height = volumeRT->SizeY;
		float Depth = volumeRT->SizeZ;
		for (int32 i = 0; i < Context.NumInstances; ++i)
		{
			*OutWidth.GetDestAndAdvance() = Width;
			*OutHeight.GetDestAndAdvance() = Height;
			*OutDepth.GetDestAndAdvance() = Depth;
		}
	}
}

void UNiagaraDataInterfaceVolumeTextureExt::SampleVolumeTexture(FVectorVMContext& Context)
{
	VectorVM::FExternalFuncInputHandler<float> XParam(Context);
	VectorVM::FExternalFuncInputHandler<float> YParam(Context);
	VectorVM::FExternalFuncInputHandler<float> ZParam(Context);
	VectorVM::FExternalFuncInputHandler<float> MipLevelParam(Context);
	VectorVM::FExternalFuncRegisterHandler<float> OutSampleR(Context);
	VectorVM::FExternalFuncRegisterHandler<float> OutSampleG(Context);
	VectorVM::FExternalFuncRegisterHandler<float> OutSampleB(Context);
	VectorVM::FExternalFuncRegisterHandler<float> OutSampleA(Context);

	for (int32 i = 0; i < Context.NumInstances; ++i)
	{
		float X = XParam.GetAndAdvance();
		float Y = YParam.GetAndAdvance();
		float Z = YParam.GetAndAdvance();
		float Mip = MipLevelParam.GetAndAdvance();
		*OutSampleR.GetDestAndAdvance() = 1.0;
		*OutSampleG.GetDestAndAdvance() = 0.0;
		*OutSampleB.GetDestAndAdvance() = 1.0;
		*OutSampleA.GetDestAndAdvance() = 1.0;
	}

}

bool UNiagaraDataInterfaceVolumeTextureExt::GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL)
{
	if (FunctionInfo.DefinitionName == SampleVolumeTextureName)
	{
		FString HLSLTextureName = TextureName + ParamInfo.DataInterfaceHLSLSymbol;
		FString HLSLSamplerName = SamplerName + ParamInfo.DataInterfaceHLSLSymbol;
		OutHLSL += TEXT("void ") + FunctionInfo.InstanceName + TEXT("(in float3 In_UV, in float MipLevel, out float4 Out_Value) \n{\n");
		OutHLSL += TEXT("\t Out_Value = ") + HLSLTextureName + TEXT(".SampleLevel(") + HLSLSamplerName + TEXT(", In_UV, MipLevel);\n");
		OutHLSL += TEXT("\n}\n");
		return true;
	}
	else if (FunctionInfo.DefinitionName == TextureDimsName)
	{
		FString DimsVar = DimensionsBaseName + ParamInfo.DataInterfaceHLSLSymbol;
		OutHLSL += TEXT("void ") + FunctionInfo.InstanceName + TEXT("(out float3 Out_Value) \n{\n");
		OutHLSL += TEXT("\t Out_Value = ") + DimsVar + TEXT(";\n");
		OutHLSL += TEXT("\n}\n");
		return true;
	}
	return false;
}

void UNiagaraDataInterfaceVolumeTextureExt::GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
	FString HLSLTextureName = TextureName + ParamInfo.DataInterfaceHLSLSymbol;
	FString HLSLSamplerName = SamplerName + ParamInfo.DataInterfaceHLSLSymbol;
	OutHLSL += TEXT("Texture3D ") + HLSLTextureName + TEXT(";\n");
	OutHLSL += TEXT("SamplerState ") + HLSLSamplerName + TEXT(";\n");
	OutHLSL += TEXT("float3 ") + DimensionsBaseName + ParamInfo.DataInterfaceHLSLSymbol + TEXT(";\n");
}


struct FNiagaraDataInterfaceParametersCS_VolumeTextureExt : public FNiagaraDataInterfaceParametersCS
{
	DECLARE_INLINE_TYPE_LAYOUT(FNiagaraDataInterfaceParametersCS_VolumeTextureExt, NonVirtual);
public:
	void Bind(const FNiagaraDataInterfaceGPUParamInfo& ParameterInfo, const class FShaderParameterMap& ParameterMap)
	{
		FString TexName = UNiagaraDataInterfaceVolumeTextureExt::TextureName + ParameterInfo.DataInterfaceHLSLSymbol;
		FString SampleName = (UNiagaraDataInterfaceVolumeTextureExt::SamplerName + ParameterInfo.DataInterfaceHLSLSymbol);
		TextureParam.Bind(ParameterMap, *TexName);
		SamplerParam.Bind(ParameterMap, *SampleName);


		Dimensions.Bind(ParameterMap, *(UNiagaraDataInterfaceVolumeTextureExt::DimensionsBaseName + ParameterInfo.DataInterfaceHLSLSymbol));

	}

	void Set(FRHICommandList& RHICmdList, const FNiagaraDataInterfaceSetArgs& Context) const
	{
		check(IsInRenderingThread());

		FRHIComputeShader* ComputeShaderRHI = Context.Shader.GetComputeShader();
		FNiagaraDataInterfaceProxyVolumeTextureExt* TextureDI = static_cast<FNiagaraDataInterfaceProxyVolumeTextureExt*>(Context.DataInterface);

		if (TextureDI && TextureDI->TextureRHI)
		{
			FRHISamplerState* SamplerStateRHI = TextureDI->SamplerStateRHI;
			if (!SamplerStateRHI)
			{
				// Fallback required because PostLoad() order affects whether RHI resources 
				// are initalized in UNiagaraDataInterfaceVolumeTextureExt::PushToRenderThreadImpl().
				SamplerStateRHI = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
			}
			SetTextureParameter(
				RHICmdList,
				ComputeShaderRHI,
				TextureParam,
				SamplerParam,
				SamplerStateRHI,
				TextureDI->TextureRHI
			);
			SetShaderValue(RHICmdList, ComputeShaderRHI, Dimensions, TextureDI->TexDims);
		}
		else
		{
			SetTextureParameter(
				RHICmdList,
				ComputeShaderRHI,
				TextureParam,
				SamplerParam,
				GBlackVolumeTexture->SamplerStateRHI,
				GBlackVolumeTexture->TextureRHI
			);
			SetShaderValue(RHICmdList, ComputeShaderRHI, Dimensions, FVector::ZeroVector);
		}
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, TextureParam);
	LAYOUT_FIELD(FShaderResourceParameter, SamplerParam);
	LAYOUT_FIELD(FShaderParameter, Dimensions);
};

IMPLEMENT_NIAGARA_DI_PARAMETER(UNiagaraDataInterfaceVolumeTextureExt, FNiagaraDataInterfaceParametersCS_VolumeTextureExt);

void UNiagaraDataInterfaceVolumeTextureExt::PushToRenderThreadImpl()
{
	FNiagaraDataInterfaceProxyVolumeTextureExt* RT_Proxy = GetProxyAs<FNiagaraDataInterfaceProxyVolumeTextureExt>();

	FVector RT_TexDims(EForceInit::ForceInitToZero);
	if (UVolumeTexture* volumeTexture = Cast<UVolumeTexture>(Texture))
	{
		RT_TexDims.X = volumeTexture->GetSizeX();
		RT_TexDims.Y = volumeTexture->GetSizeY();
		RT_TexDims.Z = volumeTexture->GetSizeZ();
	}
	else if (UTextureRenderTargetVolume* volumeRT = Cast<UTextureRenderTargetVolume>(Texture))
	{
		RT_TexDims.X = volumeRT->SizeX;
		RT_TexDims.Y = volumeRT->SizeY;
		RT_TexDims.Z = volumeRT->SizeZ;
	}

	ENQUEUE_RENDER_COMMAND(FPushDITextureToRT)
		(
			[RT_Proxy, RT_Resource = Texture ? Texture->Resource : nullptr, RT_TexDims](FRHICommandListImmediate& RHICmdList)
	{
		RT_Proxy->TextureRHI = RT_Resource ? RT_Resource->TextureRHI : nullptr;
		RT_Proxy->SamplerStateRHI = RT_Resource ? RT_Resource->SamplerStateRHI : nullptr;
		RT_Proxy->TexDims = RT_TexDims;
	}
	);
}

void UNiagaraDataInterfaceVolumeTextureExt::SetTexture(UTexture* InTexture)
{
	if (InTexture)
	{
		Texture = InTexture;
		MarkRenderDataDirty();
	}
}

#undef LOCTEXT_NAMESPACE