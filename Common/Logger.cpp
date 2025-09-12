// Logger.cpp
// Static message logger
//	
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "MarkTypes.h"
#include "XString.h"
#include "Logger.h"

static char ms_LogFile[255];			// fully qualified log file name

//----------------------------------------------------------------------------------------------------------------------
void Logger::Init(const char* path, const char* fileName)
{
	//	initialize log file
	//
	// 	inputs:
	//		path 	 = location of log file, "" to use current working directory
	//		fileName = log file name
	//

	XString pathName = path;

	pathName.Replace('\\', '/');
	memset(ms_LogFile, 0, sizeof(ms_LogFile));

	if (strlen(path) + strlen(fileName) > sizeof(ms_LogFile) - 2)
	{
		printf("%s - file name '%s%s' too long.\n", __FUNCTION__, path, fileName);
		return;
	}

	if (pathName.GetLength() > 0)			// check path and format if necessary
	{
		if (pathName.EndsWith("/"))			// path ends in '/'
			sprintf(ms_LogFile, "%s%s", pathName.c_str(), fileName);
		else
			sprintf(ms_LogFile, "%s/%s", pathName.c_str(), fileName);
	}
	else
	{
		strcpy(ms_LogFile, fileName);
	}

	FILE* pLogFile = fopen(ms_LogFile, "wt");			// try to open the file for output
	if (pLogFile)
	{
		XString strTime = GetTime();

		fprintf(pLogFile, "%s: Log file '%s' created.\n", strTime.c_str(), ms_LogFile);
		fclose(pLogFile);
	}
	else
	{
		printf("%s - unable to open '%s'.\n", __FUNCTION__, ms_LogFile);
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Logger::Write(const char* function, const char* text, ...)
{
	//	append to log file and echo to stdout
	//
	// 	inputs:
	//		function = function name
	//		text     = printf style formatting string
	//		...	     = parameters to formatting string
	//

	char buf[255];

	va_list ap;
	va_start(ap, text);
	vsprintf(buf, text, ap);
	va_end(ap);

	if (strlen(ms_LogFile) > 0)
	{
		FILE* outFile = fopen(ms_LogFile, "at");		// try to open the file for append
		if (outFile)
		{
			OutputLine(outFile, function, buf);
			fclose(outFile);
		}
	}

	// always echo to stdout
	OutputLine(stdout, function, buf);
}

void Logger::OutputLine(FILE* pFile, const char* function, const char* text)
{
	// output formatted log entry

	XString strTime = GetTime();

	fprintf(pFile, "%s: ", strTime.c_str());
	if (strlen(function) > 0)
		fprintf(pFile, "%s - ", function);
	fprintf(pFile, "%s\n", text);
}

XString Logger::GetTime()
{
	XString strTime;

	time_t result = time(nullptr);
	if (result == ((time_t)(-1)))
	{
		fprintf(stderr, "time() failed in file %s at line # %d\n", __FILE__, __LINE__ - 3);
		exit(EXIT_FAILURE);
	}

	// convert the time_t to tm
	tm* temp = localtime(&result);
	char buf[24];
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", temp->tm_year + 1900, temp->tm_mon + 1, temp->tm_mday,
		temp->tm_hour, temp->tm_min, temp->tm_sec);
	strTime = buf;

	return strTime;
}
