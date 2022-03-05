#include "D3Q19SolverInterlayer.h"

//#include <Runtime/Windows/D3D11RHI/Private/D3D11RHIPrivate.h>
//#include "Runtime\Windows\D3D11RHI\Private\D3D11StateCachePrivate.h"
#include "Runtime\Windows\D3D11RHI\Public\D3D11State.h"
#include <Runtime\Windows\D3D11RHI\Public\D3D11Resources.h>
//#include <cuda_d3d11_interop.h>
//#include <helper_cuda.h>
#include <d3d11.h>


D3Q19SolverInterlayer::D3Q19SolverInterlayer(UTextureRenderTargetVolume* velocityTexture, int* porousMedia, FIntVector blockSize)
{
	//_blockSize = blockSize;
	//_gridSize = ((VelocityTexture->SizeX + blockSize.x - 1) / blockSize.x, (VelocityTexture->SizeY + blockSize.y - 1) / blockSize.y, 1);


	//_d3d11Texture = (ID3D11Texture3D*)GetD3D11TextureFromRHITexture(VelocityTexture->GameThread_GetRenderTargetResource()->TextureRHI)->GetResource();
	//cudaGraphicsD3D11RegisterResource(&_cudaResource, _d3d11Texture, cudaGraphicsRegisterFlagsNone);
	//getLastCudaError("cudaGraphicsD3D11RegisterResource (_texture) failed");


	//unsigned int porousMediaSize = VelocityTexture->SizeX * VelocityTexture->SizeY * VelocityTexture->SizeZ;
	//cudaMalloc((void**)&_devPorousMedia, porousMediaSize * sizeof(*porousMedia));
	//cudaMemcpy(_devPorousMedia, porousMedia, porousMediaSize * sizeof(*porousMedia), cudaMemcpyHostToDevice);



	//TRefCountPtr<ID3D11Device> D3D11Device;

	//ENQUEUE_RENDER_COMMAND(Test)(
	//	[&D3D11Device](FRHICommandListImmediate& RHICmdList)
	//	{
	//		D3D11Device = (ID3D11Device*)RHIGetNativeDevice();
	//	});


	//dim3 resolution = dim3(64, 64, 64);
	//// Texture create
	//{
	//	D3D11_TEXTURE3D_DESC desc;
	//	ZeroMemory(&desc, sizeof(D3D11_TEXTURE3D_DESC));
	//	desc.Width = resolution.x;
	//	desc.Height = resolution.y;
	//	desc.Depth = resolution.z;
	//	desc.MipLevels = 1;
	//	desc.Format = DXGI_FORMAT_R32_FLOAT;
	//	desc.Usage = D3D11_USAGE_DEFAULT;
	//	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;


	//	float* rawData = new float[resolution.x * resolution .y * resolution.z];

	//	for (unsigned int i = 0; i < resolution.x * resolution.y * resolution.z; i++)
	//	{
	//		rawData[i] = 1.0;
	//	}

	//	D3D11_SUBRESOURCE_DATA resData;
	//	resData.pSysMem = rawData;
	//	resData.SysMemPitch = resolution.x * sizeof(float);
	//	resData.SysMemSlicePitch = resolution.x * resolution.y * sizeof(float);

	//	D3D11Device->CreateTexture3D(&desc, &resData, &_texture);
	//}

	////int* porousMedia = new int[resolution.x *resolution.y * resolution.z];
	//dim3 blockSizeDims = dim3(blockSize.X, blockSize.Y, blockSize.Z);
	//_logger = new UnrealCUDALogger();
	//_cudaSolver = new LBM::D3Q19Solver(_texture, porousMedia, resolution, blockSizeDims, _logger);

	//for (int i = 0; i < 100; i++)
	//{
	//	_cudaSolver->Step();
	//}

	//// _texture?
	dim3 blockSizeDims = dim3(blockSize.X, blockSize.Y, blockSize.Z);
	_texture = (ID3D11Texture3D*)GetD3D11TextureFromRHITexture(velocityTexture->GameThread_GetRenderTargetResource()->TextureRHI)->GetResource();
	_textureUE = velocityTexture;
	dim3 resolution = dim3(velocityTexture->SizeX, velocityTexture->SizeY, velocityTexture->SizeZ);
	_logger = new UnrealCUDALogger();
	_cudaSolver = new CFD::D3D11Interface(_texture, porousMedia, resolution, blockSizeDims, _logger);
}

D3Q19SolverInterlayer::~D3Q19SolverInterlayer()
{
	delete _cudaSolver;
	delete _logger;
}

void D3Q19SolverInterlayer::Step()
{
	//ID3D11Texture3D* temp = (ID3D11Texture3D*)GetD3D11TextureFromRHITexture(_textureUE->GameThread_GetRenderTargetResource()->TextureRHI)->GetResource();

	ENQUEUE_RENDER_COMMAND(CudaStep)([this]
		(FRHICommandListImmediate& RHICmdList)
		{
			_cudaSolver->Step();
		});	
}

void D3Q19SolverInterlayer::Init()
{
	_cudaSolver->Init();
}
