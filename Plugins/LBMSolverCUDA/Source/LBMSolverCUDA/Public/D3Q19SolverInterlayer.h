#pragma once

#include "CoreMinimal.h"
//#include <driver_types.h>
#include <Runtime/Engine/Classes/Engine/TextureRenderTargetVolume.h>
#include <GraphicsSolver.h>
#include "UnrealCUDALogger.h"

class LBMSOLVERCUDA_API D3Q19SolverInterlayer
{
public:
	D3Q19SolverInterlayer(UTextureRenderTargetVolume* velocityTexture, UTextureRenderTargetVolume* densityTexture, int* porousMedia, FIntVector blockSize);
	~D3Q19SolverInterlayer();
	void Step();
	void Init();
	VolumeInfo GetVelocityVolumeInfo() { return _cudaSolver->VelocityVolumeInfo; }
	VolumeInfo GetDensityVolumeInfo() { return _cudaSolver->DensityVolumeInfo; }
	VolumeInfo GetPorousVolumeInfo() { return _cudaSolver->PorousVolumeInfo; }

protected:
	//ID3D11Texture3D* _d3d11Texture;
	//cudaGraphicsResource_t _cudaResource;
	//int* _devPorousMedia;
	//dim3 _blockSize;
	//dim3 _gridSize;
	CFD::GraphicsSolver* _cudaSolver;
	UnrealCUDALogger* _logger;
	cudaGraphicsResource_t _velocityResource;
	cudaGraphicsResource_t _densityResource;
};