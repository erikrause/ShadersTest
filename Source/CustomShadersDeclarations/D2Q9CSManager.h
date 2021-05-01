#pragma once

#include "CoreMinimal.h"
#include "D2Q9CSParameters.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

////This struct act as a container for all the parameters that the client needs to pass to the Compute Shader Manager.
//struct D2Q9CSParameters
//{
//	UTextureRenderTarget2D* RenderTarget;
//
//	FIntPoint GetRenderTargetSize() const
//	{
//		return CachedRenderTargetSize;
//	}
//
//	D2Q9CSParameters() { }
//	D2Q9CSParameters(UTextureRenderTarget2D* IORenderTarget)
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
class CUSTOMSHADERSDECLARATIONS_API FD2Q9CSManager
{
public:
	//Get the instance
	static FD2Q9CSManager* Get()
	{
		if (!instance)
			instance = new FD2Q9CSManager();
		return instance;
	};

	// Call this when you want to hook onto the renderer and start executing the compute shader. The shader will be dispatched once per frame.
	void BeginRendering();

	// Stops compute shader execution
	void EndRendering();

	// Call this whenever you have new parameters to share.
	void UpdateParameters(D2Q9CSParameters& DrawParameters);

private:
	//Private constructor to prevent client from instanciating
	FD2Q9CSManager() = default;

	//The singleton instance
	static FD2Q9CSManager* instance;

	//The delegate handle to our function that will be executed each frame by the renderer
	FDelegateHandle OnPostResolvedSceneColorHandle;

	//Cached Shader Manager Parameters
	D2Q9CSParameters cachedParams;

	//Whether we have cached parameters to pass to the shader or not
	volatile bool bCachedParamsAreValid;

	//Reference to a pooled render target where the shader will write its output
	TRefCountPtr<IPooledRenderTarget> FPooledRenderTarget;
	TRefCountPtr<IPooledRenderTarget> UPooledRenderTarget;
	uint32 _iteration = 0;
	uint32 _maxIteration = 5000;

	// For texture debbuging
	void GetTexturePixels(FTexture2DRHIRef Texture, TArray<FColor>& OutPixels);

public:
	void Execute_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext);
};