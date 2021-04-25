PRAGMA_DISABLE_OPTIMIZATION

#include "Amaretto.h"

Amaretto::Amaretto(FString path)
{
	TArray<uint8> byteArray;
	FFileHelper::LoadFileToArray(byteArray, *path);

	DimX = (byteArray[1] << 8) | (byteArray[0]);
	DimY= (byteArray[3] << 8) | (byteArray[2]);

	for (int i = 4; i < byteArray.Num(); i = i + 2)
	{
		porousData.Add((byteArray[i + 1] << 8) | byteArray[i]);
	}

	/*for (uint16 x = 0; x < DimX; x++)
	{
		for (uint16 y = 0; y < DimY; y++)
		{

		}
	}*/

	porousDataRaw = new int[porousData.Num()];

	int i = 0;
	for (uint16 data : porousData)
	{
		float dataRaw = data;
		porousDataRaw[i] = dataRaw;
		i++;
	}
	
	int prob = 0;
}

int* Amaretto::GetPorousDataArray() 
{
	return porousDataRaw;
}