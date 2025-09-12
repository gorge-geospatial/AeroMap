/******************************************************************************
 * Copyright (c) 2020, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "ChamferKernel.h"

#include <memory>

#include <pdal/PDALUtils.h>
#include <pdal/PointView.h>
#include <pdal/pdal_config.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{ 
		"kernels.chamfer", 
		"Chamfer Kernel",
		"http://pdal.io/apps/chamfer.html"
	};

	CREATE_STATIC_KERNEL(ChamferKernel, s_info)

	std::string ChamferKernel::getName() const
	{
		return s_info.name;
	}

	void ChamferKernel::addSwitches(ProgramArgs& args)
	{
		Arg& source = args.add("source", "Source filename", m_sourceFile);
		source.setPositional();
		Arg& candidate = args.add("candidate", "Candidate filename", m_candidateFile);
		candidate.setPositional();
	}

	PointViewPtr ChamferKernel::loadSet(const std::string& filename, PointTableRef table)
	{
		Stage& reader = makeReader(filename, "");
		reader.prepare(table);
		PointViewSet viewSet = reader.execute(table);
		assert(viewSet.size() == 1);
		return *viewSet.begin();
	}

	int ChamferKernel::execute()
	{
		ColumnPointTable srcTable;
		PointViewPtr srcView = loadSet(m_sourceFile, srcTable);

		ColumnPointTable candTable;
		PointViewPtr candView = loadSet(m_candidateFile, candTable);

		double chamfer = Utils::computeChamfer(srcView, candView);

		MetadataNode root;
		root.add("filenames", m_sourceFile);
		root.add("filenames", m_candidateFile);
		root.add("chamfer", chamfer);
		root.add("pdal_version", Config::fullVersionString());
		Utils::toJSON(root, std::cout);

		return 0;
	}
}
