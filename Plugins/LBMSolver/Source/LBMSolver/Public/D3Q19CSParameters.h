#pragma once

#include "CoreMinimal.h"
//#include "FD3Q19CSParameters.h"
#include <Runtime/Engine/Classes/Engine/TextureRenderTargetVolume.h>

//This struct act as a container for all the parameters that the client needs to pass to the Compute Shader Manager.
struct FD3Q19CSParameters
{
	//// �������� ��� �������� ������������� ������ � �����.
	//UTextureRenderTarget2D* FRenderTarget;

	// �������� ��� �������� ��������� � �����.
	UTextureRenderTargetVolume* URenderTarget;
	UTextureRenderTargetVolume* DensityRenderTarget;

	// Test.
	int Iteration = 0;
	int* PorousDataArray;
	//float DeltaTime = 0;
	//float DeltaX = 0;

	///// <summary>
	///// ������� ���������� ��������.
	///// </summary>
	///// <param name="isOnlyXYDims"> ������� XYZ - ���������� ����� �����. </param>
	///// <returns></returns>
	//FIntPoint GetRenderTargetSize(bool isOnlyXYDims = false) const
	//{
	//	if (isOnlyXYDims)
	//	{
	//		FIntPoint XY = _cachedRenderTargetSize;
	//		XY.Y /= 19;
	//		return XY;
	//	}
	//	else
	//		return _cachedRenderTargetSize;
	//	{
	//	}
	//}

	FIntVector GetLatticeDims() const
	{
		return _latticeDims;
	}

	FD3Q19CSParameters() { }
	FD3Q19CSParameters(UTextureRenderTargetVolume* uRenderTarget, int* porousDataArray, FIntVector latticeDims, UTextureRenderTargetVolume* densityRenderTarget)
		: URenderTarget(uRenderTarget), DensityRenderTarget(densityRenderTarget)
	{
		PorousDataArray = porousDataArray;
		//_cachedRenderTargetSize = URenderTarget ? FIntPoint(URenderTarget->SizeX, URenderTarget->SizeY) : FIntPoint::ZeroValue;
		_latticeDims = latticeDims;

	}

private:
	//FIntPoint _cachedRenderTargetSize;
	FIntVector _latticeDims;
};