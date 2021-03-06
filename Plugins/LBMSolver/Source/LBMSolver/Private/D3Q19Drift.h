#include "GlobalShader.h"
#include "ShaderParameterStruct.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 32

/// <summary>
/// Internal class thet holds the parameters and connects the HLSL Shader to the engine
/// </summary>
class FD3Q19CSDrift : public FGlobalShader
{
public:
	//Declare this class as a global shader
	DECLARE_GLOBAL_SHADER(FD3Q19CSDrift);
	//Tells the engine that this shader uses a structure for its parameters
	SHADER_USE_PARAMETER_STRUCT(FD3Q19CSDrift, FGlobalShader);
	/// <summary>
	/// DECLARATION OF THE PARAMETER STRUCTURE
	/// The parameters must match the parameters in the HLSL code
	/// For each parameter, provide the C++ type, and the name (Same name used in HLSL code)
	/// </summary>
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_SRV(FRWBufferStructured, PorousData)
		//SHADER_PARAMETER_UAV(FRWBuffer<int>, PorousData)
		//SHADER_PARAMETER_STRUCT(int32, PorousData)
		//SHADER_PARAMETER_ARRAY(int, PorousData, [2048])
		SHADER_PARAMETER_SAMPLER(SamplerState, F_SamplerState)
		SHADER_PARAMETER_TEXTURE(Texture3D<float>, F_in)	// UE4 автоматически формирует SRV.
		//SHADER_PARAMETER_SRV(Texture2D<float>, F_in)
		SHADER_PARAMETER_UAV(RWTexture3D<float>, F_out)
		SHADER_PARAMETER(float, Rho0)
		//SHADER_PARAMETER(float, Tau)
		SHADER_PARAMETER(int, Iteration)
		SHADER_PARAMETER(int, Nx)
		SHADER_PARAMETER(int, Ny)
		SHADER_PARAMETER(int, Nz)
	END_SHADER_PARAMETER_STRUCT()

public:
	//Called by the engine to determine which permutations to compile for this shader
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	//Modifies the compilations environment of the shader
	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		//We're using it here to add some preprocessor defines. That way we don't have to change both C++ and HLSL code when we change the value for NUM_THREADS_PER_GROUP_DIMENSION
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}

};

// This will tell the engine to create the shader and where the shader entry point is.
//                        ShaderType              ShaderPath             Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FD3Q19CSDrift, "/CustomShaders/D3Q19CSDrift.usf", "Main", SF_Compute);