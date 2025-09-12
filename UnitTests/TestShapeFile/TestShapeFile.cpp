// TestShapeFile.cpp
// CppUnitLite test harness for ShapeFile class
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WIN32
#include <direct.h>
#endif

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "../Common/UnitTest.h"			// unit test helpers
#include "ShapeFile.h"					// interface to class under test

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
TEST(ShapeFile, Test_PointM)
{
	// Unit test for "PointM" type shape file
	//

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "PointM.shp");

	{
		ShapeFile* pShapeFile = new ShapeFile(fileName.c_str());

		ShapeFile::ShapeType shapeType = pShapeFile->GetShapeType();
		CHECK(shapeType == ShapeFile::ShapeType::PointM);

		UInt32 polyCount = pShapeFile->GetPolygonCount();
		LONGS_EQUAL(0, polyCount);

		UInt32 pointCount = pShapeFile->GetPointCount();
		LONGS_EQUAL(2, pointCount);

		ShapeFile::PointMType pt = pShapeFile->GetPointM(0);
		DOUBLES_EQUAL(118.0, pt.X, 0.0);
		DOUBLES_EQUAL(36.0, pt.Y, 0.0);
		DOUBLES_EQUAL(1.6259745558096867e-260, pt.M, 0.0);

		pt = pShapeFile->GetPointM(1);
		DOUBLES_EQUAL(0.0, pt.X, 0.0);
		DOUBLES_EQUAL(36.0, pt.Y, 0.0);
		DOUBLES_EQUAL(1.6259745558096867e-260, pt.M, 0.0);

		delete pShapeFile;
	}
}

//----------------------------------------------------------------------------
TEST(ShapeFile, Test_Polyline)
{
	// Unit test for "Polyline" type shape file
	//

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "PolyLine.shp");

	{
		ShapeFile* pShapeFile = new ShapeFile(fileName.c_str());

		ShapeFile::ShapeType shapeType = pShapeFile->GetShapeType();
		CHECK(shapeType == ShapeFile::ShapeType::Polyline);

		UInt32 polyCount = pShapeFile->GetPolygonCount();
		LONGS_EQUAL(2, polyCount);

		UInt32 partCount = pShapeFile->GetPartCount(0);

		ShapeFile::PointType pt = pShapeFile->GetPoint(0, 0, 0);
		DOUBLES_EQUAL(1.0, pt.X, 0.0);
		DOUBLES_EQUAL(5.0, pt.Y, 0.0);

		delete pShapeFile;
	}
}
