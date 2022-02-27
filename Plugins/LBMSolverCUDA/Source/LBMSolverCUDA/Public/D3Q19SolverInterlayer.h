#pragma once

#include "CoreMinimal.h"
//#include <driver_types.h>
#include <Runtime/Engine/Classes/Engine/TextureRenderTargetVolume.h>
#include "D3D11Interface.h"
#include "UnrealCUDALogger.h"

class LBMSOLVERCUDA_API D3Q19SolverInterlayer
{
public:
	D3Q19SolverInterlayer(UTextureRenderTargetVolume* velocityTexture, int* porousMedia, FIntVector blockSize);
	~D3Q19SolverInterlayer();
	void Step();

protected:
	//ID3D11Texture3D* _d3d11Texture;
	//cudaGraphicsResource_t _cudaResource;
	//int* _devPorousMedia;
	//dim3 _blockSize;
	//dim3 _gridSize;
	CFD::D3D11Interface* _cudaSolver;
	UnrealCUDALogger* _logger;
	ID3D11Texture3D* _texture;
	UTextureRenderTargetVolume* _textureUE;
};