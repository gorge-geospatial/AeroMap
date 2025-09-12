/******************************************************************************
* Copyright (c) 2014, Bradley J Chambers (brad.chambers@gmail.com)
****************************************************************************/

#include "SortKernel.h"

#include <pdal/Stage.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"kernels.sort",
		"Sort Kernel",
		"http://pdal.io/apps/sort.html"
	};

	CREATE_STATIC_KERNEL(SortKernel, s_info)

	std::string SortKernel::getName() const
	{
		return s_info.name;
	}

	SortKernel::SortKernel() : m_bCompress(false), m_bForwardMetadata(false)
	{}

	void SortKernel::addSwitches(ProgramArgs& args)
	{
		args.add("input,i", "Input filename", m_inputFile).setPositional();
		args.add("output,o", "Output filename", m_outputFile).setPositional();
		args.add("compress,z",
			"Compress output data (if supported by output format)", m_bCompress);
		args.add("metadata,m",
			"Forward metadata (VLRs, header entries, etc) from previous stages",
			m_bForwardMetadata);
	}

	int SortKernel::execute()
	{
		Stage& readerStage = makeReader(m_inputFile, m_driverOverride);
		Stage& sortStage = makeFilter("filters.mortonorder", readerStage);

		Options writerOptions;
		if (m_bCompress)
			writerOptions.add("compression", true);
		if (m_bForwardMetadata)
			writerOptions.add("forward_metadata", true);
		Stage& writer = makeWriter(m_outputFile, sortStage, "", writerOptions);

		ColumnPointTable table;
		writer.prepare(table);
		writer.execute(table);

		return 0;
	}
}
