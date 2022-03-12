PRAGMA_DISABLE_OPTIMIZATION

#include "LBMActor3D.h"
#include "AmarettoFileManager.h"
#include "D3Q19SolverInterlayer.h"
#undef UpdateResource

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
	//// �����������:		// TODO: test perfomance
	//PorousBoundariesMeshes->SetCollisionProfileName(FName("NoCollision"), false);
	//PorousBoundariesMeshes->SetCastShadow(false);
	//PorousBoundariesMeshes->SetLightAttachmentsAsGroup(true);
	//PorousBoundariesMeshes->SetRenderCustomDepth(true);

	
	//PorousBoundariesMeshes->NumCustomDataFloats = 1;	// � PerInstanceCustomData[0] �������� ������ �������� ��� ������ � ������� ���������.
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


	// ������������� 3D �������� ��� ������ ������ solver output � ��:
	URenderTarget->OverrideFormat = PF_A32B32G32R32F;//PF_FloatRGB;
	URenderTarget->SizeX = _amaretto->Dims.X;
	URenderTarget->SizeY = _amaretto->Dims.Y;
	URenderTarget->SizeZ = _amaretto->Dims.Z;
	URenderTarget->bCanCreateUAV = true;
	URenderTarget->UpdateResource();

	//DensityRenderTarget->OverrideFormat = PF_R32_FLOAT;//PF_FloatRGB;
	//DensityRenderTarget->SizeX = 64;
	//DensityRenderTarget->SizeY = 64;
	//DensityRenderTarget->SizeZ = 64;
	//DensityRenderTarget->bCanCreateUAV = true;
	//DensityRenderTarget->UpdateResource();


	//// �������: �������� ����������� ������ �� �������� � UVolumeTexture, �.�. � VolumeRenderTargetDataInterface � Niagara ���� ��������.
	////TODO: ������ � ������ �������, ������� "RenderCore" �� ������������ ������ ShaderTest


	//// ������������� CS:
	URenderTarget->WaitForPendingInitOrStreaming();	// ��� ����� GameThread_GetRenderTargetResource()->TextureRHI ������ ���������� NULL.
	//DensityRenderTarget->WaitForPendingInitOrStreaming();
	//FD3Q19CSManager::Get()->InitResources(URenderTarget, DensityRenderTarget, ProbVolText, LatticeDims);
	//FD3Q19CSManager::Get()->BeginRendering();


	_solverInterlayer = new D3Q19SolverInterlayer(URenderTarget, porousDataArray, FIntVector(16, 16, 1));
	_solverInterlayer->Init();


	// TODO: try to use ENQUEUE_RENDER_COMMAND: https://coderoad.ru/59638346/%D0%9A%D0%B0%D0%BA-%D0%B2%D1%8B-%D0%B4%D0%B8%D0%BD%D0%B0%D0%BC%D0%B8%D1%87%D0%B5%D1%81%D0%BA%D0%B8-%D0%BE%D0%B1%D0%BD%D0%BE%D0%B2%D0%BB%D1%8F%D0%B5%D1%82%D0%B5-UTextureRenderTarget2D-%D0%B2-C
	// ���: try to get RHICmdList like here: https://github.com/runedegroot/UE4MarchingCubesGPU/blob/master/Plugins/MarchingCubesComputeShader/Source/MarchingCubesComputeShader/Private/MarchingCubesComputeHelper.cpp
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
	if (iteration > 1000)
	{
		iteration = 0;
		//_solverInterlayer->Init();
		//FGenericPlatformMisc::RequestExit(false);
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Iteration: %i"), iteration));

	////Update parameters
	//FD3Q19CSParameters parameters(URenderTarget, porousDataArray, _amaretto->Dims, DensityRenderTarget);
	//parameters.Iteration = iteration;
	////parameters.DeltaTime = DeltaTime;
	////parameters.DeltaX = (0.000000016) / 64;	// TODO: add logic.
	//FD3Q19CSManager::Get()->UpdateParameters(parameters);

	_solverInterlayer->Step();
}

