// TestModel_STL.cpp
// CppUnitLite test harness for Model_STL class
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "../Common/UnitTest.h"			// unit test helpers
#include "Model_STL.h"					// interface to class under test

int main(int argc, char* argv[])
{
	// output name of executable
	if (argc > 0 && argv[0])
		printf("%s\n", argv[0]);

	SetDataPath(argv[0]);

	TestResult tr;
	TestRegistry::runAllTests(tr);

	// always pause if errors
	int failureCount = tr.GetFailureCount();
	if (failureCount > 0)
		getc(stdin);

	return failureCount;
}

//----------------------------------------------------------------------------
TEST(Model_STL, Test_Read)
{
	// Test reading a simple STL file.
	//

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "sample_ascii.stl");

	{
		Model_STL* pModel = new Model_STL();
		bool status = pModel->Load(fileName.c_str());
		CHECK(status);

		CHECK(Model::Format::STL == pModel->GetFormat());

		delete pModel;
	}
}
