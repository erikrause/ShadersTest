PRAGMA_DISABLE_OPTIMIZATION

#include "D2Q7CSManager.h"
#include "D2Q7CS.h"
//#include "D2Q7CSParameters.h"

#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "time.h"

#include "Modules/ModuleManager.h"



//Static members
FD2Q7CSManager* FD2Q7CSManager::instance = nullptr;

//Begin the execution of the compute shader each frame
void FD2Q7CSManager::BeginRendering()
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
		OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this, &FD2Q7CSManager::Execute_RenderThread);
	}
}

//Stop the compute shader execution
void FD2Q7CSManager::EndRendering()
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
void FD2Q7CSManager::UpdateParameters(D2Q7CSParameters& params)
{
	cachedParams = params;
	bCachedParamsAreValid = true;
}


/// <summary>
/// Creates an instance of the shader type parameters structure and fills it using the cached shader manager parameter structure
/// Gets a reference to the shader type from the global shaders map
/// Dispatches the shader using the parameter structure instance
/// </summary>
void FD2Q7CSManager::Execute_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext)
{
	//If there's no cached parameters to use, skip
	//If no Render Target is supplied in the cachedParams, skip
	if (!(bCachedParamsAreValid && cachedParams.FRenderTarget))
	{
		return;
	}

	//Render Thread Assertion
	check(IsInRenderingThread());

	//If the render target is not valid, get an element from the render target pool by supplying a Descriptor
	if (!FPooledRenderTarget.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("F pool is not Valid"));
		FPooledRenderTargetDesc FOutputDesc(FPooledRenderTargetDesc::Create2DDesc(cachedParams.GetRenderTargetSize(), cachedParams.FRenderTarget->GetRenderTargetResource()->TextureRHI->GetFormat(), FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));
		FOutputDesc.DebugName = TEXT("FCS_Output_RenderTarget");
		GRenderTargetPool.FindFreeElement(RHICmdList, FOutputDesc, FPooledRenderTarget, TEXT("FCS_Output_RenderTarget"));
	}
	if (!UPooledRenderTarget.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Vorticity pool is not Valid"));
		FPooledRenderTargetDesc VorticityOutputDesc(FPooledRenderTargetDesc::Create2DDesc(cachedParams.GetRenderTargetSize(true), cachedParams.URenderTarget->GetRenderTargetResource()->TextureRHI->GetFormat(), FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));
		VorticityOutputDesc.DebugName = TEXT("UCS_Output_RenderTarget");
		GRenderTargetPool.FindFreeElement(RHICmdList, VorticityOutputDesc, UPooledRenderTarget, TEXT("UCS_Output_RenderTarget"));
	}
	//auto textureUAVRef = RHICreateUnorderedAccessView(cachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI);

	//Unbind the previously bound render targets
	UnbindRenderTargets(RHICmdList);

	//Specify the resource transition, we're executing this in post scene rendering so we set it to Graphics to Compute
	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, FPooledRenderTarget->GetRenderTargetItem().UAV);
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
	
	// Test resource array. Sources: https://forums.unrealengine.com/t/get-data-back-from-compute-shader/12880 ,
	// https://cpp.hotexamples.com/ru/examples/-/TResourceArray/AddUninitialized/cpp-tresourcearray-adduninitialized-method-examples.html , 
	// https://veldrid.dev/articles/shaders.html
	TResourceArray<int> porousBuffer;
	porousBuffer.Append(cachedParams.PorousDataArray, 64 * 64 * 64);
	FRHIResourceCreateInfo CreateInfo;
	CreateInfo.ResourceArray = &porousBuffer;

	FStructuredBufferRHIRef StructResource = RHICreateStructuredBuffer(
		sizeof(int),
		porousBuffer.Num() * sizeof(int),
		BUF_UnorderedAccess | BUF_ShaderResource,
		CreateInfo
	);
	FUnorderedAccessViewRHIRef StructUAV = RHICreateUnorderedAccessView(StructResource, false, false);


	//Fill the shader parameters structure with tha cached data supplied by the client
	FD2Q7CS::FParameters PassParameters;
	PassParameters.PorousData = StructUAV;
	//PassParameters.PorousData = cachedParams.PorousDataArray;
	PassParameters.F_SamplerState = RHICreateSamplerState(SamplerStateInitializer);
	PassParameters.F_in = cachedParams.FRenderTarget->GetRenderTargetResource()->TextureRHI;
	PassParameters.F_out = FPooledRenderTarget->GetRenderTargetItem().UAV;
	PassParameters.Rho0 = 100;
	PassParameters.Tau = 0.6;
	PassParameters.IsInit = 1;		//PassParameters.IsInit = cachedParams.IsInit;
	PassParameters.Nx = cachedParams.GetRenderTargetSize().X;
	PassParameters.Ny = cachedParams.GetRenderTargetSize().Y / 9;
	PassParameters.U = UPooledRenderTarget->GetRenderTargetItem().UAV;
	//PassParameters.Dimensions = FVector2D(cachedParams.GetRenderTargetSize().X, cachedParams.GetRenderTargetSize().Y);
	//PassParameters.TimeStamp = cachedParams.TimeStamp;

	//Get a reference to our shader type from global shader map
	TShaderMapRef<FD2Q7CS> D2Q7CS(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	clock_t start, end;

	start = clock();

	//Dispatch the compute shader
	FComputeShaderUtils::Dispatch(RHICmdList, D2Q7CS, PassParameters,
		FIntVector(FMath::DivideAndRoundUp(cachedParams.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(cachedParams.GetRenderTargetSize().Y / 9, NUM_THREADS_PER_GROUP_DIMENSION), 1));

	//Copy shader's output to the render target provided by the client
	RHICmdList.CopyTexture(FPooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, cachedParams.FRenderTarget->GetRenderTargetResource()->TextureRHI, FRHICopyTextureInfo());
	RHICmdList.CopyTexture(UPooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, cachedParams.URenderTarget->GetRenderTargetResource()->TextureRHI, FRHICopyTextureInfo());
	//RHICmdList.SetComputeShader(D2Q7CS.GetComputeShader());	// зачем?

	end = clock();

	double result = ((double)end - start) / ((double)CLOCKS_PER_SEC);


	int prob = 0;
}