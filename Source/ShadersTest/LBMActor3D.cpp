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
#include "DRFileManager.h"

// Sets default values
ALBMActor3D::ALBMActor3D()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	//FString projectDir = FPaths::ProjectDir();
	//_amaretto = new AmarettoFileManager(projectDir + FString("/Porous/Berea200.amaretto"));//img, c0=22.5, c=23.4.amaretto"));	//("/Porous/cylinder64.amaretto"));// //cylinder.amaretto")); //XYZtest.amaretto"));
	//porousDataArray = _amaretto->GetPorousDataArray();

	//PorousBoundariesMeshes = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Porous boundaries"));
	//PorousBoundariesMeshes->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	//static ConstructorHelpers::FObjectFinder<UMaterial> porousBoundariesMaterialAsset(TEXT("Material'/Game/D3Q19/visualization/Porous/PorousMaterial.PorousMaterial'"));
	//static ConstructorHelpers::FObjectFinder<UStaticMesh> porousBoundariesMeshAsset(TEXT("Material'/Game/D3Q19/visualization/Porous/Cube_1x1x1.Cube_1x1x1'"));
	//PorousBoundariesMeshes->SetStaticMesh(porousBoundariesMeshAsset.Object);
	//PorousBoundariesMeshes->SetMaterial(0, porousBoundariesMaterialAsset.Object);

	//TArray<FIntVector> boundariesCoords = _amaretto->GetBoundariesCoordinates(255);
	//for (FIntVector coord : boundariesCoords)
	//{
	//	PorousBoundariesMeshes->AddInstance(FTransform(FRotator(), (FVector)coord));
	//}
	//// ???????????:		// TODO: test perfomance
	//PorousBoundariesMeshes->SetCollisionProfileName(FName("NoCollision"), false);
	//PorousBoundariesMeshes->SetCastShadow(false);
	//PorousBoundariesMeshes->SetLightAttachmentsAsGroup(true);
	//PorousBoundariesMeshes->SetRenderCustomDepth(true);

	
	//PorousBoundariesMeshes->NumCustomDataFloats = 1;	// ? PerInstanceCustomData[0] ???????? ?????? ???????? ??? ?????? ? ??????? ?????????.
	//int voxelNum = amaretto->DimX + amaretto->DimY + amaretto->DimZ;
	//for (int i = 0; i < 4; i++)
	//{

	//	PorousBoundariesMeshes->SetCustomDataValue(i, 0, i);
	//}
	SolverInterlayer = CreateDefaultSubobject<UD3Q19SolverInterlayer>(TEXT("Solver"));
}

void ALBMActor3D::InitializeLBMResources()
{
	uint32 sizeX = _digitalRock->GetSize().X;
	uint32 sizeY = _digitalRock->GetSize().Y;
	uint32 sizeZ = _digitalRock->GetSize().Z;

	// ????????????? 3D ???????? ??? ?????? ?????? solver output ? ???:
	VelocityRT->OverrideFormat = PF_A32B32G32R32F;//PF_FloatRGB;
	VelocityRT->SizeX = sizeX;
	VelocityRT->SizeY = sizeY;
	VelocityRT->SizeZ = sizeZ;
	VelocityRT->bCanCreateUAV = true;
	VelocityRT->UpdateResource();

	DensityRT->Init(sizeX, sizeY, sizeZ, PF_R32_FLOAT);
	PorousRT->Init(sizeX, sizeY, sizeZ, PF_R32_FLOAT);

	//// ???????: ???????? ??????????? ?????? ?? ???????? ? UVolumeTexture, ?.?. ? VolumeRenderTargetDataInterface ? Niagara ???? ????????.
	////TODO: ?????? ? ?????? ???????, ??????? "RenderCore" ?? ???????????? ?????? ShaderTest

	//// ????????????? CS:
	VelocityRT->WaitForPendingInitOrStreaming();	// ??? ????? GameThread_GetRenderTargetResource()->TextureRHI ?????? ?????????? NULL.
	DensityRT->WaitForPendingInitOrStreaming();
	PorousRT->WaitForPendingInitOrStreaming();
	//DensityRT->WaitForPendingInitOrStreaming();
	//FD3Q19CSManager::Get()->InitResources(VelocityRT, DensityRT, ProbVolText, LatticeDims);
	//FD3Q19CSManager::Get()->BeginRendering();

	//SolverInterlayer = NewObject<UD3Q19SolverInterlayer>();

	MaxIntensityRayCastingMaterial->SetVectorParameterValue("Resolution", FLinearColor(sizeX, sizeY, sizeZ));
	PorousRayCastingMaterial->SetVectorParameterValue("Resolution", FLinearColor(sizeX, sizeY, sizeZ));

	Iteration = 0;
}

void ALBMActor3D::SetDigitalRock(UDigitalRock* digitalRock)
{
	_digitalRock = digitalRock;
	InitializeLBMResources();
	SolverInterlayer->Init(VelocityRT, DensityRT, _digitalRock->GetPorousAsInt(), FIntVector(16, 16, 1));
	_ticksToUpdatePorousTexture = 2;
}

// Called when the game starts or when spawned
void ALBMActor3D::BeginPlay()
{
	Super::BeginPlay();

	// TODO: try to use ENQUEUE_RENDER_COMMAND: https://coderoad.ru/59638346/%D0%9A%D0%B0%D0%BA-%D0%B2%D1%8B-%D0%B4%D0%B8%D0%BD%D0%B0%D0%BC%D0%B8%D1%87%D0%B5%D1%81%D0%BA%D0%B8-%D0%BE%D0%B1%D0%BD%D0%BE%D0%B2%D0%BB%D1%8F%D0%B5%D1%82%D0%B5-UTextureRenderTarget2D-%D0%B2-C
	// ???: try to get RHICmdList like here: https://github.com/runedegroot/UE4MarchingCubesGPU/blob/master/Plugins/MarchingCubesComputeShader/Source/MarchingCubesComputeShader/Private/MarchingCubesComputeHelper.cpp
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
	
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Iteration: %i"), Iteration));

	if (_ticksToUpdatePorousTexture > 0)
	{
		_ticksToUpdatePorousTexture--;

		if (_ticksToUpdatePorousTexture == 0)
		{
			ENQUEUE_RENDER_COMMAND(InitPorousTexture)([this]
			(FRHICommandListImmediate& RHICmdList)
				{
					if (PorousRT->GetRenderTargetResource() != NULL)
					{
						FTexture3DRHIRef PorousRTRHI = PorousRT->GetRenderTargetResource()->TextureRHI.GetReference()->GetTexture3D();
						const FUpdateTextureRegion3D updateRegion(FIntVector::ZeroValue, FIntVector::ZeroValue, _digitalRock->GetSize());
						RHIUpdateTexture3D(PorousRTRHI, 0, updateRegion, _digitalRock->GetSize().X * 4, _digitalRock->GetSize().X * _digitalRock->GetSize().Y * 4, (uint8*)_digitalRock->GetPorousAsFloat());
					}
				});
		}
	}


	if (LBMIsRunning)
	{
		Iteration++;
		//TArray<FColor> rawData;
		//FTexture2DMipMap& Mip = VelocityRT->PlatformData->Mips[0];
		//void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
		//FMemory::Memcpy(Data, rawData.GetData(), (64 * 64 * 64 * 4));
		//Mip.BulkData.Unlock();

		////Update parameters
		//FD3Q19CSParameters parameters(VelocityRT, porousDataArray, _amaretto->Dims, DensityRT);
		//parameters.Iteration = Iteration;
		////parameters.DeltaTime = DeltaTime;
		////parameters.DeltaX = (0.000000016) / 64;	// TODO: add logic.
		//FD3Q19CSManager::Get()->UpdateParameters(parameters);

		SolverInterlayer->Step();
	}




	//int totalCellsNum = _amaretto->Dims.X * _amaretto->Dims.Y * _amaretto->Dims.Z;
	//int solidCellsNum = SolverInterlayer->GetPorousVolumeInfo().Sum / SolverInterlayer->GetPorousVolumeInfo().Max;
	//int fluidCellsNum = totalCellsNum - solidCellsNum;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Min velocity: %.4f"), SolverInterlayer->GetVelocityVolumeInfo().Min));
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Max velocity: %.4f"), SolverInterlayer->GetVelocityVolumeInfo().Max));
	////GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Avg velocity: %.2f"), SolverInterlayer->GetVelocityVolumeInfo().Sum / (_amaretto->Dims.X * _amaretto->Dims.Y * _amaretto->Dims.Z)));
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Avg velocity: %.4f"), SolverInterlayer->GetVelocityVolumeInfo().Sum / fluidCellsNum));
	////GEngine->ClearOnScreenDebugMessages();
}

