#pragma once

#include "CoreMinimal.h"
//#include "D3Q19CSParameters.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

//This struct act as a container for all the parameters that the client needs to pass to the Compute Shader Manager.
struct D3Q19CSParameters
{
	// “екстура дл€ хранени€ распределени€ частиц в узлах.
	UTextureRenderTarget2D* FRenderTarget;

	// “екстура дл€ хранени€ скоростей в узлах.
	UTextureRenderTarget2D* URenderTarget;

	// “екстура дл€ хранени€ позиций частиц.
	UTextureRenderTarget2D* PosRenderTarget;

	// “екстура дл€ хранени€ пористой среды.
	UTextureRenderTarget2D* PorousRenderTarget;

	// Test.
	int Iteration = 0;
	int* PorousDataArray;

	/// <summary>
	/// ¬ернуть разрешение текстуры.
	/// </summary>
	/// <param name="isOnlyXYDims"> ¬ернуть XYZ - разрешение сетки узлов. </param>
	/// <returns></returns>
	FIntPoint GetRenderTargetSize(bool isOnlyXYDims = false) const
	{
		if (isOnlyXYDims)
		{
			FIntPoint XY = _cachedRenderTargetSize;
			XY.Y /= 19;
			return XY;
		}
		else
			return _cachedRenderTargetSize;
		{
		}
	}

	FIntVector GetLatticeDims() const
	{
		return _latticeDims;
	}

	D3Q19CSParameters() { }
	D3Q19CSParameters(UTextureRenderTarget2D* IORenderTarget, UTextureRenderTarget2D* uRenderTarget, UTextureRenderTarget2D* posRenderTarget, UTextureRenderTarget2D* porousRenderTarget, int* porousDataArray, FIntVector latticeDims)
		: FRenderTarget(IORenderTarget), URenderTarget(uRenderTarget), PosRenderTarget(posRenderTarget), PorousRenderTarget(porousRenderTarget)
	{
		PorousDataArray = porousDataArray;
		_cachedRenderTargetSize = FRenderTarget ? FIntPoint(FRenderTarget->SizeX, FRenderTarget->SizeY) : FIntPoint::ZeroValue;
		_latticeDims = latticeDims;

	}

private:
	FIntPoint _cachedRenderTargetSize;
	FIntVector _latticeDims;
};