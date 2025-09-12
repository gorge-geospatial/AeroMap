/******************************************************************************
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  Implements Open FileGDB OGR driver.
 * Author:   Even Rouault, <even dot rouault at spatialys.com>
 ****************************************************************************/

#include "cpl_port.h"
#include "ogr_openfilegdb.h"

#include <cstddef>
#include <cstring>

#include "cpl_conv.h"
#include "cpl_vsi.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_core.h"

 // g++ -O2 -Wall -Wextra -g -shared -fPIC ogr/ogrsf_frmts/openfilegdb/*.cpp
 // -o ogr_OpenFileGDB.so -Iport -Igcore -Iogr -Iogr/ogrsf_frmts
 // -Iogr/ogrsf_frmts/mem -Iogr/ogrsf_frmts/openfilegdb -L. -lgdal

extern "C" void RegisterOGROpenFileGDB();

#define ENDS_WITH(str, strLen, end) \
    (strLen >= strlen(end) && EQUAL(str + strLen - strlen(end), end))

/************************************************************************/
/*                         OGROpenFileGDBDriverIdentify()               */
/************************************************************************/

static GDALIdentifyEnum OGROpenFileGDBDriverIdentifyInternal(GDALOpenInfo* poOpenInfo, const char*& pszFilename)
{
	// First check if we have to do any work.
	size_t nLen = strlen(pszFilename);
	if (ENDS_WITH(pszFilename, nLen, ".gdb") ||
		ENDS_WITH(pszFilename, nLen, ".gdb/"))
	{
		// Check that the filename is really a directory, to avoid confusion
		// with Garmin MapSource - gdb format which can be a problem when the
		// driver is loaded as a plugin, and loaded before the GPSBabel driver
		// (http://trac.osgeo.org/osgeo4w/ticket/245)
		if (STARTS_WITH(pszFilename, "/vsicurl/https://github.com/") ||
			!poOpenInfo->bStatOK ||
			!poOpenInfo->bIsDirectory)
		{
			// In case we do not manage to list the directory, try to stat one
			// file.
			VSIStatBufL stat;
			if (!(STARTS_WITH(pszFilename, "/vsicurl/") &&
				VSIStatL(CPLFormFilename(
					pszFilename, "a00000001", "gdbtable"), &stat) == 0))
			{
				return GDAL_IDENTIFY_FALSE;
			}
		}
		return GDAL_IDENTIFY_TRUE;
	}
	/* We also accept zipped GDB */
	else if (ENDS_WITH(pszFilename, nLen, ".gdb.zip") ||
		ENDS_WITH(pszFilename, nLen, ".gdb.tar") ||
		/* Canvec GBs */
		(ENDS_WITH(pszFilename, nLen, ".zip") &&
			(strstr(pszFilename, "_gdb") != nullptr ||
				strstr(pszFilename, "_GDB") != nullptr)))
	{
		return GDAL_IDENTIFY_TRUE;
	}
	/* We also accept tables themselves */
	else if (ENDS_WITH(pszFilename, nLen, ".gdbtable"))
	{
		return GDAL_IDENTIFY_TRUE;
	}
#ifdef DEBUG
	/* For AFL, so that .cur_input is detected as the archive filename */
	else if (EQUAL(CPLGetFilename(pszFilename), ".cur_input"))
	{
		// This file may be recognized or not by this driver,
		// but there were not enough elements to judge.
		return GDAL_IDENTIFY_UNKNOWN;
	}
#endif

	else if (EQUAL(pszFilename, "."))
	{
		GDALIdentifyEnum eRet = GDAL_IDENTIFY_FALSE;
		char* pszCurrentDir = CPLGetCurrentDir();
		if (pszCurrentDir)
		{
			const char* pszTmp = pszCurrentDir;
			eRet = OGROpenFileGDBDriverIdentifyInternal(poOpenInfo, pszTmp);
			CPLFree(pszCurrentDir);
		}
		return eRet;
	}

	else
	{
		return GDAL_IDENTIFY_FALSE;
	}
}

static int OGROpenFileGDBDriverIdentify(GDALOpenInfo* poOpenInfo)
{
	const char* pszFilename = poOpenInfo->pszFilename;
	return OGROpenFileGDBDriverIdentifyInternal(poOpenInfo, pszFilename);
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

static GDALDataset* OGROpenFileGDBDriverOpen(GDALOpenInfo* poOpenInfo)
{
	if (poOpenInfo->eAccess == GA_Update)
		return nullptr;

	const char* pszFilename = poOpenInfo->pszFilename;
	if (OGROpenFileGDBDriverIdentifyInternal(poOpenInfo, pszFilename) == GDAL_IDENTIFY_FALSE)
		return nullptr;

#ifdef DEBUG
	/* For AFL, so that .cur_input is detected as the archive filename */
	if (poOpenInfo->fpL != nullptr &&
		!STARTS_WITH(poOpenInfo->pszFilename, "/vsitar/") &&
		EQUAL(CPLGetFilename(poOpenInfo->pszFilename), ".cur_input"))
	{
		GDALOpenInfo oOpenInfo(
			(CPLString("/vsitar/") + poOpenInfo->pszFilename).c_str(),
			poOpenInfo->nOpenFlags);
		oOpenInfo.papszOpenOptions = poOpenInfo->papszOpenOptions;
		return OGROpenFileGDBDriverOpen(&oOpenInfo);
	}
#endif

	OGROpenFileGDBDataSource* poDS = new OGROpenFileGDBDataSource();
	if (poDS->Open(pszFilename))
	{
		return poDS;
	}

	delete poDS;
	return nullptr;
}

/***********************************************************************/
/*                       RegisterOGROpenFileGDB()                      */
/***********************************************************************/

void RegisterOGROpenFileGDB()
{
	if (!GDAL_CHECK_VERSION("OGR OpenFileGDB"))
		return;

	if (GDALGetDriverByName("OpenFileGDB") != nullptr)
		return;

	GDALDriver* poDriver = new GDALDriver();

	poDriver->SetDescription("OpenFileGDB");
	poDriver->SetMetadataItem(GDAL_DCAP_VECTOR, "YES");
	poDriver->SetMetadataItem(GDAL_DMD_LONGNAME, "ESRI FileGDB");
	poDriver->SetMetadataItem(GDAL_DMD_EXTENSION, "gdb");
	poDriver->SetMetadataItem(GDAL_DMD_HELPTOPIC, "drivers/vector/openfilegdb.html");
	poDriver->SetMetadataItem(GDAL_DCAP_VIRTUALIO, "YES");

	poDriver->pfnOpen = OGROpenFileGDBDriverOpen;
	poDriver->pfnIdentify = OGROpenFileGDBDriverIdentify;

	GetGDALDriverManager()->RegisterDriver(poDriver);
}
