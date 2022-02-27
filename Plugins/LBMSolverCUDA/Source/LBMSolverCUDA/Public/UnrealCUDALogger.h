#pragma once
#include "ILogger.h"

class UnrealCUDALogger : public ILogger
{
public:
	virtual void Log(std::string message, int iteration) override;
};