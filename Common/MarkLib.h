// MarkLib.h
// 
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#ifndef MARKLIB_H
#define MARKLIB_H

#include <vector>

#include "MarkTypes.h"
#include "XString.h"

#define UNUSED(x) (void)x;

namespace MarkLib
{
	XString FindFile(const char* targetFile, const char* searchPath);
	bool FileExists(const char* szPath);
	bool PathExists(const char* szPath);
	bool FileCopy(XString srcFile, XString dstFile, bool bOverWrite = true);

	std::vector<XString> GetFileList(XString pathName);
	std::vector<XString> GetFileList(const char* pathName);

	std::vector<XString> GetDirList(XString pathName);
	std::vector<XString> GetDirList(const char* pathName);

	std::vector<UInt8> LoadFile(const char* fileName);

	double PerfTimer(bool start = false);

	XString TrimFloat(double v, int trail = 0);
}

#endif // #ifndef MARKLIB_H
