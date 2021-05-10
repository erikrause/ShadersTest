#pragma once

#include "CoreMinimal.h"
#include "D3Q19CSParameters.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

////This struct act as a container for all the parameters that the client needs to pass to the Compute Shader Manager.
//struct D3Q19CSParameters
//{
//	UTextureRenderTarget2D* RenderTarget;
//
//	FIntPoint GetRenderTargetSize() const
//	{
//		return CachedRenderTargetSize;
//	}
//
//	D3Q19CSParameters() { }
//	D3Q19CSParameters(UTextureRenderTarget2D* IORenderTarget)
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
class CUSTOMSHADERSDECLARATIONS_API FD3Q19CSManager
{
public:
	//Get the instance
	static FD3Q19CSManager* Get()
	{
		if (!instance)
			instance = new FD3Q19CSManager();
		return instance;
	};

	// Call this when you want to hook onto the renderer and start executing the compute shader. The shader will be dispatched once per frame.
	void BeginRendering();

	// Stops compute shader execution
	void EndRendering();

	// Call this whenever you have new parameters to share.
	void UpdateParameters(D3Q19CSParameters& DrawParameters);

private:
	//Private constructor to prevent client from instanciating
	FD3Q19CSManager() = default;

	//The singleton instance
	static FD3Q19CSManager* instance;

	//The delegate handle to our function that will be executed each frame by the renderer
	FDelegateHandle OnPostResolvedSceneColorHandle;

	//Cached Shader Manager Parameters
	D3Q19CSParameters cachedParams;

	//Whether we have cached parameters to pass to the shader or not
	volatile bool bCachedParamsAreValid;

	//Reference to a pooled render target where the shader will write its output
	TRefCountPtr<IPooledRenderTarget> FPooledRenderTarget;
	TRefCountPtr<IPooledRenderTarget> UPooledRenderTarget;
	TRefCountPtr<IPooledRenderTarget> PosPooledRenderTarget;
	TRefCountPtr<IPooledRenderTarget> PorousPooledRenderTarget;
	//uint32 _iteration = 0;
	//uint32 _maxIteration = 5000;
public:
	void Execute_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext);
};