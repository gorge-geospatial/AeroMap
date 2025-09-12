// UnitTest.h
// Helpers for unit tests
//

#include <string.h>
#include <assert.h>

#include "XString.h"

XString gs_DataPath;		// location of test data


bool CloseTo(float v1, float v2, float delta)
{
	return fabs(v1 - v2) < delta;
}

bool CloseTo(double v1, double v2, double delta)
{
	return fabs(v1 - v2) < delta;
}

void SetDataPath(const char* progName)
{
	// based on assumption that the executable
	// is in [projectdir/platform/configuration]
	// and data folder is off of [projectdir]

	assert(progName);

	XString str = progName;
	str = str.GetPathName();
	int tokenCount = str.Tokenize("\\/");

	// must have at least platform/configuration
	assert(tokenCount >= 2);

	gs_DataPath = str.GetToken(0);
	for (int i = 1; i < tokenCount - 2; ++i)
		gs_DataPath = XString::CombinePath(gs_DataPath.c_str(), str.GetToken(i).c_str());
	gs_DataPath = XString::CombinePath(gs_DataPath.c_str(), "Data");
}
