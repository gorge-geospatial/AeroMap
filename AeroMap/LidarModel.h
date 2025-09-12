#ifndef LIDARMODEL_H
#define LIDARMODEL_H

#include "AeroMap.h"

#include <pdal/pdal_types.h>
#include <pdal/Log.h>
#include <pdal/Kernel.h>
//#include <pdal/PluginManager.h>
//#include <pdal/StageFactory.h>
//#include <pdal/pdal_config.h>
//#include <pdal/util/Backtrace.h>
#include <pdal/private/gdal/GDALUtils.h>

#include "LasFile.h"

class LidarModel
{
public:

	LidarModel();
	virtual ~LidarModel();

	int  SetSourceFile(const char* fileName);
	int  SetOutputFolder(const char* pathName);
	void SetResolution(double res);

	int CreateDSM();
	int CreateDTM();

private:

	enum class OutputType
	{
		DTM,		// digital terrain model (ground)
		DSM,		// ditigal surface model (canopy)
	};

	XString ms_SourceFile;		// source las/laz file
	XString ms_OutputFolder;	// path to location of output files
	double mf_Resolution;		// resolution for output grids
	std::string m_log;			// pdal log file name
	pdal::LogPtr m_logPtr;

private:

	int ExecutePipeline(pdal::StringList& cmdArgs);
	XString CreatePipeline(OutputType type);
	void CreateOutputFolder();
};

#endif // #ifndef LIDARMODEL_H