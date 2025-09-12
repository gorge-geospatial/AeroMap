// TestGdal.cpp
// General test suite for gdal.
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "../Common/UnitTest.h"			// unit test helpers
#include "TextFile.h"

#include "gdal_priv.h"
#include "cpl_conv.h"		// for CPLMalloc()
#include "ogrsf_frmts.h"

XString GetWktText(const char* fileName)
{
	// Return the contents of wkt text file
	// in the test data directory.
	//

	XString srcWkt = XString::CombinePath(gs_DataPath.c_str(), fileName);
	TextFile txtWkt(srcWkt.c_str());
	XString strWkt = txtWkt.GetText().c_str();
	return strWkt;
}

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
TEST(GDAL, Transform)
{
	// Test OGRCoordinateTransformation class.
	//

	{
		// projected -> projected

		OGRSpatialReference sourceSRS, targetSRS;
		OGRCoordinateTransformation* pCT = nullptr;

		XString strSrcWkt = GetWktText("epsg-3645.txt");	// NAD83 / Oregon North
		XString strTrgWkt = GetWktText("epsg-3646.txt");	// NAD83 / Oregon North (ft)

		OGRErr err = OGRERR_NONE;
		err = sourceSRS.importFromWkt(strSrcWkt.c_str());
		CHECK(err == OGRERR_NONE);
		err = targetSRS.importFromWkt(strTrgWkt.c_str());
		CHECK(err == OGRERR_NONE);

		pCT = OGRCreateCoordinateTransformation(&sourceSRS, &targetSRS);
		CHECK(pCT != nullptr);

		if (pCT != nullptr)
		{
			double x = 500.0;
			double y = 1000.0;

			// x,y are arrays with count indicated by first parm
			bool status = pCT->Transform(1, &x, &y);
			CHECK(status == true);

			// true just means at least 1 point transformed successfully
			// not necessarily all
			if (status == true)
			{
				// Global Mapper gave me these values, but to only
				// 2 decimal points.
				DOUBLES_EQUAL(1640.42, x, 0.001);
				DOUBLES_EQUAL(3280.84, y, 0.001);
			}
		}
	}

	{
		// geographic -> projected

		OGRSpatialReference sourceSRS, targetSRS;
		OGRCoordinateTransformation* pCT = nullptr;

		XString strSrcWkt = GetWktText("epsg-4326.txt");	// WGS_1984
		XString strTrgWkt = GetWktText("epsg-3645.txt");	// NAD83 / Oregon North

		OGRErr err = OGRERR_NONE;
		err = sourceSRS.importFromWkt(strSrcWkt.c_str());
		CHECK(err == OGRERR_NONE);
		err = targetSRS.importFromWkt(strTrgWkt.c_str());
		CHECK(err == OGRERR_NONE);

		LONGS_EQUAL(2, sourceSRS.GetAxesCount());
		LONGS_EQUAL(2, targetSRS.GetAxesCount());

		OGRCoordinateTransformationOptions options;
		options.SetAreaOfInterest(-124, 44, -120, 46);

		pCT = OGRCreateCoordinateTransformation(&sourceSRS, &targetSRS, options);
		CHECK(pCT != nullptr);

		if (pCT != nullptr)
		{
			// for wgs84, coordinates are specified in lon, lat order
			double x = 45.0;
			double y = -122.0;

			// pass in lat,lon order - get back x,y order
			bool status = pCT->Transform(1, &x, &y);
			CHECK(status == true);

			// true just means at least 1 point transformed successfully
			// not necessarily all
			if (status == true)
			{
				DOUBLES_EQUAL(2381748.49864, x, 0.001);
				DOUBLES_EQUAL(149259.40855, y, 0.001);
			}
		}
	}

	{
		// projected -> geographic

		OGRSpatialReference sourceSRS, targetSRS;
		OGRCoordinateTransformation* pCT = nullptr;

		XString strSrcWkt = GetWktText("epsg-3645.txt");	// NAD83 / Oregon North
		XString strTrgWkt = GetWktText("epsg-4326.txt");	// WGS_1984

		OGRErr err = OGRERR_NONE;
		err = sourceSRS.importFromWkt(strSrcWkt.c_str());
		CHECK(err == OGRERR_NONE);
		err = targetSRS.importFromWkt(strTrgWkt.c_str());
		CHECK(err == OGRERR_NONE);

		LONGS_EQUAL(2, sourceSRS.GetAxesCount());
		LONGS_EQUAL(2, targetSRS.GetAxesCount());

		OGRCoordinateTransformationOptions options;
		options.SetAreaOfInterest(-124, 44, -120, 46);

		pCT = OGRCreateCoordinateTransformation(&sourceSRS, &targetSRS, options);
		CHECK(pCT != nullptr);

		if (pCT != nullptr)
		{
			double x = 2381748.49864;
			double y = 149259.40855;

			// pass in lat,lon order - get back x,y order
			bool status = pCT->Transform(1, &x, &y);
			CHECK(status == true);

			// true just means at least 1 point transformed successfully
			// not necessarily all
			if (status == true)
			{
				DOUBLES_EQUAL(45.0, x, 1E-9);
				DOUBLES_EQUAL(-122.0, y, 1E-9);
			}
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, CRS)
{
	// Test OGRSpatialReference class.
	//

	{
		// set by parameters - geographic

		OGRSpatialReference srs;

		OGRErr err = srs.SetGeogCS(
			"My geographic CRS",				// name - not a key, user text
			"World Geodetic System 1984",		// datum name - key, known value from the EPSG registry
			"My WGS84 Spheroid",				// ellipsoid name = not a key, user text
			SRS_WGS84_SEMIMAJOR,
			SRS_WGS84_INVFLATTENING,
			"Greenwich",						// not a key, user text
			0.0,
			"degree",							// not a key, user text
			0.0174532925199433);				// degree / radian conversion
		CHECK(err == OGRERR_NONE);

		if (err == OGRERR_NONE)
		{
			CHECK(srs.IsGeographic() == true);
			CHECK(srs.IsProjected() == false);
			CHECK(srs.IsEmpty() == false);
			DOUBLES_EQUAL(0.017453292519943295, srs.GetAngularUnits(), 0.0);
			double inverse_flattening = srs.GetInvFlattening();
			DOUBLES_EQUAL(SRS_WGS84_INVFLATTENING, inverse_flattening, 0.0);
			double e = srs.GetEccentricity();
			DOUBLES_EQUAL(0.081819190842621486, e, 0.0);
			double semi_major = srs.GetSemiMajor();
			DOUBLES_EQUAL(SRS_WGS84_SEMIMAJOR, semi_major, 0.0);
			double semi_minor = srs.GetSemiMinor();
			DOUBLES_EQUAL(6356752.3142451793, semi_minor, 0.0);
			double pm = srs.GetPrimeMeridian();
			DOUBLES_EQUAL(0.0, pm, 0.0);

			//TODO:
			//what this returns seems to vary depending on whether or not
			//proj.db is available
			XString strDatum = srs.GetAttrValue("DATUM");
			//CHECK(strDatum.Compare("World Geodetic System 1984") == true);

			char* pszWKT = nullptr;
			const char* apszOptions[] = { "FORMAT=WKT2_2018", "MULTILINE=YES", nullptr };
			err = srs.exportToWkt(&pszWKT, apszOptions);
			CHECK(err == OGRERR_NONE);
			//printf("%s\n", pszWKT);
			CPLFree(pszWKT);
		}
	}

	{
		// set by well known id - geographic

		OGRSpatialReference srs;
		OGRErr err = srs.SetWellKnownGeogCS("WGS84");
		CHECK(err == OGRERR_NONE);

		if (err == OGRERR_NONE)
		{
			CHECK(srs.IsGeographic() == true);
			CHECK(srs.IsProjected() == false);
			CHECK(srs.IsEmpty() == false);

			XString strDatum = srs.GetAttrValue("DATUM");
			CHECK(strDatum.Compare("WGS_1984") == true);
		}
	}

	{
		// set by well known id - projected

		OGRErr err = OGRERR_NONE;
		OGRSpatialReference srs;

		// order here important
		err = srs.SetProjCS("UTM 17 (WGS84) in northern hemisphere.");		// set projected cs
		CHECK(err == OGRERR_NONE);
		err = srs.SetWellKnownGeogCS("WGS84");								// set underlying geographic cs
		CHECK(err == OGRERR_NONE);
		err = srs.SetUTM(17, TRUE);											// set projection parameters
		CHECK(err == OGRERR_NONE);

		if (err == OGRERR_NONE)
		{
			CHECK(srs.IsProjected() == true);
			CHECK(srs.IsGeographic() == false);
			CHECK(srs.IsEmpty() == false);

			XString strProj = srs.GetAttrValue("PROJECTION");
			CHECK(strProj.Compare("Transverse_Mercator") == true);
			XString strDatum = srs.GetAttrValue("DATUM");
			CHECK(strDatum.Compare("WGS_1984") == true);

			LONGS_EQUAL(17, srs.GetUTMZone());

			double cm = srs.GetProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0);
			DOUBLES_EQUAL(cm, -81.0, 0.0);
			double p1 = srs.GetProjParm(SRS_PP_STANDARD_PARALLEL_1, 0.0);
			DOUBLES_EQUAL(p1, 0.0, 0.0);
			double fe = srs.GetProjParm(SRS_PP_FALSE_EASTING, 0.0);
			DOUBLES_EQUAL(fe, 500000, 0.0);
			double fn = srs.GetProjParm(SRS_PP_FALSE_NORTHING, 0.0);
			DOUBLES_EQUAL(fn, 0.0, 0.0);
		}
	}

	{
		// set by EPSG code (if EPSG database available)

		OGRSpatialReference srs;
		OGRErr err = srs.SetWellKnownGeogCS("EPSG:4326");
		CHECK(err == OGRERR_NONE);

		CHECK(srs.IsGeographic() == true);
		CHECK(srs.IsProjected() == false);
	}

	{
		// create from wkt text - geographic

		OGRErr err = OGRERR_NONE;
		OGRSpatialReference srs;

		const char wkt[] =
			"GEOGCS[\"WGS 84\","
				"DATUM[\"WGS_1984\","
					"SPHEROID[\"WGS 84\", 6378137, 298.257223563,"
						"AUTHORITY[\"EPSG\", \"7030\"]],"
					"AUTHORITY[\"EPSG\", \"6326\"]],"
				"PRIMEM[\"Greenwich\", 0,"
					"AUTHORITY[\"EPSG\", \"8901\"]],"
				"UNIT[\"degree\", 0.0174532925199433,"
					"AUTHORITY[\"EPSG\", \"9122\"]],"
				"AXIS[\"Latitude\", NORTH],"			// these enforce lat/lon ordering
				"AXIS[\"Longitude\", EAST],"
				"AUTHORITY[\"EPSG\", \"4326\"]]";

		err = srs.importFromWkt(wkt);

		if (err == OGRERR_NONE)
		{
			CHECK(srs.IsGeographic() == true);
			CHECK(srs.IsProjected() == false);
			CHECK(srs.IsEmpty() == false);

			DOUBLES_EQUAL(6378137.0, srs.GetSemiMajor(), 0.0);
			DOUBLES_EQUAL(298.257223563, srs.GetInvFlattening(), 0.0);

			//TODO:
			//what this returns seems to vary depending on whether or not
			//proj.db is available
			XString strAxis0 = srs.GetAxis("GEOGCS", 0, nullptr);
			XString strAxis1 = srs.GetAxis("GEOGCS", 1, nullptr);
			//CHECK(strAxis0.Compare("Latitude") == true);
			//CHECK(strAxis1.Compare("Longitude") == true);

			DOUBLES_EQUAL(0.0174532925199433, srs.GetAngularUnits(nullptr), 1E-15);

			//double lonWestDeg;
			//double latSouthDeg;
			//double lonEastDeg;
			//double latNorthDeg;
			//srs.GetAreaOfUse(&lonWestDeg, &latSouthDeg, &lonEastDeg, &latNorthDeg, nullptr);
		}
	}
}

////----------------------------------------------------------------------------
//TEST(GDAL, Raster_AAIGrid)
//{
//	// Test AAIGrid driver.
//	//
//	// AAIGrid – Arc/Info ASCII Grid
//	//
//
//	constexpr char* DRIVER_NAME = "AAIGrid";
//
//	{
//		// verify driver lookup
//		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
//		CHECK(pDriver != nullptr);
//	}
//
//	{
//		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "aaigrid/pixel_per_line.asc");
//
//		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
//		CHECK(pDS != nullptr);
//
//		if (pDS != nullptr)
//		{
//			// make sure it instantiated the correct driver
//			XString strDriverName = pDS->GetDriverName();
//			CHECK(strDriverName.Compare(DRIVER_NAME));
//
//			double geoTransform[6] = { 0 };
//			CPLErr err = pDS->GetGeoTransform(geoTransform);
//			CHECK(err == CPLErr::CE_None);
//
//			if (err == CPLErr::CE_None)
//			{
////assert gt[0] == 100000.0 and gt[1] == 50 and gt[2] == 0 and gt[3] == 650600.0 and gt[4] == 0 and gt[5] == -50, \
//
//				DOUBLES_EQUAL(100000.0, geoTransform[0], 0.0);	// x origin
//				DOUBLES_EQUAL(50.0,     geoTransform[1], 0.0);	// x pixel size
//				DOUBLES_EQUAL(0.0,      geoTransform[2], 0.0);
//				DOUBLES_EQUAL(650600.0, geoTransform[3], 0.0);	// y origin
//				DOUBLES_EQUAL(0.0,      geoTransform[4], 0.0);
//				DOUBLES_EQUAL(-50.0,    geoTransform[5], 0.0);	// y pixel size
//			}
//
//			CHECK(pDS->GetSpatialRef() != nullptr);
//			//assert prj == 'PROJCS["unnamed",GEOGCS["NAD83",DATUM["North_American_Datum_1983",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],AUTHORITY["EPSG","6269"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4269"]],PROJECTION["Albers_Conic_Equal_Area"],PARAMETER["latitude_of_center",59],PARAMETER["longitude_of_center",-132.5],PARAMETER["standard_parallel_1",61.6666666666667],PARAMETER["standard_parallel_2",68],PARAMETER["false_easting",500000],PARAMETER["false_northing",500000],UNIT["METERS",1],AXIS["Easting",EAST],AXIS["Northing",NORTH]]', \
//
//			GDALRasterBand* pBand = pDS->GetRasterBand(1);
//			CHECK(pBand != nullptr);
//
//			if (pBand != nullptr)
//			{
//				DOUBLES_EQUAL(-99999.0, pBand->GetNoDataValue(), 0.0);
//				CHECK(pBand->GetRasterDataType() == GDT_Float32);
//			}
//
//			GDALClose(pDS);
//		}
//	}
//
//	{
//		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "aaigrid/pixel_per_line_comma.asc");
//
//		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
//		CHECK(pDS != nullptr);
//
//		if (pDS != nullptr)
//		{
//			// make sure it instantiated the correct driver
//			XString strDriverName = pDS->GetDriverName();
//			CHECK(strDriverName.Compare(DRIVER_NAME));
//
//			double geoTransform[6] = { 0 };
//			CPLErr err = pDS->GetGeoTransform(geoTransform);
//			CHECK(err == CPLErr::CE_None);
//
//			if (err == CPLErr::CE_None)
//			{
//				DOUBLES_EQUAL(100000.0, geoTransform[0], 0.0);	// x origin
//				DOUBLES_EQUAL(50.0,     geoTransform[1], 0.0);	// x pixel size
//				DOUBLES_EQUAL(0.0,      geoTransform[2], 0.0);
//				DOUBLES_EQUAL(650600.0, geoTransform[3], 0.0);	// y origin
//				DOUBLES_EQUAL(0.0,      geoTransform[4], 0.0);
//				DOUBLES_EQUAL(-50.0,    geoTransform[5], 0.0);	// y pixel size
//			}
//
//			GDALRasterBand* pBand = pDS->GetRasterBand(1);
//			CHECK(pBand != nullptr);
//
//			if (pBand != nullptr)
//			{
//				DOUBLES_EQUAL(-99999.0, pBand->GetNoDataValue(), 0.0);
//				CHECK(pBand->GetRasterDataType() == GDT_Float32);
//			}
//
//			GDALClose(pDS);
//		}
//	}
//
//	{
//		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "aaigrid/nodata_float.asc");
//
//		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
//		CHECK(pDS != nullptr);
//
//		if (pDS != nullptr)
//		{
//			GDALRasterBand* pBand = pDS->GetRasterBand(1);
//			CHECK(pBand != nullptr);
//
//			if (pBand != nullptr)
//			{
//				DOUBLES_EQUAL(-99999.0, pBand->GetNoDataValue(), 0.0);
//				CHECK(pBand->GetRasterDataType() == GDT_Float32);
//			}
//
//			GDALClose(pDS);
//		}
//	}
//
//	{
//		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "aaigrid/nodata_int.asc");
//
//		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
//		CHECK(pDS != nullptr);
//
//		if (pDS != nullptr)
//		{
//			GDALRasterBand* pBand = pDS->GetRasterBand(1);
//			CHECK(pBand != nullptr);
//
//			if (pBand != nullptr)
//			{
//				DOUBLES_EQUAL(-99999.0, pBand->GetNoDataValue(), 0.0);
//				CHECK(pBand->GetRasterDataType() == GDT_Int32);
//			}
//
//			GDALClose(pDS);
//		}
//	}
//}
//
//----------------------------------------------------------------------------
TEST(GDAL, Raster_AIGrid)
{
	// Test AIGrid driver.
	//
	// AIGrid – Arc/Info Binary Grid
	//

	constexpr char* DRIVER_NAME = "AIG";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "aigrid/abc3x1/prj.adf");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			double geoTransform[6] = { 0 };
			CPLErr err = pDS->GetGeoTransform(geoTransform);
			CHECK(err == CPLErr::CE_None);

			if (err == CPLErr::CE_None)
			{
				DOUBLES_EQUAL(-0.5, geoTransform[0], 0.0);		// x origin
				DOUBLES_EQUAL( 1.0, geoTransform[1], 0.0);		// x pixel size
				DOUBLES_EQUAL( 0.0, geoTransform[2], 0.0);
				DOUBLES_EQUAL( 0.5, geoTransform[3], 0.0);		// y origin
				DOUBLES_EQUAL( 0.0, geoTransform[4], 0.0);
				DOUBLES_EQUAL(-1.0, geoTransform[5], 0.0);		// y pixel size
			}

			//prj = ds.GetProjection()
			//assert prj.find('PROJCS["unnamed",GEOGCS["GDA94",DATUM["Geocentric_Datum_of_Australia_1994"') != -1, \
			//	('Projection does not match expected:\n%s' % prj)

			GDALRasterBand* pBand = pDS->GetRasterBand(1);
			CHECK(pBand != nullptr);

			if (pBand != nullptr)
			{
				DOUBLES_EQUAL(255.0, pBand->GetNoDataValue(), 0.0);
				CHECK(pBand->GetRasterDataType() == GDT_Byte);
			}

			GDALClose(pDS);
		}
	}

	{
		//XString fileName = XString::CombinePath(gs_DataPath.c_str(), "aigrid/abc3x1/");

		//GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		//CHECK(pDS != nullptr);

		//if (pDS != nullptr)
		//{
		//	GDALClose(pDS);
		//}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_ADRG)
{
	// Test ADRG file driver.
	//
	// ADRG/ARC Digitized Raster Graphics (.gen/.thf)
	//

	constexpr char* DRIVER_NAME = "ADRG";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "adrg/SMALL_ADRG/ABCDEF01.GEN");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			int sizeX = pDS->GetRasterXSize();
			int sizeY = pDS->GetRasterYSize();
			int rasterCount = pDS->GetRasterCount();
			int layerCount = pDS->GetLayerCount();

			LONGS_EQUAL(128, sizeX);
			LONGS_EQUAL(128, sizeY);
			LONGS_EQUAL(3, rasterCount);
			LONGS_EQUAL(0, layerCount);

			// from global mapper:
			//		PROJ_DESC = Geographic(Latitude / Longitude) / WGS84 / arc degrees
			//		PROJ_DATUM = WGS84

			const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
			CHECK(pSRS != nullptr);

			if (pSRS != nullptr)
			{
				CHECK(pSRS->IsGeographic() == true);
				CHECK(pSRS->IsProjected() == false);
			}

			double geoTransform[6] = { 0 };
			CPLErr err = pDS->GetGeoTransform(geoTransform);
			CHECK(err == CPLErr::CE_None);

			if (err == CPLErr::CE_None)
			{
				// from global mapper:
				//		UPPER LEFT X	= -0.0001388889
				//		UPPER LEFT Y	= 48.0001388889
				//		PIXEL WIDTH = 0.0078147 arc degrees
				//		PIXEL HEIGHT = 0.0078147 arc degrees

				DOUBLES_EQUAL(-0.00013888888888888889, geoTransform[0], 0.0);	// x origin
				DOUBLES_EQUAL(48.000138888888891, geoTransform[3], 0.0);		// y origin

				DOUBLES_EQUAL( 0.0078147046692860402, geoTransform[1], 0.0);	// x pixel size
				DOUBLES_EQUAL(-0.0078147046692860402, geoTransform[5], 0.0);	// y pixel size
			}

			GDALRasterBand* pBand = pDS->GetRasterBand(1);

			GDALColorInterp ci = pBand->GetColorInterpretation();
			LONGS_EQUAL(ci, GCI_RedBand);

			int blockX, blockY;
			pBand->GetBlockSize(&blockX, &blockY);

			DOUBLES_EQUAL(0.0, pBand->GetMinimum(), 0.0);
			DOUBLES_EQUAL(255.0, pBand->GetMaximum(), 0.0);
			DOUBLES_EQUAL(1.0, pBand->GetScale(), 0.);

			CHECK(pBand->GetRasterDataType() == GDT_Byte);
			XString unitType = pBand->GetUnitType();

			GDALClose(pDS);
		}
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "adrg/SMALL_ADRG_ZNA9/ABCDEF01.GEN");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			int sizeX = pDS->GetRasterXSize();
			int sizeY = pDS->GetRasterYSize();
			int rasterCount = pDS->GetRasterCount();
			int layerCount = pDS->GetLayerCount();

			LONGS_EQUAL(128, sizeX);
			LONGS_EQUAL(128, sizeY);
			LONGS_EQUAL(3, rasterCount);
			LONGS_EQUAL(0, layerCount);

			GDALClose(pDS);
		}
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "adrg/SMALL_ADRG_ZNA18/ABCDEF01.GEN");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			int sizeX = pDS->GetRasterXSize();
			int sizeY = pDS->GetRasterYSize();
			int rasterCount = pDS->GetRasterCount();
			int layerCount = pDS->GetLayerCount();

			LONGS_EQUAL(128, sizeX);
			LONGS_EQUAL(128, sizeY);
			LONGS_EQUAL(3, rasterCount);
			LONGS_EQUAL(0, layerCount);

			GDALClose(pDS);
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_BT)
{
	// Test Virtual Terrain Project BT files.
	//
	
	constexpr char* DRIVER_NAME = "BT";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "bt/crater_0513_v11.bt");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);
	
		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			int rasterCount = pDS->GetRasterCount();

			int sizeX = pDS->GetRasterXSize();
			int sizeY = pDS->GetRasterYSize();
			
			LONGS_EQUAL(1, rasterCount);
			LONGS_EQUAL(513, sizeX);
			LONGS_EQUAL(513, sizeY);

			double geoTransform[6] = { 0 };
			CPLErr err = pDS->GetGeoTransform(geoTransform);
			CHECK(err == CPLErr::CE_None);

			if (err == CPLErr::CE_None)
			{
				double pixelSizeX = geoTransform[1];
				double pixelSizeY = geoTransform[5];

				DOUBLES_EQUAL( 20.142909356725145, pixelSizeX, 0.0);
				DOUBLES_EQUAL(-27.251461988304094, pixelSizeY, 0.0);

				double originX = geoTransform[0];	// X Origin (top left corner)
				double originY = geoTransform[3];	// Y Origin (top left corner)

				DOUBLES_EQUAL(561133.1250, originX, 0.0);
				DOUBLES_EQUAL(4760971.500, originY, 0.0);
			}

			const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
			CHECK(pSRS != nullptr);

			if (pSRS != nullptr)
			{
				CHECK(pSRS->IsProjected() == true);
				CHECK(pSRS->IsGeographic() == false);
				LONGS_EQUAL(10, pSRS->GetUTMZone());
			}

			GDALRasterBand* pBand = pDS->GetRasterBand(1);
			CHECK(pBand != nullptr);

			if (pBand != nullptr)
			{
				LONGS_EQUAL(513, pBand->GetXSize());
				LONGS_EQUAL(513, pBand->GetYSize());

				double minElev, maxElev, meanElev, stdDev;
				CPLErr err = pBand->ComputeStatistics(0, &minElev, &maxElev, &meanElev, &stdDev, nullptr, nullptr);
				DOUBLES_EQUAL(1524.0, minElev, 0.0);
				DOUBLES_EQUAL(2478.0, maxElev, 0.0);
				DOUBLES_EQUAL(1893.7484734144116, meanElev, 0.0);
				DOUBLES_EQUAL(163.54817147960179, stdDev, 0.0);

				DOUBLES_EQUAL(pBand->GetMinimum(), minElev, 0.0);
				DOUBLES_EQUAL(pBand->GetMaximum(), maxElev, 0.0);

				GDALDataType dataType = pBand->GetRasterDataType();

				// verify subset of elevation data

				float* pScanline = (float *)CPLMalloc(sizeof(float)*sizeX);

				// read first line

				err = pBand->RasterIO(
					GF_Read,			// read or write
					0,					// xoffset - zero based offset from left
					0,					// yoffset - zero based offset from top
					sizeX,				// width of the region of the band to be accessed in pixels
					1,					// height of the region of the band to be accessed in lines
					pScanline,			// data buffer
					sizeX,				// width of data buffer
					1,					// height of data buffer
					GDT_Float32,		// buffer type - data automatically translated
					0,					// pixel spacing
					0);					// line spacing
				CHECK(err == CPLErr::CE_None);

				double height = pScanline[0];
				DOUBLES_EQUAL(1625.0, height, 0.0);
				height = pScanline[1];
				DOUBLES_EQUAL(1625.0, height, 0.0);
				height = pScanline[2];
				DOUBLES_EQUAL(1625.0, height, 0.0);
				height = pScanline[512];
				DOUBLES_EQUAL(2054.0, height, 0.0);
				
				// The pScanline buffer should be freed with CPLFree() when it is no longer used.
				CPLFree(pScanline);
			}

			GDALClose(pDS);
		}
	}

//TODO:
//unable to read this one - error parsing wkt (tried with & without TOWGS entry
	{
		// .bt file has peer .prj file
		//XString fileName = XString::CombinePath(gs_DataPath.c_str(), "bt/coiba_20m_2k_v13.bt");

		//GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		//CHECK(pDS != nullptr);
	
		//if (pDS != nullptr)
		//{
		//	// make sure it instantiated the correct driver
		//	XString strDriverName = pDS->GetDriverName();
		//	CHECK(strDriverName.Compare(DRIVER_NAME));

		//	//{
		//	//	VtpFile* pVtpFile = new VtpFile(fileName.c_str());

		//	//	CHECK(GIS::GEODATA::VtpBT == pVtpFile->GetFileType());
		//	//	CHECK(13 == pVtpFile->GetVersion());

		//	//	bool isUtm = pVtpFile->IsUTM();
		//	//	CHECK(isUtm);

		//	//	UInt32 rowCount = pVtpFile->GetRowCount();
		//	//	LONGS_EQUAL(2049, rowCount);
		//	//	UInt32 colCount = pVtpFile->GetColCount();
		//	//	LONGS_EQUAL(2049, colCount);

		//	//	DOUBLES_EQUAL(0.0, pVtpFile->GetMinElev(), 0.0);
		//	//	DOUBLES_EQUAL(414.0, pVtpFile->GetMaxElev(), 0.0);

		//	//	double yn, ys, xw, xe;
		//	//	pVtpFile->GetExtents(yn, ys, xw, xe);
		//	//	DOUBLES_EQUAL(400776.8, xw, 0.0);
		//	//	DOUBLES_EQUAL(441736.8, xe, 0.0);
		//	//	DOUBLES_EQUAL(849177.0, yn, 0.0);
		//	//	DOUBLES_EQUAL(808217.0, ys, 0.0);

		//	//	delete pVtpFile;
		//	//}
		//}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_CTG)
{
	// Test CTG driver.
	//

	constexpr char* DRIVER_NAME = "CTG";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "ctg/fake_grid_cell");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);
	
		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			int sizeX = pDS->GetRasterXSize();
			int sizeY = pDS->GetRasterYSize();
			int rasterCount = pDS->GetRasterCount();
			int layerCount = pDS->GetLayerCount();

			LONGS_EQUAL(789, sizeX);
			LONGS_EQUAL(558, sizeY);
			LONGS_EQUAL(6, rasterCount);
			LONGS_EQUAL(0, layerCount);

			double geoTransform[6] = { 0 };
			CPLErr err = pDS->GetGeoTransform(geoTransform);
			CHECK(err == CPLErr::CE_None);

			if (err == CPLErr::CE_None)
			{
				double pixelSizeX = geoTransform[1];
				double pixelSizeY = geoTransform[5];

				DOUBLES_EQUAL( 200.0, pixelSizeX, 0.0);
				DOUBLES_EQUAL(-200.0, pixelSizeY, 0.0);

				double originX = geoTransform[0];	// X Origin (top left corner)
				double originY = geoTransform[3];	// Y Origin (top left corner)

				DOUBLES_EQUAL( 421000.0, originX, 0.0);
				DOUBLES_EQUAL(5094400.0, originY, 0.0);
			}

			const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
			CHECK(pSRS != nullptr);

			if (pSRS != nullptr)
			{
				CHECK(pSRS->IsProjected() == true);
				CHECK(pSRS->IsGeographic() == false);

				char* pszWkt = nullptr;
				OGRErr err = pSRS->exportToWkt(&pszWkt);
				CPLFree(pszWkt);
				CHECK(err == OGRERR_NONE);

				double pm = pSRS->GetPrimeMeridian();
				XString strAxis0 = pSRS->GetAxis("PROJCS", 0, nullptr);
				XString strAxis1 = pSRS->GetAxis("PROJCS", 1, nullptr);
				int axisCount = pSRS->GetAxesCount();

				DOUBLES_EQUAL(0.0, pm, 0.0);
				CHECK(strAxis0.Compare("Easting"));
				CHECK(strAxis1.Compare("Northing"));
				LONGS_EQUAL(2, axisCount);

				DOUBLES_EQUAL(0.0, pSRS->GetProjParm("latitude_of_origin"), 0.0);
				DOUBLES_EQUAL(-99.0, pSRS->GetProjParm("central_meridian"), 0.0);
				DOUBLES_EQUAL(0.9996, pSRS->GetProjParm("scale_factor"), 0.0);
				DOUBLES_EQUAL(500000, pSRS->GetProjParm("false_easting"), 0.0);
				DOUBLES_EQUAL(0, pSRS->GetProjParm("false_northing"), 0.0);
			}

			DOUBLES_EQUAL(0.0, pDS->GetRasterBand(1)->GetNoDataValue(), 0.0);

			// example of converting char** to string list
			CPLStringList str(pDS->GetRasterBand(1)->GetCategoryNames(), 0);
			LONGS_EQUAL(93, str.Count());

			GDALClose(pDS);
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_Dem)
{
	// Test USGS DEM file driver.
	//

	constexpr char* DRIVER_NAME = "USGSDEM";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		// read test

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "usgsdem/OR_BOARDMAN_10M.DEM");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			int sizeX = pDS->GetRasterXSize();
			int sizeY = pDS->GetRasterYSize();
			int rasterCount = pDS->GetRasterCount();
			int layerCount = pDS->GetLayerCount();

			LONGS_EQUAL(1017, sizeX);
			LONGS_EQUAL(1423, sizeY);
			LONGS_EQUAL(1, rasterCount);
			LONGS_EQUAL(0, layerCount);

			const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
			CHECK(pSRS != nullptr);

			if (pSRS != nullptr)
			{
				CHECK(pSRS->IsProjected() == true);
				CHECK(pSRS->IsGeographic() == false);
			}

			double geoTransform[6] = { 0 };
			CPLErr err = pDS->GetGeoTransform(geoTransform);
			CHECK(err == CPLErr::CE_None);

			if (err == CPLErr::CE_None)
			{
				double pixelSizeX = geoTransform[1];
				double pixelSizeY = geoTransform[5];

				// 10m DEM files
				DOUBLES_EQUAL(10.0, pixelSizeX, 1E-9);
				DOUBLES_EQUAL(-10.0, pixelSizeY, 1E-9);

				double originX = geoTransform[0];  // X Origin (top left corner)
				double originY = geoTransform[3];  // Y Origin (top left corner)

				DOUBLES_EQUAL(286105.0, originX, 0.0);
				DOUBLES_EQUAL(5083625.0, originY, 0.0);
			}

			GDALRasterBand* pBand = pDS->GetRasterBand(1);
			CHECK(pBand != nullptr);

			if (pBand != nullptr)
			{
				double minElev, maxElev, meanElev, stdDev;
				err = pBand->GetStatistics(0, 1, &minElev, &maxElev, &meanElev, &stdDev);
				CHECK(err == CPLErr::CE_None);

				if (err == CPLErr::CE_None)
				{
					DOUBLES_EQUAL(78.900001525878906, minElev, 1E-12);
					DOUBLES_EQUAL(195.50000000000000, maxElev, 1E-12);
					DOUBLES_EQUAL(126.95572909069510, meanElev, 1E-12);
					DOUBLES_EQUAL(37.484770269623304, stdDev, 1E-12);
				}

				CHECK(pBand->GetAccess() == GDALAccess::GA_ReadOnly);
				CHECK(GDALDataType::GDT_Float32 == pBand->GetRasterDataType());
				XString strUnitType = pBand->GetUnitType();
				CHECK(strUnitType.Compare("m") == true);
				DOUBLES_EQUAL(-32767.0, pBand->GetNoDataValue(), 0.0);

				GDALColorInterp ci = pBand->GetColorInterpretation();
				LONGS_EQUAL(ci, GCI_Undefined);

				// There are a few ways to read raster data, but the most common is via 
				// the GDALRasterBand::RasterIO() method. This method will automatically 
				// take care of data type conversion, up/down sampling and windowing.
				// The following code will read the first scanline of data into a similarly 
				// sized buffer, converting it to floating point as part of the operation.
				// 

				int bandSizeX = pBand->GetXSize();
				LONGS_EQUAL(1017, bandSizeX);

				float* pScanline = (float *)CPLMalloc(sizeof(float)*bandSizeX);

				// read first row

				pBand->RasterIO(
					GF_Read,			// read or write
					0,					// xoffset - zero based offset from left
					0,					// yoffset - zero based offset from top
					bandSizeX,			// width of the region of the band to be accessed in pixels
					1,					// height of the region of the band to be accessed in lines
					pScanline,			// data buffer
					bandSizeX,			// width of data buffer
					1,					// height of data buffer
					GDT_Float32,		// buffer type - data automatically translated
					0, 0);

				DOUBLES_EQUAL(-32767.0, pScanline[0], 0.0);
				DOUBLES_EQUAL(-32767.0, pScanline[1], 0.0);
				DOUBLES_EQUAL(-32767.0, pScanline[2], 0.0);
				DOUBLES_EQUAL(-32767.0, pScanline[1014], 0.0);
				DOUBLES_EQUAL(-32767.0, pScanline[1015], 0.0);
				DOUBLES_EQUAL(-32767.0, pScanline[1016], 0.0);

				pBand->RasterIO(
					GF_Read,			// read or write
					0,					// xoffset - zero based offset from left
					100,				// yoffset - zero based offset from top
					bandSizeX,			// width of the region of the band to be accessed in pixels
					1,					// height of the region of the band to be accessed in lines
					pScanline,			// data buffer
					bandSizeX,			// width of data buffer
					1,					// height of data buffer
					GDT_Float32,		// buffer type - data automatically translated
					0, 0);

				DOUBLES_EQUAL(-32767.0, pScanline[0], 0.0);
				DOUBLES_EQUAL(-32767.0, pScanline[1], 0.0);
				DOUBLES_EQUAL(-32767.0, pScanline[2], 0.0);

				DOUBLES_EQUAL(-32767.0, pScanline[41], 0.0);
				DOUBLES_EQUAL(-32767.0, pScanline[42], 0.0);
				DOUBLES_EQUAL(-32767.0, pScanline[43], 0.0);
				DOUBLES_EQUAL(92.50, pScanline[44], 1E-3);
				DOUBLES_EQUAL(92.40, pScanline[45], 1E-3);
				DOUBLES_EQUAL(92.40, pScanline[46], 1E-3);
				DOUBLES_EQUAL(92.50, pScanline[47], 1E-3);

				DOUBLES_EQUAL(88.3, pScanline[1014], 1E-3);
				DOUBLES_EQUAL(-32767.0, pScanline[1015], 0.0);
				DOUBLES_EQUAL(-32767.0, pScanline[1016], 0.0);

				pBand->RasterIO(
					GF_Read,			// read or write
					0,					// xoffset - zero based offset from left
					1000,				// yoffset - zero based offset from top
					bandSizeX,			// width of the region of the band to be accessed in pixels
					1,					// height of the region of the band to be accessed in lines
					pScanline,			// data buffer
					bandSizeX,			// width of data buffer
					1,					// height of data buffer
					GDT_Float32,		// buffer type - data automatically translated
					0, 0);

				DOUBLES_EQUAL(-32767.000000, pScanline[10], 1E-3);
				DOUBLES_EQUAL(-32767.000000, pScanline[11], 1E-3);
				DOUBLES_EQUAL(-32767.000000, pScanline[12], 1E-3);
				DOUBLES_EQUAL(154.500000, pScanline[13], 1E-3);
				DOUBLES_EQUAL(154.500000, pScanline[14], 1E-3);
				DOUBLES_EQUAL(154.300003, pScanline[15], 1E-3);

				DOUBLES_EQUAL(169.600006, pScanline[983], 1E-3);
				DOUBLES_EQUAL(169.600006, pScanline[984], 1E-3);
				DOUBLES_EQUAL(169.600006, pScanline[985], 1E-3);
				DOUBLES_EQUAL(-32767.000000, pScanline[986], 1E-3);
				DOUBLES_EQUAL(-32767.000000, pScanline[987], 1E-3);
				DOUBLES_EQUAL(-32767.000000, pScanline[988], 1E-3);

				DOUBLES_EQUAL(-32767.000000, pScanline[1015], 1E-3);
				DOUBLES_EQUAL(-32767.000000, pScanline[1016], 1E-3);

				// read last row

				pBand->RasterIO(
					GF_Read,			// read or write
					0,					// xoffset - zero based offset from left
					1422,				// yoffset - zero based offset from top
					bandSizeX,			// width of the region of the band to be accessed in pixels
					1,					// height of the region of the band to be accessed in lines
					pScanline,			// data buffer
					bandSizeX,			// width of data buffer
					1,					// height of data buffer
					GDT_Float32,		// buffer type - data automatically translated
					0, 0);

				// entire row is "no data"
				for (int i = 0; i < 1017; ++i)
				{
					DOUBLES_EQUAL(-32767.000000, pScanline[i], 1E-3);
				}

				CPLFree(pScanline);
			}

			GDALClose(pDS);
		}
	}

	{
		// Test truncated version of http ://download.osgeo.org/gdal/data/usgsdem/022gdeme

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "usgsdem/022gdeme_truncated");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			double geoTransform[6] = { 0 };
			CPLErr err = pDS->GetGeoTransform(geoTransform);
			CHECK(err == CPLErr::CE_None);

			if (err == CPLErr::CE_None)
			{
				DOUBLES_EQUAL( -67.00041667,  geoTransform[0], 1E-8);
				DOUBLES_EQUAL(   0.00083333,  geoTransform[1], 1E-8);
				DOUBLES_EQUAL(   0.0,         geoTransform[2], 1E-8);
				DOUBLES_EQUAL(  50.000416667, geoTransform[3], 1E-8);
				DOUBLES_EQUAL(   0.0,         geoTransform[4], 1E-8);
				DOUBLES_EQUAL( -0.00083333,   geoTransform[5], 1E-8);
			}
			
			GDALClose(pDS);
		}
	}

	//{
	//	// write test

	//	// driver has no Create(), so just testing CreateCopy()

	//	GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
	//	CHECK(pDriver != nullptr);

	//	XString fileNameSrc = XString::CombinePath(gs_DataPath.c_str(), "usgsdem/OR_BOARDMAN_10M.DEM");
	//	XString fileNameDst = XString::CombinePath(gs_DataPath.c_str(), "usgsdem/temp.dem");

	//	GDALDataset* pSrcDS = (GDALDataset *)GDALOpen(fileNameSrc.c_str(), GA_ReadOnly);
	//	CHECK(pSrcDS != nullptr);

	//	if (pSrcDS != nullptr)
	//	{
	//		// params: output file, src datasource, strict flag, options, progress...
	//		GDALDataset* pDstDS = pDriver->CreateCopy(fileNameDst.c_str(), pSrcDS, 0, nullptr, nullptr, nullptr );
	//		CHECK(pDstDS != nullptr);

	//		if (pDstDS != nullptr)
	//		{
	//			// check some values in created dataset
	//			int sizeX = pDstDS->GetRasterXSize();
	//			int sizeY = pDstDS->GetRasterYSize();
	//			int rasterCount = pDstDS->GetRasterCount();

	//			LONGS_EQUAL(1017, sizeX);
	//			LONGS_EQUAL(1423, sizeY);
	//			LONGS_EQUAL(1, rasterCount);

	//			// close / reopen & check some values
	//			GDALClose(pDstDS);

	//			pDstDS = (GDALDataset *)GDALOpen(fileNameDst.c_str(), GA_ReadOnly);
	//			CHECK(pDstDS != nullptr);
	//			
	//			if (pDstDS != nullptr)
	//			{
	//				LONGS_EQUAL(1017, pDstDS->GetRasterXSize());
	//				LONGS_EQUAL(1423, pDstDS->GetRasterYSize());
	//				LONGS_EQUAL(1, pDstDS->GetRasterCount());

	//				GDALRasterBand* pBand = pDstDS->GetRasterBand(1);
	//				CHECK(pBand != nullptr);

	//				if (pBand != nullptr)
	//				{
	//					LONGS_EQUAL(1017, pBand->GetXSize());
	//					LONGS_EQUAL(1423, pBand->GetYSize());

	//					GDALDataType dataType = GDALGetRasterDataType(pBand);
	//					CHECK(dataType == GDALDataType::GDT_Int16);

	//					GDALColorInterp ci = pBand->GetColorInterpretation();
	//					LONGS_EQUAL(ci, GCI_Undefined);

	//					float* pScanline = (float *)CPLMalloc(sizeof(float)*sizeX);

	//					// read row 100

	//					pBand->RasterIO(
	//						GF_Read,			// read or write
	//						0,					// xoffset - zero based offset from left
	//						100,				// yoffset - zero based offset from top
	//						sizeX,				// width of the region of the band to be accessed in pixels
	//						1,					// height of the region of the band to be accessed in lines
	//						pScanline,			// data buffer
	//						sizeX,				// width of data buffer
	//						1,					// height of data buffer
	//						GDT_Float32,		// buffer type - data automatically translated
	//						0, 0);

	//					// these have been rounded from original values,
	//					// think because internal type is int16
	//					DOUBLES_EQUAL(-32767.0, pScanline[0], 0.0);
	//					DOUBLES_EQUAL(-32767.0, pScanline[1], 0.0);
	//					DOUBLES_EQUAL(-32767.0, pScanline[2], 0.0);

	//					DOUBLES_EQUAL(-32767.0, pScanline[41], 0.0);
	//					DOUBLES_EQUAL(-32767.0, pScanline[42], 0.0);
	//					DOUBLES_EQUAL(-32767.0, pScanline[43], 0.0);
	//					DOUBLES_EQUAL(93.00, pScanline[44], 1E-3);
	//					DOUBLES_EQUAL(92.00, pScanline[45], 1E-3);
	//					DOUBLES_EQUAL(92.00, pScanline[46], 1E-3);
	//					DOUBLES_EQUAL(93.00, pScanline[47], 1E-3);

	//					DOUBLES_EQUAL(88.0, pScanline[1014], 1E-3);
	//					DOUBLES_EQUAL(-32767.0, pScanline[1015], 0.0);
	//					DOUBLES_EQUAL(-32767.0, pScanline[1016], 0.0);

	//					CPLFree(pScanline);
	//				}
	//				GDALClose(pDstDS);
	//			}
	//		}

	//		GDALClose(pSrcDS);
	//	}
	//}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_Dted)
{
	// Test DTED driver.
	//

	constexpr char* DRIVER_NAME = "DTED";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		// Read test - Level 0

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "dted/w121_n46.dt0");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		// make sure it instantiated the correct driver
		XString strDriverName = pDS->GetDriverName();
		CHECK(strDriverName.Compare(DRIVER_NAME));

		int sizeX = pDS->GetRasterXSize();
		int sizeY = pDS->GetRasterYSize();
		int rasterCount = pDS->GetRasterCount();
		int layerCount = pDS->GetLayerCount();

		LONGS_EQUAL(121, sizeX);
		LONGS_EQUAL(121, sizeY);
		LONGS_EQUAL(1, rasterCount);
		LONGS_EQUAL(0, layerCount);

		const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
		CHECK(pSRS != nullptr);

		if (pSRS != nullptr)
		{
			CHECK(pSRS->IsGeographic() == true);
			CHECK(pSRS->IsProjected() == false);
			LONGS_EQUAL(2, pSRS->GetAxesCount());
			DOUBLES_EQUAL(0.017453292519943295, pSRS->GetAngularUnits(), 0.0);
		}

		double geoTransform[6] = { 0 };
		CPLErr err = pDS->GetGeoTransform(geoTransform);
		CHECK(err == CPLErr::CE_None);

		if (err == CPLErr::CE_None)
		{
			double pixelSizeX = geoTransform[1];
			double pixelSizeY = geoTransform[5];

			// 1/120 degree
			DOUBLES_EQUAL(0.0083333333333333332, pixelSizeX, 1E-9);
			DOUBLES_EQUAL(-0.0083333333333333332, pixelSizeY, 1E-9);

			double originX = geoTransform[0];  // X Origin (top left corner)
			double originY = geoTransform[3];  // Y Origin (top left corner)

		   // on boundary + midpoint 1/120 degree
			DOUBLES_EQUAL(-121.00416666666666, originX, 1E-9);
			DOUBLES_EQUAL(47.004166666666663, originY, 1E-9);
		}

		GDALRasterBand* pBand = pDS->GetRasterBand(1);
		CHECK(pBand != nullptr);

		if (pBand != nullptr)
		{
			LONGS_EQUAL(121, pBand->GetXSize());
			LONGS_EQUAL(121, pBand->GetYSize());

			GDALDataType dataType = GDALGetRasterDataType(pBand);
			CHECK(dataType == GDALDataType::GDT_Int16);

			GDALColorInterp ci = pBand->GetColorInterpretation();
			LONGS_EQUAL(ci, GCI_Undefined);

			__int16* pScanline = (__int16 *)CPLMalloc(sizeof(__int16)*sizeX);

			// read first row

			pBand->RasterIO(
				GF_Read,			// read or write
				0,					// xoffset - pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.
				0,					// yoffset - line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.
				sizeX,				// width of the region of the band to be accessed in pixels
				1,					// height of the region of the band to be accessed in lines
				pScanline,			// data buffer
				sizeX,				// width of data buffer
				1,					// height of data buffer
				GDT_Int16,			// buffer type - data automatically translated
				0,					// pixel spacing
				0);					// line spacing

			LONGS_EQUAL(1773, pScanline[0]);
			LONGS_EQUAL(1778, pScanline[1]);
			LONGS_EQUAL(1752, pScanline[2]);
			LONGS_EQUAL(1782, pScanline[3]);

			LONGS_EQUAL(315, pScanline[117]);
			LONGS_EQUAL(306, pScanline[118]);
			LONGS_EQUAL(237, pScanline[119]);
			LONGS_EQUAL(180, pScanline[120]);

			// read last row

			pBand->RasterIO(
				GF_Read,			// read or write
				0,					// xoffset - pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.
				120,				// yoffset - line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.
				sizeX,				// width of the region of the band to be accessed in pixels
				1,					// height of the region of the band to be accessed in lines
				pScanline,			// data buffer
				sizeX,				// width of data buffer
				1,					// height of data buffer
				GDT_Int16,			// buffer type - data automatically translated
				0,					// pixel spacing
				0);					// line spacing

			//FILE* pFile = fopen("C:/Users/mark/Desktop/test.txt", "wt");
			//for (int i = 0; i < sizeX; ++i)
			//{
				//fprintf(pFile, "[%d] %i\n", i, pScanline[i]);
			//}
			//fclose(pFile);

			LONGS_EQUAL(1114, pScanline[0]);
			LONGS_EQUAL(1134, pScanline[1]);
			LONGS_EQUAL(1202, pScanline[2]);
			LONGS_EQUAL(1188, pScanline[3]);

			LONGS_EQUAL(464, pScanline[115]);
			LONGS_EQUAL(534, pScanline[116]);
			LONGS_EQUAL(523, pScanline[117]);
			LONGS_EQUAL(499, pScanline[118]);
			LONGS_EQUAL(476, pScanline[119]);
			LONGS_EQUAL(458, pScanline[120]);

			// The pScanline buffer should be freed with CPLFree() when it is no longer used.
			CPLFree(pScanline);
		}

		GDALClose(pDS);
	}

	{
		// Read test - Level 1

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "dted/w117_n50.dt1");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			LONGS_EQUAL(601, pDS->GetRasterXSize());
			LONGS_EQUAL(1201, pDS->GetRasterYSize());
			LONGS_EQUAL(1, pDS->GetRasterCount());

			double geoTransform[6] = { 0 };
			CPLErr err = pDS->GetGeoTransform(geoTransform);
			CHECK(err == CPLErr::CE_None);

			if (err == CPLErr::CE_None)
			{
				// pixel size
				DOUBLES_EQUAL(0.001667, geoTransform[1], 1E-6);
				DOUBLES_EQUAL(-0.000833, geoTransform[5], 1E-6);
			}
			GDALClose(pDS);
		}
	}

	{
		// Read test - Level 2

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "dted/w118_n45.dt2");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			LONGS_EQUAL(3601, pDS->GetRasterXSize());
			LONGS_EQUAL(3601, pDS->GetRasterYSize());
			LONGS_EQUAL(1, pDS->GetRasterCount());

			double geoTransform[6] = { 0 };
			CPLErr err = pDS->GetGeoTransform(geoTransform);
			CHECK(err == CPLErr::CE_None);

			if (err == CPLErr::CE_None)
			{
				// pixel size
				DOUBLES_EQUAL(0.0002778, geoTransform[1], 1E-6);
				DOUBLES_EQUAL(-0.0002778, geoTransform[5], 1E-6);
			}
			GDALClose(pDS);
		}
	}

	{
		// Write test - Level 0
		// dted driver has no Create(), so just testing CreateCopy()

		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);

		XString fileNameSrc = XString::CombinePath(gs_DataPath.c_str(), "dted/w121_n46.dt0");
		XString fileNameDst = XString::CombinePath(gs_DataPath.c_str(), "dted/temp.dt0");

		GDALDataset* pSrcDS = (GDALDataset *)GDALOpen(fileNameSrc.c_str(), GA_ReadOnly);
		CHECK(pSrcDS != nullptr);

		if (pSrcDS != nullptr)
		{
			// params: output file, src datasource, strict flag, options, progress...
			GDALDataset* pDstDS = pDriver->CreateCopy(fileNameDst.c_str(), pSrcDS, 0, nullptr, nullptr, nullptr );
			CHECK(pDstDS != nullptr);

			if (pDstDS != nullptr)
			{
				// check some values in created dataset
				int sizeX = pDstDS->GetRasterXSize();
				int sizeY = pDstDS->GetRasterYSize();
				int rasterCount = pDstDS->GetRasterCount();

				LONGS_EQUAL(121, sizeX);
				LONGS_EQUAL(121, sizeY);
				LONGS_EQUAL(1, rasterCount);

				// close / reopen & check some values
				GDALClose(pDstDS);

				pDstDS = (GDALDataset *)GDALOpen(fileNameDst.c_str(), GA_ReadOnly);
				CHECK(pDstDS != nullptr);
				
				if (pDstDS != nullptr)
				{
					LONGS_EQUAL(121, pDstDS->GetRasterXSize());
					LONGS_EQUAL(121, pDstDS->GetRasterYSize());
					LONGS_EQUAL(1, pDstDS->GetRasterCount());

					GDALRasterBand* pBand = pDstDS->GetRasterBand(1);
					CHECK(pBand != nullptr);

					if (pBand != nullptr)
					{
						LONGS_EQUAL(121, pBand->GetXSize());
						LONGS_EQUAL(121, pBand->GetYSize());

						GDALDataType dataType = GDALGetRasterDataType(pBand);
						CHECK(dataType == GDALDataType::GDT_Int16);

						GDALColorInterp ci = pBand->GetColorInterpretation();
						LONGS_EQUAL(ci, GCI_Undefined);

						__int16* pScanline = (__int16 *)CPLMalloc(sizeof(__int16)*sizeX);

						// read first row

						pBand->RasterIO(
							GF_Read,			// read or write
							0,					// xoffset - pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.
							0,					// yoffset - line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.
							sizeX,				// width of the region of the band to be accessed in pixels
							1,					// height of the region of the band to be accessed in lines
							pScanline,			// data buffer
							sizeX,				// width of data buffer
							1,					// height of data buffer
							GDT_Int16,			// buffer type - data automatically translated
							0,					// pixel spacing
							0);					// line spacing

						LONGS_EQUAL(1773, pScanline[0]);
						LONGS_EQUAL(1778, pScanline[1]);
						LONGS_EQUAL(1752, pScanline[2]);
						LONGS_EQUAL(1782, pScanline[3]);

						LONGS_EQUAL(315, pScanline[117]);
						LONGS_EQUAL(306, pScanline[118]);
						LONGS_EQUAL(237, pScanline[119]);
						LONGS_EQUAL(180, pScanline[120]);

						CPLFree(pScanline);
					}
					GDALClose(pDstDS);
				}
			}

			GDALClose(pSrcDS);
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_E00GRID)
{
	// Test E00GRID driver.
	//

	constexpr char* DRIVER_NAME = "E00GRID";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "e00grid/fake_e00grid.e00");

		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			LONGS_EQUAL(0, pDS->GetLayerCount());
			LONGS_EQUAL(0, pDS->GetGCPCount());

			double geoTransform[6] = { 0 };
			CPLErr err = pDS->GetGeoTransform(geoTransform);
			CHECK(err == CPLErr::CE_None);

			if (err == CPLErr::CE_None)
			{
				DOUBLES_EQUAL(500000.0, geoTransform[0], 0.0);
				DOUBLES_EQUAL(1000.0, geoTransform[1], 0.0);
				DOUBLES_EQUAL(0.0, geoTransform[2], 0.0);
				DOUBLES_EQUAL(4000000.0, geoTransform[3], 0.0);
				DOUBLES_EQUAL(0.0, geoTransform[4], 0.0);
				DOUBLES_EQUAL(-1000.0, geoTransform[5], 0.0);
			}

			const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
			CHECK(pSRS != nullptr);

			CHECK(pSRS->IsProjected() == true);
			CHECK(pSRS->IsGeographic() == false);

			double semiMajor = pSRS->GetSemiMajor();
			double semiMinor = pSRS->GetSemiMinor();
			int axisCount = pSRS->GetAxesCount();
		
			DOUBLES_EQUAL(0.0, pSRS->GetProjParm("latitude_of_origin"), 0.0);
			DOUBLES_EQUAL(-93.0, pSRS->GetProjParm("central_meridian"), 0.0);
			DOUBLES_EQUAL(0.9996, pSRS->GetProjParm("scale_factor"), 0.0);
			DOUBLES_EQUAL(500000, pSRS->GetProjParm("false_easting"), 0.0);
			DOUBLES_EQUAL(0, pSRS->GetProjParm("false_northing"), 0.0);

			DOUBLES_EQUAL(-32767.0, pDS->GetRasterBand(1)->GetNoDataValue(), 0.0);
			XString unitType = pDS->GetRasterBand(1)->GetUnitType();
			CHECK(unitType.Compare("ft") == true);

			GDALClose(pDS);
		}
	}

	{
		// Test a fake E00GRID dataset, compressed and with statistics

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "e00grid/fake_e00grid_compressed.e00");

		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);
		
		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));
		
			double geoTransform[6] = { 0 };
			CPLErr err = pDS->GetGeoTransform(geoTransform);
			CHECK(err == CPLErr::CE_None);

			if (err == CPLErr::CE_None)
			{
				DOUBLES_EQUAL(500000.0, geoTransform[0], 0.0);
				DOUBLES_EQUAL(1000.0, geoTransform[1], 0.0);
				DOUBLES_EQUAL(0.0, geoTransform[2], 0.0);
				DOUBLES_EQUAL(4000000.0, geoTransform[3], 0.0);
				DOUBLES_EQUAL(0.0, geoTransform[4], 0.0);
				DOUBLES_EQUAL(-1000.0, geoTransform[5], 0.0);
			}

			LONGS_EQUAL(1, pDS->GetRasterCount());

			GDALRasterBand* pBand = pDS->GetRasterBand(1);
			CHECK(pBand != nullptr);

			if (pBand != nullptr)
			{
				double minElev, maxElev, meanElev, stdDev;
				err = pBand->GetStatistics(0, 1, &minElev, &maxElev, &meanElev, &stdDev);
				CHECK(err == CPLErr::CE_None);

				DOUBLES_EQUAL(1.0, minElev, 0.0);
				DOUBLES_EQUAL(50.0, maxElev, 0.0);
				DOUBLES_EQUAL(pBand->GetMinimum(), minElev, 0.0);
				DOUBLES_EQUAL(pBand->GetMaximum(), maxElev, 0.0);

				int sizeX = pBand->GetXSize();
				int sizeY = pBand->GetYSize();
				LONGS_EQUAL(5, sizeX);
				LONGS_EQUAL(4, sizeY);
			}
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_GeoTiff)
{
	// Test geotiff driver.
	//

	constexpr char* DRIVER_NAME = "GTIFF";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		// read test

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "geotiff/byte.tif");

		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		// make sure it instantiated the correct driver
		XString strDriverName = pDS->GetDriverName();
		CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

		int layerCount = pDS->GetLayerCount();
		int gcpCount = pDS->GetGCPCount();

		double geoTransform[6] = { 0 };
		CPLErr err = pDS->GetGeoTransform(geoTransform);

		CHECK(err == CPLErr::CE_None);
		if (err == CPLErr::CE_None)
		{
			double pixelSizeX = geoTransform[1];
			double pixelSizeY = geoTransform[5];

			DOUBLES_EQUAL(60.0, pixelSizeX, 0.0);
			DOUBLES_EQUAL(-60.0, pixelSizeY, 0.0);

			double originX = geoTransform[0];  // X Origin (top left corner)
			double originY = geoTransform[3];  // Y Origin (top left corner)

			DOUBLES_EQUAL(440720.0, originX, 0.0);
			DOUBLES_EQUAL(3751320.0, originY, 0.0);
		}

		// this returns the entire "prj" string
		XString strProj = pDS->GetProjectionRef();

		GDALDataset::RawBinaryLayout layout;
		bool getLayout = pDS->GetRawBinaryLayout(layout);
		CHECK(getLayout == true);

		if (getLayout)
		{
			CHECK(layout.osRawFilename == pDS->GetDescription());
			CHECK(layout.eInterleaving == GDALDataset::RawBinaryLayout::Interleaving::UNKNOWN);
			CHECK(layout.eDataType == GDT_Byte);
			CHECK(layout.bLittleEndianOrder == true);
			LONGS_EQUAL(8U, (long)layout.nImageOffset);
			LONGS_EQUAL(1, (long)layout.nPixelOffset);
			LONGS_EQUAL(20, (long)layout.nLineOffset);
			LONGS_EQUAL(0, (long)layout.nBandOffset);
		}

		GDALDataset::Bands bands = pDS->GetBands();
		LONGS_EQUAL(1, (long)bands.size());

		int sizeX = bands[0]->GetXSize();
		int sizeY = bands[0]->GetYSize();
		LONGS_EQUAL(20, sizeX);
		LONGS_EQUAL(20, sizeY);

		double bandMin = bands[0]->GetMinimum();
		double bandMax = bands[0]->GetMaximum();

		GDALRasterBand* pBand = pDS->GetRasterBand(1);
		CHECK(pBand != nullptr);

		LONGS_EQUAL(20, pBand->GetXSize());
		LONGS_EQUAL(20, pBand->GetYSize());

		GDALDataType dataType = GDALGetRasterDataType(pBand);
		CHECK(dataType == GDALDataType::GDT_Byte);

		float* pScanline = (float *)CPLMalloc(sizeof(float)*sizeX);

		pBand->RasterIO(
			GF_Read,			// read or write
			0,					// xoffset - pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.
			0,					// yoffset - line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.
			sizeX,				// width of the region of the band to be accessed in pixels
			1,					// height of the region of the band to be accessed in lines
			pScanline,			// data buffer
			sizeX,				// width of data buffer
			1,					// height of data buffer
			GDT_Float32,		// buffer type - data automatically translated
			0,					// pixel spacing
			0);					// line spacing

		DOUBLES_EQUAL(107.0, pScanline[0], 0.0);
		DOUBLES_EQUAL(123.0, pScanline[1], 0.0);
		DOUBLES_EQUAL(132.0, pScanline[2], 0.0);
		DOUBLES_EQUAL(115.0, pScanline[3], 0.0);
		DOUBLES_EQUAL(132.0, pScanline[4], 0.0);
		DOUBLES_EQUAL(132.0, pScanline[5], 0.0);
		DOUBLES_EQUAL(140.0, pScanline[6], 0.0);
		DOUBLES_EQUAL(132.0, pScanline[7], 0.0);
		DOUBLES_EQUAL(132.0, pScanline[8], 0.0);
		DOUBLES_EQUAL(132.0, pScanline[9], 0.0);

		DOUBLES_EQUAL(107.0, pScanline[10], 0.0);
		DOUBLES_EQUAL(132.0, pScanline[11], 0.0);
		DOUBLES_EQUAL(107.0, pScanline[12], 0.0);
		DOUBLES_EQUAL(132.0, pScanline[13], 0.0);
		DOUBLES_EQUAL(132.0, pScanline[14], 0.0);
		DOUBLES_EQUAL(107.0, pScanline[15], 0.0);
		DOUBLES_EQUAL(123.0, pScanline[16], 0.0);
		DOUBLES_EQUAL(115.0, pScanline[17], 0.0);
		DOUBLES_EQUAL(156.0, pScanline[18], 0.0);
		DOUBLES_EQUAL(148.0, pScanline[19], 0.0);

		// The pScanline buffer should be freed with CPLFree() when it is no longer used.
		CPLFree(pScanline);

		GDALClose(pDS);
	}

	{
		// read test

		// Collection of test GeoTIFF rasters
		std::vector<XString> files;
		files.push_back("geotiff/byte.tif");
		files.push_back("geotiff/int16.tif");
		files.push_back("geotiff/uint16.tif");
		files.push_back("geotiff/int32.tif");
		files.push_back("geotiff/uint32.tif");
		files.push_back("geotiff/float32.tif");
		files.push_back("geotiff/float64.tif");
		files.push_back("geotiff/cint16.tif");
		files.push_back("geotiff/cint32.tif");
		files.push_back("geotiff/cfloat32.tif");
		files.push_back("geotiff/cfloat64.tif");

		for each (XString file in files)
		{
			XString fileName = XString::CombinePath(gs_DataPath.c_str(), file.c_str());

			GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
			CHECK(pDS != nullptr);

			if (pDS != nullptr)
			{
				// make sure it instantiated the correct driver
				XString strDriverName = pDS->GetDriverName();
				CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

				LONGS_EQUAL(20, pDS->GetRasterXSize());
				LONGS_EQUAL(20, pDS->GetRasterYSize());

				GDALClose(pDS);
			}
		}
	}

	{
		// write test

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "geotiff/temp.tif");

		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);

		const int sizeX = 100;
		const int sizeY =  75;

		// example of how to pass create options
		char** pszOptions = nullptr;
		//pszOptions = CSLSetNameValue( pszOptions, "TILED", "YES" );
		//pszOptions = CSLSetNameValue( pszOptions, "COMPRESS", "PACKBITS" );
		pszOptions = CSLSetNameValue( pszOptions, "COMPRESS", "NONE" );

		GDALDataset* pDS = pDriver->Create(fileName.c_str(), sizeX, sizeY, 1, GDT_Byte, pszOptions);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			double geoTransform[6] = { 440720.0, 5.0, 0.0, 3751320.0, 0.0, -5.0 };
			CPLErr err = pDS->SetGeoTransform(geoTransform);
			CHECK(err == CPLErr::CE_None);

			OGRSpatialReference srs;
			OGRErr ogr_err = srs.importFromEPSG(4326);
			CHECK(ogr_err == OGRERR_NONE);

			err = pDS->SetSpatialRef(&srs);
			CHECK(err == CPLErr::CE_None);

			GDALRasterBand* pBand = pDS->GetRasterBand(1);
			CHECK(pBand != nullptr);

			if (pBand != nullptr)
			{
				unsigned char* pScanline = (unsigned char *)CPLMalloc(sizeof(unsigned char)*sizeX);

				for (int y = 0; y < sizeY; ++y)
				{
					for (int x = 0; x < sizeX; ++x)
						pScanline[x] = (unsigned char)x;

					pBand->RasterIO(
						GF_Write,			// read or write
						0,					// xoffset - pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.
						y,					// yoffset - line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.
						sizeX,				// width of the region of the band to be accessed in pixels
						1,					// height of the region of the band to be accessed in lines
						pScanline,			// data buffer
						sizeX,				// width of data buffer
						1,					// height of data buffer
						GDT_Byte,			// buffer type - data automatically translated
						0,					// pixel spacing
						0);					// line spacing
				}

				// The pScanline buffer should be freed with CPLFree() when it is no longer used.
				CPLFree(pScanline);
			}

			GDALClose(pDS);

			// re-open & read back

			GDALDataset* pReadDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
			CHECK(pReadDS != nullptr);

			if (pReadDS != nullptr)
			{
				// make sure it instantiated the correct driver
				XString strDriverName = pReadDS->GetDriverName();
				CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

				const OGRSpatialReference* pSRS = pReadDS->GetSpatialRef();
				CHECK(pSRS != nullptr);
				XString strName = pSRS->GetName();
				CHECK(strName.CompareNoCase("WGS 84"));
				LONGS_EQUAL(2, pSRS->GetAxesCount());
				LONGS_EQUAL(4326, pSRS->GetEPSGGeogCS());

				double geoTransform[6] = { 0.0 };
				CPLErr err = pReadDS->GetGeoTransform(geoTransform);
				CHECK(err == CPLErr::CE_None);

				//double geoTransform[6] = { 440720.0, 5.0, 0.0, 3751320.0, 0.0, -5.0 };
				DOUBLES_EQUAL(440720.0, geoTransform[0], 0.0);

				GDALClose(pReadDS);
			}
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_HF2)
{
	// Test HF2 driver.
	//

	constexpr char* DRIVER_NAME = "HF2";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
//TODO:
		//XString fileName = XString::CombinePath(gs_DataPath.c_str(), "hf2/MtStHelens_10m.hfz");

		//GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
		//CHECK(pDS != nullptr);

		//if (pDS != nullptr)
		//{
		//	// make sure it instantiated the correct driver
		//	XString strDriverName = pDS->GetDriverName();
		//	CHECK(strDriverName.Compare(DRIVER_NAME));

		//	GDALClose(pDS);
		//}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_HFA)
{
	// Test HFA driver.
	//

	constexpr char* DRIVER_NAME = "HFA";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "hfa/int.img");

		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			LONGS_EQUAL(1, pDS->GetRasterCount());
			GDALRasterBand* pBand = pDS->GetRasterBand(1);
			CHECK(pBand != nullptr);

			if (pBand != nullptr)
			{
				double minVal, maxVal, meanVal, stdDev;
				CPLErr err = pBand->GetStatistics(0, 1, &minVal, &maxVal, &meanVal, &stdDev);
				CHECK(err == CPLErr::CE_None);

				DOUBLES_EQUAL(40918.0, minVal, 0.0);
				DOUBLES_EQUAL(41134.0, maxVal, 0.0);
				DOUBLES_EQUAL(41019.784218148, meanVal, 1E-12);
				DOUBLES_EQUAL(44.637237445468, stdDev, 1E-12);
			}

			GDALClose(pDS);
		}
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "hfa/float.img");

		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			LONGS_EQUAL(1, pDS->GetRasterCount());
			GDALRasterBand* pBand = pDS->GetRasterBand(1);
			CHECK(pBand != nullptr);

			if (pBand != nullptr)
			{
				double minVal, maxVal, meanVal, stdDev;
				CPLErr err = pBand->GetStatistics(0, 1, &minVal, &maxVal, &meanVal, &stdDev);
				CHECK(err == CPLErr::CE_None);

				DOUBLES_EQUAL(40.91858291626, minVal, 1E-12);
				DOUBLES_EQUAL(41.134323120117, maxVal, 1E-12);
				DOUBLES_EQUAL(41.020284249223, meanVal, 1E-12);
				DOUBLES_EQUAL(0.044636441749041, stdDev, 1E-12);
			}

			GDALClose(pDS);
		}
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "hfa/87test.img");

		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			LONGS_EQUAL(1, pDS->GetRasterCount());
			GDALRasterBand* pBand = pDS->GetRasterBand(1);
			CHECK(pBand != nullptr);

			GDALClose(pDS);
		}
	}

	{
		//TODO:
		//commented this out because was generating warnings in output stream

		// Verify we read simple affine geotransforms properly.

		//XString fileName = XString::CombinePath(gs_DataPath.c_str(), "hfa/fg118-91.aux");

		//GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
		//CHECK(pDS != nullptr);

		//if (pDS != nullptr)
		//{
		//	// make sure it instantiated the correct driver
		//	XString strDriverName = pDS->GetDriverName();
			//CHECK(strDriverName.Compare(DRIVER_NAME));

		//	LONGS_EQUAL(1, pDS->GetRasterCount());

		//	double geoTransform[6] = { 0 };
		//	CPLErr err = pDS->GetGeoTransform(geoTransform);
		//	CHECK(CPLErr::CE_None == err);

		//	if (CPLErr::CE_None == err)
		//	{
		//		DOUBLES_EQUAL(11856857.07898215, geoTransform[0], 1E-6);
		//		DOUBLES_EQUAL(0.895867662235625, geoTransform[1], 1E-12);
		//		DOUBLES_EQUAL(0.02684252936279331, geoTransform[2], 1E-12);
		//		DOUBLES_EQUAL(7041861.472946444, geoTransform[3], 1E-12);
		//		DOUBLES_EQUAL(0.01962103617166367, geoTransform[4], 1E-12);
		//		DOUBLES_EQUAL(-0.9007880319529181, geoTransform[5], 1E-12);
		//	}

		//	GDALClose(pDS);
		//}
	}

	{
		// Confirm that we can read 8bit grayscale overviews for 1bit images.

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "hfa/small1bit.img");

		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			LONGS_EQUAL(1, pDS->GetRasterCount());
			GDALRasterBand* pBand = pDS->GetRasterBand(1);
			CHECK(pBand != nullptr);

			if (pBand != nullptr)
			{
				int ovc = pBand->GetOverviewCount();
				GDALRasterBand* pOvr = pBand->GetOverview(0);
				CHECK(pOvr != nullptr);
			
				//TODO:
				//in python can get checksum, but can find it for c/c++

				//band = ds.GetRasterBand(1)
				//ov = band.GetOverview(0)
				//assert ov.Checksum() == 4247, 'did not get expected overview checksum'
			}

			GDALClose(pDS);
		}
	}
}

////----------------------------------------------------------------------------
//TEST(GDAL, Raster_JPEG2000)
//{
//	// Test JPEG 2000 driver.
//	//
//	// There are a few JPEG 2000 drivers available in GDAL.
//	// 
//	// Wasn't able to get JPEG2000 driver (with libjasper) working. So am now trying 
//	// the JP2OpenJPEG driver based on the OpenJpeg library.
//	//
//
//	constexpr char* DRIVER_NAME = "JP2OpenJPEG";
//
//	{
//		// verify driver lookup
//		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
//		CHECK(pDriver != nullptr);
//	}
//
//	{
//		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "jpeg2000/byte.jp2");
//
//		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
//		CHECK(pDS != nullptr);
//
//		if (pDS != nullptr)
//		{
//			// make sure it instantiated the correct driver
//			XString strDriverName = pDS->GetDriverName();
//			CHECK(strDriverName.Compare(DRIVER_NAME));
//
//			double geoTransform[6] = { 0 };
//			CPLErr err = pDS->GetGeoTransform(geoTransform);
//			CHECK(CPLErr::CE_None == err);
//
//			if (CPLErr::CE_None == err)
//			{
//				DOUBLES_EQUAL(  440720.0, geoTransform[0], 0.0 );
//				DOUBLES_EQUAL(      60.0, geoTransform[1], 0.0 );
//				DOUBLES_EQUAL(       0.0, geoTransform[2], 0.0 );
//				DOUBLES_EQUAL( 3751320.0, geoTransform[3], 0.0 );
//				DOUBLES_EQUAL(       0.0, geoTransform[4], 0.0 );
//				DOUBLES_EQUAL(     -60.0, geoTransform[5], 0.0 );
//			}
//
//			const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
//			CHECK(pSRS != nullptr);
//
//			if (pSRS != nullptr)
//			{
//				// NAD27
//				int epsgCode = pSRS->GetEPSGGeogCS();
//				LONGS_EQUAL(epsgCode, 4267);
//
//				double pm = pSRS->GetPrimeMeridian();
//				XString strAxis0 = pSRS->GetAxis("PROJCS", 0, nullptr);
//				XString strAxis1 = pSRS->GetAxis("PROJCS", 1, nullptr);
//				int axisCount = pSRS->GetAxesCount();
//
//				DOUBLES_EQUAL(0.0, pm, 0.0);
//				CHECK(strAxis0.Compare("Easting"));
//				CHECK(strAxis1.Compare("Northing"));
//				LONGS_EQUAL(2, axisCount);
//
//				DOUBLES_EQUAL(0.0, pSRS->GetProjParm("latitude_of_origin"), 0.0);
//				DOUBLES_EQUAL(-117.0, pSRS->GetProjParm("central_meridian"), 0.0);
//				DOUBLES_EQUAL(0.9996, pSRS->GetProjParm("scale_factor"), 0.0);
//				DOUBLES_EQUAL(500000.0, pSRS->GetProjParm("false_easting"), 0.0);
//				DOUBLES_EQUAL(0.0, pSRS->GetProjParm("false_northing"), 0.0);
//			}
//
//			LONGS_EQUAL(1, pDS->GetRasterCount());
//			LONGS_EQUAL(100, pDS->GetRasterXSize());
//			LONGS_EQUAL(100, pDS->GetRasterYSize());
//
//			GDALRasterBand* pBand = pDS->GetRasterBand(1);
//			CHECK(pBand != nullptr);
//
//			if (pBand != nullptr)
//			{
//				double minVal, maxVal, meanVal, stdDev;
//				err = pBand->GetStatistics(0, 1, &minVal, &maxVal, &meanVal, &stdDev);
//				CHECK(err == CPLErr::CE_None);
//
//				DOUBLES_EQUAL(0.0, minVal, 0.0);
//				DOUBLES_EQUAL(255.0, maxVal, 0.0);
//
//				int sizeX = pBand->GetXSize();
//				int sizeY = pBand->GetYSize();
//
//				LONGS_EQUAL(GDALDataType::GDT_Byte, pBand->GetRasterDataType());
//				DOUBLES_EQUAL(1.0, pBand->GetScale(), 0.0);
//				LONGS_EQUAL(100, sizeX);
//				LONGS_EQUAL(100, sizeY);
//
//				// verify subset of image data
//
//				unsigned char* pScanline = (unsigned char *)CPLMalloc(sizeof(unsigned char)*sizeX);
//
//				// read first line
//
//				pBand->RasterIO(
//					GF_Read,			// read or write
//					0,					// xoffset - zero based offset from left
//					0,					// yoffset - zero based offset from top
//					sizeX,				// width of the region of the band to be accessed in pixels
//					1,					// height of the region of the band to be accessed in lines
//					pScanline,			// data buffer
//					sizeX,				// width of data buffer
//					1,					// height of data buffer
//					GDT_Byte,			// buffer type - data automatically translated
//					0,					// pixel spacing
//					0);					// line spacing
//
//				// these values were verified against external gis software
//				LONGS_EQUAL(107, pScanline[0]);
//				LONGS_EQUAL(123, pScanline[1]);
//				LONGS_EQUAL(132, pScanline[2]);
//				LONGS_EQUAL(115, pScanline[3]);
//				LONGS_EQUAL(132, pScanline[4]);
//
//				LONGS_EQUAL(230, pScanline[97]);
//				LONGS_EQUAL(206, pScanline[98]);
//				LONGS_EQUAL(197, pScanline[99]);
//
//				// read last line
//
//				pBand->RasterIO(
//					GF_Read,			// read or write
//					0,					// xoffset - zero based offset from left
//					99,					// yoffset - zero based offset from top
//					sizeX,				// width of the region of the band to be accessed in pixels
//					1,					// height of the region of the band to be accessed in lines
//					pScanline,			// data buffer
//					sizeX,				// width of data buffer
//					1,					// height of data buffer
//					GDT_Byte,			// buffer type - data automatically translated
//					0,					// pixel spacing
//					0);					// line spacing
//
//				// these values were verified against external gis software
//				LONGS_EQUAL(132, pScanline[0]);
//				LONGS_EQUAL(173, pScanline[1]);
//				LONGS_EQUAL(156, pScanline[2]);
//				LONGS_EQUAL(148, pScanline[3]);
//				LONGS_EQUAL( 25, pScanline[4]);
//
//				LONGS_EQUAL( 90, pScanline[97]);
//				LONGS_EQUAL(123, pScanline[98]);
//				LONGS_EQUAL(165, pScanline[99]);
//
//				// The pScanline buffer should be freed with CPLFree() when it is no longer used.
//				CPLFree(pScanline);
//			}
//
//			GDALClose(pDS);
//		}
//	}
//
//	{
//		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "jpeg2000/int16.jp2");
//
//		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
//		CHECK(pDS != nullptr);
//
//		if (pDS != nullptr)
//		{
//			// make sure it instantiated the correct driver
//			XString strDriverName = pDS->GetDriverName();
//			CHECK(strDriverName.Compare(DRIVER_NAME));
//
//			GDALClose(pDS);
//		}
//	}
//
//	{
//		//XString fileName = XString::CombinePath(gs_DataPath.c_str(), "jpeg2000/ll.jp2");
//
////TODO:
//		//Get this on open:
//		//Warning 1: It is likely that the axis order of the GMLJP2 box is not consistent
//		//with the EPSG order and that the resulting georeferencing will be incorrect. Try
//		//setting GDAL_IGNORE_AXIS_ORIENTATION=TRUE if it is the case
//
//		//GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
//		//CHECK(pDS != nullptr);
//
//		//if (pDS != nullptr)
//		{
//			// make sure it instantiated the correct driver
//			//XString strDriverName = pDS->GetDriverName();
//			//CHECK(strDriverName.Compare(DRIVER_NAME));
//
//			//GDALClose(pDS);
//		}
//	}
//
//	{
//		//TODO:
//		//don't currently auto-open zipped files
//
//		//XString fileName = XString::CombinePath(gs_DataPath.c_str(), "jpeg2000/byte.jp2.gz");
//
//		//GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
//		//CHECK(pDS != nullptr);
//
//		//if (pDS != nullptr)
//		//{
//		//	// make sure it instantiated the correct driver
//		//	XString strDriverName = pDS->GetDriverName();
//		//	CHECK(strDriverName.Compare(DRIVER_NAME));
//
//			//GDALClose(pDS);
//		//}
//	}
//
//	{
//		// Test a JP2OpenJPEG with the 3 bands having 13bit depth and the 4th one 1 bit
//
//		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "jpeg2000/3_13bit_and_1bit.jp2");
//
//		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
//		CHECK(pDS != nullptr);
//
//		if (pDS != nullptr)
//		{
//			// make sure it instantiated the correct driver
//			XString strDriverName = pDS->GetDriverName();
//			CHECK(strDriverName.Compare(DRIVER_NAME));
//
//			GDALRasterBand* pBand = pDS->GetRasterBand(1);
//			CHECK(pBand != nullptr);
//			
//			if (pBand != nullptr)
//			{
//				CHECK(pBand->GetRasterDataType() == GDT_UInt16);
//			}
//
//			GDALClose(pDS);
//		}
//	}
//
//	{
//		// Check that we can use .j2w world files (#4651)
//
//		// The sidecar file is byte_without_geotransform.j2w and looks like:
//		//		60.0000000000
//		//		0.0000000000
//		//		0.0000000000
//		//		-60.0000000000
//		//		440750.0000000000
//		//		3751290.0000000000
//
//		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "jpeg2000/byte_without_geotransform.jp2");
//
//		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
//		CHECK(pDS != nullptr);
//
//		if (pDS != nullptr)
//		{
//			// make sure it instantiated the correct driver
//			XString strDriverName = pDS->GetDriverName();
//			CHECK(strDriverName.Compare(DRIVER_NAME));
//
//			double geoTransform[6] = { 0 };
//			CPLErr err = pDS->GetGeoTransform(geoTransform);
//			CHECK(CPLErr::CE_None == err);
//
//			if (CPLErr::CE_None == err)
//			{
//				DOUBLES_EQUAL(  440720.0, geoTransform[0], 0.0 );
//				DOUBLES_EQUAL(      60.0, geoTransform[1], 0.0 );
//				DOUBLES_EQUAL(       0.0, geoTransform[2], 0.0 );
//				DOUBLES_EQUAL( 3751320.0, geoTransform[3], 0.0 );
//				DOUBLES_EQUAL(       0.0, geoTransform[4], 0.0 );
//				DOUBLES_EQUAL(     -60.0, geoTransform[5], 0.0 );
//			}
//
//			GDALClose(pDS);
//		}
//	}
//
//	{
//		// Check that we get GCPs even there's no projection info
//
//		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "jpeg2000/byte_2gcps.jp2");
//
//		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
//		CHECK(pDS != nullptr);
//
//		if (pDS != nullptr)
//		{
//			// make sure it instantiated the correct driver
//			XString strDriverName = pDS->GetDriverName();
//			CHECK(strDriverName.Compare(DRIVER_NAME));
//
//			LONGS_EQUAL(2, pDS->GetGCPCount());
//
//			GDALClose(pDS);
//		}
//	}
//
//	{
//		// Test reading PixelIsPoint file (#5437)
//
//		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "jpeg2000/byte_point.jp2");
//		
//		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
//		CHECK(pDS != nullptr);
//
//		if (pDS != nullptr)
//		{
//			// make sure it instantiated the correct driver
//			XString strDriverName = pDS->GetDriverName();
//			CHECK(strDriverName.Compare(DRIVER_NAME));
//
//			double geoTransform[6] = { 0 };
//			CPLErr err = pDS->GetGeoTransform(geoTransform);
//			CHECK(CPLErr::CE_None == err);
//
//			if (CPLErr::CE_None == err)
//			{
//				DOUBLES_EQUAL(  440690.0, geoTransform[0], 0.0 );
//				DOUBLES_EQUAL(      60.0, geoTransform[1], 0.0 );
//				DOUBLES_EQUAL(       0.0, geoTransform[2], 0.0 );
//				DOUBLES_EQUAL( 3751350.0, geoTransform[3], 0.0 );
//				DOUBLES_EQUAL(       0.0, geoTransform[4], 0.0 );
//				DOUBLES_EQUAL(     -60.0, geoTransform[5], 0.0 );
//			}
//
//			XString strAreaOrPoint = pDS->GetMetadataItem("AREA_OR_POINT");
//			CHECK(strAreaOrPoint.Compare("Point") == true);
//
//			GDALClose(pDS);
//
//			CPLSetConfigOption("GTIFF_POINT_GEO_IGNORE", "TRUE");
//
//			pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
//			CHECK(pDS != nullptr);
//
//			err = pDS->GetGeoTransform(geoTransform);
//			CHECK(CPLErr::CE_None == err);
//
//			if (CPLErr::CE_None == err)
//			{
//				DOUBLES_EQUAL(  440720.0, geoTransform[0], 0.0 );
//				DOUBLES_EQUAL(      60.0, geoTransform[1], 0.0 );
//				DOUBLES_EQUAL(       0.0, geoTransform[2], 0.0 );
//				DOUBLES_EQUAL( 3751320.0, geoTransform[3], 0.0 );
//				DOUBLES_EQUAL(       0.0, geoTransform[4], 0.0 );
//				DOUBLES_EQUAL(     -60.0, geoTransform[5], 0.0 );
//			}
//		
//			CPLSetConfigOption("GTIFF_POINT_GEO_IGNORE", nullptr);
//
//			GDALClose(pDS);
//		}
//	}
//
//	{
//		// Test reading file where GMLJP2 has nul character instead of \n (#5760)
//
//		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "jpeg2000/byte_point.jp2");
//		
//		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
//		CHECK(pDS != nullptr);
//
//		if (pDS != nullptr)
//		{
//			// make sure it instantiated the correct driver
//			XString strDriverName = pDS->GetDriverName();
//			CHECK(strDriverName.Compare(DRIVER_NAME));
//
//			CHECK(pDS->GetProjectionRef() != nullptr);
//
//			GDALClose(pDS);
//		}
//	}
//
//	{
//		// Get structure of a JPEG2000 file
//
//		//ret = gdal.GetJPEG2000StructureAsString('data/jpeg2000/byte.jp2', ['ALL=YES'])
//		//assert ret is not None
//
//		//ret = gdal.GetJPEG2000StructureAsString('data/jpeg2000/byte_tlm_plt.jp2', ['ALL=YES'])
//		//assert ret is not None
//
//		//ret = gdal.GetJPEG2000StructureAsString('data/jpeg2000/byte_one_poc.j2k', ['ALL=YES'])
//		//assert ret is not None
//	}
//
//	{
//		// Test reading a dataset whose tile dimensions are larger than dataset ones
//
//		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "jpeg2000/byte_tile_2048.jp2");
//		
//		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
//		CHECK(pDS != nullptr);
//
//		if (pDS != nullptr)
//		{
//			GDALRasterBand* pBand = pDS->GetRasterBand(1);
//			CHECK(pBand != nullptr);
//
//			if (pBand != nullptr)
//			{
//				int sizeX = 0;
//				int sizeY = 0;
//				pBand->GetBlockSize(&sizeX, &sizeY);
//				LONGS_EQUAL(20, sizeX);
//				LONGS_EQUAL(20, sizeY);
//			}
//
//			GDALClose(pDS);
//		}
//	}
//
//	{
//		// Test opening an image of small dimension with very small tiles (#7012)
//
//		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "jpeg2000/fake_sent2_preview.jp2");
//		
//		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
//		CHECK(pDS != nullptr);
//
//		if (pDS != nullptr)
//		{
//			GDALRasterBand* pBand = pDS->GetRasterBand(1);
//			CHECK(pBand != nullptr);
//
//			if (pBand != nullptr)
//			{
//				int sizeX = 0;
//				int sizeY = 0;
//				pBand->GetBlockSize(&sizeX, &sizeY);
//				LONGS_EQUAL(343, sizeX);
//				LONGS_EQUAL(343, sizeY);
//				LONGS_EQUAL(sizeX, pDS->GetRasterXSize());
//				LONGS_EQUAL(sizeY, pDS->GetRasterYSize());
//			}
//			GDALClose(pDS);
//		}
//	}
//}
//
//----------------------------------------------------------------------------
TEST(GDAL, Raster_MEM)
{
	// Test MEM driver.
	//
	// The MEM driver is for internal use, not end-user visible.
	//

	constexpr char* DRIVER_NAME = "MEM";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName("MEM"));
		CHECK(pDriver != nullptr);

		GDALDataset* pDS = pDriver->Create("", 100, 100, 3, GDALDataType::GDT_Byte, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			LONGS_EQUAL(3, pDS->GetRasterCount());

			GDALClose(pDS);
		}
	}

	{
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName("MEM"));
		CHECK(pDriver != nullptr);

		GDALDataset* pDS = pDriver->Create("", 1, 1, 2, GDALDataType::GDT_Byte, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			CPLErr err = pDS->GetRasterBand(1)->Fill(255);
			CHECK(err == CPLErr::CE_None);

			err = pDS->GetRasterBand(2)->Fill(0xAB);
			CHECK(err == CPLErr::CE_None);

			LONGS_EQUAL(2, pDS->GetRasterCount());

			GDALClose(pDS);
		}
	}

	{
		// Test set/get values.

		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName("MEM"));
		CHECK(pDriver != nullptr);

		GDALDataset* pDS = pDriver->Create("test.mem", 50, 100, 2, GDALDataType::GDT_Byte, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// set some values

			double geoTransform[6] = { 440720.0, 5.0, 0.0, 3751320.0, 0.0, -5.0 };
			CPLErr err = pDS->SetGeoTransform(geoTransform);
			CHECK(err == CPLErr::CE_None);

			LONGS_EQUAL(2, pDS->GetRasterCount());

			err = pDS->GetRasterBand(1)->Fill(255);
			CHECK(err == CPLErr::CE_None);

			err = pDS->GetRasterBand(2)->Fill(0xAB);
			CHECK(err == CPLErr::CE_None);

			pDS->GetRasterBand(1)->SetNoDataValue(-1.0);
			CHECK(err == CPLErr::CE_None);

			// read them back

			err = pDS->GetGeoTransform(geoTransform);
			CHECK(err == CPLErr::CE_None);
			DOUBLES_EQUAL(  440720.0, geoTransform[0], 0.0 );
			DOUBLES_EQUAL(       5.0, geoTransform[1], 0.0 );
			DOUBLES_EQUAL(       0.0, geoTransform[2], 0.0 );
			DOUBLES_EQUAL( 3751320.0, geoTransform[3], 0.0 );
			DOUBLES_EQUAL(       0.0, geoTransform[4], 0.0 );
			DOUBLES_EQUAL(      -5.0, geoTransform[5], 0.0 );

			double nd = pDS->GetRasterBand(1)->GetNoDataValue();
			DOUBLES_EQUAL(-1.0, nd, 0.0);

			GDALClose(pDS);
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_NGSGEOID)
{
	// Test NGSGEOID driver.
	//

	constexpr char* DRIVER_NAME = "NGSGEOID";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		// Test opening a little endian file

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "ngsgeoid/g2009u01_le_truncated.bin");

		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			double geoTransform[6] = { 0 };
			CPLErr err = pDS->GetGeoTransform(geoTransform);
			CHECK(CPLErr::CE_None == err);

			if (CPLErr::CE_None == err)
			{
				DOUBLES_EQUAL(229.99166666666667, geoTransform[0], 0.0);
				DOUBLES_EQUAL(0.016666666666670001, geoTransform[1], 0.0);
				DOUBLES_EQUAL(0.0, geoTransform[2], 0.0);
				DOUBLES_EQUAL(40.00833333333334, geoTransform[3], 0.0);
				DOUBLES_EQUAL(0.0, geoTransform[4], 0.0);
				DOUBLES_EQUAL(-0.016666666666670001, geoTransform[5], 0.0);
			}

			const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
			CHECK(pSRS != nullptr);

			if (pSRS != nullptr)
			{
				// WGS84
				int epsgCode = pSRS->GetEPSGGeogCS();
				LONGS_EQUAL(epsgCode, 4326);
			}

			GDALClose(pDS);
		}
	}

	{
		// Test opening a big endian file

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "ngsgeoid/g2009u01_be_truncated.bin");

		GDALDataset* pDS = GDALDataset::Open(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			double geoTransform[6] = { 0 };
			CPLErr err = pDS->GetGeoTransform(geoTransform);
			CHECK(CPLErr::CE_None == err);

			if (CPLErr::CE_None == err)
			{
				DOUBLES_EQUAL(229.99166666666667, geoTransform[0], 1E-12);
				DOUBLES_EQUAL(0.016666666666670001, geoTransform[1], 1E-12);
				DOUBLES_EQUAL(0.0, geoTransform[2], 1E-12);
				DOUBLES_EQUAL(40.00833333333334, geoTransform[3], 1E-12);
				DOUBLES_EQUAL(0.0, geoTransform[4], 1E-12);
				DOUBLES_EQUAL(-0.016666666666670001, geoTransform[5], 1E-12);
			}

			const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
			CHECK(pSRS != nullptr);

			if (pSRS != nullptr)
			{
				// WGS84
				int epsgCode = pSRS->GetEPSGGeogCS();
				LONGS_EQUAL(epsgCode, 4326);
			}

			GDALClose(pDS);
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_SDTS)
{
	// Test SDTS DEM file.
	//

	constexpr char* DRIVER_NAME = "SDTS";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		// USGS DEM file in SDTS format
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "sdts_raster/1646293.DEM.SDTS/7974CATD.DDF");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			XString str = pDS->GetDescription();

			int sizeX = pDS->GetRasterXSize();
			int sizeY = pDS->GetRasterYSize();
			int rasterCount = pDS->GetRasterCount();
			int layerCount = pDS->GetLayerCount();

			CHECK(pDS->GetAccess() == GA_ReadOnly);
			LONGS_EQUAL(983, sizeX);
			LONGS_EQUAL(1394, sizeY);
			LONGS_EQUAL(1, rasterCount);
			LONGS_EQUAL(0, layerCount);

			GDALDataset::Bands bands = pDS->GetBands();
			LONGS_EQUAL(1, (int)bands.size());
		
			// bands are 1 based
			GDALRasterBand* pBand = pDS->GetRasterBand(1);
			CHECK(pBand != nullptr);

			if (pBand != nullptr)
			{
				XString strUnitType = pBand->GetUnitType();
				int blockX, blockY;
				pBand->GetBlockSize(&blockX, &blockY);
				LONGS_EQUAL(983, blockX);
				LONGS_EQUAL(1, blockY);

				double minElev, maxElev, meanElev, stdDev;
				CPLErr err = pBand->ComputeStatistics(0, &minElev, &maxElev, &meanElev, &stdDev, nullptr, nullptr);
				DOUBLES_EQUAL(2.0, minElev, 0.0);
				DOUBLES_EQUAL(646.0, maxElev, 0.0);
				DOUBLES_EQUAL(163.24843077396238, meanElev, 0.0);
				DOUBLES_EQUAL(118.71023603570316, stdDev, 0.0);

				DOUBLES_EQUAL(pBand->GetMinimum(), minElev, 0.0);
				DOUBLES_EQUAL(pBand->GetMaximum(), maxElev, 0.0);

				CHECK(GDT_Int16 == pBand->GetRasterDataType());
				DOUBLES_EQUAL(1.0, pBand->GetScale(), 0.0);

				GDALColorInterp ci = pBand->GetColorInterpretation();
				LONGS_EQUAL(ci, GCI_Undefined);

				int success = 0;
				double val = pBand->GetNoDataValue(&success);
				LONGS_EQUAL(1, success);
				DOUBLES_EQUAL(-32766.0, val, 0.0);
			}

			const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
			CHECK(pSRS != nullptr);

			if (pSRS != nullptr)
			{
				CHECK(pSRS->IsProjected() == true);
				CHECK(pSRS->IsGeographic() == false);
				CHECK(pSRS->IsCompound() == false);
				LONGS_EQUAL(2, pSRS->GetAxesCount());
				DOUBLES_EQUAL(0.017453292519943295, pSRS->GetAngularUnits(), 0.0);
				DOUBLES_EQUAL(1.0, pSRS->GetLinearUnits(), 0.0);

				OSRAxisMappingStrategy strat =  pSRS->GetAxisMappingStrategy();
				LONGS_EQUAL(OAMS_TRADITIONAL_GIS_ORDER, strat);

				DOUBLES_EQUAL(0.082271854223004356, pSRS->GetEccentricity(), 0.0);
				LONGS_EQUAL(4267, pSRS->GetEPSGGeogCS());
			}

			// verify subset of elevation data

			float* pScanline = (float *)CPLMalloc(sizeof(float)*sizeX);

			// read first line

			pBand->RasterIO(
				GF_Read,			// read or write
				0,					// xoffset - zero based offset from left
				0,					// yoffset - zero based offset from top
				sizeX,				// width of the region of the band to be accessed in pixels
				1,					// height of the region of the band to be accessed in lines
				pScanline,			// data buffer
				sizeX,				// width of data buffer
				1,					// height of data buffer
				GDT_Float32,		// buffer type - data automatically translated
				0,					// pixel spacing
				0);					// line spacing

			DOUBLES_EQUAL(-32766.0, pScanline[0], 0.0);
			DOUBLES_EQUAL(-32766.0, pScanline[1], 0.0);
			DOUBLES_EQUAL(-32766.0, pScanline[2], 0.0);
			DOUBLES_EQUAL(-32766.0, pScanline[3], 0.0);
			DOUBLES_EQUAL(-32766.0, pScanline[4], 0.0);

			pBand->RasterIO(
				GF_Read,			// read or write
				0,					// xoffset - zero based offset from left
				1000,				// yoffset - zero based offset from top
				sizeX,				// width of the region of the band to be accessed in pixels
				1,					// height of the region of the band to be accessed in lines
				pScanline,			// data buffer
				sizeX,				// width of data buffer
				1,					// height of data buffer
				GDT_Float32,		// buffer type - data automatically translated
				0,					// pixel spacing
				0);					// line spacing

			DOUBLES_EQUAL(-32766.000000, pScanline[0], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[1], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[2], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[3], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[4], 0.0);
			DOUBLES_EQUAL(144.000000, pScanline[5], 0.0);
			DOUBLES_EQUAL(145.000000, pScanline[6], 0.0);
			DOUBLES_EQUAL(146.000000, pScanline[7], 0.0);
			DOUBLES_EQUAL(147.000000, pScanline[8], 0.0);

			DOUBLES_EQUAL(179.000000, pScanline[500], 0.0);
			DOUBLES_EQUAL(188.000000, pScanline[501], 0.0);
			DOUBLES_EQUAL(197.000000, pScanline[502], 0.0);
			DOUBLES_EQUAL(208.000000, pScanline[503], 0.0);
			DOUBLES_EQUAL(216.000000, pScanline[504], 0.0);
			DOUBLES_EQUAL(222.000000, pScanline[505], 0.0);
			DOUBLES_EQUAL(227.000000, pScanline[506], 0.0);
			DOUBLES_EQUAL(232.000000, pScanline[507], 0.0);

			DOUBLES_EQUAL(233.000000, pScanline[975], 0.0);
			DOUBLES_EQUAL(233.000000, pScanline[976], 0.0);
			DOUBLES_EQUAL(233.000000, pScanline[977], 0.0);
			DOUBLES_EQUAL(233.000000, pScanline[978], 0.0);
			DOUBLES_EQUAL(233.000000, pScanline[979], 0.0);
			DOUBLES_EQUAL(232.000000, pScanline[980], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[981], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[982], 0.0);

			// read last line

			pBand->RasterIO(
				GF_Read,			// read or write
				0,					// xoffset - zero based offset from left
				1393,				// yoffset - zero based offset from top
				sizeX,				// width of the region of the band to be accessed in pixels
				1,					// height of the region of the band to be accessed in lines
				pScanline,			// data buffer
				sizeX,				// width of data buffer
				1,					// height of data buffer
				GDT_Float32,		// buffer type - data automatically translated
				0,					// pixel spacing
				0);					// line spacing

			DOUBLES_EQUAL(-32766.000000, pScanline[0], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[1], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[2], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[3], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[4], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[5], 0.0);
			DOUBLES_EQUAL(137.000000, pScanline[6], 0.0);
			DOUBLES_EQUAL(138.000000, pScanline[7], 0.0);
			DOUBLES_EQUAL(138.000000, pScanline[8], 0.0);
			DOUBLES_EQUAL(140.000000, pScanline[9], 0.0);
			DOUBLES_EQUAL(140.000000, pScanline[10], 0.0);

			DOUBLES_EQUAL(-32766.000000, pScanline[400], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[401], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[402], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[403], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[404], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[405], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[406], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[407], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[408], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[409], 0.0);

			DOUBLES_EQUAL(-32766.000000, pScanline[979], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[980], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[981], 0.0);
			DOUBLES_EQUAL(-32766.000000, pScanline[982], 0.0);

			// The pScanline buffer should be freed with CPLFree() when it is no longer used.
			CPLFree(pScanline);
		}

		GDALClose(pDS);
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_Sentinel2)
{
	// Test Sentinel2 file.
	//
	//
	// SENTINEL-2 data are acquired on 13 spectral bands in the visible and near-infrared (VNIR) and Short-wavelength 
	// infrared (SWIR) spectrum, as show in the below table:
	//
	// Band name	Resolution (m)	Central wavelength (nm)	Band width (nm)	Purpose
	// B01			60				443						20				Aerosol detection
	// B02			10				490						65				Blue
	// B03			10				560						35				Green
	// B04			10				665						30				Red
	// B05			20				705						15				Vegetation classification
	// B06			20				740						15				Vegetation classification
	// B07			20				783						20				Vegetation classification
	// B08			10				842						115				Near infrared
	// B08A			20				865						20				Vegetation classification
	// B09			60				945						20				Water vapour
	// B10			60				1375					30				Cirrus
	// B11			20				1610					90				Snow / ice / cloud discrimination
	// B12			20				2190					180				Snow / ice / cloud disc
	//

	constexpr char* DRIVER_NAME = "SENTINEL2";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		// Level 1B

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "sentinel2/fake_l1b/S2B_OPER_PRD_MSIL1B.SAFE/S2B_OPER_MTD_SAFL1B.xml");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			XString md = pDS->GetMetadataItem("CLOUD_COVERAGE_ASSESSMENT");
			CHECK(md.Compare("0.0"));
			md = pDS->GetMetadataItem("DATATAKE_1_DATATAKE_SENSING_START");
			CHECK(md.Compare("2015-12-31T23:59:59.999Z"));
			md = pDS->GetMetadataItem("DATATAKE_1_DATATAKE_TYPE");
			CHECK(md.Compare("INS-NOBS"));

			// looks like we need to retrieve the "SUBDATASETS", then 
			// use those ourself to access image data - but they are not image file names, they
			// all seem to point into the same xml file, with different "filters"
			char** pp = pDS->GetMetadata("SUBDATASETS");
			XString s1_name = pp[0];
			XString s1_desc = pp[1];
			XString s2_name = pp[2];
			XString s2_desc = pp[3];
			XString s3_name = pp[4];
			XString s3_desc = pp[5];

			// if raster count is always 0, how do you read the imagery?
			// may be "sub data sets"?

			int rasterCount = pDS->GetRasterCount();

			LONGS_EQUAL(512, pDS->GetRasterXSize());
			LONGS_EQUAL(512, pDS->GetRasterYSize());

			GDALClose(pDS);
		}
	}

	{
		// Level 1C

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "sentinel2/fake_l1c/S2A_OPER_PRD_MSIL1C.SAFE/S2A_OPER_MTD_SAFL1C.xml");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			XString md = pDS->GetMetadataItem("CLOUD_COVERAGE_ASSESSMENT");
			CHECK(md.Compare("0.0"));
			md = pDS->GetMetadataItem("DATATAKE_1_DATATAKE_SENSING_START");
			CHECK(md.Compare("2015-12-31T23:59:59.999Z"));
			md = pDS->GetMetadataItem("DATATAKE_1_DATATAKE_TYPE");
			CHECK(md.Compare("INS-NOBS"));
			md = pDS->GetMetadataItem("REFERENCE_BAND");
			CHECK(md.Compare("B1"));

			// looks like we need to retrieve the "SUBDATASETS", then 
			// use those ourself to access image data
			char** pp = pDS->GetMetadata("SUBDATASETS");
			XString s1_name = pp[0];
			XString s1_desc = pp[1];
			XString s2_name = pp[2];
			XString s2_desc = pp[3];
			XString s3_name = pp[4];
			XString s3_desc = pp[5];
			XString s4_name = pp[6];
			XString s4_desc = pp[7];
			
			//int n = pDS->GetRasterCount();
			LONGS_EQUAL(512, pDS->GetRasterXSize());
			LONGS_EQUAL(512, pDS->GetRasterYSize());

			GDALClose(pDS);
		}
	}

	{
		// Level 1C Safe Compact

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "sentinel2/fake_l1c_safecompact/S2A_MSIL1C_test.SAFE/MTD_MSIL1C.xml");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			XString md = pDS->GetMetadataItem("CLOUD_COVERAGE_ASSESSMENT");
			CHECK(md.Compare("0.0"));
			md = pDS->GetMetadataItem("DATATAKE_1_DATATAKE_SENSING_START");
			CHECK(md.Compare("2015-12-31T23:59:59.999Z"));
			md = pDS->GetMetadataItem("DATATAKE_1_DATATAKE_TYPE");
			CHECK(md.Compare("INS-NOBS"));

			char** pp = pDS->GetMetadata("SUBDATASETS");
			XString s1_name = pp[0];
			XString s1_desc = pp[1];
			XString s2_name = pp[2];
			XString s2_desc = pp[3];
			XString s3_name = pp[4];
			XString s3_desc = pp[5];
			XString s4_name = pp[6];
			XString s4_desc = pp[7];

			int n = pDS->GetRasterCount();
			LONGS_EQUAL(512, pDS->GetRasterXSize());
			LONGS_EQUAL(512, pDS->GetRasterYSize());

			GDALClose(pDS);
		}
	}

	{
		// Level 2A

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "sentinel2/fake_l2a/S2A_USER_PRD_MSIL2A.SAFE/S2A_USER_MTD_SAFL2A.xml");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			XString md = pDS->GetMetadataItem("CLOUD_COVERAGE_ASSESSMENT");
			CHECK(md.Compare("0.0"));
			md = pDS->GetMetadataItem("DATATAKE_1_DATATAKE_SENSING_START");
			CHECK(md.Compare("2015-12-31T23:59:59.999Z"));
			md = pDS->GetMetadataItem("DATATAKE_1_DATATAKE_TYPE");
			CHECK(md.Compare("INS-NOBS"));

			char** pp = pDS->GetMetadata("SUBDATASETS");
			XString s1_name = pp[0];
			XString s1_desc = pp[1];
			XString s2_name = pp[2];
			XString s2_desc = pp[3];
		
			LONGS_EQUAL(512, pDS->GetRasterXSize());
			LONGS_EQUAL(512, pDS->GetRasterYSize());

			GDALClose(pDS);
		}
	}

	{
		// Level 2A MSIL2A

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "sentinel2/fake_l2a_MSIL2A/S2A_MSIL2A_20180818T094031_N0208_R036_T34VFJ_20180818T120345.SAFE/MTD_MSIL2A.xml");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			XString md = pDS->GetMetadataItem("CLOUD_COVERAGE_ASSESSMENT");
			CHECK(md.Compare("54.4"));
			md = pDS->GetMetadataItem("DATATAKE_1_DATATAKE_SENSING_START");
			CHECK(md.Compare("2018-08-18T09:40:31.024Z"));
			md = pDS->GetMetadataItem("DATATAKE_1_DATATAKE_TYPE");
			CHECK(md.Compare("INS-NOBS"));

			char** pp = pDS->GetMetadata("SUBDATASETS");
			XString s1_name = pp[0];
			XString s1_desc = pp[1];
			XString s2_name = pp[2];
			XString s2_desc = pp[3];
			XString s3_name = pp[4];
			XString s3_desc = pp[5];
			XString s4_name = pp[6];
			XString s4_desc = pp[7];

			LONGS_EQUAL(512, pDS->GetRasterXSize());
			LONGS_EQUAL(512, pDS->GetRasterYSize());

			GDALClose(pDS);
		}
	}

	{
		// Level 2A MSIL2Ap

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "sentinel2/fake_l2a_MSIL2Ap/S2A_MSIL2A_20170823T094031_N0205_R036_T34VFJ_20170823T094252.SAFE/MTD_MSIL2A.xml");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			XString md = pDS->GetMetadataItem("CLOUD_COVERAGE_ASSESSMENT");
			CHECK(md.Compare("86.3"));
			md = pDS->GetMetadataItem("DATATAKE_1_DATATAKE_SENSING_START");
			CHECK(md.Compare("2017-08-23T09:40:31.026Z"));
			md = pDS->GetMetadataItem("DATATAKE_1_DATATAKE_TYPE");
			CHECK(md.Compare("INS-NOBS"));

			char** pp = pDS->GetMetadata("SUBDATASETS");
			XString s1_name = pp[0];
			XString s1_desc = pp[1];
			XString s2_name = pp[2];
			XString s2_desc = pp[3];
			XString s3_name = pp[4];
			XString s3_desc = pp[5];
			XString s4_name = pp[6];
			XString s4_desc = pp[7];

			LONGS_EQUAL(512, pDS->GetRasterXSize());
			LONGS_EQUAL(512, pDS->GetRasterYSize());

			GDALClose(pDS);
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_TGA)
{
	// Test TGA driver.
	//

	constexpr char* DRIVER_NAME = "TGA";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "tga/cbw8.tga");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			int sizeX = pDS->GetRasterXSize();
			int sizeY = pDS->GetRasterYSize();
			int rasterCount = pDS->GetRasterCount();
			int layerCount = pDS->GetLayerCount();

			CHECK(pDS->GetAccess() == GA_ReadOnly);
			LONGS_EQUAL(128, sizeX);
			LONGS_EQUAL(128, sizeY);
			LONGS_EQUAL(1, rasterCount);
			LONGS_EQUAL(0, layerCount);

			GDALDataset::Bands bands = pDS->GetBands();
			LONGS_EQUAL(1, (int)bands.size());

			// bands are 1 based
			GDALRasterBand* pBand = pDS->GetRasterBand(1);
			CHECK(pBand != nullptr);

			if (pBand != nullptr)
			{
				XString strUnitType = pBand->GetUnitType();
				int blockX, blockY;
				pBand->GetBlockSize(&blockX, &blockY);
				LONGS_EQUAL(128, blockX);
				LONGS_EQUAL(1, blockY);

				GDALColorInterp ci = pBand->GetColorInterpretation();
				LONGS_EQUAL(ci, GCI_GrayIndex);

				double minElev, maxElev, meanElev, stdDev;
				CPLErr err = pBand->ComputeStatistics(0, &minElev, &maxElev, &meanElev, &stdDev, nullptr, nullptr);
				DOUBLES_EQUAL(0.0, minElev, 0.0);
				DOUBLES_EQUAL(254, maxElev, 0.0);
				DOUBLES_EQUAL(132.5, meanElev, 0.0);
				DOUBLES_EQUAL(73.644415945813563, stdDev, 0.0);

				CHECK(GDT_Byte == pBand->GetRasterDataType());

				const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
				CHECK(pSRS == nullptr);

				// verify subset of elevation data

				unsigned char* pScanline = (unsigned char *)CPLMalloc(sizeof(unsigned char)*sizeX);

				// read first line

				pBand->RasterIO(
					GF_Read,			// read or write
					0,					// xoffset - zero based offset from left
					0,					// yoffset - zero based offset from top
					sizeX,				// width of the region of the band to be accessed in pixels
					1,					// height of the region of the band to be accessed in lines
					pScanline,			// data buffer
					sizeX,				// width of data buffer
					1,					// height of data buffer
					GDT_Byte,			// buffer type - data automatically translated
					0,					// pixel spacing
					0);					// line spacing

				//FILE* pFile = fopen("C:/Users/mark/Desktop/test.txt", "wt");
				//for (int i = 0; i < sizeX; ++i)
				//{
				//	fprintf(pFile, "[%d] %u\n", i, pScanline[i]);
				//	//fprintf(pFile, "[%d] %f\n", i, pScanline[i]);
				//}
				//fclose(pFile);

				LONGS_EQUAL(76, pScanline[0]);
				LONGS_EQUAL(149, pScanline[8]);
				LONGS_EQUAL(0, pScanline[24]);
				LONGS_EQUAL(76, pScanline[32]);
				LONGS_EQUAL(149, pScanline[40]);
				LONGS_EQUAL(254, pScanline[127]);

				pBand->RasterIO(
					GF_Read,			// read or write
					0,					// xoffset - zero based offset from left
					64,					// yoffset - zero based offset from top
					sizeX,				// width of the region of the band to be accessed in pixels
					1,					// height of the region of the band to be accessed in lines
					pScanline,			// data buffer
					sizeX,				// width of data buffer
					1,					// height of data buffer
					GDT_Byte,			// buffer type - data automatically translated
					0,					// pixel spacing
					0);					// line spacing

				LONGS_EQUAL(76, pScanline[0]);
				LONGS_EQUAL(149, pScanline[8]);
				LONGS_EQUAL(0, pScanline[24]);
				LONGS_EQUAL(76, pScanline[32]);
				LONGS_EQUAL(149, pScanline[40]);
				LONGS_EQUAL(254, pScanline[127]);

				// read last line

				pBand->RasterIO(
					GF_Read,			// read or write
					0,					// xoffset - zero based offset from left
					127,				// yoffset - zero based offset from top
					sizeX,				// width of the region of the band to be accessed in pixels
					1,					// height of the region of the band to be accessed in lines
					pScanline,			// data buffer
					sizeX,				// width of data buffer
					1,					// height of data buffer
					GDT_Byte,			// buffer type - data automatically translated
					0,					// pixel spacing
					0);					// line spacing

				LONGS_EQUAL(76, pScanline[0]);
				LONGS_EQUAL(149, pScanline[8]);
				LONGS_EQUAL(0, pScanline[24]);
				LONGS_EQUAL(76, pScanline[32]);
				LONGS_EQUAL(149, pScanline[40]);
				LONGS_EQUAL(254, pScanline[127]);

				// The pScanline buffer should be freed with CPLFree() when it is no longer used.
				CPLFree(pScanline);
			}
		}

		GDALClose(pDS);
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Raster_XYZ)
{
	// Test XYZ driver.
	//

	constexpr char* DRIVER_NAME = "XYZ";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "xyz/w122_n45_dt0.xyz");

		GDALDataset* pDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.Compare(DRIVER_NAME));

			int sizeX = pDS->GetRasterXSize();
			int sizeY = pDS->GetRasterYSize();
			int rasterCount = pDS->GetRasterCount();
			int layerCount = pDS->GetLayerCount();

			CHECK(pDS->GetAccess() == GA_ReadOnly);
			LONGS_EQUAL(121, sizeX);
			LONGS_EQUAL(121, sizeY);
			LONGS_EQUAL(1, rasterCount);
			LONGS_EQUAL(0, layerCount);

			GDALDataset::Bands bands = pDS->GetBands();
			LONGS_EQUAL(1, (int)bands.size());

			// bands are 1 based
			GDALRasterBand* pBand = pDS->GetRasterBand(1);
			CHECK(pBand != nullptr);

			if (pBand != nullptr)
			{
				XString strUnitType = pBand->GetUnitType();
				int blockX, blockY;
				pBand->GetBlockSize(&blockX, &blockY);
				LONGS_EQUAL(121, blockX);
				LONGS_EQUAL(1, blockY);

				GDALColorInterp ci = pBand->GetColorInterpretation();
				LONGS_EQUAL(ci, GCI_Undefined);

				double minElev, maxElev, meanElev, stdDev;
				CPLErr err = pBand->ComputeStatistics(0, &minElev, &maxElev, &meanElev, &stdDev, nullptr, nullptr);
				CHECK(err == CPLErr::CE_None);
				DOUBLES_EQUAL(6.0, minElev, 0.0);
				DOUBLES_EQUAL(3259.0, maxElev, 0.0);
				DOUBLES_EQUAL(782.79591557953404, meanElev, 0.0);
				DOUBLES_EQUAL(363.85478284356844, stdDev, 0.0);

				GDALDataType dataType = pBand->GetRasterDataType();
				CHECK(GDT_Int16 == dataType);

				const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
				CHECK(pSRS == nullptr);

				// verify subset of elevation data

				GInt16* pScanline = (GInt16*)CPLMalloc(sizeof(GInt16)*sizeX);

				// read first line

				err = pBand->RasterIO(
					GF_Read,			// read or write
					0,					// xoffset - zero based offset from left
					0,					// yoffset - zero based offset from top
					sizeX,				// width of the region of the band to be accessed in pixels
					1,					// height of the region of the band to be accessed in lines
					pScanline,			// data buffer
					sizeX,				// width of data buffer
					1,					// height of data buffer
					GDT_Int16,			// buffer type - data automatically translated
					0,					// pixel spacing
					0);					// line spacing
				CHECK(err == CPLErr::CE_None);

				GInt16 height = pScanline[0];
				LONGS_EQUAL(966, height);
				height = pScanline[1];
				LONGS_EQUAL(1042, height);
				height = pScanline[2];
				LONGS_EQUAL(1061, height);
				height = pScanline[3];
				LONGS_EQUAL(1008, height);
				
				pBand->RasterIO(
					GF_Read,			// read or write
					0,					// xoffset - zero based offset from left
					64,					// yoffset - zero based offset from top
					sizeX,				// width of the region of the band to be accessed in pixels
					1,					// height of the region of the band to be accessed in lines
					pScanline,			// data buffer
					sizeX,				// width of data buffer
					1,					// height of data buffer
					GDT_Int16,			// buffer type - data automatically translated
					0,					// pixel spacing
					0);					// line spacing

				//FILE* pFile = fopen("D:/temp/test.txt", "wt");
				//for (int i = 0; i < sizeX; ++i)
				//{
				//	fprintf(pFile, "[%d] %d\n", i, pScanline[i]);
				//	//fprintf(pFile, "[%d] %f\n", i, pScanline[i]);
				//}
				//fclose(pFile);

				height = pScanline[0];
				LONGS_EQUAL(796, height);
				height = pScanline[1];
				LONGS_EQUAL(871, height);
				height = pScanline[2];
				LONGS_EQUAL(1007, height);
				height = pScanline[3];
				LONGS_EQUAL(1034, height);

				// read last line

				pBand->RasterIO(
					GF_Read,			// read or write
					0,					// xoffset - zero based offset from left
					120,				// yoffset - zero based offset from top
					sizeX,				// width of the region of the band to be accessed in pixels
					1,					// height of the region of the band to be accessed in lines
					pScanline,			// data buffer
					sizeX,				// width of data buffer
					1,					// height of data buffer
					GDT_Int16,			// buffer type - data automatically translated
					0,					// pixel spacing
					0);					// line spacing

				height = pScanline[0];
				LONGS_EQUAL(976, height);
				height = pScanline[1];
				LONGS_EQUAL(1094, height);
				height = pScanline[2];
				LONGS_EQUAL(1070, height);
				height = pScanline[3];
				LONGS_EQUAL(1111, height);

				// The pScanline buffer should be freed with CPLFree() when it is no longer used.
				CPLFree(pScanline);
			}
		}

		GDALClose(pDS);
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Vector_ArcGen)
{
	// Test ARCGEN - Arc/Info Generate driver.
	//

	constexpr char* DRIVER_NAME = "ARCGEN";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		// Read points

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "arcgen/points.gen");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			int layerCount = pDS->GetLayerCount();
			LONGS_EQUAL(1, layerCount);

			OGRLayer* pLayer = pDS->GetLayer(0);
			CHECK(pLayer != nullptr);

			if (pLayer != nullptr)
			{
				XString layerName = pLayer->GetName();
				CHECK(layerName.CompareNoCase("points"));

				int featCount = (int)pLayer->GetFeatureCount();
				LONGS_EQUAL(2, featCount);

				CHECK(OGRwkbGeometryType::wkbPoint == pLayer->GetGeomType());

				OGRFeature* pFeature0 = pLayer->GetFeature(0);
				OGRFeature* pFeature1 = pLayer->GetFeature(1);
				OGRFeature* pFeature2 = pLayer->GetFeature(2);

				CHECK(pFeature0 != nullptr);
				CHECK(pFeature1 != nullptr);
				CHECK(pFeature2 == nullptr);

				CHECK(pFeature0->GetFID() == 0);
				CHECK(pFeature1->GetFID() == 1);

				LONGS_EQUAL(1, pFeature0->GetFieldCount());
				LONGS_EQUAL(1, pFeature1->GetFieldCount());

				LONGS_EQUAL(1, pFeature0->GetFieldAsInteger(0));

				OGRGeometry* pGeometry = pFeature0->GetGeomFieldRef(0);
				CHECK(pGeometry != nullptr);

				if (pGeometry != nullptr)
				{
					OGRwkbGeometryType geomType = wkbFlatten(pGeometry->getGeometryType());
					CHECK(geomType == wkbPoint);
				
					OGRPoint* pPoint = pGeometry->toPoint();
					DOUBLES_EQUAL(2.0, pPoint->getX(), 0.0);
					DOUBLES_EQUAL(49.0, pPoint->getY(), 0.0);
				}
			}

			GDALClose(pDS);
		}
	}

	{
		// Read points25d

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "arcgen/points25d.gen");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));
			
			int layerCount = pDS->GetLayerCount();
			LONGS_EQUAL(1, layerCount);

			OGRLayer* pLayer = pDS->GetLayer(0);
			CHECK(pLayer != nullptr);

			CHECK(OGRwkbGeometryType::wkbPoint25D == pLayer->GetGeomType());

			OGRFeature* pFeature = pLayer->GetNextFeature();
			LONGS_EQUAL(1, pFeature->GetFieldAsInteger(0));

			// 'POINT (2 49 10)'
		}
	}

	{
		// Read lines

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "arcgen/lines.gen");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);
		
		OGRLayer* pLayer = pDS->GetLayer(0);
		CHECK(pLayer != nullptr);

		CHECK(OGRwkbGeometryType::wkbLineString == pLayer->GetGeomType());

		OGRFeature* pFeature = pLayer->GetNextFeature();
		LONGS_EQUAL(1, pFeature->GetFieldAsInteger(0));

		// not sure what geom field count is - number of polygons?
		int geomFieldCount = pFeature->GetGeomFieldCount();
		LONGS_EQUAL(1, geomFieldCount);

		//	'LINESTRING (2 49,3 50)'
	}

	{
		// Read lines25d

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "arcgen/lines25d.gen");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);
		
		OGRLayer* pLayer = pDS->GetLayer(0);
		CHECK(pLayer != nullptr);

		CHECK(OGRwkbGeometryType::wkbLineString25D == pLayer->GetGeomType());

		OGRFeature* pFeature = pLayer->GetNextFeature();
		LONGS_EQUAL(1, pFeature->GetFieldAsInteger(0));
		
		// 'LINESTRING (2 49 10,3 50 10)'
	}

	{
		// Read polygons

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "arcgen/polygons.gen");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);
		
		OGRLayer* pLayer = pDS->GetLayer(0);
		CHECK(pLayer != nullptr);

		CHECK(OGRwkbGeometryType::wkbPolygon == pLayer->GetGeomType());

		OGRFeature* pFeature = pLayer->GetNextFeature();
		CHECK(pFeature != nullptr);

		if (pFeature != nullptr)
		{
			LONGS_EQUAL(1, pFeature->GetFieldAsInteger(0));
		
			// not sure what geom field count is - number of polygons?
			int geomFieldCount = pFeature->GetGeomFieldCount();
			LONGS_EQUAL(1, geomFieldCount);

			OGRGeometry* pGeometry = pFeature->GetGeomFieldRef(0);
			if ((pGeometry != nullptr)
			&&  (wkbFlatten(pGeometry->getGeometryType()) == wkbPolygon))
			{
				OGRPolygon* pPoly = pGeometry->toPolygon();
				CHECK(pPoly != nullptr);

				// it's a 2d polygon
				int dim = pPoly->getDimension();
				LONGS_EQUAL(2, dim);

				// it has an outer ring (which they all should) and that
				// ring has 5 points
				OGRLinearRing* pOuterRing = pPoly->getExteriorRing();
				LONGS_EQUAL(5, pOuterRing->getNumPoints());
					
				// it does not have any inner rings (holes)
				int innerRingCount = pPoly->getNumInteriorRings();
				LONGS_EQUAL(0, innerRingCount);

				// Here's a good example of reading polygon points.
				// 'POLYGON ((2 49,2 50,3 50,3 49,2 49))'

				OGRPoint pt;
				pOuterRing->getPoint(0, &pt);
				DOUBLES_EQUAL(2.0, pt.getX(), 0.0);
				DOUBLES_EQUAL(49.0, pt.getY(), 0.0);
				pOuterRing->getPoint(1, &pt);
				DOUBLES_EQUAL(2.0, pt.getX(), 0.0);
				DOUBLES_EQUAL(50.0, pt.getY(), 0.0);
				pOuterRing->getPoint(2, &pt);
				DOUBLES_EQUAL(3.0, pt.getX(), 0.0);
				DOUBLES_EQUAL(50.0, pt.getY(), 0.0);
				pOuterRing->getPoint(3, &pt);
				DOUBLES_EQUAL(3.0, pt.getX(), 0.0);
				DOUBLES_EQUAL(49.0, pt.getY(), 0.0);
				pOuterRing->getPoint(4, &pt);
				DOUBLES_EQUAL(2.0, pt.getX(), 0.0);
				DOUBLES_EQUAL(49.0, pt.getY(), 0.0);
			}
		}
	}

	{
		// Read polygons25d

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "arcgen/polygons25d.gen");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		OGRLayer* pLayer = pDS->GetLayer(0);
		CHECK(pLayer != nullptr);

		CHECK(OGRwkbGeometryType::wkbPolygon25D == pLayer->GetGeomType());

		OGRFeature* pFeature = pLayer->GetNextFeature();
		LONGS_EQUAL(1, pFeature->GetFieldAsInteger(0));
		
		// 'POLYGON ((2 49 10,2 50 10,3 50 10,3 49 10,2 49 10))'
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Vector_BNA)
{
	// Test BNA driver.
	//
	// The BNA format is an ASCII exchange format for 2D vector data supported by many software packages.
	// It only contains geometry and a few identifiers per record. Attributes must be stored into external
	// files. It does not support any coordinate system information.
	//

	constexpr char* DRIVER_NAME = "BNA";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "test.bna");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			int layerCount = pDS->GetLayerCount();
			LONGS_EQUAL(4, layerCount);

			OGRLayer* pLayer = pDS->GetLayer(0);
			CHECK(pLayer != nullptr);

			if (pLayer != nullptr)
			{
				XString layerName = pLayer->GetName();
				CHECK(layerName.CompareNoCase("test_points"));

				int featCount = (int)pLayer->GetFeatureCount();
				LONGS_EQUAL(2, featCount);

				OGRFeature* pFeature0 = pLayer->GetFeature(0);
				OGRFeature* pFeature1 = pLayer->GetFeature(1);
				OGRFeature* pFeature2 = pLayer->GetFeature(2);

				CHECK(pFeature0 != nullptr);
				CHECK(pFeature1 != nullptr);
				CHECK(pFeature2 == nullptr);

				CHECK(pFeature0->GetFID() == 0);
				CHECK(pFeature1->GetFID() == 1);

				LONGS_EQUAL(2, pFeature0->GetFieldCount());
				LONGS_EQUAL(2, pFeature1->GetFieldCount());

				OGRFeatureDefn* pFDefn = pLayer->GetLayerDefn();
				CHECK(pFDefn != nullptr);

				if (pFDefn != nullptr)
				{
					LONGS_EQUAL(2, pFDefn->GetFieldCount());
					LONGS_EQUAL(1, pFDefn->GetGeomFieldCount());

					pLayer->ResetReading();
					OGRFeature* pFeature;
					while ((pFeature = pLayer->GetNextFeature()) != nullptr)
					{
						OGRFieldDefn* pFieldDef0 = pFDefn->GetFieldDefn(0);
						OGRFieldDefn* pFieldDef1 = pFDefn->GetFieldDefn(1);

						CHECK(pFieldDef0->GetType() == OGRFieldType::OFTString);
						CHECK(pFieldDef1->GetType() == OGRFieldType::OFTString);

						XString name0 = pFieldDef0->GetNameRef();
						XString name1 = pFieldDef1->GetNameRef();

						CHECK(name0.CompareNoCase("Primary ID"));
						CHECK(name1.CompareNoCase("Secondary ID"));

						OGRFeature::DestroyFeature(pFeature);
					}
				}
			}

			GDALClose(pDS);
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Vector_GeoJSON)
{
	// Test GeoJSON driver.
	//

	constexpr char* DRIVER_NAME = "GeoJSON";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		// Test file-based DS with standalone "Point" feature object.

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "geojson/point.geojson");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			LONGS_EQUAL(1, pDS->GetLayerCount());

			OGRLayer* pLayer = pDS->GetLayerByName("point");
			CHECK(pLayer != nullptr);
			
			if (pLayer != nullptr)
			{
				OGREnvelope extent;
				OGRErr err = pLayer->GetExtent(&extent, 1);
				LONGS_EQUAL(err, OGRERR_NONE);

				DOUBLES_EQUAL(extent.MinX, 100.0, 0.0);
				DOUBLES_EQUAL(extent.MaxX, 100.0, 0.0);
				DOUBLES_EQUAL(extent.MinY,   0.0, 0.0);
				DOUBLES_EQUAL(extent.MaxY,   0.0, 0.0);

				LONGS_EQUAL(OGRwkbGeometryType::wkbPoint, pLayer->GetGeomType());
			}
			GDALClose(pDS);
		}
	}

	{
		// Test file-based DS with standalone "LineString" feature object.

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "geojson/linestring.geojson");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			LONGS_EQUAL(1, pDS->GetLayerCount());

			OGRLayer* pLayer = pDS->GetLayerByName("linestring");
			CHECK(pLayer != nullptr);
			
			if (pLayer != nullptr)
			{
				OGREnvelope extent;
				OGRErr err = pLayer->GetExtent(&extent, 1);
				LONGS_EQUAL(err, OGRERR_NONE);

				DOUBLES_EQUAL(extent.MinX, 100.0, 0.0);
				DOUBLES_EQUAL(extent.MaxX, 101.0, 0.0);
				DOUBLES_EQUAL(extent.MinY,   0.0, 0.0);
				DOUBLES_EQUAL(extent.MaxY,   1.0, 0.0);

				LONGS_EQUAL(OGRwkbGeometryType::wkbLineString, pLayer->GetGeomType());
			}
			GDALClose(pDS);
		}
	}

	{
		// Test file-based DS with standalone "polygon" feature object.

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "geojson/polygon.geojson");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			LONGS_EQUAL(1, pDS->GetLayerCount());

			OGRLayer* pLayer = pDS->GetLayerByName("polygon");
			CHECK(pLayer != nullptr);
			
			if (pLayer != nullptr)
			{
				OGREnvelope extent;
				OGRErr err = pLayer->GetExtent(&extent, 1);
				LONGS_EQUAL(err, OGRERR_NONE);

				DOUBLES_EQUAL(extent.MinX, 100.0, 0.0);
				DOUBLES_EQUAL(extent.MaxX, 101.0, 0.0);
				DOUBLES_EQUAL(extent.MinY,   0.0, 0.0);
				DOUBLES_EQUAL(extent.MaxY,   1.0, 0.0);

				LONGS_EQUAL(OGRwkbGeometryType::wkbPolygon, pLayer->GetGeomType());
			}
			GDALClose(pDS);
		}
	}

	{
		// Test file-based DS with standalone "GeometryCollection" feature object.

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "geojson/geometrycollection.geojson");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			LONGS_EQUAL(1, pDS->GetLayerCount());

			OGRLayer* pLayer = pDS->GetLayerByName("geometrycollection");
			CHECK(pLayer != nullptr);
			
			if (pLayer != nullptr)
			{
				OGREnvelope extent;
				OGRErr err = pLayer->GetExtent(&extent, 1);
				LONGS_EQUAL(err, OGRERR_NONE);

				DOUBLES_EQUAL(extent.MinX, 100.0, 0.0);
				DOUBLES_EQUAL(extent.MaxX, 102.0, 0.0);
				DOUBLES_EQUAL(extent.MinY,   0.0, 0.0);
				DOUBLES_EQUAL(extent.MaxY,   1.0, 0.0);

				LONGS_EQUAL(OGRwkbGeometryType::wkbGeometryCollection, pLayer->GetGeomType());
			}
			GDALClose(pDS);
		}
	}

	{
		// Test file-based DS with standalone "MultiPoint" feature object.

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "geojson/multipoint.geojson");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			LONGS_EQUAL(1, pDS->GetLayerCount());

			OGRLayer* pLayer = pDS->GetLayerByName("multipoint");
			CHECK(pLayer != nullptr);
			
			if (pLayer != nullptr)
			{
				OGREnvelope extent;
				OGRErr err = pLayer->GetExtent(&extent, 1);
				LONGS_EQUAL(err, OGRERR_NONE);

				DOUBLES_EQUAL(extent.MinX, 100.0, 0.0);
				DOUBLES_EQUAL(extent.MaxX, 101.0, 0.0);
				DOUBLES_EQUAL(extent.MinY,   0.0, 0.0);
				DOUBLES_EQUAL(extent.MaxY,   1.0, 0.0);

				LONGS_EQUAL(OGRwkbGeometryType::wkbMultiPoint, pLayer->GetGeomType());
			}
			GDALClose(pDS);
		}
	}

	{
		// Test file-based DS with standalone "MultiLineString" feature object.

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "geojson/multilinestring.geojson");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			LONGS_EQUAL(1, pDS->GetLayerCount());

			OGRLayer* pLayer = pDS->GetLayerByName("multilinestring");
			CHECK(pLayer != nullptr);
			
			if (pLayer != nullptr)
			{
				OGREnvelope extent;
				OGRErr err = pLayer->GetExtent(&extent, 1);
				LONGS_EQUAL(err, OGRERR_NONE);

				DOUBLES_EQUAL(extent.MinX, 100.0, 0.0);
				DOUBLES_EQUAL(extent.MaxX, 103.0, 0.0);
				DOUBLES_EQUAL(extent.MinY,   0.0, 0.0);
				DOUBLES_EQUAL(extent.MaxY,   3.0, 0.0);

				LONGS_EQUAL(OGRwkbGeometryType::wkbMultiLineString, pLayer->GetGeomType());
			}
			GDALClose(pDS);
		}
	}

	{
		// Test file-based DS with standalone "MultiPolygon" feature object.

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "geojson/multipolygon.geojson");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			LONGS_EQUAL(1, pDS->GetLayerCount());

			OGRLayer* pLayer = pDS->GetLayerByName("multipolygon");
			CHECK(pLayer != nullptr);
			
			if (pLayer != nullptr)
			{
				OGREnvelope extent;
				OGRErr err = pLayer->GetExtent(&extent, 1);
				LONGS_EQUAL(err, OGRERR_NONE);

				DOUBLES_EQUAL(extent.MinX, 100.0, 0.0);
				DOUBLES_EQUAL(extent.MaxX, 103.0, 0.0);
				DOUBLES_EQUAL(extent.MinY,   0.0, 0.0);
				DOUBLES_EQUAL(extent.MaxY,   3.0, 0.0);

				LONGS_EQUAL(OGRwkbGeometryType::wkbMultiPolygon, pLayer->GetGeomType());
			}
			GDALClose(pDS);
		}
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "geojson/srs_name.geojson");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			LONGS_EQUAL(1, pDS->GetLayerCount());

			OGRLayer* pLayer = pDS->GetLayerByName("srs_name");
			CHECK(pLayer != nullptr);
			
			if (pLayer != nullptr)
			{
				OGREnvelope extent;
				OGRErr err = pLayer->GetExtent(&extent, 1);
				LONGS_EQUAL(err, OGRERR_NONE);

				DOUBLES_EQUAL(extent.MinX, 100.0, 0.0);
				DOUBLES_EQUAL(extent.MaxX, 102.0, 0.0);
				DOUBLES_EQUAL(extent.MinY,   0.0, 0.0);
				DOUBLES_EQUAL(extent.MaxY,   1.0, 0.0);

				LONGS_EQUAL(OGRwkbGeometryType::wkbGeometryCollection, pLayer->GetGeomType());

				OGRSpatialReference* pSRS = pLayer->GetSpatialRef();
				CHECK(pSRS != nullptr);

				XString strAuth = pSRS->GetAuthorityCode("PROJCS");
				LONGS_EQUAL(26915, strAuth.GetInt());
			}
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Vector_GTM)
{
	// Test GPSTrackMaker driver.
	//
	// GPSTrackMaker is a program that is compatible with more than 160 GPS models. 
	// It allows you to create your own maps. It supports vector maps and images.
	//
	// The OGR driver has support for reading and writing GTM 211 files (.gtm); however, 
	// in this implementation we are not supporting images and routes. Waypoints and tracks are supported.
	//
	// Although GTM has support for many data, like NAD 1967, SAD 1969, and others, the output file of the
	// OGR driver will be using WGS 1984. And the GTM driver will only read properly GTM files georeferenced
	// as WGS 1984 (if not the case a warning will be issued).
	//
	// The OGR driver supports just POINT, LINESTRING, and MULTILINESTRING.
	//

	constexpr char* DRIVER_NAME = "GPSTrackMaker";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		// Test waypoints gtm layer.

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "gtm/samplemap.gtm");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			LONGS_EQUAL(2, pDS->GetLayerCount());

			OGRLayer* pLayer = pDS->GetLayerByName("samplemap_waypoints");
			CHECK(pLayer != nullptr);
			
			if (pLayer != nullptr)
			{
				LONGS_EQUAL(3, static_cast<long>(pLayer->GetFeatureCount()));

				// Test 1st feature
				OGRFeature* pFeature = pLayer->GetNextFeature();
				XString str = pFeature->GetFieldAsString("name");
				CHECK(str.Compare("WAY6") == true);
				str = pFeature->GetFieldAsString("comment");
				CHECK(str.Compare("Santa Cruz Stadium") == true);
				int n = pFeature->GetFieldAsInteger("icon");
				LONGS_EQUAL(92, n);

				// feat.GetField('time') == '2009/12/18 17:32:41'
				int year, month, day, hour, minute, second, tzFlag;
				n = pFeature->GetFieldAsDateTime(3, &year, &month, &day, &hour, &minute, &second, &tzFlag);
				LONGS_EQUAL(2009, year);
				LONGS_EQUAL(12, month);
				LONGS_EQUAL(18, day);
				LONGS_EQUAL(17, hour);
				LONGS_EQUAL(32, minute);
				LONGS_EQUAL(41, second);

				OGRGeometry* pGeometry = pFeature->GetGeomFieldRef(0);
				if (pGeometry != nullptr)
				{
					OGRwkbGeometryType geomType = pGeometry->getGeometryType();
					CHECK(geomType == wkbPoint);
				
					OGRPoint* pPoint = pGeometry->toPoint();
					DOUBLES_EQUAL(-47.789974212646484, pPoint->getX(), 0.0);
					DOUBLES_EQUAL(-21.201919555664062, pPoint->getY(), 0.0);
				}

				// Test 2nd feature
				pFeature = pLayer->GetNextFeature();
				str = pFeature->GetFieldAsString("name");
				CHECK(str.Compare("WAY6") == true);
				str = pFeature->GetFieldAsString("comment");
				CHECK(str.Compare("Joe\'s Goalkeeper Pub") == true);
				n = pFeature->GetFieldAsInteger("icon");
				LONGS_EQUAL(4, n);

				pGeometry = pFeature->GetGeomFieldRef(0);
				CHECK(pGeometry != nullptr);

				if (pGeometry != nullptr)
				{
					OGRwkbGeometryType geomType = pGeometry->getGeometryType();
					CHECK(geomType == wkbPoint);
				
					OGRPoint* pPoint = pGeometry->toPoint();
					DOUBLES_EQUAL(-47.909481048583984, pPoint->getX(), 0.0);
					DOUBLES_EQUAL(-21.294229507446289, pPoint->getY(), 0.0);
				}

				//    # Test 3rd feature
				pFeature = pLayer->GetNextFeature();
				str = pFeature->GetFieldAsString("name");
				CHECK(str.Compare("33543400") == true);
				str = pFeature->GetFieldAsString("comment");
				CHECK(str.Compare("City Hall") == true);
				n = pFeature->GetFieldAsInteger("icon");
				LONGS_EQUAL(61, n);

				pGeometry = pFeature->GetGeomFieldRef(0);
				CHECK(pGeometry != nullptr);

				if (pGeometry != nullptr)
				{
					OGRwkbGeometryType geomType = pGeometry->getGeometryType();
					CHECK(geomType == wkbPoint);
				
					OGRPoint* pPoint = pGeometry->toPoint();
					DOUBLES_EQUAL(-47.806097491943362, pPoint->getX(), 0.0);
					DOUBLES_EQUAL(-21.176849600708007, pPoint->getY(), 0.0);
				}

			}
			GDALClose(pDS);
		}
	}

	{
		// Test tracks gtm layer.

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "gtm/samplemap.gtm");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			LONGS_EQUAL(2, pDS->GetLayerCount());

			OGRLayer* pLayer = pDS->GetLayerByName("samplemap_tracks");
			CHECK(pLayer != nullptr);
			
			if (pLayer != nullptr)
			{
				LONGS_EQUAL(3, static_cast<long>(pLayer->GetFeatureCount()));

				// Test 1st feature
				OGRFeature* pFeature = pLayer->GetNextFeature();
				XString str = pFeature->GetFieldAsString("name");
				CHECK(str.Compare("San Sebastian Street") == true);
				int n = pFeature->GetFieldAsInteger("type");
				LONGS_EQUAL(2, n);
				n = pFeature->GetFieldAsInteger("color");
				LONGS_EQUAL(0, n);

				OGRGeometry* pGeometry = pFeature->GetGeomFieldRef(0);
				CHECK(pGeometry != nullptr);

				if (pGeometry != nullptr)
				{
					OGRwkbGeometryType geomType = pGeometry->getGeometryType();
					CHECK(geomType == wkbLineString);
				
					OGRLineString* pLineString = pGeometry->toLineString();
					int numPoints = pLineString->getNumPoints();
					LONGS_EQUAL(4, numPoints);

					OGRPoint pt;
					pLineString->getPoint(0, &pt);
					DOUBLES_EQUAL(-47.807481607448054, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.177795963939211, pt.getY(), 0.0);
					pLineString->getPoint(1, &pt);
					DOUBLES_EQUAL(-47.808151245117188, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.177299499511719, pt.getY(), 0.0);
					pLineString->getPoint(2, &pt);
					DOUBLES_EQUAL(-47.809136624130645, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.176562836150087, pt.getY(), 0.0);
					pLineString->getPoint(3, &pt);
					DOUBLES_EQUAL(-47.809931418108405, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.175971104366582, pt.getY(), 0.0);
				}
				
				// Test 2nd feature
				pFeature = pLayer->GetNextFeature();
				str = pFeature->GetFieldAsString("name");
				CHECK(str.Compare("Barao do Amazonas Street") == true);
				n = pFeature->GetFieldAsInteger("type");
				LONGS_EQUAL(1, n);
				n = pFeature->GetFieldAsInteger("color");
				LONGS_EQUAL(0, n);

				pGeometry = pFeature->GetGeomFieldRef(0);
				CHECK(pGeometry != nullptr);

				if (pGeometry != nullptr)
				{
					OGRwkbGeometryType geomType = pGeometry->getGeometryType();
					CHECK(geomType == wkbLineString);
				
					OGRLineString* pLineString = pGeometry->toLineString();
					int numPoints = pLineString->getNumPoints();
					LONGS_EQUAL(4, numPoints);

					OGRPoint pt;
					pLineString->getPoint(0, &pt);
					DOUBLES_EQUAL(-47.808751751608561, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.178029550275486, pt.getY(), 0.0);
					pLineString->getPoint(1, &pt);
					DOUBLES_EQUAL(-47.808151245117188, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.177299499511719, pt.getY(), 0.0);
					pLineString->getPoint(2, &pt);
					DOUBLES_EQUAL(-47.807561550927701, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.176617693474089, pt.getY(), 0.0);
					pLineString->getPoint(3, &pt);
					DOUBLES_EQUAL(-47.806959118447779, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.175900153727685, pt.getY(), 0.0);
				}

				// Test 3rd feature
				pFeature = pLayer->GetNextFeature();
				str = pFeature->GetFieldAsString("name");
				CHECK(str.Compare("Curupira Park") == true);
				n = pFeature->GetFieldAsInteger("type");
				LONGS_EQUAL(17, n);
				n = pFeature->GetFieldAsInteger("color");
				LONGS_EQUAL(46848, n);

				pGeometry = pFeature->GetGeomFieldRef(0);
				CHECK(pGeometry != nullptr);

				if (pGeometry != nullptr)
				{
					OGRwkbGeometryType geomType = pGeometry->getGeometryType();
					CHECK(geomType == wkbLineString);
				
					OGRLineString* pLineString = pGeometry->toLineString();
					int numPoints = pLineString->getNumPoints();
					LONGS_EQUAL(7, numPoints);

					OGRPoint pt;
					pLineString->getPoint(0, &pt);
					DOUBLES_EQUAL(-47.7894287109375, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.194473266601562, pt.getY(), 0.0);
					pLineString->getPoint(1, &pt);
					DOUBLES_EQUAL(-47.793514591064451, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.197530536743162, pt.getY(), 0.0);
					pLineString->getPoint(2, &pt);
					DOUBLES_EQUAL(-47.797027587890625, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.19483757019043, pt.getY(), 0.0);
					pLineString->getPoint(3, &pt);
					DOUBLES_EQUAL(-47.794818878173828, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.192028045654297, pt.getY(), 0.0);
					pLineString->getPoint(4, &pt);
					DOUBLES_EQUAL(-47.794120788574219, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.193340301513672, pt.getY(), 0.0);
					pLineString->getPoint(5, &pt);
					DOUBLES_EQUAL(-47.792263031005859, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.194267272949219, pt.getY(), 0.0);
					pLineString->getPoint(6, &pt);
					DOUBLES_EQUAL(-47.7894287109375, pt.getX(), 0.0);
					DOUBLES_EQUAL(-21.194473266601562, pt.getY(), 0.0);
				}
			}
			GDALClose(pDS);
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Vector_MITAB)
{
	// Test MapInfo TAB and MIF/MID driver.
	//

	// Doc says driver name is "MITAB", but gdal expects this.
	constexpr char* DRIVER_NAME = "MapInfo File";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "mitab/small.mif");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			OGRLayer* pLayer = pDS->GetLayer(0);
			CHECK(pLayer != nullptr);

			if (pLayer != nullptr)
			{
				OGRFeature* pFeature = pLayer->GetNextFeature();
				CHECK(pFeature != nullptr);

				if (pFeature != nullptr)
				{
					XString strName = pFeature->GetFieldAsString("NAME");
					int floodZone = pFeature->GetFieldAsInteger("FLOODZONE");
					CHECK(strName.Compare(" S. 11th St."));
					LONGS_EQUAL(10, floodZone);

					OGRGeometry* pGeometry = pFeature->GetGeometryRef();
					CHECK(pGeometry != nullptr);

					if ((pGeometry != nullptr)
					&&  (wkbFlatten(pGeometry->getGeometryType()) == wkbPolygon))
					{
						OGRPolygon* pPoly = pGeometry->toPolygon();
						CHECK(pPoly != nullptr);

						// it's a 2d polygon
						int dim = pPoly->getDimension();
						LONGS_EQUAL(2, dim);

						// it has an outer ring (which they all should) and that
						// ring has 6 points
						OGRLinearRing* pOuterRing = pPoly->getExteriorRing();
						LONGS_EQUAL(6, pOuterRing->getNumPoints());
					
						// it does not have any inner rings (holes)
						int innerRingCount = pPoly->getNumInteriorRings();
						LONGS_EQUAL(0, innerRingCount);

						// Here's a good example of reading polygon points.
						OGRPoint pt;
						pOuterRing->getPoint(0, &pt);
						DOUBLES_EQUAL(407131.721, pt.getX(), 0.0);
						DOUBLES_EQUAL(155322.441, pt.getY(), 0.0);
						pOuterRing->getPoint(1, &pt);
						DOUBLES_EQUAL(407134.468, pt.getX(), 0.0);
						DOUBLES_EQUAL(155329.616, pt.getY(), 0.0);
						pOuterRing->getPoint(5, &pt);
						DOUBLES_EQUAL(407131.721, pt.getX(), 0.0);
						DOUBLES_EQUAL(155322.441, pt.getY(), 0.0);
						//'POLYGON ((407131.721 155322.441,407134.468 155329.616,407142.741 155327.242,407141.503 155322.467,407140.875 155320.049,407131.721 155322.441))'
					}
				}

				pFeature = pLayer->GetNextFeature();
				CHECK(pFeature != nullptr);

				if (pFeature != nullptr)
				{
					XString strName = pFeature->GetFieldAsString("OWNER");
					CHECK(strName.Compare("Guarino \"Chucky\" Sandra"));
				}
			}

			GDALClose(pDS);
		}
	}

	{
		// Verify support for NTF datum with non-greenwich datum per
		// http://trac.osgeo.org/gdal/ticket/1416
		//
		// This test also exercises SRS reference counting as described in issue:
		// http://trac.osgeo.org/gdal/ticket/1680

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "mitab/small_ntf.mif");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if(pDS != nullptr)
		{
			LONGS_EQUAL(1, pDS->GetLayerCount());
			const OGRSpatialReference* pSRS = pDS->GetLayer(0)->GetSpatialRef();
			DOUBLES_EQUAL(2.337229, pSRS->GetPrimeMeridian(), 1E-6);
			GDALClose(pDS);
		}
	}

	{
		// Test .mif without .mid (#5141)

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "mitab/nomid.mif");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if(pDS != nullptr)
		{
			LONGS_EQUAL(1, pDS->GetLayerCount());
			OGRLayer* pLayer = pDS->GetLayer(0);
			CHECK(pLayer != nullptr);
			OGRFeature* pFeature = pLayer->GetNextFeature();
			CHECK(pFeature != nullptr);
			GDALClose(pDS);
		}
	}

	{
		// Check read support of non-spatial .tab/.data without .map or .id (#5718)
		// We only check read-only behaviour though.

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "mitab/aspatial-table.tab");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if(pDS != nullptr)
		{
			LONGS_EQUAL(1, pDS->GetLayerCount());
			OGRLayer* pLayer = pDS->GetLayer(0);
			CHECK(pLayer != nullptr);
			if (pLayer != nullptr)
			{
				LONGS_EQUAL(2, (long)pLayer->GetFeatureCount());

				OGRFeature* pFeature = pLayer->GetNextFeature();
				CHECK(pFeature != nullptr);

				if (pFeature != nullptr)
				{
					LONGS_EQUAL(1, pFeature->GetFieldAsInteger("a"));
					LONGS_EQUAL(2, pFeature->GetFieldAsInteger("b"));
					XString s = pFeature->GetFieldAsString("d");
					CHECK(s.Compare("hello"));
				}
			}
			GDALClose(pDS);
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Vector_OpenFileGDB)
{
	// Test OpenFileGDB driver.
	//

	constexpr char* DRIVER_NAME = "OpenFileGDB";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "filegdb/curves.gdb");

		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);

		const char* pszAllowedDrivers[] = { DRIVER_NAME, NULL };

		//TODO:
		//fails i think because it's "finding" the wrong driver - fixed by specifying allowed drivers
		//now, using correct driver, still fails to open, think in OGROpenFileGDBDriverIdentifyInternal()
		//in ogropenfilegdbdriver.cpp
		//GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, pszAllowedDrivers, nullptr, nullptr);
		//CHECK(pDS != nullptr);

		//if (pDS != nullptr)
		//{
		//}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Vector_SDTS)
{
	// Test SDTS/Vector driver.
	//
	// File looks like:
	//		('ARDF', 164, ogr.wkbNone, [('ENTITY_LABEL', '1700005')]),
	//		('ARDM', 21, ogr.wkbNone, [('ROUTE_NUMBER', 'SR 1200')]),
	//		('AHDR', 1, ogr.wkbNone, [('BANNER', 'USGS-NMD  DLG DATA - CHARACTER FORMAT - 09-29-87 VERSION                ')]),
	//		('NP01', 4, ogr.wkbPoint, [('RCID', '1')]),
	//		('NA01', 34, ogr.wkbPoint, [('RCID', '2')]),
	//		('NO01', 88, ogr.wkbPoint, [('RCID', '1')]),
	//		('LE01', 27, ogr.wkbLineString, [('RCID', '1')]),
	//		('PC01', 35, ogr.wkbPolygon, [('RCID', '1')])
	//

	constexpr char* DRIVER_NAME = "OGR_SDTS";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "sdts_vector/D3607551_rd0s_1_sdts_truncated//TR01CATD.DDF");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			LONGS_EQUAL(8, pDS->GetLayerCount());

			OGRLayer* pLayerARDF = pDS->GetLayerByName("ARDF");
			OGRLayer* pLayerARDM = pDS->GetLayerByName("ARDM");
			OGRLayer* pLayerAHDR = pDS->GetLayerByName("AHDR");
			OGRLayer* pLayerNP01 = pDS->GetLayerByName("NP01");
			OGRLayer* pLayerNA01 = pDS->GetLayerByName("NA01");
			OGRLayer* pLayerNO01 = pDS->GetLayerByName("NO01");
			OGRLayer* pLayerLE01 = pDS->GetLayerByName("LE01");
			OGRLayer* pLayerPC01 = pDS->GetLayerByName("PC01");

			CHECK(pLayerARDF != nullptr);
			CHECK(pLayerARDM != nullptr);
			CHECK(pLayerAHDR != nullptr);
			CHECK(pLayerNP01 != nullptr);
			CHECK(pLayerNA01 != nullptr);
			CHECK(pLayerNO01 != nullptr);
			CHECK(pLayerLE01 != nullptr);
			CHECK(pLayerPC01 != nullptr);

			LONGS_EQUAL(164, static_cast<long>(pLayerARDF->GetFeatureCount()));
			LONGS_EQUAL( 21, static_cast<long>(pLayerARDM->GetFeatureCount()));
			LONGS_EQUAL(  1, static_cast<long>(pLayerAHDR->GetFeatureCount()));
			LONGS_EQUAL(  4, static_cast<long>(pLayerNP01->GetFeatureCount()));
			LONGS_EQUAL( 34, static_cast<long>(pLayerNA01->GetFeatureCount()));
			LONGS_EQUAL( 88, static_cast<long>(pLayerNO01->GetFeatureCount()));
			LONGS_EQUAL( 27, static_cast<long>(pLayerLE01->GetFeatureCount()));
			LONGS_EQUAL( 35, static_cast<long>(pLayerPC01->GetFeatureCount()));

			LONGS_EQUAL(OGRwkbGeometryType::wkbNone, pLayerARDF->GetGeomType());
			LONGS_EQUAL(OGRwkbGeometryType::wkbNone, pLayerARDM->GetGeomType());
			LONGS_EQUAL(OGRwkbGeometryType::wkbNone, pLayerAHDR->GetGeomType());
			LONGS_EQUAL(OGRwkbGeometryType::wkbPoint, pLayerNP01->GetGeomType());
			LONGS_EQUAL(OGRwkbGeometryType::wkbPoint, pLayerNA01->GetGeomType());
			LONGS_EQUAL(OGRwkbGeometryType::wkbPoint, pLayerNO01->GetGeomType());
			LONGS_EQUAL(OGRwkbGeometryType::wkbLineString, pLayerLE01->GetGeomType());
			LONGS_EQUAL(OGRwkbGeometryType::wkbPolygon, pLayerPC01->GetGeomType());

			//int n = pLayerARDF->GetFeatureCount();
			//XString s = pLayerARDF->GetGeometryColumn();

			//OGRFeature* pFeature = pLayerPC01->GetFeature(1);
			//XString s0 = pFeature->GetFieldAsString(0);
			//XString s1 = pFeature->GetFieldAsString(1);
			//XString s2 = pFeature->GetFieldAsString(2);

			GDALClose(pDS);
		}
	}
}

//----------------------------------------------------------------------------
TEST(GDAL, Vector_ShapeFile)
{
	// Test ShapeFile driver.
	//
	
	constexpr char* DRIVER_NAME = "ESRI ShapeFile";

	{
		// verify driver lookup
		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);
	}

	{
		// simple read test

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "shapefile/poly.shp");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			int layerCount = pDS->GetLayerCount();
			LONGS_EQUAL(1, layerCount);

			OGRLayer* pLayer = pDS->GetLayer(0);
			CHECK(pLayer != nullptr);

			if (pLayer != nullptr)
			{
				LONGS_EQUAL(10, static_cast<long>(pLayer->GetFeatureCount()));
				XString layerName = pLayer->GetName();
				CHECK(layerName.CompareNoCase("poly"));

				OGRFeatureDefn* pFDefn = pLayer->GetLayerDefn();
				CHECK(pFDefn != nullptr);

				if (pFDefn != nullptr)
				{
					LONGS_EQUAL(3, pFDefn->GetFieldCount());

					pLayer->ResetReading();
					OGRFeature* pFeature;
					while ((pFeature = pLayer->GetNextFeature()) != nullptr)
					{
						OGRFieldDefn* pFieldDef0 = pFDefn->GetFieldDefn(0);
						OGRFieldDefn* pFieldDef1 = pFDefn->GetFieldDefn(1);
						OGRFieldDefn* pFieldDef2 = pFDefn->GetFieldDefn(2);

						CHECK(pFieldDef0->GetType() == OGRFieldType::OFTReal);
						CHECK(pFieldDef1->GetType() == OGRFieldType::OFTInteger64);
						CHECK(pFieldDef2->GetType() == OGRFieldType::OFTString);

						XString name0 = pFieldDef0->GetNameRef();
						XString name1 = pFieldDef1->GetNameRef();
						XString name2 = pFieldDef2->GetNameRef();

						CHECK(name0.CompareNoCase("AREA"));
						CHECK(name1.CompareNoCase("EAS_ID"));
						CHECK(name2.CompareNoCase("PRFEDEA"));

						OGRFeature::DestroyFeature(pFeature);
					}
				}
			}

			GDALClose(pDS);
		}
	}

	{
		// verify can read peer files / prj

		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "shapefile/cyprus/cyprus_coastline.shp");

		GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		CHECK(pDS != nullptr);

		if (pDS != nullptr)
		{
			// make sure it instantiated the correct driver
			XString strDriverName = pDS->GetDriverName();
			CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

			// data source has no srs
			const OGRSpatialReference* pSRS =  pDS->GetSpatialRef();
			CHECK(pSRS == nullptr);

			int layerCount = pDS->GetLayerCount();
			LONGS_EQUAL(1, layerCount);

			if (pDS->GetLayerCount() == 1)
			{
				OGRLayer* pLayer = pDS->GetLayer(0);
				const OGRSpatialReference* pLayerSRS =  pLayer->GetSpatialRef();
				CHECK(pLayerSRS != nullptr);

				int axisCount = pLayerSRS->GetAxesCount();
				LONGS_EQUAL(2, axisCount);

				CHECK(pLayerSRS->IsGeographic() == true);
				CHECK(pLayerSRS->IsEmpty() == false);
				CHECK(pLayerSRS->IsDerivedGeographic() == false);
				CHECK(pLayerSRS->IsProjected() == false);
				CHECK(pLayerSRS->IsGeocentric() == false);
				CHECK(pLayerSRS->IsLocal() == false);
				CHECK(pLayerSRS->IsVertical() == false);
				CHECK(pLayerSRS->IsCompound() == false);

				char* pWkt = nullptr;
				// for some reason proj can't format if it has towgs84 in it
				// i see there are "striptowgs84..." methods ...?
				//pLayerSRS->exportToPrettyWkt(&pWkt);
				XString str = pWkt;
				CPLFree(pWkt);
			}

			GDALClose(pDS);
		}
	}

	{
		// read zip file containing multiple file

//TODO:
//ok, zipped files seem not to work at all
		//XString fileName = XString::CombinePath(gs_DataPath.c_str(), "shapefile/cyprus.shp.zip");

		//GDALDataset* pDS = (GDALDataset*)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		//CHECK(pDS != nullptr);

		//if (pDS != nullptr)
		//{
		//	// make sure it instantiated the correct driver
		//	XString strDriverName = pDS->GetDriverName();
		//	CHECK(strDriverName.CompareNoCase(DRIVER_NAME));

		//	int n = pDS->GetLayerCount();

		//	GDALClose(pDS);
		//}
	}

	{
		// write test

		GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
		CHECK(pDriver != nullptr);

		if (pDriver != nullptr)
		{
			// Create the datasource. The ESRI Shapefile driver allows us to create a directory full of shapefiles, 
			// or a single shapefile as a datasource. In this case we will explicitly create a single file by including
			// the extension in the name. The second, third, fourth and fifth argument are related to raster dimensions.
			// The last argument to the call is a list of option values, but we will just be using defaults in this case.

			XString fileName = XString::CombinePath(gs_DataPath.c_str(), "point_out.shp");
			GDALDataset* pDS = pDriver->Create(fileName.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
			CHECK(pDS != nullptr);

			if (pDS != nullptr)
			{
				// Now we create the output layer. In this case since the datasource is a single file, 
				// we can only have one layer. We pass wkbPoint to specify the type of geometry supported 
				// by this layer. In this case we aren’t passing any coordinate system information or other
				// special layer creation options.

				OGRLayer* pLayer = pDS->CreateLayer("point_out", nullptr, wkbPoint, nullptr);
				CHECK(pLayer != nullptr);

				if (pLayer != nullptr)
				{
					// Now that the layer exists, we need to create any attribute fields that should appear on the layer.
					// Fields must be added to the layer before any features are written. To create a field we initialize
					// an OGRField object with the information about the field. In the case of Shapefiles, the field width
					// and precision is significant in the creation of the output .dbf file, so we set it specifically, 
					// though generally the defaults are OK. For this example we will just have one attribute, a name string
					// associated with the x,y point.
					//
					// Note that the template OGRField we pass to OGRLayer::CreateField() is copied internally. We retain ownership of the object.

					OGRFieldDefn oField("Name", OFTString);
					oField.SetWidth(32);
					if (pLayer->CreateField(&oField) != OGRERR_NONE)
					{
						// To write a feature to disk, we must create a local OGRFeature, set attributes and attach geometry
						// before trying to write it to the layer. It is imperative that this feature be instantiated from 
						// the OGRFeatureDefn associated with the layer it will be written to.

						{
							OGRFeature* pFeature = OGRFeature::CreateFeature(pLayer->GetLayerDefn());
							pFeature->SetField("Name", "TestPoint1");

							// We create a local geometry object, and assign its copy (indirectly) to the feature. The OGRFeature::SetGeometryDirectly()
							// differs from OGRFeature::SetGeometry() in that the direct method gives ownership of the geometry to the feature. This is 
							// generally more efficient as it avoids an extra deep object copy of the geometry.

							OGRPoint pt;
							pt.setX(101.0);
							pt.setY(201.0);

							pFeature->SetGeometry(&pt);

							OGRErr err = pLayer->CreateFeature(pFeature);
							CHECK(err == OGRERR_NONE);

							OGRFeature::DestroyFeature(pFeature);
						}
					}
				}

				GDALClose(pDS);
			}
		}
	}
}
