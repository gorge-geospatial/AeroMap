/******************************************************************************
* Copyright (c) 2016, Bradley J Chambers (brad.chambers@gmail.com)
****************************************************************************/

#include "HausdorffKernel.h"

#include <memory>

#include <pdal/PDALUtils.h>
#include <pdal/PointView.h>
#include <pdal/pdal_config.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"kernels.hausdorff",
		"Hausdorff Kernel",
		"http://pdal.io/apps/hausdorff.html"
	};

	CREATE_STATIC_KERNEL(HausdorffKernel, s_info)

	std::string HausdorffKernel::getName() const
	{
		return s_info.name;
	}

	void HausdorffKernel::addSwitches(ProgramArgs& args)
	{
		Arg& source = args.add("source", "Source filename", m_sourceFile);
		source.setPositional();
		Arg& candidate = args.add("candidate", "Candidate filename", m_candidateFile);
		candidate.setPositional();
	}

	PointViewPtr HausdorffKernel::loadSet(const std::string& filename,
		PointTableRef table)
	{
		Stage& reader = makeReader(filename, "");
		reader.prepare(table);
		PointViewSet viewSet = reader.execute(table);
		assert(viewSet.size() == 1);
		return *viewSet.begin();
	}

	int HausdorffKernel::execute()
	{
		ColumnPointTable srcTable;
		PointViewPtr srcView = loadSet(m_sourceFile, srcTable);

		ColumnPointTable candTable;
		PointViewPtr candView = loadSet(m_candidateFile, candTable);

		std::pair<double, double> result = Utils::computeHausdorffPair(srcView, candView);

		MetadataNode root;
		root.add("filenames", m_sourceFile);
		root.add("filenames", m_candidateFile);
		root.add("hausdorff", result.first);
		root.add("modified_hausdorff", result.second);
		root.add("pdal_version", Config::fullVersionString());
		Utils::toJSON(root, std::cout);

		return 0;
	}
}
