// TestGis.cpp
// CppUnitLite test harness for Gis class
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
#include "Gis.h"						// interface to class under test

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
TEST(Gis, Project_UTM)
{
	// Test UTM projections
	//

	{
		// origin of zone 10

		double lat = 0.0;
		double lon = -123.0;
		double xm = 0.0;
		double ym = 0.0;

		int utmZone = GIS::GetUTMZone(lon);
		LONGS_EQUAL(10, utmZone);

		double utmLon = GIS::GetUTMLongitude(10);
		DOUBLES_EQUAL(-126.0, utmLon, 0.0);

		GIS::LatLonToXY_UTM(lat, lon, xm, ym, GIS::Ellipsoid::Clarke1866);
		DOUBLES_EQUAL(500000.0, xm, 0.0);
		DOUBLES_EQUAL(0.0, ym, 0.0);
	}

	{
		// origin of zone 10

		double lat = 0.0;
		double lon = 0.0;

		GIS::XYToLatLon_UTM(10, GIS::Hemi::N, 500000.0, 0.0, lat, lon, GIS::Ellipsoid::Clarke1866);
		DOUBLES_EQUAL(0.0, lat, 0.0);
		DOUBLES_EQUAL(-123.0, lon, 0.0);
	}

	{
		double lat = 45.0;
		double lon = -124.0;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_UTM(lat, lon, xm, ym, GIS::Ellipsoid::Clarke1866);
		DOUBLES_EQUAL(421182.37089256174, xm, 0.0);
		DOUBLES_EQUAL(4983219.5921163736, ym, 0.0);
	}

	{
		// southern latitude

		double lat = -45.0;
		double lon = -124.0;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_UTM(lat, lon, xm, ym, GIS::Ellipsoid::Clarke1866);
		DOUBLES_EQUAL(421182.37089256174, xm, 0.0);
		DOUBLES_EQUAL(3954046.9308184907, ym, 0.0);
	}

	{
		// WGS84 Ellipsoid

		double lat = 45.0;
		double lon = -124.0;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_UTM(lat, lon, xm, ym, GIS::Ellipsoid::WGS_84);
		DOUBLES_EQUAL(421184.69708326762, xm, 0.0);
		DOUBLES_EQUAL(4983436.7686033379, ym, 0.0);
	}

}

//----------------------------------------------------------------------------
TEST(Gis, Project_LCC)
{
	// Test Lambert Conformal Conic projections
	//

	{
		// Clarke 1866 Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_LambertCC(lat, lon, xm, ym, 33.0, 45.0, GIS::Ellipsoid::Clarke1866);
		DOUBLES_EQUAL(-1961029.0039014339, xm, 0.0);
		DOUBLES_EQUAL(2808560.1142551778, ym, 0.0);

		GIS::XYToLatLon_LambertCC(xm, ym, lat, lon, 33.0, 45.0, GIS::Ellipsoid::Clarke1866);
		DOUBLES_EQUAL(45.729361, lat, 1E-4);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}

	{
		// Clarke 1880 Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_LambertCC(lat, lon, xm, ym, 33.0, 45.0, GIS::Ellipsoid::Clarke1880);
		DOUBLES_EQUAL(-1961059.7126995767, xm, 0.0);
		DOUBLES_EQUAL(2808534.9429783765, ym, 0.0);

		GIS::XYToLatLon_LambertCC(xm, ym, lat, lon, 33.0, 45.0, GIS::Ellipsoid::Clarke1880);
		DOUBLES_EQUAL(45.729361, lat, 1E-4);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}

	{
		// Airy Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_LambertCC(lat, lon, xm, ym, 33.0, 45.0, GIS::Ellipsoid::Airy);
		DOUBLES_EQUAL(-1960781.9873068826, xm, 0.0);
		DOUBLES_EQUAL(2808400.3376018433, ym, 0.0);

		GIS::XYToLatLon_LambertCC(xm, ym, lat, lon, 33.0, 45.0, GIS::Ellipsoid::Airy);
		DOUBLES_EQUAL(45.729361, lat, 1E-4);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}

	{
		// Australian Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_LambertCC(lat, lon, xm, ym, 33.0, 45.0, GIS::Ellipsoid::Australian);
		DOUBLES_EQUAL(-1960977.4787890275, xm, 0.0);
		DOUBLES_EQUAL(2808632.8772207396, ym, 0.0);

		GIS::XYToLatLon_LambertCC(xm, ym, lat, lon, 33.0, 45.0, GIS::Ellipsoid::Australian);
		DOUBLES_EQUAL(45.729361, lat, 1E-4);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}

	{
		// Bessel Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_LambertCC(lat, lon, xm, ym, 33.0, 45.0, GIS::Ellipsoid::Bessel);
		DOUBLES_EQUAL(-1960732.8480259681, xm, 0.0);
		DOUBLES_EQUAL(2808322.2555498388, ym, 0.0);

		GIS::XYToLatLon_LambertCC(xm, ym, lat, lon, 33.0, 45.0, GIS::Ellipsoid::Bessel);
		DOUBLES_EQUAL(45.729361, lat, 1E-4);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}

	{
		// Everest Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_LambertCC(lat, lon, xm, ym, 33.0, 45.0, GIS::Ellipsoid::Everest);
		DOUBLES_EQUAL(-1960677.3013695376, xm, 0.0);
		DOUBLES_EQUAL(2808315.0329696564, ym, 0.0);

		GIS::XYToLatLon_LambertCC(xm, ym, lat, lon, 33.0, 45.0, GIS::Ellipsoid::Everest);
		DOUBLES_EQUAL(45.729361, lat, 1E-4);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}

	{
		// GRS_80 Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_LambertCC(lat, lon, xm, ym, 33.0, 45.0, GIS::Ellipsoid::GRS_80);
		DOUBLES_EQUAL(-1960970.3285446879, xm, 0.0);
		DOUBLES_EQUAL(2808622.9463575426, ym, 0.0);

		GIS::XYToLatLon_LambertCC(xm, ym, lat, lon, 33.0, 45.0, GIS::Ellipsoid::GRS_80);
		DOUBLES_EQUAL(45.729361, lat, 1E-4);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}

	{
		// Intl_1924 Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_LambertCC(lat, lon, xm, ym, 33.0, 45.0, GIS::Ellipsoid::Intl_1924);
		DOUBLES_EQUAL(-1961061.7184546064, xm, 0.0);
		DOUBLES_EQUAL(2808697.9082200732, ym, 0.0);

		GIS::XYToLatLon_LambertCC(xm, ym, lat, lon, 33.0, 45.0, GIS::Ellipsoid::Intl_1924);
		DOUBLES_EQUAL(45.729361, lat, 1E-4);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}

	{
		// Krasovsky Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_LambertCC(lat, lon, xm, ym, 33.0, 45.0, GIS::Ellipsoid::Krasovsky);
		DOUBLES_EQUAL(-1961003.0490560678, xm, 0.0);
		DOUBLES_EQUAL(2808671.7156639118, ym, 0.0);

		GIS::XYToLatLon_LambertCC(xm, ym, lat, lon, 33.0, 45.0, GIS::Ellipsoid::Krasovsky);
		DOUBLES_EQUAL(45.729361, lat, 1E-4);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}

	{
		// WGS_72 Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_LambertCC(lat, lon, xm, ym, 33.0, 45.0, GIS::Ellipsoid::WGS_72);
		DOUBLES_EQUAL(-1960969.6798490100, xm, 0.0);
		DOUBLES_EQUAL(2808622.1501757130, ym, 0.0);

		GIS::XYToLatLon_LambertCC(xm, ym, lat, lon, 33.0, 45.0, GIS::Ellipsoid::WGS_72);
		DOUBLES_EQUAL(45.729361, lat, 1E-4);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}

	{
		// WGS_84 Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_LambertCC(lat, lon, xm, ym, 33.0, 45.0, GIS::Ellipsoid::WGS_84);
		DOUBLES_EQUAL(-1960970.3260264595, xm, 0.0);
		DOUBLES_EQUAL(2808622.9526562234, ym, 0.0);

		GIS::XYToLatLon_LambertCC(xm, ym, lat, lon, 33.0, 45.0, GIS::Ellipsoid::WGS_84);
		DOUBLES_EQUAL(45.729361, lat, 1E-4);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}
}

//----------------------------------------------------------------------------
TEST(Gis, Project_BOCC)
{
	// Bipolar Oblique Conic Conformal
	//
	//Note:
	//doesn't seem to work at all

	{
		// Park Center

		double lat = 45.729361;
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_BipolarObliqueConicConformal(lat, lon, xm, ym, 6370997.0);
		//DOUBLES_EQUAL(-3571707.5505072055, xm, 0.0);
		//DOUBLES_EQUAL(3931906.8829899258, ym, 0.0);

		GIS::XYToLatLon_BipolarObliqueConicConformal(xm, ym, lat, lon, 6370997.0);
		//DOUBLES_EQUAL(  45.729361, lat, 1E-9);
		//DOUBLES_EQUAL(-121.488296, lon, 1E-9);
	}
}

//----------------------------------------------------------------------------
TEST(Gis, Project_EquidistantConic)
{
	// Equidistant Conic
	//

	{
		// Clarke 1866 Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_EquidistantConic(lat, lon, xm, ym, 29.5, 45.5, GIS::Ellipsoid::Clarke1866);
		DOUBLES_EQUAL(-1961268.3600434700, xm, 0.0);
		DOUBLES_EQUAL(2787742.3509614728, ym, 0.0);

		GIS::XYToLatLon_EquidistantConic(xm, ym, lat, lon, 29.5, 45.5, GIS::Ellipsoid::Clarke1866);
		DOUBLES_EQUAL(45.729361, lat, 1E-8);
		DOUBLES_EQUAL(-121.488296, lon, 1E-8);
	}

	{
		// WGS84 Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_EquidistantConic(lat, lon, xm, ym, 29.5, 45.5, GIS::Ellipsoid::WGS_84);
		DOUBLES_EQUAL(-1961209.6385660900, xm, 0.0);
		DOUBLES_EQUAL(2787804.1671053804, ym, 0.0);

		GIS::XYToLatLon_EquidistantConic(xm, ym, lat, lon, 29.5, 45.5, GIS::Ellipsoid::WGS_84);
		DOUBLES_EQUAL(45.729361, lat, 1E-8);
		DOUBLES_EQUAL(-121.488296, lon, 1E-8);
	}
}

//----------------------------------------------------------------------------
TEST(Gis, Project_AlbersEqualAreaConic)
{
	// Albers Equal-Area Conic
	//

	{
		// Clarke 1866 Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_AlbersEqualAreaConic(lat, lon, xm, ym, 29.5, 45.5, GIS::Ellipsoid::Clarke1866);
		DOUBLES_EQUAL(-1961623.7843194583, xm, 0.0);
		DOUBLES_EQUAL(2791386.2296579275, ym, 0.0);

		GIS::XYToLatLon_AlbersEqualAreaConic(xm, ym, lat, lon, 29.5, 45.5, GIS::Ellipsoid::Clarke1866);
		DOUBLES_EQUAL(45.729361, lat, 1E-6);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}

	{
		// WGS_84 Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_AlbersEqualAreaConic(lat, lon, xm, ym, 29.5, 45.5, GIS::Ellipsoid::WGS_84);
		DOUBLES_EQUAL(-1961565.0697979990, xm, 0.0);
		DOUBLES_EQUAL(2791448.2281152308, ym, 0.0);

		GIS::XYToLatLon_AlbersEqualAreaConic(xm, ym, lat, lon, 29.5, 45.5, GIS::Ellipsoid::WGS_84);
		DOUBLES_EQUAL(45.729361, lat, 1E-6);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}
}

//----------------------------------------------------------------------------
TEST(Gis, Project_Mercator)
{
	// Mercator
	//

	{
		// Clarke 1866 Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_Mercator(lat, lon, xm, ym, GIS::Ellipsoid::Clarke1866);
		DOUBLES_EQUAL(6513563.9675256191, xm, 0.0);
		DOUBLES_EQUAL(5706198.0237542707, ym, 0.0);

		GIS::XYToLatLon_Mercator(xm, ym, lat, lon, GIS::Ellipsoid::Clarke1866);
		DOUBLES_EQUAL(45.729361, lat, 1E-4);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}

	{
		// WGS84 Ellipsoid

		double lat = 45.729361;		// Park Center
		double lon = -121.488296;
		double xm = 0.0;
		double ym = 0.0;

		GIS::LatLonToXY_Mercator(lat, lon, xm, ym, GIS::Ellipsoid::WGS_84);
		DOUBLES_EQUAL(6513493.0947267478, xm, 0.0);
		DOUBLES_EQUAL(5706475.9512932170, ym, 0.0);

		GIS::XYToLatLon_Mercator(xm, ym, lat, lon, GIS::Ellipsoid::WGS_84);
		DOUBLES_EQUAL(45.729361, lat, 1E-4);
		DOUBLES_EQUAL(-121.488296, lon, 1E-6);
	}
}

//----------------------------------------------------------------------------
TEST(Gis, RectGeo)
{
	// Test RectGeo type
	//

	{
		RectGeo rect(45.0, 43.0, 55.0, 56.0);

		DOUBLES_EQUAL(45.0, rect.yn, 0.0);
		DOUBLES_EQUAL(43.0, rect.ys, 0.0);
		DOUBLES_EQUAL(56.0, rect.xe, 0.0);
		DOUBLES_EQUAL(55.0, rect.xw, 0.0);

		CHECK(rect.Contains(55.2, 44.0) == true);

		DOUBLES_EQUAL(1.0, rect.DX(), 0.0);
		DOUBLES_EQUAL(2.0, rect.DY(), 0.0);

		RectGeo rect2(45.75, 44.5, 55.75, 56.5);
		rect.Union(rect2);

		DOUBLES_EQUAL(45.75, rect.yn, 0.0);
		DOUBLES_EQUAL(43.0, rect.ys, 0.0);
		DOUBLES_EQUAL(56.5, rect.xe, 0.0);
		DOUBLES_EQUAL(55.0, rect.xw, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(Gis, RectLL)
{
	// Test RectLL class
	//

	{
		RectLL rect(45.0, 43.0, 56.0, 55.0);

		DOUBLES_EQUAL(45.0, rect.latN, 0.0);
		DOUBLES_EQUAL(43.0, rect.latS, 0.0);
		DOUBLES_EQUAL(56.0, rect.lonE, 0.0);
		DOUBLES_EQUAL(55.0, rect.lonW, 0.0);

		CHECK(rect.Contains(44.0, 55.2) == true);

		DOUBLES_EQUAL(1.0, rect.DX(), 0.0);
		DOUBLES_EQUAL(2.0, rect.DY(), 0.0);

		RectLL rect2(45.75, 44.5, 56.5, 55.75);
		rect.Union(rect2);

		DOUBLES_EQUAL(45.75, rect.latN, 0.0);
		DOUBLES_EQUAL(43.0, rect.latS, 0.0);
		DOUBLES_EQUAL(56.5, rect.lonE, 0.0);
		DOUBLES_EQUAL(55.0, rect.lonW, 0.0);
	}

	{
		RectLL rect;
		rect.latN = 30.0;
		rect.latS = 32.0;
		rect.lonE = 11.0;
		rect.lonW = 12.0;
		rect.Normalize();

		DOUBLES_EQUAL(32.0, rect.latN, 0.0);
		DOUBLES_EQUAL(30.0, rect.latS, 0.0);
		DOUBLES_EQUAL(12.0, rect.lonE, 0.0);
		DOUBLES_EQUAL(11.0, rect.lonW, 0.0);

	}
}

//----------------------------------------------------------------------------
TEST(Gis, LatLonConvert)
{
	// Test converting to/from degrees/minutes,seconds to/from 
	// decimal degrees.
	//

	{
		double deg = GIS::DMSToDD(45, 30, 0, GIS::Hemi::N);
		DOUBLES_EQUAL(45.5, deg, 0.0);
	}

	{
		double deg = GIS::DMSToDD(45, 30, 0, GIS::Hemi::S);
		DOUBLES_EQUAL(-45.5, deg, 0.0);
	}

	{
		double deg = GIS::DMSToDD(145, 30, 0, GIS::Hemi::E);
		DOUBLES_EQUAL(145.5, deg, 0.0);
	}

	{
		double deg = GIS::DMSToDD(145, 30, 0, GIS::Hemi::W);
		DOUBLES_EQUAL(-145.5, deg, 0.0);
	}

	{
		int deg = 0;
		int min = 0;
		double sec = 0.0;
		GIS::Hemi hemi = GIS::Hemi::N;

		GIS::DDToDMS(45.5, deg, min, sec, hemi);
		LONGS_EQUAL(45, deg);
		LONGS_EQUAL(30, min);
		DOUBLES_EQUAL(0.0, sec, 0.0);
	}

	{
		// Hemisphere only a hint to indicate lat or lon.
		// If negative value is passed, ensure returned 
		// value is correct (eg. south == negative).

		int deg = 0;
		int min = 0;
		double sec = 0.0;
		GIS::Hemi hemi = GIS::Hemi::N;

		GIS::DDToDMS(-45.5, deg, min, sec, hemi);
		LONGS_EQUAL(45, deg);
		LONGS_EQUAL(30, min);
		DOUBLES_EQUAL(0.0, sec, 0.0);
		CHECK(GIS::Hemi::S == hemi);
	}

	{
		int deg = 0;
		int min = 0;
		double sec = 0.0;
		GIS::Hemi hemi = GIS::Hemi::S;

		GIS::DDToDMS(45.5, deg, min, sec, hemi);
		LONGS_EQUAL(45, deg);
		LONGS_EQUAL(30, min);
		DOUBLES_EQUAL(0.0, sec, 0.0);
	}

	{
		int deg = 0;
		int min = 0;
		double sec = 0.0;
		GIS::Hemi hemi = GIS::Hemi::E;

		GIS::DDToDMS(145.5, deg, min, sec, hemi);
		LONGS_EQUAL(145, deg);
		LONGS_EQUAL(30, min);
		DOUBLES_EQUAL(0.0, sec, 0.0);
	}

	{
		// Check sign flip for longitude.

		int deg = 0;
		int min = 0;
		double sec = 0.0;
		GIS::Hemi hemi = GIS::Hemi::E;

		GIS::DDToDMS(-145.5, deg, min, sec, hemi);
		LONGS_EQUAL(145, deg);
		LONGS_EQUAL(30, min);
		DOUBLES_EQUAL(0.0, sec, 0.0);
		CHECK(GIS::Hemi::W == hemi);
	}

	{
		int deg = 0;
		int min = 0;
		double sec = 0.0;
		GIS::Hemi hemi = GIS::Hemi::W;

		GIS::DDToDMS(145.5, deg, min, sec, hemi);
		LONGS_EQUAL(145, deg);
		LONGS_EQUAL(30, min);
		DOUBLES_EQUAL(0.0, sec, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(Gis, RectLL_General)
{
	{
		// Contains()

		RectLL rect;
		rect.Clear();

		rect.latN = 45.0;
		rect.latS = 44.0;
		rect.lonE = -123.0;
		rect.lonW = -124.0;

		CHECK(rect.Contains(44.5, -123.5) == true);
		CHECK(rect.Contains(44.0, -123.5) == true);
		CHECK(rect.Contains(44.0, -123.0) == true);
		CHECK(rect.Contains(43.99, -123.0) == false);
	}

	{
		// Constructor

		RectLL rect(-45.0, -46.0, 12.0, 11.0);

		CHECK(rect.Contains(-45.5, 11.5) == true);
		CHECK(rect.Contains(-46.5, 11.5) == false);
	}

	{
		// Constructor - out of order

		RectLL rect(45.0, 46.0, 11.0, 12.0);

		CHECK(rect.Contains(45.5, 11.5) == true);
		DOUBLES_EQUAL(1.0, rect.DX(), 0.0);
		DOUBLES_EQUAL(1.0, rect.DY(), 0.0);
	}

	{
		// Union

		RectLL rect1(45.0, 46.0, 12.0, 11.0);
		RectLL rect2(45.5, 46.5, 12.5, 11.5);

		rect1.Union(rect2);

		DOUBLES_EQUAL(46.5, rect1.latN, 0.0);
		DOUBLES_EQUAL(45.0, rect1.latS, 0.0);
		DOUBLES_EQUAL(12.5, rect1.lonE, 0.0);
		DOUBLES_EQUAL(11.0, rect1.lonW, 0.0);
	}

	{
		// Union

		RectLL rect(45.12, 48.170, 12.98, 11.02);

		rect.Expand(0.5);

		DOUBLES_EQUAL(48.5, rect.latN, 0.0);
		DOUBLES_EQUAL(45.0, rect.latS, 0.0);
		DOUBLES_EQUAL(13.0, rect.lonE, 0.0);
		DOUBLES_EQUAL(11.0, rect.lonW, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(Gis, StateID)
{
	{
		XString id = GIS::StateToID("Alabama");
		CHECK(id.Compare("AL"));

		id = GIS::StateToID("Oregon");
		CHECK(id.Compare("OR"));

		id = GIS::StateToID("South Carolina");
		CHECK(id.Compare("SC"));
	}

	{
		XString name = GIS::IDToState("AL");
		CHECK(name.Compare("Alabama"));

		name = GIS::IDToState("OR");
		CHECK(name.Compare("Oregon"));

		name = GIS::IDToState("SC");
		CHECK(name.Compare("South Carolina"));
	}
}
