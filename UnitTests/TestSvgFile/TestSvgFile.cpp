// TestSvgFile.cpp
// CppUnitLite test harness for SvgFile class
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

#include "CppUnitLite/TestHarness.h"
#include "../Common/UnitTest.h"			// unit test helpers
#include "SvgFile.h"					// interface to class under test

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
TEST(SvgFile, Read)
{
	// Read test svg file.
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "test1.svg");
		SvgFile* pSvgFile = new SvgFile();

		bool status = pSvgFile->Load(fileName.c_str());
		CHECK(status == true);

		if (status == true)
		{
			LONGS_EQUAL(2, pSvgFile->GetElementCount());

			SvgFile::ElemDesc* pElem0 = pSvgFile->GetElement(0);
			CHECK(pElem0->type == SvgFile::ElemType::PolyLine);
			LONGS_EQUAL(545, (long)pElem0->path.size());

			SvgFile::ElemDesc* pElem1 = pSvgFile->GetElement(1);
			CHECK(pElem1->type == SvgFile::ElemType::PolyLine);
			LONGS_EQUAL(1111, (long)pElem1->path.size());
		}
		delete pSvgFile;
	}
}
