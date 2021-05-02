PRAGMA_DISABLE_OPTIMIZATION

#include "LBMActor.h"
#include "Amaretto.h"

#include "Kismet/GameplayStatics.h"
#include <Runtime/Engine/Classes/Kismet/KismetRenderingLibrary.h>
#include "CustomShadersDeclarations/D2Q9CSManager.h"

// Sets default values
ALBMActor::ALBMActor()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	static_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	

	Amaretto* amarettoTest = new Amaretto(FString("C:\\img, c0=22.5, c=23.4.amaretto"));
	porousDataArray = amarettoTest->GetPorousDataArray();

	int prob2 = 0;
}

// Called when the game starts or when spawned
void ALBMActor::BeginPlay()
{
	Super::BeginPlay();
	FD2Q9CSManager::Get()->BeginRendering();

	// TODO: try to use ENQUEUE_RENDER_COMMAND: https://coderoad.ru/59638346/%D0%9A%D0%B0%D0%BA-%D0%B2%D1%8B-%D0%B4%D0%B8%D0%BD%D0%B0%D0%BC%D0%B8%D1%87%D0%B5%D1%81%D0%BA%D0%B8-%D0%BE%D0%B1%D0%BD%D0%BE%D0%B2%D0%BB%D1%8F%D0%B5%D1%82%D0%B5-UTextureRenderTarget2D-%D0%B2-C
	// ÈËÈ: try to get RHICmdList like here: https://github.com/runedegroot/UE4MarchingCubesGPU/blob/master/Plugins/MarchingCubesComputeShader/Source/MarchingCubesComputeShader/Private/MarchingCubesComputeHelper.cpp
	//RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, 11000, 512);	// try InitCustomFormat().

	//Assuming that the static mesh is already using the material that we're targeting, we create an instance and assign it to it
	UMaterialInstanceDynamic* MID = static_mesh->CreateAndSetMaterialInstanceDynamic(0);
	MID->SetTextureParameterValue("InputTexture", (UTexture*)FRenderTarget);
	//auto prob = (*RenderTarget).get;

	textureResource = (FTextureRenderTarget2DResource*)FRenderTarget->Resource;
}

void ALBMActor::BeginDestroy()
{
	FD2Q9CSManager::Get()->EndRendering();
	Super::BeginDestroy();
}

// Called every frame
void ALBMActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	iteration++;
	if (iteration > 1000)
	{
		iteration = 0;
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Iteration: %i"), iteration));

	//Update parameters
	D2Q9CSParameters parameters(FRenderTarget, URenderTarget, porousDataArray);
	parameters.Iteration = iteration;
	FD2Q9CSManager::Get()->UpdateParameters(parameters);
	auto probTex = FRenderTarget->GameThread_GetRenderTargetResource();
	//probTex->

	if (FRenderTarget != NULL)
	{
		TArray<FLinearColor> colorBuffer;
		if (textureResource->ReadLinearColorPixels(colorBuffer))
		{
			int Nx = 400;
			int x = 0;
			int y = 0;
			TArray<float> vels;

			for (x = 0; x < 6; x++)
			{
				for (int i = 0; i < 9; i++)
				{
					int id = x + y * 9 * Nx + i * Nx;
					vels.Add(colorBuffer[id].R);
				}
			}

			TArray<float> vels2;

			for (x = 390; x < Nx; x++)
			{
				for (int i = 0; i < 9; i++)
				{
					int id = x + y * 9 * Nx + i * Nx;
					vels2.Add(colorBuffer[id].R);
				}
			}

			int prob2 = 0;
		}
	}
		//// ->GetRenderTargetResource();
		//TArray<FFloat16Color> ColorBuffer16;
		//if (textureResource->ReadFloat16Pixels(ColorBuffer16))
		//{
		//	//float x = ColorBuffer16[0].R.GetFloat();
		//	//float y = ColorBuffer16[0].G.GetFloat();
		//	//float z = ColorBuffer16[0].B.GetFloat();

		//	float x1 = ColorBuffer16[51].R.GetFloat();
		//	float x2 = ColorBuffer16[1].R.GetFloat();
		//	float x3 = ColorBuffer16[400].R.GetFloat();
		//	float x4 = ColorBuffer16[401].R.GetFloat();
		//	float x5 = ColorBuffer16[451].R.GetFloat();

		//	TArray<float> arr;
		//	for (int i = 0; i < 1600; i++)
		//	{
		//		arr.Add(ColorBuffer16[i].R.GetFloat());
		//	}
		//	TArray<float> arr2;
		//	for (int i = 1500; i < 2400; i++)
		//	{
		//		arr2.Add(ColorBuffer16[i].R.GetFloat());
		//	}
		//	int prob = 0;
		//	//float x1 = ColorBuffer16[1].R.GetFloat();
		//	//float x1 = ColorBuffer16[1].R.GetFloat();

		//	int Nx = 400;
		//	int x = 0;
		//	int y = 0;
		//	TArray<float> vels;

		//	for (x = 0; x < 13; x++)
		//	{
		//		for (int i = 0; i < 9; i++)
		//		{
		//			int id = x + y * 9 * Nx + i * Nx;
		//			vels.Add(ColorBuffer16[id].R);
		//		}
		//	}

		//	TArray<float> vels2;

		//	for (x = 390; x < Nx; x++)
		//	{
		//		for (int i = 0; i < 9; i++)
		//		{
		//			int id = x + y * 9 * Nx + i * Nx;
		//			vels2.Add(ColorBuffer16[id].R);
		//		}
		//	}

		//	int prob2 = 0;
		//}
	//}
}

