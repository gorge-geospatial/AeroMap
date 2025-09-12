// TestLasFile.cpp
// CppUnitLite test harness for LasFile class
//
// Implemented Tests:
//
//		Version_10_0
//		Version_10_1
//
//		Version_11_1
//
//		Version_12_1
//		Version_12_2
//		Version_12_3
//		Version_12_4
//
//		Version_13_5
//
//		Version_14_6
//		Version_14_7
//		Version_14_8
//		Version_14_9
//		Version_14_10
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WIN32
#include <direct.h>
#endif

#include "CppUnitLite/TestHarness.h"
#include "../Common/UnitTest.h"			// unit test helpers
#include "LasFile.h"					// interface to class under test

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
TEST(LasFile, Read_Version_10_0)
{
	// Read test for las/laz files, version 1.0, point format 0.
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "srs-1.0_0.las");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAS);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(0, lasFile.GetVersionMinor());

		LONGS_EQUAL(0, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		//CHECK_EQUAL(XString("las2las (version 190507)"), XString(lasFile.GetGenSoftware()));
		CHECK_EQUAL(XString("LAStools (c) by rapidlasso GmbH"), XString(lasFile.GetSystemID()));

		LONGS_EQUAL(0, lasFile.GetCreateDay());
		LONGS_EQUAL(0, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(289814.15, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(289818.50, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(4320978.61, lasFile.GetMinY(), 0.0);
		DOUBLES_EQUAL(4320980.59, lasFile.GetMaxY(), 0.0);
		DOUBLES_EQUAL(170.58, lasFile.GetMinZ(), 0.0);
		DOUBLES_EQUAL(170.76, lasFile.GetMaxZ(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);

		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		
		CHECK(XString(varRec.Desc).Compare(""));
		LONGS_EQUAL(72, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		CHECK(XString(varRec.Desc).Compare(""));
		LONGS_EQUAL(40, varRec.LenAfterHdr);
		LONGS_EQUAL(34736, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		CHECK(XString(varRec.Desc).Compare(""));
		LONGS_EQUAL(256, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));

		// Point Records

		CHECK(lasFile.GetPointCount() == 10);

		LasFile::PointRecType_0 pt = lasFile.GetPointRec_0(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(28981415, pt.X);
		LONGS_EQUAL(432097861, pt.Y);
		LONGS_EQUAL(17076, pt.Z);
		LONGS_EQUAL(260, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.FileMarker);
		LONGS_EQUAL(0, pt.UserBitField);

		pt = lasFile.GetPointRec_0(1);
		LONGS_EQUAL(28981464, pt.X);
		LONGS_EQUAL(432097884, pt.Y);
		LONGS_EQUAL(17076, pt.Z);
		LONGS_EQUAL(280, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.FileMarker);
		LONGS_EQUAL(0, pt.UserBitField);

		pt = lasFile.GetPointRec_0(2);
		LONGS_EQUAL(28981512, pt.X);
		LONGS_EQUAL(432097906, pt.Y);
		LONGS_EQUAL(17075, pt.Z);
		LONGS_EQUAL(280, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.FileMarker);
		LONGS_EQUAL(0, pt.UserBitField);

		pt = lasFile.GetPointRec_0(9);
		LONGS_EQUAL(28981850, pt.X);
		LONGS_EQUAL(432098059, pt.Y);
		LONGS_EQUAL(17058, pt.Z);
		LONGS_EQUAL(260, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.FileMarker);
		LONGS_EQUAL(0, pt.UserBitField);
	}

	{
		// test compressed version of same file

		XString fileName = XString::CombinePath(gs_DataPath, "srs-1.0_0.laz");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAZ);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(0, lasFile.GetVersionMinor());

		LONGS_EQUAL(0, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		//CHECK_EQUAL(XString("las2las (version 190507)"), XString(lasFile.GetGenSoftware()));
		CHECK_EQUAL(XString("LAStools (c) by rapidlasso GmbH"), XString(lasFile.GetSystemID()));

		LONGS_EQUAL(0, lasFile.GetCreateDay());
		LONGS_EQUAL(0, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(289814.15, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(289818.50, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(4320978.61, lasFile.GetMinY(), 0.0);
		DOUBLES_EQUAL(4320980.59, lasFile.GetMaxY(), 0.0);
		DOUBLES_EQUAL(170.58, lasFile.GetMinZ(), 0.0);
		DOUBLES_EQUAL(170.76, lasFile.GetMaxZ(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);

		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);

		CHECK(XString(varRec.Desc).Compare(""));
		LONGS_EQUAL(72, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(0xAABB, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		CHECK(XString(varRec.Desc).Compare(""));
		LONGS_EQUAL(40, varRec.LenAfterHdr);
		LONGS_EQUAL(34736, varRec.RecordID);
		LONGS_EQUAL(0xAABB, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		CHECK(XString(varRec.Desc).Compare(""));
		LONGS_EQUAL(256, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(0xAABB, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));

		// Point Records

		CHECK(lasFile.GetPointCount() == 10);

		LasFile::PointRecType_0 pt = lasFile.GetPointRec_0(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(28981415, pt.X);
		LONGS_EQUAL(432097861, pt.Y);
		LONGS_EQUAL(17076, pt.Z);
		LONGS_EQUAL(260, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.FileMarker);
		LONGS_EQUAL(0, pt.UserBitField);

		pt = lasFile.GetPointRec_0(1);
		LONGS_EQUAL(28981464, pt.X);
		LONGS_EQUAL(432097884, pt.Y);
		LONGS_EQUAL(17076, pt.Z);
		LONGS_EQUAL(280, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.FileMarker);
		LONGS_EQUAL(0, pt.UserBitField);

		pt = lasFile.GetPointRec_0(2);
		LONGS_EQUAL(28981512, pt.X);
		LONGS_EQUAL(432097906, pt.Y);
		LONGS_EQUAL(17075, pt.Z);
		LONGS_EQUAL(280, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.FileMarker);
		LONGS_EQUAL(0, pt.UserBitField);

		pt = lasFile.GetPointRec_0(9);
		LONGS_EQUAL(28981850, pt.X);
		LONGS_EQUAL(432098059, pt.Y);
		LONGS_EQUAL(17058, pt.Z);
		LONGS_EQUAL(260, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.FileMarker);
		LONGS_EQUAL(0, pt.UserBitField);
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Version_10_1)
{
	// Test for version 1.0, point format 1, las file
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "srs-1.0_1.las");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAS);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(0, lasFile.GetVersionMinor());

		LONGS_EQUAL(1, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString str = lasFile.GetGenSoftware();
		CHECK(str.Compare("TerraScan"));
		str = lasFile.GetSystemID();
		CHECK(str.Compare("MODIFIED"));

		LONGS_EQUAL(0, lasFile.GetCreateDay());
		LONGS_EQUAL(0, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(289814.15, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(289818.50, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(4320978.61, lasFile.GetMinY(), 0.0);
		DOUBLES_EQUAL(4320980.59, lasFile.GetMaxY(), 0.0);
		DOUBLES_EQUAL(170.58, lasFile.GetMinZ(), 0.0);
		DOUBLES_EQUAL(170.76, lasFile.GetMaxZ(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);

		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		str = varRec.Desc;
		CHECK(str.Compare(""));
		LONGS_EQUAL(72, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare(""));
		LONGS_EQUAL(40, varRec.LenAfterHdr);
		LONGS_EQUAL(34736, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare(""));
		LONGS_EQUAL(256, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));


		// Point Records

		CHECK(lasFile.GetPointCount() == 10);

		LasFile::PointRecType_1 pt = lasFile.GetPointRec_1(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(28981415, pt.X);
		LONGS_EQUAL(432097861, pt.Y);
		LONGS_EQUAL(17076, pt.Z);
		LONGS_EQUAL(260, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.UserData);
		LONGS_EQUAL(0, pt.PointSourceID);
		DOUBLES_EQUAL(499450.80599405419, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(1);
		LONGS_EQUAL(28981464, pt.X);
		LONGS_EQUAL(432097884, pt.Y);
		LONGS_EQUAL(17076, pt.Z);
		LONGS_EQUAL(280, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.UserData);
		LONGS_EQUAL(0, pt.PointSourceID);
		DOUBLES_EQUAL(499450.80600805435, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(2);
		LONGS_EQUAL(28981512, pt.X);
		LONGS_EQUAL(432097906, pt.Y);
		LONGS_EQUAL(17075, pt.Z);
		LONGS_EQUAL(280, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.UserData);
		LONGS_EQUAL(0, pt.PointSourceID);
		DOUBLES_EQUAL(499450.80602205446, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(9);
		LONGS_EQUAL(28981850, pt.X);
		LONGS_EQUAL(432098059, pt.Y);
		LONGS_EQUAL(17058, pt.Z);
		LONGS_EQUAL(260, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.UserData);
		LONGS_EQUAL(0, pt.PointSourceID);
		DOUBLES_EQUAL(499450.80612005544, pt.GPSTime, 0.0);
	}

	{
		// test compressed version of same file
		XString fileName = XString::CombinePath(gs_DataPath, "srs-1.0_1.laz");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAZ);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(0, lasFile.GetVersionMinor());

		LONGS_EQUAL(1, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString str = lasFile.GetGenSoftware();
		CHECK(str.Compare("TerraScan"));
		str = lasFile.GetSystemID();
		CHECK(str.Compare("MODIFIED"));

		LONGS_EQUAL(0, lasFile.GetCreateDay());
		LONGS_EQUAL(0, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(289814.15, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(289818.50, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(4320978.61, lasFile.GetMinY(), 0.0);
		DOUBLES_EQUAL(4320980.59, lasFile.GetMaxY(), 0.0);
		DOUBLES_EQUAL(170.58, lasFile.GetMinZ(), 0.0);
		DOUBLES_EQUAL(170.76, lasFile.GetMaxZ(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		str = varRec.Desc;
		CHECK(str.Compare(""));
		LONGS_EQUAL(72, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare(""));
		LONGS_EQUAL(40, varRec.LenAfterHdr);
		LONGS_EQUAL(34736, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare(""));
		LONGS_EQUAL(256, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));


		// Point Records

		CHECK(lasFile.GetPointCount() == 10);

		LasFile::PointRecType_1 pt = lasFile.GetPointRec_1(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(28981415, pt.X);
		LONGS_EQUAL(432097861, pt.Y);
		LONGS_EQUAL(17076, pt.Z);
		LONGS_EQUAL(260, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.UserData);
		LONGS_EQUAL(0, pt.PointSourceID);
		DOUBLES_EQUAL(499450.80599405419, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(1);
		LONGS_EQUAL(28981464, pt.X);
		LONGS_EQUAL(432097884, pt.Y);
		LONGS_EQUAL(17076, pt.Z);
		LONGS_EQUAL(280, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.UserData);
		LONGS_EQUAL(0, pt.PointSourceID);
		DOUBLES_EQUAL(499450.80600805435, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(2);
		LONGS_EQUAL(28981512, pt.X);
		LONGS_EQUAL(432097906, pt.Y);
		LONGS_EQUAL(17075, pt.Z);
		LONGS_EQUAL(280, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.UserData);
		LONGS_EQUAL(0, pt.PointSourceID);
		DOUBLES_EQUAL(499450.80602205446, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(9);
		LONGS_EQUAL(28981850, pt.X);
		LONGS_EQUAL(432098059, pt.Y);
		LONGS_EQUAL(17058, pt.Z);
		LONGS_EQUAL(260, pt.Intensity);
		LONGS_EQUAL(48, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(0, pt.ScanAngle);
		LONGS_EQUAL(0, pt.UserData);
		LONGS_EQUAL(0, pt.PointSourceID);
		DOUBLES_EQUAL(499450.80612005544, pt.GPSTime, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Version_11_1)
{
	// Test for version 1.1, point format 1, las file
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "fusa-1.1_1.las");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAS);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(1, lasFile.GetVersionMinor());

		LONGS_EQUAL(1, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString str = lasFile.GetGenSoftware();
		CHECK(str.Compare("lasclassify (110920) unlicensed"));
		str = lasFile.GetSystemID();
		CHECK(str.Compare("LAStools (c) by Martin Isenburg"));

		LONGS_EQUAL(40, lasFile.GetCreateDay());
		LONGS_EQUAL(2010, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(277750.00, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(277999.99, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(6122250.00, lasFile.GetMinY(), 0.0);
		DOUBLES_EQUAL(6122499.99, lasFile.GetMaxY(), 0.0);
		DOUBLES_EQUAL(42.21, lasFile.GetMinZ(), 0.0);
		DOUBLES_EQUAL(64.35, lasFile.GetMaxZ(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);

		// Variable Length Records

		LONGS_EQUAL(1, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		str = varRec.Desc;
		CHECK(str.Compare("by LAStools of Martin Isenburg"));
		LONGS_EQUAL(40, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		// Point Records

		CHECK(lasFile.GetPointCount() == 277573);

		LasFile::PointRecType_1 pt = lasFile.GetPointRec_1(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(27799997, pt.X);
		LONGS_EQUAL(612234220, pt.Y);
		LONGS_EQUAL(6435, pt.Z);
		LONGS_EQUAL(10, pt.Intensity);
		LONGS_EQUAL(17, pt.ReturnMask);
		LONGS_EQUAL(5, pt.Classification);
		LONGS_EQUAL(89, pt.ScanAngle);
		LONGS_EQUAL(145, pt.UserData);
		LONGS_EQUAL(1, pt.PointSourceID);
		DOUBLES_EQUAL(5880.9630280000001, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(1);
		LONGS_EQUAL(27799997, pt.X);
		LONGS_EQUAL(612234253, pt.Y);
		LONGS_EQUAL(6430, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(9, pt.ReturnMask);
		LONGS_EQUAL(5, pt.Classification);
		LONGS_EQUAL(89, pt.ScanAngle);
		LONGS_EQUAL(144, pt.UserData);
		LONGS_EQUAL(1, pt.PointSourceID);
		DOUBLES_EQUAL(5880.9630319999997, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(277572);
		LONGS_EQUAL(27775001, pt.X);
		LONGS_EQUAL(612225418, pt.Y);
		LONGS_EQUAL(4243, pt.Z);
		LONGS_EQUAL(35, pt.Intensity);
		LONGS_EQUAL(9, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(80, pt.ScanAngle);
		LONGS_EQUAL(0, pt.UserData);
		LONGS_EQUAL(1, pt.PointSourceID);
		DOUBLES_EQUAL(5886.7397380000002, pt.GPSTime, 0.0);
	}

	{
		// test compressed version of same file

		XString fileName = XString::CombinePath(gs_DataPath, "fusa-1.1_1.laz");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAZ);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(1, lasFile.GetVersionMinor());

		LONGS_EQUAL(1, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString str = lasFile.GetGenSoftware();
		CHECK(str.Compare("lasclassify (110920) unlicensed"));
		str = lasFile.GetSystemID();
		CHECK(str.Compare("LAStools (c) by Martin Isenburg"));

		LONGS_EQUAL(40, lasFile.GetCreateDay());
		LONGS_EQUAL(2010, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(277750.00, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(277999.99, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(6122250.00, lasFile.GetMinY(), 0.0);
		DOUBLES_EQUAL(6122499.99, lasFile.GetMaxY(), 0.0);
		DOUBLES_EQUAL(42.21, lasFile.GetMinZ(), 0.0);
		DOUBLES_EQUAL(64.35, lasFile.GetMaxZ(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);

		// Variable Length Records

		LONGS_EQUAL(1, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		str = varRec.Desc;
		CHECK(str.Compare("by LAStools of Martin Isenburg"));
		LONGS_EQUAL(40, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		// Point Records

		CHECK(lasFile.GetPointCount() == 277573);

		LasFile::PointRecType_1 pt = lasFile.GetPointRec_1(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(27799997, pt.X);
		LONGS_EQUAL(612234220, pt.Y);
		LONGS_EQUAL(6435, pt.Z);
		LONGS_EQUAL(10, pt.Intensity);
		LONGS_EQUAL(17, pt.ReturnMask);
		LONGS_EQUAL(5, pt.Classification);
		LONGS_EQUAL(89, pt.ScanAngle);
		LONGS_EQUAL(145, pt.UserData);
		LONGS_EQUAL(1, pt.PointSourceID);
		DOUBLES_EQUAL(5880.9630280000001, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(1);
		LONGS_EQUAL(27799997, pt.X);
		LONGS_EQUAL(612234253, pt.Y);
		LONGS_EQUAL(6430, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(9, pt.ReturnMask);
		LONGS_EQUAL(5, pt.Classification);
		LONGS_EQUAL(89, pt.ScanAngle);
		LONGS_EQUAL(144, pt.UserData);
		LONGS_EQUAL(1, pt.PointSourceID);
		DOUBLES_EQUAL(5880.9630319999997, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(277572);
		LONGS_EQUAL(27775001, pt.X);
		LONGS_EQUAL(612225418, pt.Y);
		LONGS_EQUAL(4243, pt.Z);
		LONGS_EQUAL(35, pt.Intensity);
		LONGS_EQUAL(9, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(80, pt.ScanAngle);
		LONGS_EQUAL(0, pt.UserData);
		LONGS_EQUAL(1, pt.PointSourceID);
		DOUBLES_EQUAL(5886.7397380000002, pt.GPSTime, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Version_12_1)
{
	// Test for version 1.2, point format 1, las file
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "points-1.2_1.las");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAS);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(2, lasFile.GetVersionMinor());

		LONGS_EQUAL(1, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString strSystemID = lasFile.GetSystemID();
		XString strGenSoftware = lasFile.GetGenSoftware();
		CHECK(strSystemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(strGenSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000, lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		CHECK(XString(varRec.Desc).Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		CHECK(XString(varRec.Desc).Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		CHECK(XString(varRec.Desc).Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_1 pt = lasFile.GetPointRec_1(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
		LONGS_EQUAL(54, pt.Intensity);
		LONGS_EQUAL(90, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(1);
		LONGS_EQUAL(96730291, pt.X);
		LONGS_EQUAL(143876843, pt.Y);
		LONGS_EQUAL(14383, pt.Z);
		LONGS_EQUAL(9, pt.Intensity);
		LONGS_EQUAL(89, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(2);
		LONGS_EQUAL(96730284, pt.X);
		LONGS_EQUAL(143876718, pt.Y);
		LONGS_EQUAL(14446, pt.Z);
		LONGS_EQUAL(35, pt.Intensity);
		LONGS_EQUAL(89, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(199, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905186073738, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(73, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-10, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}

	{
		// test compressed version of same file

		XString fileName = XString::CombinePath(gs_DataPath, "points-1.2_1.laz");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAZ);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(2, lasFile.GetVersionMinor());

		LONGS_EQUAL(1, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString strSystemID = lasFile.GetSystemID();
		XString strGenSoftware = lasFile.GetGenSoftware();
		CHECK(strSystemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(strGenSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000, lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		CHECK(XString(varRec.Desc).Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		CHECK(XString(varRec.Desc).Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		CHECK(XString(varRec.Desc).Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_1 pt = lasFile.GetPointRec_1(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
		LONGS_EQUAL(54, pt.Intensity);
		LONGS_EQUAL(90, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(1);
		LONGS_EQUAL(96730291, pt.X);
		LONGS_EQUAL(143876843, pt.Y);
		LONGS_EQUAL(14383, pt.Z);
		LONGS_EQUAL(9, pt.Intensity);
		LONGS_EQUAL(89, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(2);
		LONGS_EQUAL(96730284, pt.X);
		LONGS_EQUAL(143876718, pt.Y);
		LONGS_EQUAL(14446, pt.Z);
		LONGS_EQUAL(35, pt.Intensity);
		LONGS_EQUAL(89, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(199, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905186073738, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_1(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(73, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-10, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Version_12_2)
{
	// Test for version 1.2, point format 2, las file
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "1.2_2.las");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAS);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(2, lasFile.GetVersionMinor());

		LONGS_EQUAL(2, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString str = lasFile.GetGenSoftware();
		CHECK(str.Compare("libLAS 1.2"));
		str = lasFile.GetSystemID();
		CHECK(str.Compare("libLAS"));

		LONGS_EQUAL(78, lasFile.GetCreateDay());
		LONGS_EQUAL(2008, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(470692.44753800001, lasFile.GetMinX(), 1E-12);
		DOUBLES_EQUAL(470692.44753800001, lasFile.GetMaxX(), 1E-12);
		DOUBLES_EQUAL(4602888.9046419999, lasFile.GetMinY(), 1E-12);
		DOUBLES_EQUAL(4602888.9046419999, lasFile.GetMaxY(), 1E-12);
		DOUBLES_EQUAL(16.000000000000000, lasFile.GetMinZ(), 1E-12);
		DOUBLES_EQUAL(16.000000000000000, lasFile.GetMaxZ(), 1E-12);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(2, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		str = varRec.Desc;
		CHECK(str.Compare(""));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(0, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare(""));
		LONGS_EQUAL(39, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(0, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));


		// Point Records

		CHECK(lasFile.GetPointCount() == 1);

		LasFile::PointRecType_2 pt = lasFile.GetPointRec_2(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(47069244, pt.X);
		LONGS_EQUAL(460288890, pt.Y);
		LONGS_EQUAL(1600, pt.Z);
		LONGS_EQUAL(0, pt.Intensity);
		LONGS_EQUAL(2, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-13, pt.ScanAngle);
		LONGS_EQUAL(0, pt.UserData);
		LONGS_EQUAL(0, pt.PointSourceID);
		LONGS_EQUAL(255, pt.Red);
		LONGS_EQUAL(12, pt.Green);
		LONGS_EQUAL(234, pt.Blue);
	}

	{
		// test compressed version of same file

		XString fileName = XString::CombinePath(gs_DataPath, "1.2_2.laz");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAZ);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(2, lasFile.GetVersionMinor());

		LONGS_EQUAL(2, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString str = lasFile.GetGenSoftware();
		CHECK(str.Compare("libLAS 1.2"));
		str = lasFile.GetSystemID();
		CHECK(str.Compare("libLAS"));

		LONGS_EQUAL(78, lasFile.GetCreateDay());
		LONGS_EQUAL(2008, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(470692.44753800001, lasFile.GetMinX(), 1E-12);
		DOUBLES_EQUAL(470692.44753800001, lasFile.GetMaxX(), 1E-12);
		DOUBLES_EQUAL(4602888.9046419999, lasFile.GetMinY(), 1E-12);
		DOUBLES_EQUAL(4602888.9046419999, lasFile.GetMaxY(), 1E-12);
		DOUBLES_EQUAL(16.000000000000000, lasFile.GetMinZ(), 1E-12);
		DOUBLES_EQUAL(16.000000000000000, lasFile.GetMaxZ(), 1E-12);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(2, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		str = varRec.Desc;
		CHECK(str.Compare(""));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(0, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare(""));
		LONGS_EQUAL(39, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(0, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));


		// Point Records

		CHECK(lasFile.GetPointCount() == 1);

		LasFile::PointRecType_2 pt = lasFile.GetPointRec_2(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(47069244, pt.X);
		LONGS_EQUAL(460288890, pt.Y);
		LONGS_EQUAL(1600, pt.Z);
		LONGS_EQUAL(0, pt.Intensity);
		LONGS_EQUAL(2, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-13, pt.ScanAngle);
		LONGS_EQUAL(0, pt.UserData);
		LONGS_EQUAL(0, pt.PointSourceID);
		LONGS_EQUAL(255, pt.Red);
		LONGS_EQUAL(12, pt.Green);
		LONGS_EQUAL(234, pt.Blue);
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Version_12_3)
{
	// Test for version 1.2, point format 3, las file
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "100-points-1.2_3.las");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAS);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(2, lasFile.GetVersionMinor());

		LONGS_EQUAL(3, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString str = lasFile.GetGenSoftware();
		CHECK(str.Compare("PDAL 1.5.0 (284af8)"));
		str = lasFile.GetSystemID();
		CHECK(str.Compare("PDAL"));

		LONGS_EQUAL(114, lasFile.GetCreateDay());
		LONGS_EQUAL(2017, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());
		LONGS_EQUAL(0, lasFile.GetVarRecCount());

		// Min/Max are scaled values
		DOUBLES_EQUAL(635717.85, lasFile.GetMinX(), 1E-9);
		DOUBLES_EQUAL(638944.95, lasFile.GetMaxX(), 1E-9);
		DOUBLES_EQUAL(848953.74, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(853483.30, lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(409.19, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(530.61, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);

		// Point Records

		CHECK(lasFile.GetPointCount() == 100);

		LasFile::PointRecType_3 pt = lasFile.GetPointRec_3(0);

		// point XYZ are unscaled integer values
		LONGS_EQUAL(63678232, pt.X);
		LONGS_EQUAL(84904318, pt.Y);
		LONGS_EQUAL(42641, pt.Z);
		LONGS_EQUAL(157, pt.Intensity);
		LONGS_EQUAL(73, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-8, pt.ScanAngle);
		LONGS_EQUAL(124, pt.UserData);
		LONGS_EQUAL(7327, pt.PointSourceID);
		DOUBLES_EQUAL(246100.22682092362, pt.GPSTime, 0.0);
		LONGS_EQUAL(140, pt.Red);
		LONGS_EQUAL(104, pt.Green);
		LONGS_EQUAL(132, pt.Blue);

		pt = lasFile.GetPointRec_3(99);
		LONGS_EQUAL(63773891, pt.X);
		LONGS_EQUAL(85333488, pt.Y);
		LONGS_EQUAL(42106, pt.Z);
		LONGS_EQUAL(105, pt.Intensity);
		LONGS_EQUAL(18, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(7, pt.ScanAngle);
		LONGS_EQUAL(124, pt.UserData);
		LONGS_EQUAL(7334, pt.PointSourceID);
		DOUBLES_EQUAL(249771.25198606169, pt.GPSTime, 0.0);
		LONGS_EQUAL(99, pt.Red);
		LONGS_EQUAL(103, pt.Green);
		LONGS_EQUAL(116, pt.Blue);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath, "100-points-1.2_3.laz");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAZ);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(2, lasFile.GetVersionMinor());

		LONGS_EQUAL(3, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString str = lasFile.GetGenSoftware();
		CHECK(str.Compare("PDAL 1.5.0 (284af8)"));
		str = lasFile.GetSystemID();
		CHECK(str.Compare("PDAL"));

		LONGS_EQUAL(114, lasFile.GetCreateDay());
		LONGS_EQUAL(2017, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());
		LONGS_EQUAL(0, lasFile.GetVarRecCount());

		// Min/Max are scaled values
		DOUBLES_EQUAL(635717.85, lasFile.GetMinX(), 1E-9);
		DOUBLES_EQUAL(638944.95, lasFile.GetMaxX(), 1E-9);
		DOUBLES_EQUAL(848953.74, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(853483.30, lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(409.19, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(530.61, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);

		// Point Records

		CHECK(lasFile.GetPointCount() == 100);

		LasFile::PointRecType_3 pt = lasFile.GetPointRec_3(0);

		// point XYZ are unscaled integer values
		LONGS_EQUAL(63678232, pt.X);
		LONGS_EQUAL(84904318, pt.Y);
		LONGS_EQUAL(42641, pt.Z);
		LONGS_EQUAL(157, pt.Intensity);
		LONGS_EQUAL(73, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-8, pt.ScanAngle);
		LONGS_EQUAL(124, pt.UserData);
		LONGS_EQUAL(7327, pt.PointSourceID);
		DOUBLES_EQUAL(246100.22682092362, pt.GPSTime, 0.0);
		LONGS_EQUAL(140, pt.Red);
		LONGS_EQUAL(104, pt.Green);
		LONGS_EQUAL(132, pt.Blue);

		pt = lasFile.GetPointRec_3(99);
		LONGS_EQUAL(63773891, pt.X);
		LONGS_EQUAL(85333488, pt.Y);
		LONGS_EQUAL(42106, pt.Z);
		LONGS_EQUAL(105, pt.Intensity);
		LONGS_EQUAL(18, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(7, pt.ScanAngle);
		LONGS_EQUAL(124, pt.UserData);
		LONGS_EQUAL(7334, pt.PointSourceID);
		DOUBLES_EQUAL(249771.25198606169, pt.GPSTime, 0.0);
		LONGS_EQUAL(99, pt.Red);
		LONGS_EQUAL(103, pt.Green);
		LONGS_EQUAL(116, pt.Blue);
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Version_12_4)
{
	// Test for version 1.2, point format 4, las file
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "points-1.2_4.las");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAS);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(2, lasFile.GetVersionMinor());

		LONGS_EQUAL(4, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString strSystemID = lasFile.GetSystemID();
		XString strGenSoftware = lasFile.GetGenSoftware();
		CHECK(strSystemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(strGenSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000,     lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		CHECK(XString(varRec.Desc).Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		CHECK(XString(varRec.Desc).Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		CHECK(XString(varRec.Desc).Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_4 pt = lasFile.GetPointRec_4(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
		LONGS_EQUAL(54, pt.Intensity);
		LONGS_EQUAL(90, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_4(1);
		LONGS_EQUAL(96730291, pt.X);
		LONGS_EQUAL(143876843, pt.Y);
		LONGS_EQUAL(14383, pt.Z);
		LONGS_EQUAL(9, pt.Intensity);
		LONGS_EQUAL(89, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_4(2);
		LONGS_EQUAL(96730284, pt.X);
		LONGS_EQUAL(143876718, pt.Y);
		LONGS_EQUAL(14446, pt.Z);
		LONGS_EQUAL(35, pt.Intensity);
		LONGS_EQUAL(89, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(199, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905186073738, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_4(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(73, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-10, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}

	{
		// test compressed version of same file

		XString fileName = XString::CombinePath(gs_DataPath, "points-1.2_4.laz");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAZ);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(2, lasFile.GetVersionMinor());

		LONGS_EQUAL(4, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString strSystemID = lasFile.GetSystemID();
		XString strGenSoftware = lasFile.GetGenSoftware();
		CHECK(strSystemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(strGenSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(227, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000, lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		XString str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare("OGR variant of OpenGIS WKT SRS"));
		LONGS_EQUAL(697, varRec.LenAfterHdr);
		LONGS_EQUAL(2112, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("liblas"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_4 pt = lasFile.GetPointRec_4(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
		LONGS_EQUAL(54, pt.Intensity);
		LONGS_EQUAL(90, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_4(1);
		LONGS_EQUAL(96730291, pt.X);
		LONGS_EQUAL(143876843, pt.Y);
		LONGS_EQUAL(14383, pt.Z);
		LONGS_EQUAL(9, pt.Intensity);
		LONGS_EQUAL(89, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_4(2);
		LONGS_EQUAL(96730284, pt.X);
		LONGS_EQUAL(143876718, pt.Y);
		LONGS_EQUAL(14446, pt.Z);
		LONGS_EQUAL(35, pt.Intensity);
		LONGS_EQUAL(89, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(199, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905186073738, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_4(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(73, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-10, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Version_13_5)
{
	// Test for version 1.3, point format 5, las file
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "points-1.3_5.las");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAS);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(3, lasFile.GetVersionMinor());

		LONGS_EQUAL(5, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString strSystemID = lasFile.GetSystemID();
		XString strGenSoftware = lasFile.GetGenSoftware();
		CHECK(strSystemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(strGenSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(235, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000,     lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		XString str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare("OGR variant of OpenGIS WKT SRS"));
		LONGS_EQUAL(697, varRec.LenAfterHdr);
		LONGS_EQUAL(2112, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("liblas"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_5 pt = lasFile.GetPointRec_5(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
		LONGS_EQUAL(54, pt.Intensity);
		LONGS_EQUAL(90, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_5(1);
		LONGS_EQUAL(96730291, pt.X);
		LONGS_EQUAL(143876843, pt.Y);
		LONGS_EQUAL(14383, pt.Z);
		LONGS_EQUAL(9, pt.Intensity);
		LONGS_EQUAL(89, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_5(2);
		LONGS_EQUAL(96730284, pt.X);
		LONGS_EQUAL(143876718, pt.Y);
		LONGS_EQUAL(14446, pt.Z);
		LONGS_EQUAL(35, pt.Intensity);
		LONGS_EQUAL(89, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(199, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905186073738, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_5(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(73, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-10, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}

	{
		// test compressed version of same file

		XString fileName = XString::CombinePath(gs_DataPath, "points-1.3_5.laz");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAZ);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(3, lasFile.GetVersionMinor());

		LONGS_EQUAL(5, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString strSystemID = lasFile.GetSystemID();
		XString strGenSoftware = lasFile.GetGenSoftware();
		CHECK(strSystemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(strGenSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(235, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000,     lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		XString str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare("OGR variant of OpenGIS WKT SRS"));
		LONGS_EQUAL(697, varRec.LenAfterHdr);
		LONGS_EQUAL(2112, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("liblas"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_5 pt = lasFile.GetPointRec_5(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
		LONGS_EQUAL(54, pt.Intensity);
		LONGS_EQUAL(90, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_5(1);
		LONGS_EQUAL(96730291, pt.X);
		LONGS_EQUAL(143876843, pt.Y);
		LONGS_EQUAL(14383, pt.Z);
		LONGS_EQUAL(9, pt.Intensity);
		LONGS_EQUAL(89, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_5(2);
		LONGS_EQUAL(96730284, pt.X);
		LONGS_EQUAL(143876718, pt.Y);
		LONGS_EQUAL(14446, pt.Z);
		LONGS_EQUAL(35, pt.Intensity);
		LONGS_EQUAL(89, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(11, pt.ScanAngle);
		LONGS_EQUAL(199, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905186073738, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_5(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(73, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-10, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Version_14_6)
{
	// Test for version 1.4, point format 6, las file
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "points-1.4_6.las");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAS);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(4, lasFile.GetVersionMinor());

		LONGS_EQUAL(6, lasFile.GetPointFormat());

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(4, lasFile.GetVersionMinor());

		LONGS_EQUAL(6, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString strSystemID = lasFile.GetSystemID();
		XString strGenSoftware = lasFile.GetGenSoftware();
		CHECK(strSystemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(strGenSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(375, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000, lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		XString str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare("OGR variant of OpenGIS WKT SRS"));
		LONGS_EQUAL(697, varRec.LenAfterHdr);
		LONGS_EQUAL(2112, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("liblas"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_6 pt = lasFile.GetPointRec_6(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
		LONGS_EQUAL(54, pt.Intensity);
		LONGS_EQUAL(16434, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(1833, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_6(1);
//		LONGS_EQUAL(96730291, pt.X);
//		LONGS_EQUAL(143876843, pt.Y);
//		LONGS_EQUAL(14383, pt.Z);
//		LONGS_EQUAL(34318, pt.Intensity);
//		LONGS_EQUAL(50, pt.ReturnMask);
//		LONGS_EQUAL(4, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_6(2);
//		LONGS_EQUAL(96730284, pt.X);
//		LONGS_EQUAL(143876718, pt.Y);
//		LONGS_EQUAL(14446, pt.Z);
//		LONGS_EQUAL(25930, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1500, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
//		DOUBLES_EQUAL(209404165.65391803, pt.GPSTime, 0.0);
//
		pt = lasFile.GetPointRec_6(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(16401, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-1667, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}

	{
		// test compressed version of same file

		XString fileName = XString::CombinePath(gs_DataPath, "points-1.4_6.laz");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAZ);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(4, lasFile.GetVersionMinor());

		LONGS_EQUAL(6, lasFile.GetPointFormat());

//		LONGS_EQUAL(0, lasFile.GetFileSourceID());
//		LONGS_EQUAL(17, lasFile.GetGlobalEncoding());
//
//		CHECK(XString(lasFile.GetSystemID()).Compare("LAStools (c) by rapidlasso GmbH"));
//		CHECK(XString(lasFile.GetGenSoftware()).Compare("TerraScan + OT"));
//
		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(375, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000,     lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		XString str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare("OGR variant of OpenGIS WKT SRS"));
		LONGS_EQUAL(697, varRec.LenAfterHdr);
		LONGS_EQUAL(2112, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("liblas"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_6 pt = lasFile.GetPointRec_6(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
		LONGS_EQUAL(54, pt.Intensity);
		LONGS_EQUAL(16434, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(1833, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_6(1);
//		LONGS_EQUAL(96730291, pt.X);
//		LONGS_EQUAL(143876843, pt.Y);
//		LONGS_EQUAL(14383, pt.Z);
//		LONGS_EQUAL(34318, pt.Intensity);
//		LONGS_EQUAL(50, pt.ReturnMask);
//		LONGS_EQUAL(4, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

//		pt = lasFile.GetPointRec_6(2);
//		LONGS_EQUAL(96730284, pt.X);
//		LONGS_EQUAL(143876718, pt.Y);
//		LONGS_EQUAL(14446, pt.Z);
//		LONGS_EQUAL(25930, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1500, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
//		DOUBLES_EQUAL(209404165.65391803, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_6(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(16401, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-1667, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Version_14_7)
{
	// Test for version 1.4, point format 7, las file
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "points-1.4_7.las");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAS);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(4, lasFile.GetVersionMinor());

		LONGS_EQUAL(7, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString systemID = lasFile.GetSystemID();
		XString genSoftware = lasFile.GetGenSoftware();
		CHECK(systemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(genSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(375, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000, lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		XString str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare("OGR variant of OpenGIS WKT SRS"));
		LONGS_EQUAL(697, varRec.LenAfterHdr);
		LONGS_EQUAL(2112, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("liblas"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_7 pt = lasFile.GetPointRec_7(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
		LONGS_EQUAL(54, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_7(1);
//		LONGS_EQUAL(96730291, pt.X);
//		LONGS_EQUAL(143876843, pt.Y);
//		LONGS_EQUAL(14383, pt.Z);
//		LONGS_EQUAL(34318, pt.Intensity);
//		LONGS_EQUAL(50, pt.ReturnMask);
//		LONGS_EQUAL(4, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
//		LONGS_EQUAL(0, pt.Red);
//		LONGS_EQUAL(0, pt.Green);
//		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

//		pt = lasFile.GetPointRec_7(2);
//		LONGS_EQUAL(96730284, pt.X);
//		LONGS_EQUAL(143876718, pt.Y);
//		LONGS_EQUAL(14446, pt.Z);
//		LONGS_EQUAL(25930, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1500, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
//		LONGS_EQUAL(0, pt.Red);
//		LONGS_EQUAL(0, pt.Green);
//		LONGS_EQUAL(0, pt.Blue);
//		DOUBLES_EQUAL(209404165.65391803, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_7(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(16401, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-1667, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}

	{
		// test compressed version of same file

		XString fileName = XString::CombinePath(gs_DataPath, "points-1.4_7.laz");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAZ);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(4, lasFile.GetVersionMinor());

		LONGS_EQUAL(7, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString systemID = lasFile.GetSystemID();
		XString genSoftware = lasFile.GetGenSoftware();
		CHECK(systemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(genSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(375, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000, lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		XString str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare("OGR variant of OpenGIS WKT SRS"));
		LONGS_EQUAL(697, varRec.LenAfterHdr);
		LONGS_EQUAL(2112, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("liblas"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_7 pt = lasFile.GetPointRec_7(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
		LONGS_EQUAL(54, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_7(1);
//		LONGS_EQUAL(96730291, pt.X);
//		LONGS_EQUAL(143876843, pt.Y);
//		LONGS_EQUAL(14383, pt.Z);
//		LONGS_EQUAL(34318, pt.Intensity);
//		LONGS_EQUAL(50, pt.ReturnMask);
//		LONGS_EQUAL(4, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_7(2);
//		LONGS_EQUAL(96730284, pt.X);
//		LONGS_EQUAL(143876718, pt.Y);
//		LONGS_EQUAL(14446, pt.Z);
//		LONGS_EQUAL(25930, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1500, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
//		DOUBLES_EQUAL(209404165.65391803, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_7(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(16401, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-1667, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Version_14_8)
{
	// Test for version 1.4, point format 8, las file
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "points-1.4_8.las");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAS);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(4, lasFile.GetVersionMinor());

		LONGS_EQUAL(8, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString systemID = lasFile.GetSystemID();
		XString genSoftware = lasFile.GetGenSoftware();
		CHECK(systemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(genSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(375, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000, lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		XString str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare("OGR variant of OpenGIS WKT SRS"));
		LONGS_EQUAL(697, varRec.LenAfterHdr);
		LONGS_EQUAL(2112, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("liblas"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_8 pt = lasFile.GetPointRec_8(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
		LONGS_EQUAL(54, pt.Intensity);
		LONGS_EQUAL(16434, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(1833, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_8(1);
		LONGS_EQUAL(96730291, pt.X);
		LONGS_EQUAL(143876843, pt.Y);
		LONGS_EQUAL(14383, pt.Z);
		LONGS_EQUAL(9, pt.Intensity);
		LONGS_EQUAL(16433, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(1833, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_8(2);
		LONGS_EQUAL(96730284, pt.X);
		LONGS_EQUAL(143876718, pt.Y);
		LONGS_EQUAL(14446, pt.Z);
		LONGS_EQUAL(35, pt.Intensity);
		LONGS_EQUAL(16433, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(1833, pt.ScanAngle);
		LONGS_EQUAL(199, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(87603.905186073738, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_8(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(16401, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-1667, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}

	{
		// test compressed version of same file

		XString fileName = XString::CombinePath(gs_DataPath, "points-1.4_8.laz");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAZ);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(4, lasFile.GetVersionMinor());

		LONGS_EQUAL(8, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString systemID = lasFile.GetSystemID();
		XString genSoftware = lasFile.GetGenSoftware();
		CHECK(systemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(genSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(375, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000, lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		XString str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare("OGR variant of OpenGIS WKT SRS"));
		LONGS_EQUAL(697, varRec.LenAfterHdr);
		LONGS_EQUAL(2112, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("liblas"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_8 pt = lasFile.GetPointRec_8(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
		LONGS_EQUAL(54, pt.Intensity);
		LONGS_EQUAL(16434, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(1833, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_8(1);
		LONGS_EQUAL(96730291, pt.X);
		LONGS_EQUAL(143876843, pt.Y);
		LONGS_EQUAL(14383, pt.Z);
		LONGS_EQUAL(9, pt.Intensity);
		LONGS_EQUAL(16433, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(1833, pt.ScanAngle);
		LONGS_EQUAL(198, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_8(2);
		LONGS_EQUAL(96730284, pt.X);
		LONGS_EQUAL(143876718, pt.Y);
		LONGS_EQUAL(14446, pt.Z);
		LONGS_EQUAL(35, pt.Intensity);
		LONGS_EQUAL(16433, pt.ReturnMask);
		LONGS_EQUAL(1, pt.Classification);
		LONGS_EQUAL(1833, pt.ScanAngle);
		LONGS_EQUAL(199, pt.UserData);
		LONGS_EQUAL(15006, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(87603.905186073738, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_8(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(16401, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-1667, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		LONGS_EQUAL(0, pt.Red);
		LONGS_EQUAL(0, pt.Green);
		LONGS_EQUAL(0, pt.Blue);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Version_14_9)
{
	// Test for version 1.4, point format 9, las file
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "points-1.4_9.las");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAS);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(4, lasFile.GetVersionMinor());

		LONGS_EQUAL(9, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString systemID = lasFile.GetSystemID();
		XString genSoftware = lasFile.GetGenSoftware();
		CHECK(systemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(genSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(375, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000, lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		XString str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare("OGR variant of OpenGIS WKT SRS"));
		LONGS_EQUAL(697, varRec.LenAfterHdr);
		LONGS_EQUAL(2112, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("liblas"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_9 pt = lasFile.GetPointRec_9(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
//		LONGS_EQUAL(33575, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_9(1);
//		LONGS_EQUAL(96730291, pt.X);
//		LONGS_EQUAL(143876843, pt.Y);
//		LONGS_EQUAL(14383, pt.Z);
//		LONGS_EQUAL(34318, pt.Intensity);
//		LONGS_EQUAL(50, pt.ReturnMask);
//		LONGS_EQUAL(4, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
//		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_9(2);
//		LONGS_EQUAL(96730284, pt.X);
//		LONGS_EQUAL(143876718, pt.Y);
//		LONGS_EQUAL(14446, pt.Z);
//		LONGS_EQUAL(25930, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1500, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
//		DOUBLES_EQUAL(209404165.65391803, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_9(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(16401, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-1667, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}

	{
		// test compressed version of same file

		XString fileName = XString::CombinePath(gs_DataPath, "points-1.4_9.laz");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAZ);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(4, lasFile.GetVersionMinor());

		LONGS_EQUAL(9, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString systemID = lasFile.GetSystemID();
		XString genSoftware = lasFile.GetGenSoftware();
		CHECK(systemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(genSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(375, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000,     lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		XString str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare("OGR variant of OpenGIS WKT SRS"));
		LONGS_EQUAL(697, varRec.LenAfterHdr);
		LONGS_EQUAL(2112, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("liblas"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_9 pt = lasFile.GetPointRec_9(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
//		LONGS_EQUAL(33575, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
//		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_9(1);
//		LONGS_EQUAL(96730291, pt.X);
//		LONGS_EQUAL(143876843, pt.Y);
//		LONGS_EQUAL(14383, pt.Z);
//		LONGS_EQUAL(34318, pt.Intensity);
//		LONGS_EQUAL(50, pt.ReturnMask);
//		LONGS_EQUAL(4, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_9(2);
//		LONGS_EQUAL(96730284, pt.X);
//		LONGS_EQUAL(143876718, pt.Y);
//		LONGS_EQUAL(14446, pt.Z);
//		LONGS_EQUAL(25930, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1500, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
//		DOUBLES_EQUAL(209404165.65391803, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_9(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(16401, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-1667, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Version_14_10)
{
	// Test for version 1.4, point format 10, las file
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "points-1.4_10.las");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAS);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(4, lasFile.GetVersionMinor());

		LONGS_EQUAL(10, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString systemID = lasFile.GetSystemID();
		XString genSoftware = lasFile.GetGenSoftware();
		CHECK(systemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(genSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(375, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.4600000000, lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990000, lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
		XString str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoKeyDirectoryTag"));
		LONGS_EQUAL(64, varRec.LenAfterHdr);
		LONGS_EQUAL(34735, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(1);
		str = varRec.Desc;
		CHECK(str.Compare("GeoTIFF GeoAsciiParamsTag"));
		LONGS_EQUAL(34, varRec.LenAfterHdr);
		LONGS_EQUAL(34737, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("LASF_Projection"));

		varRec = lasFile.GetVarRec_10(2);
		str = varRec.Desc;
		CHECK(str.Compare("OGR variant of OpenGIS WKT SRS"));
		LONGS_EQUAL(697, varRec.LenAfterHdr);
		LONGS_EQUAL(2112, varRec.RecordID);
		LONGS_EQUAL(43707, varRec.Signature);
		str = varRec.UserID;
		CHECK(str.Compare("liblas"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_10 pt = lasFile.GetPointRec_10(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
//		LONGS_EQUAL(33575, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
//		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_10(1);
//		LONGS_EQUAL(96730291, pt.X);
//		LONGS_EQUAL(143876843, pt.Y);
//		LONGS_EQUAL(14383, pt.Z);
//		LONGS_EQUAL(34318, pt.Intensity);
//		LONGS_EQUAL(50, pt.ReturnMask);
//		LONGS_EQUAL(4, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
//		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

//		pt = lasFile.GetPointRec_10(2);
//		LONGS_EQUAL(96730284, pt.X);
//		LONGS_EQUAL(143876718, pt.Y);
//		LONGS_EQUAL(14446, pt.Z);
//		LONGS_EQUAL(25930, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1500, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
//		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
//		DOUBLES_EQUAL(209404165.65391803, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_10(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(16401, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-1667, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}

	{
		// test compressed version of same file

		XString fileName = XString::CombinePath(gs_DataPath, "points-1.4_10.laz");
		LasFile lasFile(fileName.c_str());

		CHECK(lasFile.GetFileType() == GIS::GEODATA::LAZ);

		CHECK(fileName.Compare(lasFile.GetFileName()));

		LONGS_EQUAL(1, lasFile.GetVersionMajor());
		LONGS_EQUAL(4, lasFile.GetVersionMinor());

		LONGS_EQUAL(10, lasFile.GetPointFormat());

		LONGS_EQUAL(0, lasFile.GetFileSourceID());
		LONGS_EQUAL(0, lasFile.GetGlobalEncoding());

		XString systemID = lasFile.GetSystemID();
		XString genSoftware = lasFile.GetGenSoftware();
		CHECK(systemID.Compare("LAStools (c) by rapidlasso GmbH"));
		CHECK(genSoftware.Compare("las2las (version 180706)"));

		LONGS_EQUAL(53, lasFile.GetCreateDay());
		LONGS_EQUAL(2019, lasFile.GetCreateYear());
		LONGS_EQUAL(375, lasFile.GetHeaderSize());

		// Min/Max are scaled values
		DOUBLES_EQUAL(966876.73999999999, lasFile.GetMinX(), 0.0);
		DOUBLES_EQUAL(967303.01000000001, lasFile.GetMaxX(), 0.0);
		DOUBLES_EQUAL(1438530.460,        lasFile.GetMinY(), 1E-9);
		DOUBLES_EQUAL(1438949.990,        lasFile.GetMaxY(), 1E-9);
		DOUBLES_EQUAL(78.079999999999998, lasFile.GetMinZ(), 1E-9);
		DOUBLES_EQUAL(223.20000000000002, lasFile.GetMaxZ(), 1E-9);
		DOUBLES_EQUAL(0.01, lasFile.GetXScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetYScale(), 0.0);
		DOUBLES_EQUAL(0.01, lasFile.GetZScale(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetXOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetYOffset(), 0.0);
		DOUBLES_EQUAL(0.0, lasFile.GetZOffset(), 0.0);


		// Variable Length Records

		LONGS_EQUAL(3, lasFile.GetVarRecCount());

		LasFile::VarRecType_10 varRec = lasFile.GetVarRec_10(0);
//		CHECK(XString(varRec.Desc).Compare("by LAStools of rapidlasso GmbH"));
//		LONGS_EQUAL(48, varRec.LenAfterHdr);
//		LONGS_EQUAL(34735, varRec.RecordID);
//		LONGS_EQUAL(0, varRec.Signature);
//		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));
//
//		varRec = lasFile.GetVarRec_10(1);
//		CHECK(XString(varRec.Desc).Compare("by LAStools of rapidlasso GmbH"));
//		LONGS_EQUAL(847, varRec.LenAfterHdr);
//		LONGS_EQUAL(2112, varRec.RecordID);
//		LONGS_EQUAL(0, varRec.Signature);
//		CHECK(XString(varRec.UserID).Compare("LASF_Projection"));


		// Point Records

		LONGS_EQUAL(114200, static_cast<long>(lasFile.GetPointCount()));

		LasFile::PointRecType_10 pt = lasFile.GetPointRec_10(0);

		// Point XYZ are unscaled integer values
		LONGS_EQUAL(96730297, pt.X);
		LONGS_EQUAL(143876416, pt.Y);
		LONGS_EQUAL(12451, pt.Z);
//		LONGS_EQUAL(33575, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
//		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);
//
//		pt = lasFile.GetPointRec_10(1);
//		LONGS_EQUAL(96730291, pt.X);
//		LONGS_EQUAL(143876843, pt.Y);
//		LONGS_EQUAL(14383, pt.Z);
//		LONGS_EQUAL(34318, pt.Intensity);
//		LONGS_EQUAL(50, pt.ReturnMask);
//		LONGS_EQUAL(4, pt.Classification);
//		LONGS_EQUAL(-1333, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
//		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
		DOUBLES_EQUAL(87603.905174328858, pt.GPSTime, 0.0);

//		pt = lasFile.GetPointRec_10(2);
//		LONGS_EQUAL(96730284, pt.X);
//		LONGS_EQUAL(143876718, pt.Y);
//		LONGS_EQUAL(14446, pt.Z);
//		LONGS_EQUAL(25930, pt.Intensity);
//		LONGS_EQUAL(49, pt.ReturnMask);
//		LONGS_EQUAL(1, pt.Classification);
//		LONGS_EQUAL(-1500, pt.ScanAngle);
//		LONGS_EQUAL(0, pt.UserData);
//		LONGS_EQUAL(8, pt.PointSourceID);
//		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
//		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
//		DOUBLES_EQUAL(209404165.65391803, pt.GPSTime, 0.0);

		pt = lasFile.GetPointRec_10(114199);
		LONGS_EQUAL(96687866, pt.X);
		LONGS_EQUAL(143853377, pt.Y);
		LONGS_EQUAL(7881, pt.Z);
		LONGS_EQUAL(15, pt.Intensity);
		LONGS_EQUAL(16401, pt.ReturnMask);
		LONGS_EQUAL(2, pt.Classification);
		LONGS_EQUAL(-1667, pt.ScanAngle);
		LONGS_EQUAL(163, pt.UserData);
		LONGS_EQUAL(15009, pt.PointSourceID);
		DOUBLES_EQUAL(0.0, pt.WaveTimeOffset, 0.0);
		DOUBLES_EQUAL(0.0, pt.Xt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Yt, 0.0);
		DOUBLES_EQUAL(0.0, pt.Zt, 0.0);
		DOUBLES_EQUAL(89184.181665349504, pt.GPSTime, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, DataSize)
{
	CHECK(sizeof(UInt8) == 1);
	CHECK(sizeof(UInt16) == 2);		// 16 bits - unsigned
	CHECK(sizeof(UInt32) == 4);		// 32 bits - unsigned
	CHECK(sizeof(UInt64) == 8);		// 64 bits

	CHECK(sizeof(Int8) == 1);		// 8 bits - signed
	CHECK(sizeof(Int16) == 2);		// 16 bits - signed
	CHECK(sizeof(Int32) == 4);		// 32 bits - signed
	CHECK(sizeof(Int64) == 8);		// 64 bits - signed

	// verify that point record sizes are what i think they are

	LONGS_EQUAL(20, sizeof(LasFile::PointRecType_0));
	LONGS_EQUAL(28, sizeof(LasFile::PointRecType_1));
	LONGS_EQUAL(26, sizeof(LasFile::PointRecType_2));
	LONGS_EQUAL(34, sizeof(LasFile::PointRecType_3));
	LONGS_EQUAL(57, sizeof(LasFile::PointRecType_4));
	LONGS_EQUAL(63, sizeof(LasFile::PointRecType_5));
	LONGS_EQUAL(30, sizeof(LasFile::PointRecType_6));
	LONGS_EQUAL(36, sizeof(LasFile::PointRecType_7));
	LONGS_EQUAL(38, sizeof(LasFile::PointRecType_8));
	LONGS_EQUAL(59, sizeof(LasFile::PointRecType_9));
	LONGS_EQUAL(67, sizeof(LasFile::PointRecType_10));
}
