#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LBMActor3D.generated.h"

UCLASS()
class SHADERSTEST_API ALBMActor3D : public AActor
{
	GENERATED_BODY()

//Properties
public:
	UPROPERTY()
		USceneComponent* Root;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shader)
		class UTextureRenderTargetVolume* VelocityRT;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shader)
		class UTextureRenderTargetVolume* DensityRT;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shader)
		FIntVector LatticeDims = FIntVector(64, 64, 64);

	//UPROPERTY(BlueprintReadWrite, Category = Porous)
		UInstancedStaticMeshComponent* PorousBoundariesMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int iteration = -1;

public:
	// Sets default values for this pawn's properties
	ALBMActor3D();


protected:

	class AmarettoFileManager* _amaretto;
	class D3Q19SolverInterlayer* _solverInterlayer;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

	//AmarettoFileManager* _amarettoTest;
	int* porousDataArray;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};