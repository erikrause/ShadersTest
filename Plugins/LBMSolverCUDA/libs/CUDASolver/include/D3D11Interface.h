#pragma once

#include <d3d11.h>
#include <vector_types.h>
#include <driver_types.h>
#include "ILogger.h"
#include "CUDAErrorChecker.h"
#include <cuda_d3d11_interop.h>

namespace CFD
{
	class D3D11Interface
	{
	public:

		D3D11Interface(ID3D11Texture3D* velocityTexture, const int* porousMedia, const dim3 resolution, const dim3 blockSize, ILogger* logger);
		~D3D11Interface();
		void Step();

	protected:

		cudaGraphicsResource_t _velocityResource;
		//cudaGraphicsResource_t _cudaResource2;
		ID3D11Texture3D* _velocityTexture;
		//ID3D11Texture3D* _velocityTexture2;
		dim3 _cudaBlockSize;
		dim3 _cudaGridSize;
		dim3 _resolution;
		int* _devPorousMedia;
		ILogger* _logger;
		CUDAErrorChecker* _cudaErrorChecker;
		cudaArray_t _latticeGridIn;
		cudaArray_t _latticeGridOut;
		cudaSurfaceObject_t _latticeGridInSurf = 0;
		cudaSurfaceObject_t _latticeGridOutSurf = 0;

		

		int _iteration = 0;
	};
}