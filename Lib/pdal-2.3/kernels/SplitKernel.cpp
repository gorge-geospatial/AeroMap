/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
* Copyright (c) 2013, Howard Butler (hobu.inc@gmail.com)
****************************************************************************/

#include "SplitKernel.h"

#include <io/BufferReader.h>
#include <pdal/StageFactory.h>
#include <pdal/util/Utils.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"kernels.split",
		"Split Kernel",
		"http://pdal.io/apps/split.html"
	};

	CREATE_STATIC_KERNEL(SplitKernel, s_info)

	std::string SplitKernel::getName() const
	{
		return s_info.name;
	}

	void SplitKernel::addSwitches(ProgramArgs& args)
	{
		args.add("input,i", "Input filename", m_inputFile).setPositional();
		args.add("output,o", "Output filename", m_outputFile).setPositional();
		args.add("length", "Edge length for splitter cells", m_length, 0.0);
		args.add("capacity", "Point capacity of chipper cells", m_capacity);
		args.add("origin_x", "Origin in X axis for splitter cells", m_xOrigin, std::numeric_limits<double>::quiet_NaN());
		args.add("origin_y", "Origin in Y axis for splitter cells", m_yOrigin, std::numeric_limits<double>::quiet_NaN());
	}

	void SplitKernel::validateSwitches(ProgramArgs& args)
	{
		if (m_length && m_capacity)
			throw pdal_error("Can't specify both length and capacity.");
		if (!m_length && !m_capacity)
			m_capacity = 100000;
		if (m_outputFile.back() == Utils::dirSeparator)
			m_outputFile += m_inputFile;
	}

	namespace
	{
		std::string makeFilename(const std::string& s, int i)
		{
			std::string out = s;
			auto pos = out.find_last_of('.');
			if (pos == out.npos)
				pos = out.length();
			out.insert(pos, std::string("_") + std::to_string(i));
			return out;
		}
	}

	int SplitKernel::execute()
	{
		Stage& reader = makeReader(m_inputFile, m_driverOverride);

		Options filterOpts;
		std::string driver = (m_length ? "filters.splitter" : "filters.chipper");
		if (m_length)
		{
			filterOpts.add("length", m_length);
			filterOpts.add("origin_x", m_xOrigin);
			filterOpts.add("origin_y", m_yOrigin);
		}
		else
		{
			filterOpts.add("capacity", m_capacity);
		}
		Stage& f = makeFilter(driver, reader, filterOpts);

		ColumnPointTable table;
		f.prepare(table);
		PointViewSet pvSet = f.execute(table);

		int filenum = 1;
		for (auto& pvp : pvSet)
		{
			BufferReader reader;
			reader.addView(pvp);

			std::string filename = makeFilename(m_outputFile, filenum++);
			Stage& writer = makeWriter(filename, reader, "");

			writer.prepare(table);
			writer.execute(table);
		}
		return 0;
	}
}
