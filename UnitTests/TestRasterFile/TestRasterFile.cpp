// TestRasterFile.cpp
// CppUnitLite test harness for RasterFile class
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "../Common/UnitTest.h"			// unit test helpers
#include "RasterFile.h"					// interface to class under test

int main(int argc, char* argv[])
{
	// output name of executable
	if (argc > 0 && argv[0])
		printf("%s\n", argv[0]);

	SetDataPath(argv[0]);

	GDALAllRegister();

	TestResult tr;
	TestRegistry::runAllTests(tr);

	// always pause if errors
	int failureCount = tr.GetFailureCount();
	if (failureCount > 0)
		getc(stdin);

	return failureCount;
}

//----------------------------------------------------------------------------
TEST(RasterFile, Test_W121_N45_DT0)
{
	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "w121_n45.dt0");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		CHECK(GIS::GEODATA::DTED0 == pFile->GetFileType());

		// make sure gdal uses the driver we expect
		XString strDriver = pFile->GetDriverName();
		CHECK(strDriver.CompareNoCase("DTED") == true);

		CHECK(pFile->IsGeographic() == true);
		CHECK(pFile->IsProjected() == false);

		LONGS_EQUAL(121, pFile->GetSizeX());
		LONGS_EQUAL(121, pFile->GetSizeY());

		RectD extents = pFile->GetExtents();
		DOUBLES_EQUAL(  46.004166666666663, extents.y0, 0.0);
		DOUBLES_EQUAL(  44.995833333333330, extents.y1, 0.0);
		DOUBLES_EQUAL(-121.00416666666666, extents.x0, 0.0);
		DOUBLES_EQUAL(-119.99583333333332, extents.x1, 0.0);

		int minHeight = (int)pFile->GetMinElev();
		int maxHeight = (int)pFile->GetMaxElev();
		LONGS_EQUAL(18, minHeight);
		LONGS_EQUAL(1762, maxHeight);

		double height = pFile->GetHeight(0, 0);
		DOUBLES_EQUAL(1114.0, height, 0.0);
		height = pFile->GetHeight(1, 0);
		DOUBLES_EQUAL(1056.0, height, 0.0);
		height = pFile->GetHeight(2, 0);
		DOUBLES_EQUAL(900.0, height, 0.0);

		height = pFile->GetHeight(0, 1);
		DOUBLES_EQUAL(1134, height, 0.0);
		height = pFile->GetHeight(0, 2);
		DOUBLES_EQUAL(1202, height, 0.0);

		height = pFile->GetHeight(118, 0);
		DOUBLES_EQUAL(933, height, 0.0);
		height = pFile->GetHeight(119, 0);
		DOUBLES_EQUAL(982, height, 0.0);
		height = pFile->GetHeight(120, 0);
		DOUBLES_EQUAL(1028, height, 0.0);

		height = pFile->GetHeight(0, 118);
		DOUBLES_EQUAL(499, height, 0.0);
		height = pFile->GetHeight(0, 119);
		DOUBLES_EQUAL(476, height, 0.0);
		height = pFile->GetHeight(0, 120);
		DOUBLES_EQUAL(458, height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(121, 0);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(0, 121);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		CHECK(pFile->Contains(-120.5, 45.5) == true);
		CHECK(pFile->Contains(-121.5, 44.5) == false);
		CHECK(pFile->Contains(-122.5, 45.5) == false);
		CHECK(pFile->Contains(-120.5, 44.5) == false);

		delete pFile;
	}
}

//----------------------------------------------------------------------------
TEST(RasterFile, W121_N46_DT0)
{
	// These values validated against other geospatial software.

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "w121_n46.dt0");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		CHECK(GIS::GEODATA::DTED0 == pFile->GetFileType());

		// make sure gdal uses the driver we expect
		XString strDriver = pFile->GetDriverName();
		CHECK(strDriver.CompareNoCase("DTED") == true);

		CHECK(pFile->IsGeographic() == true);
		CHECK(pFile->IsProjected() == false);

		LONGS_EQUAL(121, pFile->GetSizeX());
		LONGS_EQUAL(121, pFile->GetSizeY());

		RectD extents = pFile->GetExtents();
		DOUBLES_EQUAL(47.004166666666663, extents.y0, 0.0);
		DOUBLES_EQUAL(45.995833333333330, extents.y1, 0.0);
		DOUBLES_EQUAL(-121.00416666666666, extents.x0, 0.0);
		DOUBLES_EQUAL(-119.99583333333332, extents.x1, 0.0);

		int minHeight = (int)pFile->GetMinElev();
		int maxHeight = (int)pFile->GetMaxElev();
		DOUBLES_EQUAL(180, minHeight, 0.0);
		DOUBLES_EQUAL(1841, maxHeight, 0.0);

		double height = pFile->GetHeight(0, 0);
		DOUBLES_EQUAL(1773, height, 0.0);
		height = pFile->GetHeight(1, 0);
		DOUBLES_EQUAL(1543, height, 0.0);
		height = pFile->GetHeight(2, 0);
		DOUBLES_EQUAL(1426, height, 0.0);

		height = pFile->GetHeight(0, 1);
		DOUBLES_EQUAL(1778, height, 0.0);
		height = pFile->GetHeight(1, 1);
		DOUBLES_EQUAL(1555, height, 0.0);
		height = pFile->GetHeight(2, 1);
		DOUBLES_EQUAL(1474, height, 0.0);

		height = pFile->GetHeight(120, 120);
		DOUBLES_EQUAL(458, height, 0.0);
		height = pFile->GetHeight(119, 120);
		DOUBLES_EQUAL(473, height, 0.0);
		height = pFile->GetHeight(118, 120);
		DOUBLES_EQUAL(482, height, 0.0);
		height = pFile->GetHeight(117, 120);
		DOUBLES_EQUAL(488, height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(121, 0);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(0, 121);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		CHECK(pFile->Contains(-120.5, 46.5) == true);
		CHECK(pFile->Contains(-121.5, 45.5) == false);
		CHECK(pFile->Contains(-122.5, 46.5) == false);
		CHECK(pFile->Contains(-120.5, 45.5) == false);

		delete pFile;
	}
}

//----------------------------------------------------------------------------
TEST(RasterFile, Test_W122_N45_DT0)
{
	// These values validated against other geospatial software.

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "w122_n45.dt0");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		CHECK(GIS::GEODATA::DTED0 == pFile->GetFileType());

		// make sure gdal uses the driver we expect
		XString strDriver = pFile->GetDriverName();
		CHECK(strDriver.CompareNoCase("DTED") == true);

		CHECK(pFile->IsGeographic() == true);
		CHECK(pFile->IsProjected() == false);

		LONGS_EQUAL(121, pFile->GetSizeX());
		LONGS_EQUAL(121, pFile->GetSizeY());

		RectD ext = pFile->GetExtents();
		DOUBLES_EQUAL(46.004166666666663, ext.y0, 0.0);
		DOUBLES_EQUAL(44.995833333333330, ext.y1, 0.0);
		DOUBLES_EQUAL(-122.00416666666666, ext.x0, 0.0);
		DOUBLES_EQUAL(-120.99583333333332, ext.x1, 0.0);

		int minHeight = (int)pFile->GetMinElev();
		int maxHeight = (int)pFile->GetMaxElev();
		LONGS_EQUAL(6, minHeight);
		LONGS_EQUAL(3259, maxHeight);

		double height = pFile->GetHeight(0, 0);
		DOUBLES_EQUAL(966, height, 0.0);
		height = pFile->GetHeight(1, 0);
		DOUBLES_EQUAL(967, height, 0.0);
		height = pFile->GetHeight(2, 0);
		DOUBLES_EQUAL(1101, height, 0.0);

		height = pFile->GetHeight(0, 1);
		DOUBLES_EQUAL(1042, height, 0.0);
		height = pFile->GetHeight(1, 1);
		DOUBLES_EQUAL(1013, height, 0.0);
		height = pFile->GetHeight(2, 1);
		DOUBLES_EQUAL(1031, height, 0.0);

		height = pFile->GetHeight(120, 120);
		DOUBLES_EQUAL(1028, height, 0.0);
		height = pFile->GetHeight(119, 120);
		DOUBLES_EQUAL(982, height, 0.0);
		height = pFile->GetHeight(118, 120);
		DOUBLES_EQUAL(933, height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(121, 0);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(0, 121);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		CHECK(pFile->Contains(-121.5, 45.5) == true);
		CHECK(pFile->Contains(-121.5, 46.5) == false);
		CHECK(pFile->Contains(-121.5, 44.5) == false);
		CHECK(pFile->Contains(-120.5, 45.5) == false);

		delete pFile;
	}
}

//----------------------------------------------------------------------------
TEST(RasterFile, Test_W122_N46_DT0)
{
	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "w122_n46.dt0");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		CHECK(GIS::GEODATA::DTED0 == pFile->GetFileType());

		// make sure gdal uses the driver we expect
		XString strDriver = pFile->GetDriverName();
		CHECK(strDriver.CompareNoCase("DTED") == true);

		CHECK(pFile->IsGeographic() == true);
		CHECK(pFile->IsProjected() == false);

		LONGS_EQUAL(121, pFile->GetSizeX());
		LONGS_EQUAL(121, pFile->GetSizeY());

		RectD ext = pFile->GetExtents();
		DOUBLES_EQUAL(47.004166666666663, ext.y0, 0.0);
		DOUBLES_EQUAL(45.995833333333330, ext.y1, 0.0);
		DOUBLES_EQUAL(-122.00416666666666, ext.x0, 0.0);
		DOUBLES_EQUAL(-120.99583333333332, ext.x1, 0.0);

		int minHeight = (int)pFile->GetMinElev();
		int maxHeight = (int)pFile->GetMaxElev();
		LONGS_EQUAL(269, minHeight);
		LONGS_EQUAL(4301, maxHeight);

		double height = pFile->GetHeight(0, 0);
		DOUBLES_EQUAL(505, height, 0.0);
		height = pFile->GetHeight(1, 0);
		DOUBLES_EQUAL(725, height, 0.0);
		height = pFile->GetHeight(2, 0);
		DOUBLES_EQUAL(896, height, 0.0);

		height = pFile->GetHeight(0, 1);
		DOUBLES_EQUAL(467, height, 0.0);
		height = pFile->GetHeight(1, 1);
		DOUBLES_EQUAL(650, height, 0.0);
		height = pFile->GetHeight(2, 1);
		DOUBLES_EQUAL(907, height, 0.0);

		height = pFile->GetHeight(120, 120);
		DOUBLES_EQUAL(1114, height, 0.0);
		height = pFile->GetHeight(119, 120);
		DOUBLES_EQUAL(988, height, 0.0);
		height = pFile->GetHeight(118, 120);
		DOUBLES_EQUAL(896, height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(121, 0);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(0, 121);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		CHECK(pFile->Contains(-121.5, 46.5) == true);
		CHECK(pFile->Contains(-122.5, 45.5) == false);
		CHECK(pFile->Contains(-123.5, 46.5) == false);
		CHECK(pFile->Contains(-121.5, 45.5) == false);

		delete pFile;
	}
}

//----------------------------------------------------------------------------
TEST(RasterFile, Test_W117_N50_DT1)
{
	// Verify reading a DTED level 1 file.
	// w117_n50.dt1 is a non-square file.
	//
	// Values here are verified against other geospatial software.

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "w117_n50.dt1");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		CHECK(GIS::GEODATA::DTED1 == pFile->GetFileType());

		// make sure gdal uses the driver we expect
		XString strDriver = pFile->GetDriverName();
		CHECK(strDriver.CompareNoCase("DTED") == true);

		CHECK(pFile->IsGeographic() == true);
		CHECK(pFile->IsProjected() == false);

		LONGS_EQUAL(601, pFile->GetSizeX());
		LONGS_EQUAL(1201, pFile->GetSizeY());

		RectD ext = pFile->GetExtents();
		DOUBLES_EQUAL(51.000416666666666, ext.y0, 0.0);
		DOUBLES_EQUAL(49.999583333333334, ext.y1, 0.0);
		DOUBLES_EQUAL(-117.00083333333333, ext.x0, 0.0);
		DOUBLES_EQUAL(-115.99916666666667, ext.x1, 0.0);

		int minHeight = (int)pFile->GetMinElev();
		int maxHeight = (int)pFile->GetMaxElev();
		LONGS_EQUAL(524, minHeight);
		LONGS_EQUAL(3371, maxHeight);

		// (0,0) is NW corner
		double height = pFile->GetHeight(0, 0);
		DOUBLES_EQUAL(1805, height, 0.0);
		height = pFile->GetHeight(1, 0);
		DOUBLES_EQUAL(1807, height, 0.0);
		height = pFile->GetHeight(2, 0);
		DOUBLES_EQUAL(1827, height, 0.0);

		// (1200,0) is SW corner
		height = pFile->GetHeight(1200, 0);
		DOUBLES_EQUAL(1569, height, 0.0);
		height = pFile->GetHeight(1199, 0);
		DOUBLES_EQUAL(1583, height, 0.0);
		height = pFile->GetHeight(1198, 0);
		DOUBLES_EQUAL(1582, height, 0.0);

		// (0,1) is the start of the 2nd column 
		height = pFile->GetHeight(0, 1);
		DOUBLES_EQUAL(1819, height, 0.0);
		height = pFile->GetHeight(1, 1);
		DOUBLES_EQUAL(1852, height, 0.0);
		height = pFile->GetHeight(2, 1);
		DOUBLES_EQUAL(1870, height, 0.0);

		// (1200,1200) is the SE corner
		height = pFile->GetHeight(1200, 600);
		DOUBLES_EQUAL(2153, height, 0.0);
		height = pFile->GetHeight(1199, 600);
		DOUBLES_EQUAL(2140, height, 0.0);
		height = pFile->GetHeight(1198, 600);
		DOUBLES_EQUAL(2112, height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(1201, 0);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(0, 1201);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		CHECK(pFile->Contains(-116.5, 50.5) == true);
		CHECK(pFile->Contains(-117.0, 51.0) == true);
		CHECK(pFile->Contains(-117.5, 51.5) == false);
		CHECK(pFile->Contains(-112.5, 50.5) == false);

		delete pFile;
	}
}

//----------------------------------------------------------------------------
TEST(RasterFile, Test_W123_N43_DT1)
{
	// Verify reading a DTED level 1 file. These are (up to) 1201x1201.
	//
	// Values here are verified against other geospatial software.

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "w123_n43.dt1");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		CHECK(GIS::GEODATA::DTED1 == pFile->GetFileType());

		// make sure gdal uses the driver we expect
		XString strDriver = pFile->GetDriverName();
		CHECK(strDriver.CompareNoCase("DTED") == true);

		CHECK(pFile->IsGeographic() == true);
		CHECK(pFile->IsProjected() == false);

		LONGS_EQUAL(1201, pFile->GetSizeX());
		LONGS_EQUAL(1201, pFile->GetSizeY());

		RectD ext = pFile->GetExtents();
		DOUBLES_EQUAL(-123.00041666666667,  ext.x0, 0.0);
		DOUBLES_EQUAL(  44.000416666666666, ext.y0, 0.0);
		DOUBLES_EQUAL(-121.99958333333333,  ext.x1, 0.0);
		DOUBLES_EQUAL(  42.999583333333334, ext.y1, 0.0);

		int minHeight = (int)pFile->GetMinElev();
		int maxHeight = (int)pFile->GetMaxElev();
		LONGS_EQUAL(115, minHeight);
		LONGS_EQUAL(2677, maxHeight);

		// (0,0) is NW corner
		double height = pFile->GetHeight(0, 0);
		DOUBLES_EQUAL(139, height, 0.0);
		height = pFile->GetHeight(1, 0);
		DOUBLES_EQUAL(144, height, 0.0);
		height = pFile->GetHeight(2, 0);
		DOUBLES_EQUAL(143, height, 0.0);

		// (1200,0) is SW corner
		height = pFile->GetHeight(1200, 0);
		DOUBLES_EQUAL(525, height, 0.0);
		height = pFile->GetHeight(1199, 0);
		DOUBLES_EQUAL(526, height, 0.0);
		height = pFile->GetHeight(1198, 0);
		DOUBLES_EQUAL(519, height, 0.0);

		// (0,1) is the start of the 2nd column 
		height = pFile->GetHeight(0, 1);
		DOUBLES_EQUAL(137, height, 0.0);
		height = pFile->GetHeight(1, 1);
		DOUBLES_EQUAL(142, height, 0.0);
		height = pFile->GetHeight(2, 1);
		DOUBLES_EQUAL(144, height, 0.0);

		// (1200,1200) is the SE corner
		height = pFile->GetHeight(1200, 1200);
		DOUBLES_EQUAL(1651, height, 0.0);
		height = pFile->GetHeight(1199, 1200);
		DOUBLES_EQUAL(1653, height, 0.0);
		height = pFile->GetHeight(1198, 1200);
		DOUBLES_EQUAL(1651, height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(1201, 0);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(0, 1201);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		CHECK(pFile->Contains(-122.5, 43.5) == true);
		CHECK(pFile->Contains(-121.5, 44.5) == false);
		CHECK(pFile->Contains(-122.5, 45.5) == false);
		CHECK(pFile->Contains(-120.5, 44.5) == false);

		delete pFile;
	}
}

//----------------------------------------------------------------------------
TEST(RasterFile, Test_W123_N44_DT1)
{
	// Verify reading a DTED level 1 file. These are (up to) 1201x1201.
	//
	// Values here are verified against other geospatial software.

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "w123_n44.dt1");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		CHECK(GIS::GEODATA::DTED1 == pFile->GetFileType());

		// make sure gdal uses the driver we expect
		XString strDriver = pFile->GetDriverName();
		CHECK(strDriver.CompareNoCase("DTED") == true);

		CHECK(pFile->IsGeographic() == true);
		CHECK(pFile->IsProjected() == false);

		LONGS_EQUAL(1201, pFile->GetSizeX());
		LONGS_EQUAL(1201, pFile->GetSizeY());

		RectD ext = pFile->GetExtents();
		DOUBLES_EQUAL(-123.00041666666667,  ext.x0, 0.0);
		DOUBLES_EQUAL(  45.000416666666666, ext.y0, 0.0);
		DOUBLES_EQUAL(-121.99958333333333,  ext.x1, 0.0);
		DOUBLES_EQUAL(  43.999583333333334, ext.y1, 0.0);

		int minHeight = (int)pFile->GetMinElev();
		int maxHeight = (int)pFile->GetMaxElev();

		LONGS_EQUAL(   0, minHeight);
		LONGS_EQUAL(1793, maxHeight);

		// (0,0) is NW corner
		double height = pFile->GetHeight(0, 0);
		DOUBLES_EQUAL(55, height, 0.0);
		height = pFile->GetHeight(1, 0);
		DOUBLES_EQUAL(49, height, 0.0);
		height = pFile->GetHeight(2, 0);
		DOUBLES_EQUAL(49, height, 0.0);

		// (1200,0) is SW corner
		height = pFile->GetHeight(1200, 0);
		DOUBLES_EQUAL(139, height, 0.0);
		height = pFile->GetHeight(1199, 0);
		DOUBLES_EQUAL(140, height, 0.0);
		height = pFile->GetHeight(1198, 0);
		DOUBLES_EQUAL(142, height, 0.0);

		// (0,1) is the start of the 2nd column 
		height = pFile->GetHeight(0, 1);
		DOUBLES_EQUAL(49, height, 0.0);
		height = pFile->GetHeight(1, 1);
		DOUBLES_EQUAL(42, height, 0.0);
		height = pFile->GetHeight(2, 1);
		DOUBLES_EQUAL(39, height, 0.0);

		// (1200,1200) is the SE corner
		height = pFile->GetHeight(1200, 1200);
		DOUBLES_EQUAL(1438, height, 0.0);
		height = pFile->GetHeight(1199, 1200);
		DOUBLES_EQUAL(1437, height, 0.0);
		height = pFile->GetHeight(1198, 1200);
		DOUBLES_EQUAL(1432, height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(1201, 0);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(0, 1201);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		CHECK(pFile->Contains(-122.5, 44.5) == true);
		CHECK(pFile->Contains(-121.5, 44.5) == false);
		CHECK(pFile->Contains(-122.5, 45.5) == false);
		CHECK(pFile->Contains(-120.5, 44.5) == false);

		delete pFile;
	}
}

//----------------------------------------------------------------------------
TEST(RasterFile, Test_W124_N43_DT1)
{
	// Verify reading a DTED level 1 file. These are (up to) 1201x1201.
	//
	// Values here are verified against other geospatial software.

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "w124_n43.dt1");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		CHECK(GIS::GEODATA::DTED1 == pFile->GetFileType());

		// make sure gdal uses the driver we expect
		XString strDriver = pFile->GetDriverName();
		CHECK(strDriver.CompareNoCase("DTED") == true);

		CHECK(pFile->IsGeographic() == true);
		CHECK(pFile->IsProjected() == false);

		LONGS_EQUAL(1201, pFile->GetSizeX());
		LONGS_EQUAL(1201, pFile->GetSizeY());

		RectD ext = pFile->GetExtents();
		DOUBLES_EQUAL(-124.00041666666667,  ext.x0, 0.0);
		DOUBLES_EQUAL(  44.000416666666666, ext.y0, 0.0);
		DOUBLES_EQUAL(-122.99958333333333,  ext.x1, 0.0);
		DOUBLES_EQUAL(  42.999583333333334, ext.y1, 0.0);

		int minHeight = (int)pFile->GetMinElev();
		int maxHeight = (int)pFile->GetMaxElev();
		LONGS_EQUAL(7, minHeight);
		LONGS_EQUAL(1280, maxHeight);

		// (0,0) is NW corner
		double height = pFile->GetHeight(0, 0);
		DOUBLES_EQUAL(40, height, 0.0);
		height = pFile->GetHeight(1, 0);
		DOUBLES_EQUAL(72, height, 0.0);
		height = pFile->GetHeight(2, 0);
		DOUBLES_EQUAL(92, height, 0.0);

		// (1200,0) is SW corner
		height = pFile->GetHeight(1200, 0);
		DOUBLES_EQUAL(55, height, 0.0);
		height = pFile->GetHeight(1199, 0);
		DOUBLES_EQUAL(43, height, 0.0);
		height = pFile->GetHeight(1198, 0);
		DOUBLES_EQUAL(54, height, 0.0);

		// (0,1) is the start of the 2nd column 
		height = pFile->GetHeight(0, 1);
		DOUBLES_EQUAL(46, height, 0.0);
		height = pFile->GetHeight(1, 1);
		DOUBLES_EQUAL(84, height, 0.0);
		height = pFile->GetHeight(2, 1);
		DOUBLES_EQUAL(119, height, 0.0);

		// (1200,1200) is the SE corner
		height = pFile->GetHeight(1200, 1200);
		DOUBLES_EQUAL(525, height, 0.0);
		height = pFile->GetHeight(1199, 1200);
		DOUBLES_EQUAL(526, height, 0.0);
		height = pFile->GetHeight(1198, 1200);
		DOUBLES_EQUAL(519, height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(1201, 0);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(0, 1201);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		CHECK(pFile->Contains(-123.5, 43.5) == true);
		CHECK(pFile->Contains(-121.5, 44.5) == false);
		CHECK(pFile->Contains(-122.5, 45.5) == false);
		CHECK(pFile->Contains(-120.5, 44.5) == false);

		delete pFile;
	}
}

//----------------------------------------------------------------------------
TEST(RasterFile, Test_W124_N44_DT1)
{
	// Verify reading a DTED level 1 file. These are (up to) 1201x1201.
	//
	// Values here are verified against other geospatial software.

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "w124_n44.dt1");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		CHECK(GIS::GEODATA::DTED1 == pFile->GetFileType());

		// make sure gdal uses the driver we expect
		XString strDriver = pFile->GetDriverName();
		CHECK(strDriver.CompareNoCase("DTED") == true);

		CHECK(pFile->IsGeographic() == true);
		CHECK(pFile->IsProjected() == false);

		LONGS_EQUAL(1201, pFile->GetSizeX());
		LONGS_EQUAL(1201, pFile->GetSizeX());

		RectD ext = pFile->GetExtents();
		DOUBLES_EQUAL(-124.00041666666667,  ext.x0, 0.0);
		DOUBLES_EQUAL(  45.000416666666666, ext.y0, 0.0);
		DOUBLES_EQUAL(-122.99958333333333,  ext.x1, 0.0);
		DOUBLES_EQUAL(  43.999583333333334, ext.y1, 0.0);

		int minHeight = (int)pFile->GetMinElev();
		int maxHeight = (int)pFile->GetMaxElev();
		LONGS_EQUAL(   0, minHeight);
		LONGS_EQUAL(1237, maxHeight);

		// (1200,0) is SW corner, the value here is confirmed with other
		// geospatial software, not just init read from class
		double height = pFile->GetHeight(1200, 0);
		DOUBLES_EQUAL(40, height, 0.0);
		height = pFile->GetHeight(1199, 0);
		DOUBLES_EQUAL(16, height, 0.0);
		height = pFile->GetHeight(1198, 0);
		DOUBLES_EQUAL(11, height, 0.0);

		// (0,0) is NW corner, these values verified against other
		// geospatial software
		height = pFile->GetHeight(0, 0);
		DOUBLES_EQUAL(33, height, 0.0);
		height = pFile->GetHeight(1, 0);
		DOUBLES_EQUAL(26, height, 0.0);
		height = pFile->GetHeight(2, 0);
		DOUBLES_EQUAL(24, height, 0.0);
		height = pFile->GetHeight(3, 0);
		DOUBLES_EQUAL(18, height, 0.0);
		height = pFile->GetHeight(4, 0);
		DOUBLES_EQUAL(14, height, 0.0);

		// (0,1) is the start of the 2nd column 
		height = pFile->GetHeight(0, 1);
		DOUBLES_EQUAL(29, height, 0.0);
		height = pFile->GetHeight(1, 1);
		DOUBLES_EQUAL(23, height, 0.0);
		height = pFile->GetHeight(2, 1);
		DOUBLES_EQUAL(21, height, 0.0);

		// (1200,1200) is the SE corner
		height = pFile->GetHeight(1200, 1200);
		DOUBLES_EQUAL(139, height, 0.0);
		height = pFile->GetHeight(1199, 1200);
		DOUBLES_EQUAL(140, height, 0.0);
		height = pFile->GetHeight(1198, 1200);
		DOUBLES_EQUAL(142, height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(1201, 0);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(0, 1201);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		CHECK(pFile->Contains(-123.5, 44.5) == true);
		CHECK(pFile->Contains(-121.5, 44.5) == false);
		CHECK(pFile->Contains(-122.5, 45.5) == false);
		CHECK(pFile->Contains(-120.5, 44.5) == false);

		delete pFile;
	}
}

//----------------------------------------------------------------------------
TEST(RasterFile, Test_W118_N45_DT2)
{
	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "w118_n45.dt2");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		CHECK(GIS::GEODATA::DTED2 == pFile->GetFileType());

		// make sure gdal uses the driver we expect
		XString strDriver = pFile->GetDriverName();
		CHECK(strDriver.CompareNoCase("DTED") == true);

		CHECK(pFile->IsGeographic() == true);
		CHECK(pFile->IsProjected() == false);

		LONGS_EQUAL(3601, pFile->GetSizeX());
		LONGS_EQUAL(3601, pFile->GetSizeY());

		RectD ext = pFile->GetExtents();
		DOUBLES_EQUAL(-118.00013888888888,  ext.x0, 0.0);
		DOUBLES_EQUAL(  46.000138888888884, ext.y0, 0.0);
		DOUBLES_EQUAL(-116.99986111111110,  ext.x1, 0.0);
		DOUBLES_EQUAL(  44.999861111111109, ext.y1, 0.0);

		int minHeight = (int)pFile->GetMinElev();
		int maxHeight = (int)pFile->GetMaxElev();
		LONGS_EQUAL(365, minHeight);
		LONGS_EQUAL(3002, maxHeight);

		// (0,0) is NW corner
		double height = pFile->GetHeight(0, 0);
		DOUBLES_EQUAL(1030, height, 0.0);
		height = pFile->GetHeight(1, 0);
		DOUBLES_EQUAL(1049, height, 0.0);
		height = pFile->GetHeight(2, 0);
		DOUBLES_EQUAL(1069, height, 0.0);

		// (0,1) is the start of the 2nd column 
		height = pFile->GetHeight(0, 1);
		DOUBLES_EQUAL(1024, height, 0.0);
		height = pFile->GetHeight(1, 1);
		DOUBLES_EQUAL(1044, height, 0.0);
		height = pFile->GetHeight(2, 1);
		DOUBLES_EQUAL(1065, height, 0.0);

		// (3600,3600) is the SE corner
		height = pFile->GetHeight(3600, 3600);
		DOUBLES_EQUAL(1577, height, 0.0);
		height = pFile->GetHeight(3599, 3600);
		DOUBLES_EQUAL(1576, height, 0.0);
		height = pFile->GetHeight(3598, 3600);
		DOUBLES_EQUAL(1573, height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(3601, 0);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		// off end, returns "no data"
		height = pFile->GetHeight(0, 3601);
		DOUBLES_EQUAL(pFile->GetNoData(), height, 0.0);

		CHECK(pFile->Contains(-117.5, 45.5) == true);
		CHECK(pFile->Contains(-121.5, 44.5) == false);
		CHECK(pFile->Contains(-122.5, 45.5) == false);
		CHECK(pFile->Contains(-120.5, 44.5) == false);

		delete pFile;
	}
}

//----------------------------------------------------------------------------
TEST(RasterFile, Test_30M)
{
	// Test for fix to "no lat/lon" issue.

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "VT_ANDOVER_30M.DEM");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		XString strDriver = pFile->GetDriverName();

		// make sure gdal uses the driver we expect
		CHECK(strDriver.CompareNoCase("USGSDEM") == true);

		CHECK(GIS::GEODATA::DEM30 == pFile->GetFileType());

		CHECK(pFile->IsProjected() == true);
		CHECK(pFile->IsGeographic() == false);

		LONGS_EQUAL(351, pFile->GetSizeX());
		LONGS_EQUAL(474, pFile->GetSizeY());

		DOUBLES_EQUAL( 30.0, pFile->GetPixelSizeX(), 0.0);
		DOUBLES_EQUAL(-30.0, pFile->GetPixelSizeY(), 0.0);

		double minElev = pFile->GetMinElev();
		double maxElev = pFile->GetMaxElev();
		//DOUBLES_EQUAL(212.0, pFile->GetMinHeightMeters(), 0.0);
		//DOUBLES_EQUAL(862.0, pFile->GetMaxHeightMeters(), 0.0);
		LONGS_EQUAL(18, pFile->GetUtmZone());

		RectD ext = pFile->GetExtents();
		DOUBLES_EQUAL(682275.00000000000, ext.x0, 0.0);
		DOUBLES_EQUAL(4805025.0000000000, ext.y0, 0.0);
		DOUBLES_EQUAL(692805.00000000000, ext.x1, 0.0);
		DOUBLES_EQUAL(4790805.0000000000, ext.y1, 0.0);

		//LONGS_EQUAL(774, pFile->GetHeight(0, 0));
		//LONGS_EQUAL(-9999, pFile->GetHeight(350, 0));
		//LONGS_EQUAL(690, pFile->GetHeight(1, 1));
		//LONGS_EQUAL(679, pFile->GetHeight(2, 2));

		delete pFile;
	}
}

//----------------------------------------------------------------------------
TEST(RasterFile, Test_Ella_10M)
{
	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "OR_ELLA_10M.DEM");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		CHECK(GIS::GEODATA::DEM10 == pFile->GetFileType());
		//CHECK(XString(pFile->GetLocationName()).Compare("ELLA"));

		// make sure gdal uses the driver we expect
		XString strDriver = pFile->GetDriverName();
		CHECK(strDriver.CompareNoCase("USGSDEM") == true);

		LONGS_EQUAL(1022, pFile->GetSizeX());
		LONGS_EQUAL(1425, pFile->GetSizeY());

		DOUBLES_EQUAL( 10.0, pFile->GetPixelSizeX(), 0.0);
		DOUBLES_EQUAL(-10.0, pFile->GetPixelSizeY(), 0.0);

		DOUBLES_EQUAL(149.8, pFile->GetMinElev(), 0.01);
		DOUBLES_EQUAL(307.3, pFile->GetMaxElev(), 0.01);
		LONGS_EQUAL(11, pFile->GetUtmZone());

		RectD ext = pFile->GetExtents();
		DOUBLES_EQUAL( 275875.0, ext.x0, 0.0);
		DOUBLES_EQUAL(5070085.0, ext.y0, 0.0);
		DOUBLES_EQUAL( 286095.0, ext.x1, 0.0);
		DOUBLES_EQUAL(5055835.0, ext.y1, 0.0);

		double noData = pFile->GetNoData();
		DOUBLES_EQUAL(noData, pFile->GetHeight(0, 0), 0.0);
		DOUBLES_EQUAL(noData, pFile->GetHeight(1, 1), 0.0);
		DOUBLES_EQUAL(noData, pFile->GetHeight(2, 2), 0.0);

		// these are valude
		double height = pFile->GetHeight(ext.x0, ext.y0);
		DOUBLES_EQUAL(noData, height, 0.0);
		height = pFile->GetHeight(ext.x0 + 500.0*pFile->GetPixelSizeX(), ext.y0 + 500.0*pFile->GetPixelSizeY());
		DOUBLES_EQUAL(181.3, height, 0.01);

		// this one is off the edge
		height = pFile->GetHeight(ext.x0 - 1000.0, ext.y0 + 1000.0);
		DOUBLES_EQUAL(noData, height, 0.0);

		delete pFile;
	}
}

//----------------------------------------------------------------------------
TEST(RasterFile, Test_Lyle_10M)
{
	// Elevations stored internally in meters (scale by 0.1).
	//

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "WA_LYLE_10M.DEM");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		CHECK(GIS::GEODATA::DEM10 == pFile->GetFileType());
		//CHECK(XString(pFile->GetLocationName()).Compare("LYLE"));

		// make sure gdal uses the driver we expect
		XString strDriver = pFile->GetDriverName();
		CHECK(strDriver.CompareNoCase("USGSDEM") == true);

		LONGS_EQUAL(1003, pFile->GetSizeX());
		LONGS_EQUAL(1411, pFile->GetSizeY());

		DOUBLES_EQUAL( 10.0, pFile->GetPixelSizeX(), 0.0);
		DOUBLES_EQUAL(-10.0, pFile->GetPixelSizeY(), 0.0);

		DOUBLES_EQUAL(20.0, pFile->GetMinElev(), 0.0);
		DOUBLES_EQUAL(662.50, pFile->GetMaxElev(), 0.0);
		LONGS_EQUAL(10, pFile->GetUtmZone());

		RectD ext = pFile->GetExtents();
		DOUBLES_EQUAL( 626395.0, ext.x0, 0.0);
		DOUBLES_EQUAL(5067555.0, ext.y0, 0.0);
		DOUBLES_EQUAL( 636425.0, ext.x1, 0.0);
		DOUBLES_EQUAL(5053445.0, ext.y1, 0.0);

		DOUBLES_EQUAL(pFile->GetNoData(), pFile->GetHeight(0, 0), 0.0);
		DOUBLES_EQUAL(pFile->GetNoData(), pFile->GetHeight(1, 1), 0.0);
		DOUBLES_EQUAL(pFile->GetNoData(), pFile->GetHeight(2, 2), 0.0);

		double height = pFile->GetHeight(ext.x0 + 500.0*pFile->GetPixelSizeX(), ext.y0 + 500.0*pFile->GetPixelSizeY());;
		DOUBLES_EQUAL(70.0, height, 0.001);
		height = pFile->GetHeight(ext.x0 + 510.0*pFile->GetPixelSizeX(), ext.y0 + 500.0*pFile->GetPixelSizeY());;
		DOUBLES_EQUAL(100.5, height, 0.001);

		delete pFile;
	}
}

//----------------------------------------------------------------------------
TEST(RasterFile, Test_BridalVeil_10M)
{
	// Min/max elevation stored in feet, not meters.
	//

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "OR_BRIDAL_VEIL_10M.DEM");

	{
		RasterFile* pFile = new RasterFile(fileName.c_str(), true);

		CHECK(GIS::GEODATA::DEM10 == pFile->GetFileType());
		//CHECK(XString(pFile->GetLocationName()).Compare("BRIDAL VEIL"));

		// make sure gdal uses the driver we expect
		XString strDriver = pFile->GetDriverName();
		CHECK(strDriver.CompareNoCase("USGSDEM") == true);

		LONGS_EQUAL( 990, pFile->GetSizeX());
		LONGS_EQUAL(1400, pFile->GetSizeY());

		DOUBLES_EQUAL( 10.0, pFile->GetPixelSizeX(), 0.0);
		DOUBLES_EQUAL(-10.0, pFile->GetPixelSizeY(), 0.0);

		DOUBLES_EQUAL(0.0, pFile->GetMinElev(), 0.0);
		// 2431 feet, 740.96879887372734 meters
		LONGS_EQUAL(2431, (int)pFile->GetMaxElev());

		LONGS_EQUAL(10, pFile->GetUtmZone());

		RectD ext = pFile->GetExtents();
		DOUBLES_EQUAL( 558465.0, ext.x0, 0.0);
		DOUBLES_EQUAL(5052545.0, ext.y0, 0.0);
		DOUBLES_EQUAL( 568365.0, ext.x1, 0.0);
		DOUBLES_EQUAL(5038545.0, ext.y1, 0.0);

		delete pFile;
	}
}
