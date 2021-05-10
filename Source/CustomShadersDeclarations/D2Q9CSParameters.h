#pragma once

#include "CoreMinimal.h"
//#include "D2Q9CSParameters.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

//This struct act as a container for all the parameters that the client needs to pass to the Compute Shader Manager.
struct D2Q9CSParameters
{
	// “екстура дл€ хранени€ распределени€ частиц в узлах.
	UTextureRenderTarget2D* FRenderTarget;

	// “екстура дл€ хранени€ скоростей в узлах.
	UTextureRenderTarget2D* URenderTarget;

	// “екстура дл€ хранени€ позиций частиц.
	UTextureRenderTarget2D* PosRenderTarget;

	// Test.
	int Iteration = 0;
	int* PorousDataArray;

	/// <summary>
	/// ¬ернуть разрешение текстуры.
	/// </summary>
	/// <param name="isOnlyXYDims"> ¬ернуть XY - разрешение сетки узлов. </param>
	/// <returns></returns>
	FIntPoint GetRenderTargetSize(bool isOnlyXYDims = false) const
	{
		if (isOnlyXYDims)
		{
			FIntPoint XY = CachedRenderTargetSize;
			XY.Y /= 9;
			return XY;
		}
		else
			return CachedRenderTargetSize;
		{
		}
	}

	D2Q9CSParameters() { }
	D2Q9CSParameters(UTextureRenderTarget2D* IORenderTarget, UTextureRenderTarget2D* uRenderTarget, UTextureRenderTarget2D* posRenderTarget, int* porousDataArray)
		: FRenderTarget(IORenderTarget), URenderTarget(uRenderTarget), PosRenderTarget(posRenderTarget)
	{
		PorousDataArray = porousDataArray;
		CachedRenderTargetSize = FRenderTarget ? FIntPoint(FRenderTarget->SizeX, FRenderTarget->SizeY) : FIntPoint::ZeroValue;
	}

private:
	FIntPoint CachedRenderTargetSize;
};