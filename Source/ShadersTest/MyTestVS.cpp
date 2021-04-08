//#include "MyTestVS.h"
//
//#include "GlobalShader.h"
////#include "ShaderParameterStruct.h"
////#include "RenderGraphUtils.h"
////#include "RenderTargetPool.h"
//#include "RHIDefinitions.h"
//
//
//// This can go on a header or cpp file
//class FMyTestVS : public FGlobalShader
//{
//	DECLARE_EXPORTED_SHADER_TYPE(FMyTestVS, Global, /*MYMODULE_API*/);
//	//DECLARE_GLOBAL_SHADER(FMyTestVS);
//
//	FMyTestVS() { }
//	FMyTestVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
//		: FGlobalShader(Initializer)
//	{
//	}
//
//	//static bool ShouldCache(EShaderPlatform Platform)
//	//{
//	//	return true;
//	//}
//
//
//	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
//	{
//		// Useful when adding a permutation of a particular shader
//		return true;
//	}
//};
//
//// This needs to go on a cpp file
//
////IMPLEMENT_SHADER_TYPE(, FMyTestVS, "/Engine/Private/MyTest.usf", "MainVS", SF_Vertex);
//IMPLEMENT_GLOBAL_SHADER(FMyTestVS, "/Engine/Private/MyTest.usf", "MainVS", SF_Vertex);

PRAGMA_DISABLE_OPTIMIZATION

#include "MyTestVS.h"

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "Misc/Paths.h"

#include "Modules/ModuleManager.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 32
//
/// <summary>
/// Internal class thet holds the parameters and connects the HLSL Shader to the engine
/// </summary>
class FWhiteNoiseCS : public FGlobalShader
{
public:
	//Declare this class as a global shader
	DECLARE_GLOBAL_SHADER(FWhiteNoiseCS);
	//Tells the engine that this shader uses a structure for its parameters
	SHADER_USE_PARAMETER_STRUCT(FWhiteNoiseCS, FGlobalShader);
	/// <summary>
	/// DECLARATION OF THE PARAMETER STRUCTURE
	/// The parameters must match the parameters in the HLSL code
	/// For each parameter, provide the C++ type, and the name (Same name used in HLSL code)
	/// </summary>
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWTexture2D<float>, OutputTexture)
		SHADER_PARAMETER(FVector2D, Dimensions)
		SHADER_PARAMETER(UINT, TimeStamp)
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

 //This will tell the engine to create the shader and where the shader entry point is.
 //                       ShaderType              ShaderPath             Shader function name    Type
//IMPLEMENT_GLOBAL_SHADER(FWhiteNoiseCS, "/CustomShaders", "MainComputeShader", SF_Compute);