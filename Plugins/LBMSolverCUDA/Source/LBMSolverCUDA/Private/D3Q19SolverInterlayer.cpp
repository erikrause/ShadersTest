#include "D3Q19SolverInterlayer.h"

//#include <Runtime/Windows/D3D11RHI/Private/D3D11RHIPrivate.h>
//#include "Runtime\Windows\D3D11RHI\Private\D3D11StateCachePrivate.h"
#include "Runtime\Windows\D3D11RHI\Public\D3D11State.h"
#include <Runtime\Windows\D3D11RHI\Public\D3D11Resources.h>
#include <cuda_d3d11_interop.h>
//#include <helper_cuda.h>
#include <d3d11.h>

UD3Q19SolverInterlayer::UD3Q19SolverInterlayer() 
{

}

UD3Q19SolverInterlayer::~UD3Q19SolverInterlayer()
{
	delete _cudaSolver;
	delete _logger;
}

void UD3Q19SolverInterlayer::Step()
{
	//ID3D11Texture3D* temp = (ID3D11Texture3D*)GetD3D11TextureFromRHITexture(_textureUE->GameThread_GetRenderTargetResource()->TextureRHI)->GetResource();

	ENQUEUE_RENDER_COMMAND(CudaStep)([this]
		(FRHICommandListImmediate& RHICmdList)
		{
			_cudaSolver->Step();
		});	
}

FVolumeInfoDTO UD3Q19SolverInterlayer::GetVelocityVolumeInfo()
{
	VolumeInfo volumeInfo;

	ENQUEUE_RENDER_COMMAND(CudaStep)([this, &volumeInfo]
	(FRHICommandListImmediate& RHICmdList)
		{
			volumeInfo = _cudaSolver->VelocityVolumeInfo;
		});

	return FVolumeInfoDTO(volumeInfo.Min, volumeInfo.Max, volumeInfo.Sum);
}

FVolumeInfoDTO UD3Q19SolverInterlayer::GetDensityVolumeInfo()
{
	VolumeInfo volumeInfo;

	ENQUEUE_RENDER_COMMAND(CudaStep)([this, &volumeInfo]
	(FRHICommandListImmediate& RHICmdList)
		{
			volumeInfo = _cudaSolver->DensityVolumeInfo;
		});

	return FVolumeInfoDTO(volumeInfo.Min, volumeInfo.Max, volumeInfo.Sum);
}

FVolumeInfoDTO UD3Q19SolverInterlayer::GetPorousVolumeInfo()
{
	VolumeInfo volumeInfo;

	ENQUEUE_RENDER_COMMAND(CudaStep)([this, &volumeInfo]
	(FRHICommandListImmediate& RHICmdList)
		{
			volumeInfo = _cudaSolver->PorousVolumeInfo;
		});

	return FVolumeInfoDTO(volumeInfo.Min, volumeInfo.Max, volumeInfo.Sum);
}

void UD3Q19SolverInterlayer::Init(UTextureRenderTargetVolume* velocityTexture, UTextureRenderTargetVolume* densityTexture, int* porousMedia, FIntVector blockSize)
{
	ENQUEUE_RENDER_COMMAND(CudaStep)([this, velocityTexture, densityTexture, porousMedia, blockSize]
	(FRHICommandListImmediate& RHICmdList)
		{
			check(velocityTexture->SizeX == densityTexture->SizeX &&
				velocityTexture->SizeY == densityTexture->SizeY &&
				velocityTexture->SizeZ == densityTexture->SizeZ)

			dim3 blockSizeDims = dim3(blockSize.X, blockSize.Y, blockSize.Z);
			ID3D11Texture3D* velocityD3D11Texture = (ID3D11Texture3D*)GetD3D11TextureFromRHITexture(velocityTexture->GetRenderTargetResource()->TextureRHI)->GetResource();	// TODO: сделать проверку на текущий API.
			ID3D11Texture3D* densityD3D11Texture = (ID3D11Texture3D*)GetD3D11TextureFromRHITexture(densityTexture->GetRenderTargetResource()->TextureRHI)->GetResource();
			dim3 resolution = dim3(velocityTexture->SizeX, velocityTexture->SizeY, velocityTexture->SizeZ);
			_logger = new UnrealCUDALogger();

			cudaGraphicsD3D11RegisterResource(&_velocityResource, velocityD3D11Texture, cudaGraphicsRegisterFlagsNone);
			cudaGraphicsD3D11RegisterResource(&_densityResource, densityD3D11Texture, cudaGraphicsRegisterFlagsNone);
			_cudaSolver = new CFD::GraphicsSolver(_velocityResource, _densityResource, porousMedia, resolution, blockSizeDims, _logger);

			_cudaSolver->Init();
		});
}
