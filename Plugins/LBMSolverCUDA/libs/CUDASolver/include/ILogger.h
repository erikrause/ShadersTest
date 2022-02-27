#pragma once
#include <string>

class ILogger
{
public:
	virtual ~ILogger() {};
	virtual void Log(std::string message, int iteration) = 0;
};