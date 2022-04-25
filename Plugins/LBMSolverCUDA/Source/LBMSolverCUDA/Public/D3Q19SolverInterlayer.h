#pragma once

#include "CoreMinimal.h"
//#include <driver_types.h>
#include <Runtime/Engine/Classes/Engine/TextureRenderTargetVolume.h>
#include <GraphicsSolver.h>
#include "UnrealCUDALogger.h"
#include <VolumeInfoDTO.h>
#include "D3Q19SolverInterlayer.generated.h"

UCLASS(BlueprintType)
class LBMSOLVERCUDA_API UD3Q19SolverInterlayer : public UObject
{
	GENERATED_BODY()

public:
	UD3Q19SolverInterlayer();
	void Init(UTextureRenderTargetVolume* velocityTexture, UTextureRenderTargetVolume* densityTexture, int* porousMedia, FIntVector blockSize);
	~UD3Q19SolverInterlayer();
	void Step();
	FVolumeInfoDTO GetVelocityVolumeInfo();
	FVolumeInfoDTO GetDensityVolumeInfo();
	FVolumeInfoDTO GetPorousVolumeInfo();

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