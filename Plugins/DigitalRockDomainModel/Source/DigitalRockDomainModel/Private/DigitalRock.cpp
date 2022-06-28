#include "DigitalRock.h"

void UDigitalRock::Init(TArray<uint8> byteArray, FIntVector size)
{
	_byteArray = byteArray;
	_size = size;
}

float* UDigitalRock::GetPorousAsFloat()
{
	float* data = new float[_byteArray.Num()];

	for (int i = 0; i < _byteArray.Num(); i++)
	{
		data[i] = _byteArray[i];
	}

	return data;
}

int* UDigitalRock::GetPorousAsInt()
{
	int* data = new int[_byteArray.Num()];

	for (int i = 0; i < _byteArray.Num(); i++)
	{
		data[i] = _byteArray[i];
	}

	return data;
}

FIntVector UDigitalRock::GetSize()
{
	return _size;
}