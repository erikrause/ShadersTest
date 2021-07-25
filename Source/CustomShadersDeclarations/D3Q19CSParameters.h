#pragma once

#include "CoreMinimal.h"
//#include "D3Q19CSParameters.h"
#include <Runtime/Engine/Classes/Engine/TextureRenderTargetVolume.h>

//This struct act as a container for all the parameters that the client needs to pass to the Compute Shader Manager.
struct D3Q19CSParameters
{
	//// �������� ��� �������� ������������� ������ � �����.
	//UTextureRenderTarget2D* FRenderTarget;

	// �������� ��� �������� ��������� � �����.
	UTextureRenderTargetVolume* URenderTarget;

	// Test.
	int Iteration = 0;
	int* PorousDataArray;

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

	D3Q19CSParameters() { }
	D3Q19CSParameters(UTextureRenderTargetVolume* uRenderTarget, int* porousDataArray, FIntVector latticeDims)
		: URenderTarget(uRenderTarget)
	{
		PorousDataArray = porousDataArray;
		//_cachedRenderTargetSize = URenderTarget ? FIntPoint(URenderTarget->SizeX, URenderTarget->SizeY) : FIntPoint::ZeroValue;
		_latticeDims = latticeDims;

	}

private:
	//FIntPoint _cachedRenderTargetSize;
	FIntVector _latticeDims;
};