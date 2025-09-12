// RasterFile.cpp
// Lightweight front-end for all GDAL raster files.
//

#include <assert.h>

#include "MarkLib.h"
#include "XString.h"
#include "Logger.h"
#include "Calc.h"

#include "RasterFile.h"

RasterFile::RasterFile(const char* fileName, bool loadData)
	: mp_DS(nullptr)
	, mp_Data(nullptr)
	, m_SizeX(0)
	, m_SizeY(0)
	, mf_PixelSizeX(0.0)
	, mf_PixelSizeY(0.0)
	, mf_MinElev(0.0)
	, mf_MaxElev(0.0)
	, mf_Mean(0.0)
	, mf_StdDev(0.0)
	, mf_NoData(NAN)
	, m_UtmZone(0)
	, mb_HasGeoTransform(false)
{
	Load(fileName, loadData);
}

RasterFile::~RasterFile()
{
	if (mp_Data)
	{
		CPLFree(mp_Data);
		mp_Data = nullptr;
	}

	GDALClose(mp_DS);
}

void RasterFile::Load(const char* fileName, bool loadData)
{
	// Load all data/metadata & close dataset.
	//

	mp_DS = (GDALDataset *)GDALOpen(fileName, GA_ReadOnly);
	if (mp_DS == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to open dataset: %s", fileName);
	}
	else
	{
		ms_DriverName = mp_DS->GetDriverName();

		m_SizeX = mp_DS->GetRasterXSize();
		m_SizeY = mp_DS->GetRasterYSize();

		int bandCount = mp_DS->GetRasterCount();

		const OGRSpatialReference* pSRS = mp_DS->GetSpatialRef();
		if (mp_DS != nullptr)
		{
			double geoTransform[6] = { 0 };
			CPLErr err = mp_DS->GetGeoTransform(geoTransform);
			mb_HasGeoTransform = (err == CPLErr::CE_None);

			if (mb_HasGeoTransform)
			{
				mf_PixelSizeX = geoTransform[1];
				mf_PixelSizeY = geoTransform[5];

				// these are the gdal extents which straddle the outer rows/columns, 
				// giving a slightly larger (1/2 pixel size) extent than what i would
				// use. it's not necessarily wrong, just be aware - possibly adding 
				// something like an "on edge" flag to the getextents() method.
				m_Extents.x0 = geoTransform[0];		// nw corner
				m_Extents.y0 = geoTransform[3];
				m_Extents.x1 = m_Extents.x0 + mf_PixelSizeX * static_cast<double>(m_SizeX);		// se corner
				m_Extents.y1 = m_Extents.y0 + mf_PixelSizeY * static_cast<double>(m_SizeY);

				m_UtmZone = pSRS->GetUTMZone();
			}
		}

		if (loadData == true)
		{
			assert(bandCount == 1);

			GDALRasterBand* pBand = mp_DS->GetRasterBand(1);
			assert(pBand != nullptr);

			int bandSizeX = pBand->GetXSize();
			int bandSizeY = pBand->GetYSize();

			// read all data as 32 bit floats

			if (mp_Data)
				CPLFree(mp_Data);

			//MarkLib::PerfTimer(true);

			mp_Data = (float *)CPLMalloc(sizeof(float)*bandSizeX*bandSizeY);

			pBand->RasterIO(
				GF_Read,			// read or write
				0,					// xoffset - zero based offset from left
				0,					// yoffset - zero based offset from top
				bandSizeX,			// width of the region of the band to be accessed in pixels
				bandSizeY,			// height of the region of the band to be accessed in lines
				mp_Data,			// data buffer
				bandSizeX,			// width of data buffer
				bandSizeY,			// height of data buffer
				GDT_Float32,		// buffer type - data automatically translated
				0, 0);

			//double et = MarkLib::PerfTimer();
			//Logger::Write(__FUNCTION__, "Load data %s time = %0.3f", fileName, et);

			int bSuccess = 0;
			mf_NoData = pBand->GetNoDataValue(&bSuccess);
			if (bSuccess == 0)
				Logger::Write(__FUNCTION__, "Unable to access 'no data' value: %s", fileName);

			//MarkLib::PerfTimer(true);
			mf_MinElev = 0.0;
			mf_MaxElev = 0.0;;
			bool bInit = false;
			for (int i = 0; i < bandSizeX*bandSizeY; ++i)
			{
				// valid elevation value
				if (mp_Data[i] != mf_NoData)
				{
					// range not yet initialized
					if (bInit == false)
					{
						mf_MinElev = mp_Data[i];
						mf_MaxElev = mp_Data[i];
						bInit = true;
					}
					else
					{
						if (mp_Data[i] < mf_MinElev)
							mf_MinElev = mp_Data[i];
						if (mp_Data[i] > mf_MaxElev)
							mf_MaxElev = mp_Data[i];
					}
				}
			}
			//et = MarkLib::PerfTimer();
			//Logger::Write(__FUNCTION__, "Calc min/max time = %0.3f", et);
		}
	}
}

const char* RasterFile::GetDriverName()
{
	return ms_DriverName;
}

const GIS::GEODATA RasterFile::GetFileType()
{
	// There are a number of ways this could be determined.
	//

	GIS::GEODATA fileType = GIS::GEODATA::None;

	if (ms_DriverName.CompareNoCase("USGSDEM"))
	{
		if ((int)GetPixelSizeX() == 10)
			fileType = GIS::GEODATA::DEM10;
		else if ((int)GetPixelSizeX() == 30)
			fileType = GIS::GEODATA::DEM30;
		else
			assert(false);
	}
	else if (ms_DriverName.CompareNoCase("DTED"))
	{
		if (GetSizeY() == 121)
			fileType = GIS::GEODATA::DTED0;
		else if (GetSizeY() == 1201)
			fileType = GIS::GEODATA::DTED1;
		else if (GetSizeY() == 3601)
			fileType = GIS::GEODATA::DTED2;
		else
			assert(false);
	}
	else
	{
		Logger::Write(__FUNCTION__, "Unhandled file type: %s", ms_DriverName.c_str());
		assert(false);
	}

	return fileType;
}

bool RasterFile::HasGeoTransform()
{
	// Is there geo transform in this file?
	// Non-geospatial files like bitmaps & plain 
	// tiff files will return false here.
	//

	return mb_HasGeoTransform;
}

bool RasterFile::IsProjected()
{
	if (mp_DS)
	{
		if (mp_DS->GetSpatialRef())
			return mp_DS->GetSpatialRef()->IsProjected();
	}
	return false;
}

bool RasterFile::IsGeographic()
{
	if (mp_DS)
	{
		if (mp_DS->GetSpatialRef())
			return mp_DS->GetSpatialRef()->IsGeographic();
	}
	return false;
}

double RasterFile::GetHeight(int row, int col)
{
	// get height by row,col

	double height = mf_NoData;

	if (row >= GetSizeY() || row < 0)
		return height;
	if (col >= GetSizeX() || col < 0)
		return height;

	height = mp_Data[row*GetSizeX() + col];

	return height;
}

double RasterFile::GetHeight(double x, double y)
{
	// Return height of data point at specified coordinates
	//
	// Inputs:
	//		x = E-W offset to profile, meters
	//		y = N-S offset to data point, meters
	//
	// Outputs:
	//		height = height value at specified data point, meters
	//

	double height = mf_NoData;

	int col = (int)((x - m_Extents.x0) / mf_PixelSizeX);
	int row = (int)((y - m_Extents.y0) / mf_PixelSizeY);

	height = GetHeight(row, col);

	return height;
}

double RasterFile::GetNoData()
{
	return mf_NoData;
}

int RasterFile::GetSizeX()
{
	return m_SizeX;
}

int RasterFile::GetSizeY()
{
	return m_SizeY;
}

double RasterFile::GetPixelSizeX()
{
	return mf_PixelSizeX;
}

double RasterFile::GetPixelSizeY()
{
	return mf_PixelSizeY;
}

double RasterFile::GetMinElev()
{
	return mf_MinElev;
}

double RasterFile::GetMaxElev()
{
	return mf_MaxElev;
}

RectD RasterFile::GetExtents()
{
	return m_Extents;
}

int RasterFile::GetUtmZone()
{
	return m_UtmZone;
}

bool RasterFile::Contains(double x, double y)
{
	// m_Extents are whatever gdal says they are,
	// need to normalize (w/o modifying member var)
	// to see if within ranges
	RectD ext = m_Extents;
	ext.Normalize();

	if (x < ext.x0 || x > ext.x1)
		return false;
	if (y < ext.y0 || y > ext.y1)
		return false;

	return true;
}