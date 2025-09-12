#ifndef GIS_H
#define GIS_H

#include "Calc.h"

struct PointLL	// double point for specifying lat/lon
{
	double lat;
	double lon;

	PointLL()
	{
		lat = 0.0;
		lon = 0.0;
	}
};

struct RectLL	// double rect for specifying lat/lon
{
	double latN;
	double latS;
	double lonE;
	double lonW;

	RectLL()
	{
		Clear();
	}
	RectLL(double latN, double latS, double lonE, double lonW)
	{
		this->latN = latN;
		this->latS = latS;
		this->lonE = lonE;
		this->lonW = lonW;

		Normalize();
	}
	double DX()
	{
		return lonE - lonW;
	}
	double DY()
	{
		return latN - latS;
	}
	void Clear()
	{
		latN = latS = lonE = lonW = 0.0;
	}
	void Union(const RectLL& rect)
	{
		// convert this rect to the union of this one
		// and the rect parameters
		//

		if (latN < rect.latN)
			latN = rect.latN;
		if (latS > rect.latS)
			latS = rect.latS;
		if (lonE < rect.lonE)
			lonE = rect.lonE;
		if (lonW > rect.lonW)
			lonW = rect.lonW;
	}
	bool Contains(double lat, double lon)
	{
		// return true if lat/lon inside 
		// this RectLL

		if (lat > this->latN)
			return false;
		if (lat < this->latS)
			return false;
		if (lon < this->lonW)
			return false;
		if (lon > this->lonE)
			return false;

		return true;
	}
	void Normalize()
	{
		if (latN < latS)
		{
			double t = latN;
			latN = latS;
			latS = t;
		}
		if (lonE < lonW)
		{
			double t = lonE;
			lonE = lonW;
			lonW = t;
		}
	}
	void Expand(double delta)
	{
		// expand region to next delta degree
		// (ex 4.273 -> 4.5, delta == 0.5)

		latN = ModGTE(latN, delta);
		latS = ModLTE(latS, delta);
		lonE = ModGTE(lonE, delta);
		lonW = ModLTE(lonW, delta);
	}
};

// general purpose rect for geospatial data,
// does not assume any projection, xy vs lat/lon,
// etc.
struct RectGeo
{
	double yn;		// northernmost point
	double ys;
	double xw;		// westernmost point
	double xe;

	RectGeo()
	{
		Clear();
	}
	RectGeo(double yn, double ys, double xw, double xe)
	{
		this->yn = yn;
		this->ys = ys;
		this->xw = xw;
		this->xe = xe;
	}
	void Clear()
	{
		yn = ys = xw = xe = 0.0;
	}
	double DX()
	{
		return xe - xw;
	}
	double DY()
	{
		return yn - ys;
	}
	void Union(const RectGeo& rect)
	{
		// Convert this rect to the union of this one
		// and the rect parameters

		if (yn < rect.yn)
			yn = rect.yn;
		if (ys > rect.ys)
			ys = rect.ys;
		if (xw > rect.xw)
			xw = rect.xw;
		if (xe < rect.xe)
			xe = rect.xe;
	}
	bool Contains(double x, double y)
	{
		// Return true if point inside this rect

		if ((x >= this->xw && x <= this->xe) || (x >= this->xe && x <= this->xw))
			return true;
		if ((y >= this->ys && y <= this->yn) || (y >= this->yn && y <= this->ys))
			return true;

		return false;
	}
};

class GIS
{
public:

	enum class GEODATA		// geospatial file types
	{
		None,

		// raster
		DEM10,				// USGS DEM - 10m spacing
		DEM30,				// USGS DEM - 30m spacing
		DTED0,				// DTED level 0, EGM96 implied
		DTED1,
		DTED2,
		E00GRID,			// ESRI E00 grid format
		GeoTIFF,
		HFA,				// Erdas Imagine
		NITF,
		PDS,				// Planetary Data Systems v3
		SDTS_DEM,			// USGS DEM file, SDTS format
		TIFF,				// standard TIFF file (includes bigtiff)
		VtpBT,				// Virtual Terrain Project BT file
		XYZ,				// ASCII Gridded XYZ file

		TerrainFile,		// AeroMap Terrain model

		// vector
		ArcGen,				// ARCGEN - Arc/Info Generate driver.
		BNA,				// BNA vector exchange format
		GeoJSON,
		ShapeFile,			// SHP file

		// point cloud
		LAS,				// lidar LAS file
		LAZ,				// lidar LAZ file

		Count
	};

	enum class Projection		// projection IDs
	{
		None,					// no projection, this means "not set"
		AlbersEqualAreaConic,	// AlbersEqualAreaConic
		BipolarObliqueCC,		// BipolarObliqueConicConformal
		EquidistantConic,		// EquidistantConic
		LambertCC,				// Lambert Conformal Conic
		Mercator,				// Mercator
		UTM,					// Universal Transverse Mercator
		Geographic,				// My definition for lat/lon
		Count					// # of projections supported
	};

	enum class Ellipsoid
	{
		None,
		Airy,
		Australian,
		Bessel,
		Clarke1866,
		Clarke1880,
		Everest,
		GRS_80,
		Intl_1924,
		Krasovsky,
		WGS_72,
		WGS_84,
		Count
	};

	enum class Datum
	{
		None,
		NAD27,		// North American Datum of 1927
		NAD83,		// North American Datum of 1983
		WGS84		// World Geodetic System of 1984
	};

	enum class Hemi		// hemispheres
	{
		N,
		S,
		E,
		W
	};

public:
	GIS();
	virtual ~GIS();

	// Albers Equal-Area Conic
	static void	LatLonToXY_AlbersEqualAreaConic(double lat, double lon, double& xm, double& ym,
											    double stdParallel1 = 29.5, double stdParallel2 = 45.5,
												Ellipsoid ellipsoid = Ellipsoid::Clarke1866);
	static void	XYToLatLon_AlbersEqualAreaConic(double xm, double ym, double& lat, double& lon,
											    double stdParallel1 = 29.5, double stdParallel2 = 45.5,
												Ellipsoid ellipsoid = Ellipsoid::Clarke1866);

	// Bipolar Oblique Conic Conformal
	static void	LatLonToXY_BipolarObliqueConicConformal(double lat, double lon, double& xm, double& ym,
														double R = 6370997.0);
	static void	XYToLatLon_BipolarObliqueConicConformal(double xm, double ym, double& lat, double& lon, 
														double R = 6370997.0);

	// Cassini
	//static void LatLonToXY_Cassini( double lat, double lon, double& xm, double& ym, Ellipsoid ellipsoid = Ellipsoid::Clarke1866 );
	//static void XYToLatLon_Cassini( double xm, double ym, double& lat, double& lon, Ellipsoid ellipsoid = Ellipsoid::Clarke1866 );

	// Equidistant Conic projection
	static void	LatLonToXY_EquidistantConic(double lat, double lon, double& xm, double& ym,
											double stdParallel1 = 29.5, double stdParallel2 = 45.5, 
											Ellipsoid ellipsoid = Ellipsoid::Clarke1866);
	static void	XYToLatLon_EquidistantConic(double xm, double ym, double& lat, double& lon,
											double stdParallel1 = 29.5, double stdParallel2 = 45.5, 
											Ellipsoid ellipsoid = Ellipsoid::Clarke1866);

	// Lambert Conformal Conic
	static void	LatLonToXY_LambertCC(double lat, double lon, double& xm, double& ym,
									 double stdParallel1 = 33.0, double stdParallel2 = 45.0,
									 Ellipsoid ellipsoid = Ellipsoid::Clarke1866);
	static void	XYToLatLon_LambertCC(double xm, double ym, double& lat, double& lon,
									 double stdParallel1 = 33.0, double stdParallel2 = 45.0,
									 Ellipsoid ellipsoid = Ellipsoid::Clarke1866);

	// Mercator
	static void	LatLonToXY_Mercator(double lat, double lon, double& xm, double& ym, Ellipsoid ellipsoid = Ellipsoid::Clarke1866);
	static void	XYToLatLon_Mercator(double xm, double ym, double& lat, double& lon, Ellipsoid ellipsoid = Ellipsoid::Clarke1866);

	// Polyconic projection
	//static void LatLonToXY_PolyConic(double lat, double lon, double& xm, double& ym, Ellipsoid ellipsoid = Ellipsoid::Clarke1866);
	//static void XYToLatLon_PolyConic(double xm, double ym, double& lat, double& lon, Ellipsoid ellipsoid = Ellipsoid::Clarke1866);

	// Universal Transverse Mercator
	static void		LatLonToXY_UTM(double lat, double lon, double& xm, double& ym, Ellipsoid ellipsoid = Ellipsoid::Clarke1866);
	static void		XYToLatLon_UTM(int zone, Hemi hemi, double xm, double ym, double& lat, double& lon, Ellipsoid ellipsoid = Ellipsoid::Clarke1866);
	static void		XYToLatLon_UTM(int zone, char hemi, double xm, double ym, double& lat, double& lon, Ellipsoid ellipsoid = Ellipsoid::Clarke1866);
	static int		GetUTMZone(double lon);
	static double	GetUTMLongitude(int utmZone);

	static double	DMSToDD(int deg, int min, double sec, Hemi hemi);
	static void		DDToDMS(double degrees, int& deg, int& min, double& sec, Hemi& hemi);

	static char*	ProjectionStr(Projection proj);
	static GEODATA  GetFileType(const char* fileName);
	static char*	GeodataToString(GEODATA geoType);

	static bool IsRaster(GEODATA type);
	static bool IsVector(GEODATA type);
	static bool IsPointCloud(GEODATA type);

	// utilities for country/state names
	static char* StateToID(const char* stateName);
	static char* IDToState(const char* stateID);

private:

	static double	CalcPhi(double e, double t);

	static int		GetEllipsoidIndex(const char* ellipsoid);
	static int      GetEllipsoidIndex(Ellipsoid ellipsoid);
	static char*	GetEllipsoidName(unsigned int ellipseIndex);
};

#endif // #ifndef GIS_H
