#pragma once

#include "CoreMinimal.h"

class Amaretto
{

public:

	Amaretto(FString path);
	//static  ReadFile(string path);

	uint16 DimX;
	uint16 DimY;
	uint16 DimZ;

	/// <summary>
	/// �������� ������ �������� �����.
	/// </summary>
	/// <returns> ��������� �� ������. </returns>
	int* GetPorousDataArray();

	/// <summary>
	/// �������� �������� � ��������� �����������.
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="z"></param>
	/// <returns></returns>
	int GetPorousData(uint16 x, uint16 y, uint16 z);


	/// <summary>
	/// �������� ���������� ����� �� ������� ����-������.
	/// </summary>
	/// /// <param name="valueInsideBoundaries"> �������� �������, ������� �������� ������ �������� ������ ���� ������ ������. </param>
	/// <returns></returns>
	TArray<FIntVector> GetBoundariesCoordinates(int16 valueInsideBoundaries = 0);


protected:

	TArray<uint16> porousData;
	int* porousDataRaw;
};
