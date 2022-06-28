#include "DRFileManager.h"
#include "Misc/Paths.h"

UDRFileManager::UDRFileManager()
{

}

FString UDRFileManager::_getPorousFolder()
{
	return FPaths::Combine(FPaths::LaunchDir(), "Porous");
}

UDigitalRock* UDRFileManager::GetDigitalRockAmaretto(FString path)
{
	UDigitalRock* digitalRock = NewObject<UDigitalRock>();

	TArray<uint8> byteArray;
	FFileHelper::LoadFileToArray(byteArray, *path);

	FIntVector size;
	size.X = (byteArray[1] << 8) | (byteArray[0]);
	size.Y = (byteArray[3] << 8) | (byteArray[2]);
	size.Z = (byteArray.Num() - 4) / (2 * size.X * size.Y);
	TArray<uint16> porousData;

	for (int i = 4; i < byteArray.Num(); i = i + 2)
	{
		porousData.Add((byteArray[i + 1] << 8) | byteArray[i]);
	}

	TArray<uint8> porousDataShort;

	for (uint16 data : porousData)
	{
		porousDataShort.Add(data);
	}

	digitalRock->Init(porousDataShort, size);
	return digitalRock;
}

UDigitalRock* UDRFileManager::GetDigitalRock(FString path, FIntVector size)
{
	UDigitalRock* digitalRock = NewObject<UDigitalRock>();

	TArray<uint8> byteArray;
	FFileHelper::LoadFileToArray(byteArray, *path);
	digitalRock->Init(byteArray, size);
	return digitalRock;
}

TArray<uint8> UDRFileManager::_getByteArray(FString path)
{
	TArray<uint8> byteArray;
	FFileHelper::LoadFileToArray(byteArray, *path);

	return byteArray;
}
