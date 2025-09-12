// LidarModel.cpp
// Create DSM/DTM from point clouds.
//

#include <pdal/Kernel.h>

#include "LidarModel.h"

LidarModel::LidarModel()
	: m_logPtr(nullptr)
	, mf_Resolution(0.0)
{
	m_log = XString::CombinePath(GetApp()->GetAppDataPath().c_str(), "pdal.log").c_str();

	m_logPtr = pdal::Log::makeLog("PDAL", m_log);
	m_logPtr->setLevel(pdal::LogLevel::Debug);
	m_logPtr->get(pdal::LogLevel::Debug) << "Debugging..." << std::endl;
	pdal::PluginManager<pdal::Stage>::setLog(m_logPtr);
	pdal::PluginManager<pdal::Kernel>::setLog(m_logPtr);
}

LidarModel::~LidarModel()
{
}

int LidarModel::CreateDTM()
{
	// Creat digital terrain model (ground) from lidar.
	//

	GetApp()->LogWrite("Creating DTM ...");

	CreateOutputFolder();

	pdal::StringList cmdArgs;
	XString pipeLine = CreatePipeline(OutputType::DTM);
	cmdArgs.push_back(pipeLine.c_str());

	int status = ExecutePipeline(cmdArgs);

	GetApp()->LogWrite("DTM complete.");

	return status;
}

int LidarModel::CreateDSM()
{
	// Creat digital surface model (canopy) from lidar.
	//

	GetApp()->LogWrite("Creating DSM ...");

	CreateOutputFolder();

	pdal::StringList cmdArgs;
	XString pipeLine = CreatePipeline(OutputType::DSM);
	cmdArgs.push_back(pipeLine.c_str());

	int status = ExecutePipeline(cmdArgs);

	GetApp()->LogWrite("DSM complete.");

	return status;
}

int LidarModel::ExecutePipeline(pdal::StringList& cmdArgs)
{
	// Execute pdal pipeline command.
	// 
	// Inputs:
	//		cmdArgs = parameter list
	//

	int ret = 0;
	std::string command = "pipeline";

	command = pdal::Utils::tolower(command);
	if (!command.empty())
	{
		std::string name("kernels." + command);

		pdal::Kernel* kernel = pdal::PluginManager<pdal::Kernel>::createObject(name);
		if (kernel)
		{
			m_logPtr->setLeader("pdal " + command);
			ret = kernel->run(cmdArgs, m_logPtr);
			delete kernel;
		}
		else
		{
			m_logPtr->get(pdal::LogLevel::Error) << "Command '" << command << "' not recognized" << std::endl << std::endl;
			ret = 1;
		}
	}

	return ret;
}

XString LidarModel::CreatePipeline(OutputType type)
{
	// Create pipeline definition file for type of output being
	// generated.
	//

	XString outputFile;
	XString limitsEntry;
	XString resolution = XString::Format("%0.2f", mf_Resolution);

	switch (type) {
	case OutputType::DTM:
		limitsEntry = "Classification[2:2]";
		outputFile = XString::CombinePath(ms_OutputFolder.c_str(), "dtm.tif");
		break;
	case OutputType::DSM:
		limitsEntry = "Classification[5:5]";
		outputFile = XString::CombinePath(ms_OutputFolder.c_str(), "dsm.tif");
		break;
	}

	XString fileName = XString::CombinePath(GetApp()->GetAppDataPath().c_str(), "pipeline-gen.txt");
	FILE* pFile = fopen(fileName.c_str(), "wt");
	if (pFile != nullptr)
	{
		fprintf(pFile, "{\n");
		fprintf(pFile, "    \"pipeline\": [\n");
		fprintf(pFile, "        \"%s\",\n", ms_SourceFile.c_str());					// input file
		fprintf(pFile, "        {\n");
		fprintf(pFile, "            \"type\": \"filters.range\",\n");				// use range filter
		fprintf(pFile, "            \"limits\": \"%s\"\n", limitsEntry.c_str());
		fprintf(pFile, "        },\n");
		fprintf(pFile, "        {\n");
		fprintf(pFile, "            \"filename\":    \"%s\",\n", outputFile.c_str());
		fprintf(pFile, "            \"gdaldriver\":  \"GTiff\",\n");				// writing geotiff file
		fprintf(pFile, "            \"output_type\": \"all\",\n");
		fprintf(pFile, "            \"resolution\":  \"%s\",\n", resolution.c_str());
		fprintf(pFile, "            \"type\":        \"writers.gdal\"\n");			// writer
		fprintf(pFile, "        }\n");
		fprintf(pFile, "    ]\n");
		fprintf(pFile, "}\n");

		fclose(pFile);
	}
	else
	{
		Logger::Write(__FUNCTION__, "Unable to create pipeline: %s", fileName.c_str());
	}

	// pipeline notes:
	// 
	// output_type
	//
	//		A comma separated list of statistics for which to produce raster layers. The supported values are “min”, “max”, “mean”, “idw”, 
	//		“count”, “stdev” and “all”. The option may be specified more than once. [Default: “all”]
	// 
	// limits
	//
	//		A comma-separated list of Ranges. If more than one range is specified for a dimension, the criteria are treated as being logically ORed together.
	//		Ranges for different dimensions are treated as being logically ANDed.
	//
	//		Example:	Classification[1:2], Red[1:50], Blue[25:75], Red[75:255], Classification[6:7]
	//					This specification will select points that have the classification of 1, 2, 6 or 7 and have a blue value or 25-75 and have a 
	//					red value of 1-50 or 75-255. In this case, all values are inclusive.

	// lidar point types (from 1.4 spec):
	//		0 - unclassified
	//		1 - unclassified
	//		2 - ground
	//		3 - low vegetation
	//		4 - med vegetation
	//		5 - high vegetation
	//		6 - building
	//		7 - low point (noise)
	//		8 - reserved
	//		9 - water
	//		10 - rail
	//		11 - road 
	//		12 - reserved
	//		13 - wire guard (shield)
	//		14 - wire conductor (phase)
	//		15 transmission tower
	//		16 - wire structure connector
	//		17 - bridge deck
	//		18 - high noise

	return fileName;
}

int LidarModel::SetSourceFile(const char* fileName)
{
	// Use existing las file as source.
	//

	ms_SourceFile = fileName;

	if (QFile::exists(fileName) == false)
		return 1;

	return 0;
}

int LidarModel::SetOutputFolder(const char* pathName)
{
	ms_OutputFolder = pathName;
	return 0;
}

void LidarModel::SetResolution(double res)
{
	// Set resolution for output grids.
	//

	assert(res > 0.0);

	mf_Resolution = res;
}

void LidarModel::CreateOutputFolder()
{
	// Create output folder if it doesn't 
	// exist.
	//
	
	QDir dir(ms_OutputFolder.c_str());
	if (dir.exists() == false)
	{
		bool status = dir.mkpath(ms_OutputFolder.c_str());
		if (status == false)
			Logger::Write(__FUNCTION__, "mkpath(%s) failed.", ms_OutputFolder.c_str());
	}

}