#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LBMActor3D.generated.h"

UCLASS()
class SHADERSTEST_API ALBMActor3D : public APawn
{
	GENERATED_BODY()

//Properties
public:
	UPROPERTY()
		USceneComponent* Root;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* static_mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shader)
		class UTextureRenderTarget2D* FRenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shader)
		class UTextureRenderTarget2D* URenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shader)
		class UTextureRenderTarget2D* PosRenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shader)
		class UTextureRenderTarget2D* PorousRenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shader)
		FIntVector LatticeDims = FIntVector(64, 64, 64);
public:
	// Sets default values for this pawn's properties
	ALBMActor3D();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

	//Amaretto* _amarettoTest;
	int* porousDataArray;
	int iteration = -1;

	// for test:
	TArray<FLinearColor> uBuffer;
	TArray<FLinearColor> posBuffer;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};