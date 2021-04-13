#pragma once

#include "CoreMinimal.h"
//#include "D2Q7CSParameters.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

//This struct act as a container for all the parameters that the client needs to pass to the Compute Shader Manager.
struct D2Q7CSParameters
{
	UTextureRenderTarget2D* RenderTarget;
	bool IsInit = true;

	FIntPoint GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}

	D2Q7CSParameters() { }
	D2Q7CSParameters(UTextureRenderTarget2D* IORenderTarget)
		: RenderTarget(IORenderTarget)
	{
		CachedRenderTargetSize = RenderTarget ? FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY) : FIntPoint::ZeroValue;
	}

private:
	FIntPoint CachedRenderTargetSize;
};