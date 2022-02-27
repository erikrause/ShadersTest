#include "UnrealCUDALogger.h"
#include "Containers/UnrealString.h"
#include <Runtime/Core/Public/Logging/LogMacros.h>

void UnrealCUDALogger::Log(std::string message, int iteration)
{
	FString unrealMessage(message.c_str());
	UE_LOG(LogTemp, Error, TEXT("%s . Iteration #%i"), *unrealMessage, iteration);
}
