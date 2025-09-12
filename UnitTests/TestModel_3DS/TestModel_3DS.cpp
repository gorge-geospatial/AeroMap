// TestModel_3DS.cpp
// CppUnitLite test harness for Model_3DS class
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "../Common/UnitTest.h"			// unit test helpers
#include "Model_3DS.h"					// interface to class under test

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
TEST(Model3DS, Test_Read)
{
	// Test reading a 3DS file.
	//

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "test_X200.3ds");

	{
		Model_3DS* pModel = new Model_3DS();
		bool status = pModel->Load(fileName.c_str());
		CHECK(status);

		CHECK(Model::Format::ThreeDS == pModel->GetFormat());

		LONGS_EQUAL(3, pModel->GetMeshCount());
		
		LONGS_EQUAL(766, pModel->GetFaceCount(0));
		LONGS_EQUAL(385, pModel->GetVertexCount(0));
		CHECK(strcmp(pModel->GetMeshName(0), "VIFS01") == 0);
		int meshIndex = pModel->GetMeshIndex("VIFS01");
		LONGS_EQUAL(0, meshIndex);

		LONGS_EQUAL(3346, pModel->GetFaceCount(1));
		LONGS_EQUAL(1675, pModel->GetVertexCount(1));
		CHECK(strcmp(pModel->GetMeshName(1), "VIFS03") == 0);
		meshIndex = pModel->GetMeshIndex("VIFS03");
		LONGS_EQUAL(1, meshIndex);

		LONGS_EQUAL(5108, pModel->GetFaceCount(2));
		LONGS_EQUAL(2554, pModel->GetVertexCount(2));
		CHECK(strcmp(pModel->GetMeshName(2), "VIFS04") == 0);
		meshIndex = pModel->GetMeshIndex("VIFS04");
		LONGS_EQUAL(2, meshIndex);

		delete pModel;
	}
}
