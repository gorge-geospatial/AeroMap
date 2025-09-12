#ifndef RASTERFILE_H
#define RASTERFILE_H

#include "Gis.h"
#include "XString.h"

#include "gdal_priv.h"
#include "cpl_conv.h"		// for CPLMalloc()

class RasterFile
{
public:

	RasterFile(const char* fileName, bool loadData = false);
	~RasterFile();

	const char* GetDriverName();
	const GIS::GEODATA GetFileType();

	bool IsProjected();
	bool IsGeographic();
	bool HasGeoTransform();

	double GetHeight(int row, int col);			// get height by row,col
	double GetHeight(double x, double y);		// get height by internal units
	double GetNoData();							// get "no data" value

	int GetSizeX();
	int GetSizeY();

	double GetPixelSizeX();
	double GetPixelSizeY();

	double GetMinElev();
	double GetMaxElev();

	RectD GetExtents();							// get xy extents, native units
	bool Contains(double x, double y);

	int GetUtmZone();				// return UTM zone #, if applicable

private:

	GDALDataset* mp_DS;				// backing dataset
	XString ms_DriverName;			// name of gdal driver being used

	int m_SizeX;					// raster dimensions
	int m_SizeY;

	bool mb_HasGeoTransform;		// all geo files have one (eg, regular tiff or other image does not)
	double mf_PixelSizeX;			// pixel size, units depend on srs
	double mf_PixelSizeY;

	double mf_MinElev;
	double mf_MaxElev;
	double mf_Mean;					// these are byproducts of needed statistics calc
	double mf_StdDev;

	RectD m_Extents;				// file extents, units depend on srs

	float* mp_Data;					// all elevation data stored as 32-bit floats
	double mf_NoData;				// value used by raster band for "no data"

	int m_UtmZone;					// utm zone #, if applicable

private:

	void Load(const char* fileName, bool loadData);
};

#endif // #ifndef RASTERFILE_H
