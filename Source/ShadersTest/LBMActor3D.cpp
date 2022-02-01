PRAGMA_DISABLE_OPTIMIZATION

#include "LBMActor3D.h"
#include "AmarettoFileManager.h"

#include "Kismet/GameplayStatics.h"
#include <Runtime/Engine/Classes/Kismet/KismetRenderingLibrary.h>
#include "D3Q19CSManager.h"
#include <Runtime/Engine/Classes/Engine/TextureRenderTargetVolume.h>
#include <Runtime\Engine\Classes\Engine\VolumeTexture.h>
#include <Runtime/Core/Public/PixelFormat.h>

// Sets default values
ALBMActor3D::ALBMActor3D()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	static_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));

	FString projectDir = FPaths::ProjectDir();
	_amaretto = new AmarettoFileManager(projectDir + FString("/Porous/img, c0=22.5, c=23.4.amaretto"));	//("/Porous/cylinder64.amaretto"));// //cylinder.amaretto")); //XYZtest.amaretto"));
	porousDataArray = _amaretto->GetPorousDataArray();

	PorousBoundariesMeshes = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Porous boundaries"));
	PorousBoundariesMeshes->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	static ConstructorHelpers::FObjectFinder<UMaterial> porousBoundariesMaterialAsset(TEXT("Material'/Game/D3Q19/visualization/Porous/PorousMaterial.PorousMaterial'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> porousBoundariesMeshAsset(TEXT("Material'/Game/D3Q19/visualization/Porous/Cube_1x1x1.Cube_1x1x1'"));
	PorousBoundariesMeshes->SetStaticMesh(porousBoundariesMeshAsset.Object);
	PorousBoundariesMeshes->SetMaterial(0, porousBoundariesMaterialAsset.Object);

	TArray<FIntVector> boundariesCoords = _amaretto->GetBoundariesCoordinates();
	for (FIntVector coord : boundariesCoords)
	{
		PorousBoundariesMeshes->AddInstance(FTransform(FRotator(), (FVector)coord));
	}
	//// Оптимизации:		// TODO: test perfomance
	//PorousBoundariesMeshes->SetCollisionProfileName(FName("NoCollision"), false);
	//PorousBoundariesMeshes->SetCastShadow(false);
	//PorousBoundariesMeshes->SetLightAttachmentsAsGroup(true);
	//PorousBoundariesMeshes->SetRenderCustomDepth(true);

	
	//PorousBoundariesMeshes->NumCustomDataFloats = 1;	// в PerInstanceCustomData[0] хранится индекс инстанса для чтения в шейдере материала.
	//int voxelNum = amaretto->DimX + amaretto->DimY + amaretto->DimZ;
	//for (int i = 0; i < 4; i++)
	//{

	//	PorousBoundariesMeshes->SetCustomDataValue(i, 0, i);
	//}
}

// Called when the game starts or when spawned
void ALBMActor3D::BeginPlay()
{
	Super::BeginPlay();

	// Инициализация 3D текстуры для записи CS output в неё:
	URenderTarget->OverrideFormat = PF_A32B32G32R32F;//PF_FloatRGB;
	URenderTarget->SizeX = 64;
	URenderTarget->SizeY = 64;
	URenderTarget->SizeZ = 64;
	URenderTarget->bCanCreateUAV = true;
	URenderTarget->UpdateResource();

	DensityRenderTarget->OverrideFormat = PF_R32_FLOAT;//PF_FloatRGB;
	DensityRenderTarget->SizeX = 64;
	DensityRenderTarget->SizeY = 64;
	DensityRenderTarget->SizeZ = 64;
	DensityRenderTarget->bCanCreateUAV = true;
	DensityRenderTarget->UpdateResource();


	//// Костыль: пришлось скопировать ссылку на текстуру в UVolumeTexture, т.к. у VolumeRenderTargetDataInterface в Niagara нету сэмплера.
	////TODO: убрать в модуль шейдера, удалить "RenderCore" из зависимостей модуля ShaderTest
	//ProbVolText->TextureReference = URenderTarget->TextureReference;
	//ProbVolText->Resource = URenderTarget->Resource;



	// Инициализация CS:
	URenderTarget->WaitForPendingInitOrStreaming();	// без этого GameThread_GetRenderTargetResource()->TextureRHI иногда возвращает NULL.
	DensityRenderTarget->WaitForPendingInitOrStreaming();
	FD3Q19CSManager::Get()->InitResources(URenderTarget, DensityRenderTarget, ProbVolText, LatticeDims);
	FD3Q19CSManager::Get()->BeginRendering();


	// TODO: try to use ENQUEUE_RENDER_COMMAND: https://coderoad.ru/59638346/%D0%9A%D0%B0%D0%BA-%D0%B2%D1%8B-%D0%B4%D0%B8%D0%BD%D0%B0%D0%BC%D0%B8%D1%87%D0%B5%D1%81%D0%BA%D0%B8-%D0%BE%D0%B1%D0%BD%D0%BE%D0%B2%D0%BB%D1%8F%D0%B5%D1%82%D0%B5-UTextureRenderTarget2D-%D0%B2-C
	// ИЛИ: try to get RHICmdList like here: https://github.com/runedegroot/UE4MarchingCubesGPU/blob/master/Plugins/MarchingCubesComputeShader/Source/MarchingCubesComputeShader/Private/MarchingCubesComputeHelper.cpp
	//RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, 11000, 512);	// try InitCustomFormat().
}

void ALBMActor3D::BeginDestroy()
{
	FD3Q19CSManager::Get()->EndRendering();
	Super::BeginDestroy();
}

// Called every frame
void ALBMActor3D::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	iteration++;
	if (iteration > 2000)
	{
		iteration = 0;
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Iteration: %i"), iteration));

	//Update parameters
	FD3Q19CSParameters parameters(URenderTarget, porousDataArray, _amaretto->Dims, DensityRenderTarget);
	parameters.Iteration = iteration;
	//parameters.DeltaTime = DeltaTime;
	//parameters.DeltaX = (0.000000016) / 64;	// TODO: add logic.
	FD3Q19CSManager::Get()->UpdateParameters(parameters);

	//if (URenderTarget != NULL)
	//{
	//	if (URenderTarget->GameThread_GetRenderTargetResource()->ReadLinearColorPixels(uBuffer))
	//	{
	//		int prob = 1;
	//	}
	//}

	//if (PosRenderTarget != NULL)
	//{
	//	if (PosRenderTarget->GameThread_GetRenderTargetResource()->ReadLinearColorPixels(posBuffer))
	//	{
	//		int Nx = 400;
	//		int x = 0;
	//		int y = 0;
	//		TArray<float> vels;

	//		for (x = 0; x < 6; x++)
	//		{
	//			for (int i = 0; i < 9; i++)
	//			{
	//				int id = x + y * 9 * Nx + i * Nx;
	//				vels.Add(posBuffer[id].R);
	//			}
	//		}

	//		TArray<float> vels2;

	//		for (x = 390; x < Nx; x++)
	//		{
	//			for (int i = 0; i < 9; i++)
	//			{
	//				int id = x + y * 9 * Nx + i * Nx;
	//				vels2.Add(posBuffer[id].R);
	//			}
	//		}

	//		int prob2 = 0;
	//	}
	//}
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

