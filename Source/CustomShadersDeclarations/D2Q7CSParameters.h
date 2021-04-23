#pragma once

#include "CoreMinimal.h"
//#include "D2Q7CSParameters.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

//This struct act as a container for all the parameters that the client needs to pass to the Compute Shader Manager.
struct D2Q7CSParameters
{
	// �������� ��� �������� ������������� ������ � �����.
	UTextureRenderTarget2D* FRenderTarget;

	// �������� ��� �������� ��������� � �����.
	UTextureRenderTarget2D* URenderTarget;

	// Test.
	int IsInit = 1;

	/// <summary>
	/// ������� ���������� ��������.
	/// </summary>
	/// <param name="isOnlyXYDims"> ������� XY - ���������� ����� �����. </param>
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
		{
			return CachedRenderTargetSize;
		}
	}

	D2Q7CSParameters() { }
	D2Q7CSParameters(UTextureRenderTarget2D* IORenderTarget, UTextureRenderTarget2D* uRenderTarget)
		: FRenderTarget(IORenderTarget), URenderTarget(uRenderTarget)
	{
		CachedRenderTargetSize = FRenderTarget ? FIntPoint(FRenderTarget->SizeX, FRenderTarget->SizeY) : FIntPoint::ZeroValue;
	}

private:
	FIntPoint CachedRenderTargetSize;
};