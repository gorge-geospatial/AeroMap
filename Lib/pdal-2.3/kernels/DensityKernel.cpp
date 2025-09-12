/******************************************************************************
* Copyright (c) 2015, Howard Butler (howard@hobu.co)
****************************************************************************/

#include "DensityKernel.h"

#include "../filters/HexBinFilter.h"
#include "private/density/OGR.h"

#include <pdal/util/FileUtils.h>

namespace pdal
{
	static PluginInfo const s_info
	{
		"kernels.density",
		"Density Kernel",
		"http://pdal.io/apps/density.html"
	};

	CREATE_STATIC_KERNEL(DensityKernel, s_info)

	std::string DensityKernel::getName() const { return s_info.name; }

	void DensityKernel::addSwitches(ProgramArgs& args)
	{
		args.add("input,i", "input point cloud file name", m_inputFile).setPositional();
		args.add("output,o", "output vector data source", m_outputFile).setPositional();
		args.add("ogrdriver,f", "OGR driver name to use ", m_driverName, "ESRI Shapefile");
		args.add("lyr_name", "OGR layer name to use", m_layerName, "");
		args.add("sample_size", "Sample size for auto-edge length calculation", m_sampleSize, 5000U);
		args.add("threshold", "Required cell density", m_density, 15);
		args.add("edge_length", "Length of hex edge", m_edgeLength);
		args.add("hole_cull_area_tolerance", "Tolerance area to apply to holes before cull", m_cullArea);
		args.add("smooth", "Smooth boundary output", m_doSmooth, true);
	}

	void DensityKernel::outputDensity(pdal::SpatialReference const& reference)
	{
		HexBin* hexbin = dynamic_cast<HexBin*>(m_hexbinStage);
		if (!hexbin)
			throw pdal::pdal_error("unable to fetch filters.hexbin stage!");

		hexer::HexGrid* grid = hexbin->grid();

		OGR writer(m_outputFile, reference.getWKT(), m_driverName, m_layerName);
		writer.writeDensity(grid);
	}

	int DensityKernel::execute()
	{
		if (m_inputFile == "STDIN" ||
			(FileUtils::extension(m_inputFile) == ".xml" ||
				FileUtils::extension(m_inputFile) == ".json"))
		{
			m_manager.readPipeline(m_inputFile);
		}
		else
		{
			m_manager.makeReader(m_inputFile, "");
		}
		Options options;
		options.add("sample_size", m_sampleSize);
		options.add("threshold", m_density);
		options.add("edge_length", m_edgeLength);
		options.add("hole_cull_area_tolerance", m_cullArea);
		options.add("smooth", m_doSmooth);
		m_hexbinStage = &(m_manager.makeFilter("filters.hexbin",
			*m_manager.getStage(), options));
		m_manager.execute();
		outputDensity(m_manager.pointTable().anySpatialReference());
		return 0;
	}
}
