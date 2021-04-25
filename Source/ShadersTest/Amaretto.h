#pragma once

#include "CoreMinimal.h"

class Amaretto
{

public:

	Amaretto(FString path);
	//static  ReadFile(string path);

	uint16 DimX;
	uint16 DimY;

	/// <summary>
	/// Получить массив пористой среды.
	/// </summary>
	/// <returns> Указатель на массив. </returns>
	int* GetPorousDataArray();


protected:

	TArray<uint16> porousData;
	int* porousDataRaw;
};
