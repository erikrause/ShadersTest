#pragma once

#include "CoreMinimal.h"
#include "DigitalRock.h"
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
		class UTextureRenderTargetVolume* PorousRT;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shader)
		FIntVector LatticeDims = FIntVector(64, 64, 64);

	//UPROPERTY(BlueprintReadWrite, Category = Porous)
		UInstancedStaticMeshComponent* PorousBoundariesMeshes;

	UFUNCTION(BlueprintCallable, Category = CFD)
		void InitializeLBMResources();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int Iteration = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UD3Q19SolverInterlayer* SolverInterlayer;

	UFUNCTION(BlueprintCallable, Category = Data)
		void SetDigitalRock(UDigitalRock* digitalRock);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterialInstanceDynamic* MaxIntensityRayCastingMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterialInstanceDynamic* PorousRayCastingMaterial;

	// Sets default values for this pawn's properties
	ALBMActor3D();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool LBMIsRunning = false;

protected:

	//class AmarettoFileManager* _amaretto;

	class UDigitalRock* _digitalRock;

	int _ticksToUpdatePorousTexture = 0;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

	//AmarettoFileManager* _amarettoTest;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};