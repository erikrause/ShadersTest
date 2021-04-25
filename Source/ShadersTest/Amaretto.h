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
	/// �������� ������ �������� �����.
	/// </summary>
	/// <returns> ��������� �� ������. </returns>
	int* GetPorousDataArray();


protected:

	TArray<uint16> porousData;
	int* porousDataRaw;
};
