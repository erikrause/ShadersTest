PRAGMA_DISABLE_OPTIMIZATION

#include "D3Q19CSManager.h"
#include "D3Q19CSDrift.h"
#include "D3Q19CSCollision.h"
#include "ParticlesCS.h"

#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "time.h"

#include "Modules/ModuleManager.h"

#define Q 19

//Static members
FD3Q19CSManager* FD3Q19CSManager::instance = nullptr;

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
void FD3Q19CSManager::UpdateParameters(D3Q19CSParameters& params)
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

	//If the render target is not valid, get an element from the render target pool by supplying a Descriptor
	if (!FPooledRenderTarget.IsValid())
	{
		//https://answers.unrealengine.com/questions/805711/write-texture2drhi-or-uav-to-render-target.html
		FRHIResourceCreateInfo resourceCreateInfo;
		FTexture = RHICreateTexture2D(latticeDims.X * latticeDims.Z, latticeDims.Z * Q, PF_R32_FLOAT, 1, 1, TexCreate_None, resourceCreateInfo);
		//auto fTextureSRV = RHICreateShaderResourceView(FTexture, FRHITextureSRVCreateInfo());

		//FRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, 11000, 512);
		//FTexture2DRHIRef = RHIAsyncCreateTexture2D
		//FTextureRenderTarget2DResource tex = TextureResource
		UE_LOG(LogTemp, Warning, TEXT("F pool is not Valid"));	// TODO: pixel format to R32.
		FPooledRenderTargetDesc FOutputDesc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(latticeDims.X * latticeDims.Z, latticeDims.Y * Q), EPixelFormat::PF_R32_FLOAT, FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));
		FOutputDesc.DebugName = TEXT("FCS_Output_RenderTarget");
		GRenderTargetPool.FindFreeElement(RHICmdList, FOutputDesc, FPooledRenderTarget, TEXT("FCS_Output_RenderTarget"));
	}
	if (!UPooledRenderTarget.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Vorticity pool is not Valid"));
		FPooledRenderTargetDesc VorticityOutputDesc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(latticeDims.X * latticeDims.Z, latticeDims.Y), cachedParams.URenderTarget->GetRenderTargetResource()->TextureRHI->GetFormat(), FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));	// TODO: optimize format.
		VorticityOutputDesc.DebugName = TEXT("UCS_Output_RenderTarget");
		GRenderTargetPool.FindFreeElement(RHICmdList, VorticityOutputDesc, UPooledRenderTarget, TEXT("UCS_Output_RenderTarget"));
	}
	//auto textureUAVRef = RHICreateUnorderedAccessView(cachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI);
	if (!PorousStructSRV.IsValid())
	{
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
	}

	//Unbind the previously bound render targets
	UnbindRenderTargets(RHICmdList);

	//Specify the resource transition, we're executing this in post scene rendering so we set it to Graphics to Compute
	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, FPooledRenderTarget->GetRenderTargetItem().UAV);		// TODO: проверить работоспособность без этих строк.
	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, UPooledRenderTarget->GetRenderTargetItem().UAV);

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
	DriftCSParameters.F_in = FTexture;//fTextureSRV;//FSRV;	//cachedParams.FRenderTarget->GetRenderTargetResource()->TextureRHI;
	DriftCSParameters.F_out = FPooledRenderTarget->GetRenderTargetItem().UAV;	// TODO: for UAV try bind.
	DriftCSParameters.Rho0 = 100;
	DriftCSParameters.Iteration = cachedParams.Iteration;
	//DriftCSParameters.Tau = 0.6;
	DriftCSParameters.Nx = cachedParams.GetLatticeDims().X;
	DriftCSParameters.Ny = cachedParams.GetLatticeDims().Y;
	DriftCSParameters.Nz = cachedParams.GetLatticeDims().Z;

	// TODO: F_in = F_out in collision.
	FD3Q19CSCollision::FParameters CollisionCSParameters;
	CollisionCSParameters.F_in = FTexture;//fTextureSRV;//FSRV;	//FPooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture;
	CollisionCSParameters.F_out = FPooledRenderTarget->GetRenderTargetItem().UAV;
	CollisionCSParameters.Iteration = cachedParams.Iteration;
	CollisionCSParameters.Nx = cachedParams.GetLatticeDims().X;
	CollisionCSParameters.Ny = cachedParams.GetLatticeDims().Y;
	CollisionCSParameters.Nz = cachedParams.GetLatticeDims().Z;
	CollisionCSParameters.PorousData = PorousStructSRV;
	CollisionCSParameters.U = UPooledRenderTarget->GetRenderTargetItem().UAV;

	//FParticlesCS::FParameters ParticlesCSParameters;
	//ParticlesCSParameters.Iteration = cachedParams.Iteration;
	//ParticlesCSParameters.Nx = cachedParams.GetLatticeDims().X;
	//ParticlesCSParameters.Ny = cachedParams.GetLatticeDims().Y;
	//ParticlesCSParameters.Nz = cachedParams.GetLatticeDims().Z;
	//ParticlesCSParameters.Pos_in = cachedParams.PosRenderTarget->GetRenderTargetResource()->TextureRHI;
	//ParticlesCSParameters.Pos_out = PosPooledRenderTarget->GetRenderTargetItem().UAV;
	//ParticlesCSParameters.U = cachedParams.URenderTarget->GetRenderTargetResource()->TextureRHI;

	//Get a reference to our shader type from global shader map
	TShaderMapRef<FD3Q19CSDrift> D3Q19CSDrift(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	TShaderMapRef<FD3Q19CSCollision> D3Q19CSCollision(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	//TShaderMapRef<FParticlesCS> Particles(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	clock_t start, end;
	start = clock();

	// DRIFT:
	FComputeShaderUtils::Dispatch(RHICmdList, D3Q19CSDrift, DriftCSParameters,
		FIntVector(FMath::DivideAndRoundUp(latticeDims.X * latticeDims.Z, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(latticeDims.Y, NUM_THREADS_PER_GROUP_DIMENSION), 1));
	//Copy shader's output to the render target provided by the client
	RHICmdList.CopyTexture(FPooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, FTexture, FRHICopyTextureInfo());

	// COLLISION:
	FComputeShaderUtils::Dispatch(RHICmdList, D3Q19CSCollision, CollisionCSParameters,
		FIntVector(FMath::DivideAndRoundUp(latticeDims.X * latticeDims.Z, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(latticeDims.Y, NUM_THREADS_PER_GROUP_DIMENSION), 1));
	//RHICmdList.CopyTexture(FPooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, cachedParams.FRenderTarget->GetRenderTargetResource()->TextureRHI, FRHICopyTextureInfo());
	RHICmdList.CopyTexture(FPooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, FTexture, FRHICopyTextureInfo());
	RHICmdList.CopyTexture(UPooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, cachedParams.URenderTarget->GetRenderTargetResource()->TextureRHI, FRHICopyTextureInfo());
	//RHICmdList.SetComputeShader(D3Q19CSDrift.GetComputeShader());	// зачем?


	//// PARTICLES UPDATE:
	//FComputeShaderUtils::Dispatch(RHICmdList, Particles, ParticlesCSParameters,
	//	FIntVector(FMath::DivideAndRoundUp(cachedParams.PosRenderTarget->SizeX, NUM_THREADS_PER_GROUP_DIMENSION * NUM_THREADS_PER_GROUP_DIMENSION), 1, 1));
	//RHICmdList.CopyTexture(PosPooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, cachedParams.PosRenderTarget->GetRenderTargetResource()->TextureRHI, FRHICopyTextureInfo());

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