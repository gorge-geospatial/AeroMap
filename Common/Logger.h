#ifndef LOGGER_H
#define LOGGER_H

#include "XString.h"

class Logger
{
public:
	static void Init(const char* path, const char* fileName);
	static void Write(const char* function, const char* text, ...);

private:
	static XString GetTime();
	inline static void OutputLine(FILE* pFile, const char* function, const char* text);
};

#endif // LOGGER_H
