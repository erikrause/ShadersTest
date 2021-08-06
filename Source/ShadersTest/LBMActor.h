#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LBMActor.generated.h"

UCLASS()
class SHADERSTEST_API ALBMActor : public APawn
{
	GENERATED_BODY()

//Properties
public:
	UPROPERTY()
		USceneComponent* Root;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* static_mesh;	// Временно, для визуализации по срезам.

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shader)
	//	class UTextureRenderTarget2D* FRenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LBM)
		class UTextureRenderTargetVolume* URenderTarget;

	UPROPERTY(EditAnywhere, Category = LBM)
		int MaxIterations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LBM)
		FIntVector _latticeDims = FIntVector(64, 64, 64);

	//UFUNCTION(BlueprintCallable)
		void SetLatticeDims(FIntVector newLatticeDims);

	//UPROPERTY(BlueprintReadWrite, Category = Porous)
		UInstancedStaticMeshComponent* PorousBoundariesMeshes;

	// Sets default values for this pawn's properties
	ALBMActor();


protected:

	class AmarettoFileManager* _amaretto;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

	//AmarettoFileManager* _amarettoTest;
	int* porousDataArray;
	int currentIteration = -1;

	// for test:
	TArray<FLinearColor> uBuffer;
	TArray<FLinearColor> posBuffer;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};