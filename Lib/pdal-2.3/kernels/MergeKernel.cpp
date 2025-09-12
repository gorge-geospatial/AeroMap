/******************************************************************************
* Copyright (c) 2015, Hobu Inc. (info@hobu.co)
****************************************************************************/

#include "MergeKernel.h"

#include <filters/MergeFilter.h>
#include <pdal/StageFactory.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"kernels.merge",
		"Merge Kernel",
		"http://pdal.io/apps/merge.html"
	};

	CREATE_STATIC_KERNEL(MergeKernel, s_info)

	std::string MergeKernel::getName() const
	{
		return s_info.name;
	}

	void MergeKernel::addSwitches(ProgramArgs& args)
	{
		args.add("files,f", "input/output files", m_files).setPositional();
	}

	void MergeKernel::validateSwitches(ProgramArgs& args)
	{
		if (m_files.size() < 2)
			throw pdal_error("Must specify an input and output file.");
		m_outputFile = m_files.back();
		m_files.resize(m_files.size() - 1);
	}

	int MergeKernel::execute()
	{
		MergeFilter filter;

		for (size_t i = 0; i < m_files.size(); ++i)
		{
			Stage& reader = makeReader(m_files[i], m_driverOverride);
			filter.setInput(reader);
		}

		Stage& writer = makeWriter(m_outputFile, filter, "");

		ColumnPointTable table;
		writer.prepare(table);
		writer.execute(table);
		return 0;
	}
}
