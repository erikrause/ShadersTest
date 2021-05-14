PRAGMA_DISABLE_OPTIMIZATION

#include "Amaretto.h"

Amaretto::Amaretto(FString path)
{
	TArray<uint8> byteArray;
	FFileHelper::LoadFileToArray(byteArray, *path);

	Dims.X = (byteArray[1] << 8) | (byteArray[0]);
	Dims.Y = (byteArray[3] << 8) | (byteArray[2]);
	Dims.Z = (byteArray.Num() - 4) / (2 * Dims.X * Dims.Y);

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

int Amaretto::GetPorousData(uint16 x, uint16 y, uint16 z)
{
	return porousDataRaw[x + y * Dims.X + z * Dims.X * Dims.Y];
}

TArray<FIntVector> Amaretto::GetBoundariesCoordinates(int16 valueInsideBoundaries)
{
	TArray<FIntVector> BoundariesCoords;

	for (uint16 x = 0; x < Dims.X; x++)
		for (uint16 y = 0; y < Dims.Y; y++)
			for (uint16 z = 0; z < Dims.Z; z++)
			{
				if (GetPorousData(x, y, z) == valueInsideBoundaries)
				{
					//PorousTarget[int2(x_tex_u, y_tex_u)] = 0;
					const int neigbor_count = 6;
					const FIntVector neigbour_dirs[neigbor_count] = { FIntVector(-1, 0, 0), FIntVector(0, -1, 0), FIntVector(0, 0, -1), FIntVector(1, 0, 0), FIntVector(0, 1, 0), FIntVector(0, 0, 1) };
					for (int i = 0; i < neigbor_count; i++)
					{
						FIntVector neighbour_coord = FIntVector(x, y, z) + neigbour_dirs[i];
						if (neighbour_coord.X == -1 || neighbour_coord.Y == -1 || neighbour_coord.Z == -1 ||
							neighbour_coord.X == Dims.X || neighbour_coord.Y == Dims.Y || neighbour_coord.Z == Dims.Z)
						{
							BoundariesCoords.Add(FIntVector(x, y, z));
							i = neigbor_count; // end for loop.
						}
						else if (GetPorousData(neighbour_coord.X, neighbour_coord.Y, neighbour_coord.Z) != valueInsideBoundaries)
						{
							BoundariesCoords.Add(FIntVector(x, y, z));
							i = neigbor_count; // end for loop.
						}
					}
				}
			}
	return BoundariesCoords;
}
