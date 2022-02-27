#pragma once
#include <driver_types.h>
#include "ILogger.h"

//#define checkError(val) _checkError(val, __LINE__)

class CUDAErrorChecker 
{
public:

	CUDAErrorChecker(ILogger* logger);

	void CheckError(const cudaError_t error, const int line, const int iteration);
	
protected:

	ILogger* _logger;
};