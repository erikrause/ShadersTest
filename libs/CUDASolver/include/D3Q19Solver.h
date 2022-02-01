#pragma once

#include <d3d11.h>
#include <vector_types.h>
#include <driver_types.h>

namespace LBM
{
	class D3Q19Solver
	{
	public:

		D3Q19Solver(ID3D11Texture3D* velocityTexture1, ID3D11Texture3D* velocityTexture2, int* porousMedia, dim3 resolution, dim3 blockSize);
		~D3Q19Solver();
		void Step();
		static void AddTest(int const* const a, int const* const b, int* const c, const int size);

	protected:

		cudaGraphicsResource_t _cudaResource1;
		cudaGraphicsResource_t _cudaResource2;
		ID3D11Texture3D* _velocityTexture1;
		ID3D11Texture3D* _velocityTexture2;
		dim3 _blockSize;
		dim3 _gridSize;
		int* _devPorousMedia;
	};
}