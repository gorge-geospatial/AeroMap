// MarkLib.cpp - library of common functions
// 
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <dirent.h>		// posix directory support
#endif

#include <assert.h>

#include "MarkLib.h"

namespace MarkLib
{

double PerfTimer(bool start)
{
	// Start/stop high frequency timer
	//
	// Usage:
	//		1. call PerfTimer(true) to start timer
	//		2. call dt = PerfTimer() to get elapsed time in seconds
	//
	// Outputs:
	//		return = elapsed time, seconds

	double elapsedTime = 0.0;

	static __int64 perfFreq = 0;			// high freq ticks per second
	static __int64 perfTime0 = 0;
	static __int64 perfTime1 = 0;

	if (perfFreq == 0)
	{
		// Get ticks-per-second of the performance counter.
		//   Note the necessary typecast to a LARGE_INTEGER structure
		if (!QueryPerformanceFrequency((LARGE_INTEGER*)&perfFreq))
		{
			// High frequency performance counter not supported
			return -1.0;
		}
	}

	if (start == true)
	{
		// record starting time
		QueryPerformanceCounter((LARGE_INTEGER*)&perfTime0);
	}
	else
	{
		// calculate elapsed time
		QueryPerformanceCounter((LARGE_INTEGER*)&perfTime1);
		__int64 dt = perfTime1 - perfTime0;
		elapsedTime = (double)dt / (double)perfFreq;
		perfTime0 = 0;
	}

	return elapsedTime;
}

std::vector<UInt8> LoadFile(const char* fileName)
{
	// load binary file

	std::vector<UInt8> fileContents;

	FILE* pFile = fopen(fileName, "rb");
	if (pFile)
	{
		int c;
		while ((c = fgetc(pFile)) != EOF)
		{
			assert(c < 256);
			fileContents.push_back(static_cast<char>(c));
		}

		fclose(pFile);
	}

	return fileContents;
}

XString FindFile(const char* targetFile, const char* searchPath)
{
	//	return the fully-qualified path/file name or "" if not found
	//
	//	searches from specified node down
	//
	// 	inputs:
	//		targetFile = file to find
	//		searchPath = starting node, or null to use current dir
	//

	XString filePath;		// return value
	static int depth = 0;

#ifdef WIN32
	XString curPath;		// current dir
	WIN32_FIND_DATAA fdFile;
	HANDLE hFind = NULL;
	char sPath[2048];

	if (searchPath)
	{
		curPath = searchPath;
	}
	else
	{
		curPath = _getcwd(sPath, sizeof(sPath));
	}

	// is the target file in this dir?
	filePath = XString::CombinePath(curPath.c_str(), targetFile);
	if (FileExists(filePath.c_str()))
		return filePath;

	//Specify a file mask. *.* = We want everything!
	sprintf(sPath, "%s\\*.*", curPath.c_str());


	if ((hFind = FindFirstFileA(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		// Path not found
		return XString("");
	}

	do
	{
		// find first file will always return "." and ".." as the first two directories
		if (strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0)
		{
			//Build up our file path using the passed in
			//  [searchPath] and the file/foldername we just found:
			sprintf(sPath, "%s\\%s", curPath.c_str(), fdFile.cFileName);

			// is this a directory?
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// yes, see if it contains the target file
				filePath = FindFile(targetFile, sPath); //Recursion, I love it!
				if (filePath.GetLength() > 0)
					return filePath;
			}
		}
	} while (FindNextFileA(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!
#else
//TODO:
//implement
//copied from https://stackoverflow.com/questions/8149569/scan-a-directory-to-find-files-in-c
//but haven't worked with it yet
    DIR* dp;
    struct dirent* entry;
    struct stat statbuf;
    if ((dp = opendir(searchPath)) == NULL)
    {
        fprintf(stderr,"cannot open directory: %s\n", searchPath);
        return filePath;
    }
    //chdir(searchPath);
    while ((entry = readdir(dp)) != NULL)
	{
		lstat(entry->d_name, &statbuf);
		if (S_ISDIR(statbuf.st_mode))
		{
			/* Found a directory, but ignore . and .. */
			if (strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
				continue;
			printf("%*s%s/\n",depth,"",entry->d_name);
			/* Recurse at a new indent level */
			//printdir(entry->d_name,depth+4);
			//..instead recurse into my FindFile()
		}
		else
		{
		 	printf("%*s%s\n", depth, "", entry->d_name);	
		}
	}
	//chdir("..");
	closedir(dp);
#endif

	// not found
	return XString("");
}

bool FileExists(const char* szPath)
{
#ifdef WIN32
	DWORD dwAttrib = ::GetFileAttributesA( szPath );
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
//TODO:
//untested
	struct stat buffer;
  	return (stat (szPath, &buffer) == 0); 
#endif
}

bool PathExists(const char* szPath)
{
#ifdef WIN32
	DWORD dwAttrib = ::GetFileAttributesA(szPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
//TODO:
//untested
	struct stat buffer;
  	return (stat (szPath, &buffer) == 0); 
#endif
}

std::vector<XString> GetFileList(XString pathName)
{
	return GetFileList(pathName.c_str());
}

std::vector<XString> GetFileList(const char* pathName)
{
	// return list of files in directory

	std::vector<XString> fileList;	// return values

#ifdef WIN32
	WIN32_FIND_DATAA fdFile;
	HANDLE hFind = NULL;

	char sPath[2048];

	//Specify a file mask. *.* = We want everything!
	sprintf(sPath, "%s\\*.*", pathName);

	if ((hFind = FindFirstFileA(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		return fileList;
	}

	do
	{
		// find first file will always return "." and ".." as the first two directories
		if (strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0)
		{
			//Build up our file path using the passed in
			//  [pathName] and the file/foldername we just found:
			sprintf(sPath, "%s\\%s", pathName, fdFile.cFileName);

			//Is the entity a File or Folder?
			if ((fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				fileList.push_back(XString(sPath));
			}
		}
	} while (FindNextFileA(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!
#else
	struct dirent *entry;
	DIR *dp;

	dp = opendir(pathName);
	if (dp == nullptr) 
	{
		perror("opendir");
		return fileList;
	}

	while ((entry = readdir(dp)))
	{
		// if it's a regular file
		if (entry->d_type == DT_REG)
		{
			puts(entry->d_name); 
			fileList.push_back(entry->d_name);
		}
	}

	closedir(dp);
#endif

	return fileList;
}

std::vector<XString> GetDirList(XString pathName)
{
	return GetDirList(pathName.c_str());
}

std::vector<XString> GetDirList(const char* pathName)
{
	// return list of directories in directory

	std::vector<XString> fileList;	// return values

#ifdef WIN32
	WIN32_FIND_DATAA fdFile;
	HANDLE hFind = NULL;

	char sPath[2048];

	//Specify a file mask. *.* = We want everything!
	sprintf(sPath, "%s\\*.*", pathName);

	if ((hFind = FindFirstFileA(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		return fileList;
	}

	do
	{
		// find first file will always return "." and ".." as the first two directories
		if (strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0)
		{
			//Build up our file path using the passed in
			//  [pathName] and the file/foldername we just found:
			sprintf(sPath, "%s\\%s", pathName, fdFile.cFileName);

			//Is the entity a File or Folder?
			if ((fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				fileList.push_back(XString(sPath));
			}
		}
	} while (FindNextFileA(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!
#else
	struct dirent *entry;
	DIR *dp;

	dp = opendir(pathName);
	if (dp == nullptr) 
	{
		perror("opendir");
		return fileList;
	}

	while ((entry = readdir(dp)))
	{
		// if it's a directory
		if (entry->d_type == DT_DIR)
		{
			puts(entry->d_name); 
			fileList.push_back(entry->d_name);
		}	
	}

	closedir(dp);
#endif

	return fileList;
}

bool FileCopy(XString srcFile, XString dstFile, bool bOverWrite)
{
	bool status = true;
	if (!CopyFileA(srcFile.c_str(), dstFile.c_str(), bOverWrite ? FALSE : TRUE))
		status = false;

	return status;
}

XString TrimFloat(double v, int trail)
{
	// Return a formatted float with no more
	// than "trail" trailing zeros.
	//
	// So you could still end up with more
	// digits to the right that 'trail', just
	// not more trailing zeros.
	//
	// Examples:
	//		"1.230000" trail = 1 => "1.23"
	//		"1.000000" trail = 1 => "1.0"
	//		"1.000005" trail = 1 => "1.000005"
	//		"1.000000" trail = 0 => "1"
	//

	assert(trail >= 0);

	XString s = XString::Format("%f", v);
	if (s.Find('.') > -1)
	{
		int count = s.GetLength() - s.Find('.') - 1;
		while ((count > trail) && (s.Right(1) == "0"))
		{
			// trim trailing zeros until we get down
			// to desired number, or encounter non-zero
			s.DeleteRight(1);
			--count;
		}

		// if all zeros have been removed, remove decimal
		if (s.EndsWith("."))
			s.DeleteRight(1);
	}

	return s;
}

} // MarkLib
