#pragma once

#include "CoreMinimal.h"
#include "D3Q19CSParameters.h"

/* Не используется (TODO) */
enum LbmPrecision
{
	Single	=0,
	Half	=1
};

////This struct act as a container for all the parameters that the client needs to pass to the Compute Shader Manager.
//struct FD3Q19CSParameters
//{
//	UTextureRenderTarget2D* RenderTarget;
//
//	FIntPoint GetRenderTargetSize() const
//	{
//		return CachedRenderTargetSize;
//	}
//
//	FD3Q19CSParameters() { }
//	FD3Q19CSParameters(UTextureRenderTarget2D* IORenderTarget)
//		: RenderTarget(IORenderTarget)
//	{
//		CachedRenderTargetSize = RenderTarget ? FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY) : FIntPoint::ZeroValue;
//	}
//
//private:
//	FIntPoint CachedRenderTargetSize;
//};

/// <summary>
/// A singleton Shader Manager for our Shader Type
/// </summary>
class LBMSOLVER_API FD3Q19CSManager
{
public:
	//Get the instance
	static FD3Q19CSManager* Get()
	{
		if (!instance)
			instance = new FD3Q19CSManager();
		return instance;
		//return new FD3Q19CSManager();
	};

	void InitResources(UTextureRenderTargetVolume* UTextureRenderTargetVolume, FIntVector latticeDims, LbmPrecision lbmPrecision = LbmPrecision::Single);

	// Call this when you want to hook onto the renderer and start executing the compute shader. The shader will be dispatched once per frame.
	void BeginRendering();

	// Stops compute shader execution
	void EndRendering();

	// Call this whenever you have new parameters to share.
	void UpdateParameters(FD3Q19CSParameters& DrawParameters);

private:
	//Private constructor to prevent client from instanciating
	FD3Q19CSManager() = default;

	//The singleton instance
	static FD3Q19CSManager* instance;

	//The delegate handle to our function that will be executed each frame by the renderer
	FDelegateHandle OnPostResolvedSceneColorHandle;

	//Cached Shader Manager Parameters
	FD3Q19CSParameters cachedParams;

	//Whether we have cached parameters to pass to the shader or not
	volatile bool bCachedParamsAreValid;

	
	/* Распределение частиц на входе */
	FTexture3DRHIRef FInputTexture;

	/* Распределение частиц на выходе шейдеров Drift и Collision */
	FTexture3DRHIRef FOutputTexture;
	FUnorderedAccessViewRHIRef FOutputTexture_UAV;
	FShaderResourceViewRHIRef FOutputTexture_SRV;

	/* Значения глобальных скоростей в узлах */
	FTexture3DRHIRef UOutputTexture;
	FUnorderedAccessViewRHIRef UOutputTexture_UAV;

	/* Ресурс для хранения пористой структуры */
	FShaderResourceViewRHIRef PorousStructSRV;	// TODO: make 3D and do perfomance comprasion.

	//uint32 _iteration = 0;
	//uint32 _maxIteration = 5000;
public:
	void Execute_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext);
};