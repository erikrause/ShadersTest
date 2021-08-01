#include "D3Q19CSManager.h"
#include "D3Q19Drift.h"
#include "D3Q19Collision.h"

#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "time.h"
#include <Runtime\Engine\Classes\Engine\VolumeTexture.h>

#include "Modules/ModuleManager.h"

#define Q 19

//Static members
FD3Q19CSManager* FD3Q19CSManager::instance = nullptr;

void FD3Q19CSManager::InitResources(UTextureRenderTargetVolume* UTextureRenderTargetVolume, UVolumeTexture* probVolText, FIntVector latticeDims, LbmPrecision lbmPrecision)
{
	// TODO: create UAV from URenderTarget:	auto bprob = cachedParams.URenderTarget->bCanCreateUAV;

	//https://answers.unrealengine.com/questions/805711/write-texture2drhi-or-uav-to-render-target.html
	FRHIResourceCreateInfo resourceCreateInfo;
	FInputTexture = RHICreateTexture3D(latticeDims.X, latticeDims.Y, latticeDims.Z * Q, PF_R32_FLOAT, 1, TexCreate_ShaderResource, resourceCreateInfo);	//TODO: try TexCreate_ShaderResource
	//auto fTextureSRV = RHICreateShaderResourceView(FInputTexture, FRHITextureSRVCreateInfo());
	//FRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, 11000, 512);

	FOutputTexture = RHICreateTexture3D(latticeDims.X, latticeDims.Y, latticeDims.Z * Q, EPixelFormat::PF_R32_FLOAT, 1, TexCreate_ShaderResource | TexCreate_UAV, resourceCreateInfo);
	FOutputTexture_UAV = RHICreateUnorderedAccessView(FOutputTexture);
	FOutputTexture_SRV = RHICreateShaderResourceView(FOutputTexture, FRHITextureSRVCreateInfo());

	check(UTextureRenderTargetVolume->bCanCreateUAV);
	////UOutputTexture = RHICreateTexture3D(latticeDims.X, latticeDims.Y, latticeDims.Z, EPixelFormat::PF_R32_FLOAT, 1, TexCreate_ShaderResource | TexCreate_UAV, resourceCreateInfo);
	UOutputTexture = UTextureRenderTargetVolume->GameThread_GetRenderTargetResource()->TextureRHI.GetReference()->GetTexture3D();
	UOutputTexture_UAV = RHICreateUnorderedAccessView(UOutputTexture);

	//UTextureRenderTargetVolume->GetRenderTargetResource()->TextureRHI;
	ProbVolTexRHI = probVolText->Resource->TextureRHI;//GetRenderTargetResource()->TextureRHI
}

//Begin the execution of the compute shader each frame
void FD3Q19CSManager::BeginRendering()
{
	//If the handle is already initalized and valid, no need to do anything
	if (OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}
	bCachedParamsAreValid = false;
	//Get the Renderer Module and add our entry to the callbacks so it can be executed each frame after the scene rendering is done
	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this, &FD3Q19CSManager::Execute_RenderThread);
	}

}

//Stop the compute shader execution
void FD3Q19CSManager::EndRendering()
{
	//If the handle is not valid then there's no cleanup to do
	if (!OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}

	//Get the Renderer Module and remove our entry from the ResolvedSceneColorCallbacks
	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		RendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}

	OnPostResolvedSceneColorHandle.Reset();
}

//Update the parameters by a providing an instance of the Parameters structure used by the shader manager
void FD3Q19CSManager::UpdateParameters(FD3Q19CSParameters& params)
{
	cachedParams = params;
	bCachedParamsAreValid = true;
}


/// <summary>
/// Creates an instance of the shader type parameters structure and fills it using the cached shader manager parameter structure
/// Gets a reference to the shader type from the global shaders map
/// Dispatches the shader using the parameter structure instance
/// </summary>
void FD3Q19CSManager::Execute_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext)
{
	//If there's no cached parameters to use, skip
	//If no Render Target is supplied in the cachedParams, skip
	if (!(bCachedParamsAreValid && cachedParams.URenderTarget))
	{
		return;
	}

	//Render Thread Assertion
	check(IsInRenderingThread());

	FIntVector latticeDims = cachedParams.GetLatticeDims();


	// Test resource array. Sources: https://forums.unrealengine.com/t/get-data-back-from-compute-shader/12880 ,
	// https://cpp.hotexamples.com/ru/examples/-/TResourceArray/AddUninitialized/cpp-tresourcearray-adduninitialized-method-examples.html , 
	// https://veldrid.dev/articles/shaders.html
	TResourceArray<int> porousBuffer;		// TODO: кэшировать буфер.
	porousBuffer.Append(cachedParams.PorousDataArray, latticeDims.X * latticeDims.Y * latticeDims.Z);
	FRHIResourceCreateInfo CreateInfo;
	CreateInfo.ResourceArray = &porousBuffer;

	//TRefCountPtr<FRHIStructuredBuffer>
	FStructuredBufferRHIRef StructResource = RHICreateStructuredBuffer(
		sizeof(int),
		porousBuffer.Num() * sizeof(int),
		BUF_UnorderedAccess | BUF_ShaderResource,
		CreateInfo
	);
	PorousStructSRV = RHICreateShaderResourceView(StructResource);
	//FUnorderedAccessViewRHIRef StructUAV = RHICreateUnorderedAccessView(StructResource, false, false);


	
	//Unbind the previously bound render targets
	//UnbindRenderTargets(RHICmdList);	- code from 4.25. TODO: есть ли замена в 4.26?

	//Specify the resource transition, we're executing this in post scene rendering so we set it to Graphics to Compute
	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, FOutputTexture_UAV);		// TODO: проверить работоспособность без этих строк.
	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, UOutputTexture_UAV);

	// CREATE THE SAMPLER STATE RHI RESOURCE.
	//ESamplerAddressMode SamplerAddressMode = Owner->SamplerAddressMode;
	FSamplerStateInitializerRHI SamplerStateInitializer(ESamplerFilter::SF_Point);
		//(ESamplerFilter)UDeviceProfileManager::Get().GetActiveProfile()->GetTextureLODSettings()->GetSamplerFilter(Owner),
	//	SamplerAddressMode,
	//	SamplerAddressMode,
	//	SamplerAddressMode
	//);
	//SamplerStateRHI = RHICreateSamplerState(SamplerStateInitializer);
	

	
	//FShaderResourceViewRHIRef FSRV = RHICreateShaderResourceView(FPooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, FRHITextureSRVCreateInfo());	не работает.


	//Fill the shader parameters structure with tha cached data supplied by the client
	FD3Q19CSDrift::FParameters DriftCSParameters;
	DriftCSParameters.PorousData = PorousStructSRV;
	DriftCSParameters.F_SamplerState = RHICreateSamplerState(SamplerStateInitializer);
	DriftCSParameters.F_in = FInputTexture.GetReference();	// FPooledRenderTarget.GetReference()->GetRenderTargetItem().ShaderResourceTexture;
	DriftCSParameters.F_out = FOutputTexture_UAV;	// TODO: for UAV try bind.
	DriftCSParameters.Rho0 = 100;
	DriftCSParameters.Iteration = cachedParams.Iteration;
	//DriftCSParameters.Tau = 0.6;
	DriftCSParameters.Nx = cachedParams.GetLatticeDims().X;
	DriftCSParameters.Ny = cachedParams.GetLatticeDims().Y;
	DriftCSParameters.Nz = cachedParams.GetLatticeDims().Z;

	// TODO: F_in = F_out in collision.
	FD3Q19CSCollision::FParameters CollisionCSParameters;
	CollisionCSParameters.F_in = FInputTexture.GetReference();	// FPooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture;
	CollisionCSParameters.F_out = FOutputTexture_UAV;
	CollisionCSParameters.Iteration = cachedParams.Iteration;
	CollisionCSParameters.Nx = cachedParams.GetLatticeDims().X;
	CollisionCSParameters.Ny = cachedParams.GetLatticeDims().Y;
	CollisionCSParameters.Nz = cachedParams.GetLatticeDims().Z;
	CollisionCSParameters.PorousData = PorousStructSRV;
	CollisionCSParameters.U = UOutputTexture_UAV;

	//Get a reference to our shader type from global shader map
	TShaderMapRef<FD3Q19CSDrift> D3Q19CSDrift(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	TShaderMapRef<FD3Q19CSCollision> D3Q19CSCollision(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	//TShaderMapRef<FParticlesCS> Particles(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	clock_t start, end;
	start = clock();

	// DRIFT:
	FComputeShaderUtils::Dispatch(RHICmdList, D3Q19CSDrift, DriftCSParameters,
		FIntVector(FMath::DivideAndRoundUp(latticeDims.X, NUM_THREADS_PER_GROUP_DIMENSION),
				   FMath::DivideAndRoundUp(latticeDims.Y, NUM_THREADS_PER_GROUP_DIMENSION),
				   FMath::DivideAndRoundUp(latticeDims.Z, 1)));
	//Copy shader's output to the render target provided by the client
	RHICmdList.CopyTexture(FOutputTexture, FInputTexture, FRHICopyTextureInfo());	// TODO: try swap texture.

	// COLLISION:
	FComputeShaderUtils::Dispatch(RHICmdList, D3Q19CSCollision, CollisionCSParameters,
		FIntVector(FMath::DivideAndRoundUp(latticeDims.X, NUM_THREADS_PER_GROUP_DIMENSION),
				   FMath::DivideAndRoundUp(latticeDims.Y, NUM_THREADS_PER_GROUP_DIMENSION),
				   FMath::DivideAndRoundUp(latticeDims.Z, 1)));
	//RHICmdList.CopyTexture(FPooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, cachedParams.FRenderTarget->GetRenderTargetResource()->TextureRHI, FRHICopyTextureInfo());
	RHICmdList.CopyTexture(FOutputTexture, FInputTexture, FRHICopyTextureInfo());
	RHICmdList.CopyTexture(UOutputTexture, ProbVolTexRHI, FRHICopyTextureInfo());
	//RHICmdList.CopyTexture(UOutputTexture, cachedParams.URenderTarget->GetRenderTargetResource()->TextureRHI, FRHICopyTextureInfo());
	//RHICmdList.SetComputeShader(D3Q19CSDrift.GetComputeShader());	// зачем?
	// TODO: unbind?


	end = clock();
	double result = ((double)end - start) / ((double)CLOCKS_PER_SEC);
	int prob = 0;
}


// Create & fill texture. Source: https://www.reddit.com/r/unrealengine/comments/gxmmmr/pass_an_array_of_floatsfcolors_to_material/

//UTexture2D* CreateTextureFrom32BitFloat(TArray<float> data, int width, int height) {
//	UTexture2D* Texture;
//	Texture = UTexture2D::CreateTransient(width, height, PF_R32_FLOAT);
//	if (!Texture) return nullptr;
//#if WITH_EDITORONLY_DATA
//	Texture->MipGenSettings = TMGS_NoMipmaps;
//#endif
//	Texture->NeverStream = true;
//	Texture->SRGB = 0;
//	Texture->LODGroup = TextureGroup::TEXTUREGROUP_Pixels2D;
//	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
//	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
//	FMemory::Memcpy(Data, data.GetData(), width * height * 4);
//	Mip.BulkData.Unlock();
//	Texture->UpdateResource();
//	return Texture;
//}