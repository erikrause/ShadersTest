#pragma once

#include "CoreMinimal.h"
#include "VolumeInfoDTO.generated.h"

USTRUCT(BlueprintType)
struct LBMSOLVERCUDA_API FVolumeInfoDTO
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CFD")
    float Min;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CFD")
    float Max;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CFD")
    float Sum;

    FVolumeInfoDTO() {};
    FVolumeInfoDTO(float min, float max, float sum) : Min(min), Max(max), Sum(sum)
    {

    }
};