/******************************************************************************
 * Copyright (c) 2015, Hobu Inc. (info@hobu.co)
 ****************************************************************************/

#include "DividerFilter.h"

namespace pdal
{
	static PluginInfo const s_info
	{
		"filters.divider",
		"Divide points into approximately equal sized groups based on a simple scheme",
		"http://pdal.io/stages/filters.divider.html"
	};

	CREATE_STATIC_STAGE(DividerFilter, s_info)

	std::string DividerFilter::getName() const { return s_info.name; }

	std::istream& operator>>(std::istream& in, DividerFilter::Mode& mode)
	{
		std::string s;
		in >> s;

		s = Utils::tolower(s);
		if (s == "round_robin")
			mode = DividerFilter::Mode::RoundRobin;
		else if (s == "partition")
			mode = DividerFilter::Mode::Partition;
		else
			throw pdal_error("filters.divider: Invalid 'mode' option '" + s + "'. "
				"Valid options are 'partition' and 'round_robin'");
		return in;
	}

	std::ostream& operator<<(std::ostream& out, const DividerFilter::Mode& mode)
	{
		switch (mode)
		{
		case DividerFilter::Mode::RoundRobin:
			out << "round_robin";
		case DividerFilter::Mode::Partition:
			out << "partition";
		}
		return out;
	}

	void DividerFilter::addArgs(ProgramArgs& args)
	{
		args.add("mode", "A mode of 'partition' will write sequential points "
			"to an output view until the view meets its predetermined size. "
			"'round_robin' mode will iterate through the output views as it "
			"writes sequential points.", m_mode, DividerFilter::Mode::Partition);
		m_cntArg = &args.add("count", "Number of output views", m_size);
		m_capArg = &args.add("capacity", "Maximum number of points in each "
			"output view", m_size);
	}

	void DividerFilter::initialize()
	{
		if (m_cntArg->set() && m_capArg->set())
			throwError("Can't specify both option 'count' and option 'capacity.");
		if (!m_cntArg->set() && !m_capArg->set())
			throwError("Must specify either option 'count' or option 'capacity'.");
		if (m_cntArg->set())
		{
			m_sizeMode = SizeMode::Count;
			if (m_size < 2 || m_size > 1000)
				throwError("Option 'count' must be in the range [2, 1000].");
		}
		if (m_capArg->set())
			m_sizeMode = SizeMode::Capacity;
	}

	PointViewSet DividerFilter::run(PointViewPtr inView)
	{
		PointViewSet result;

		if (inView->empty())
		{
			result.insert(inView);
			return result;
		}

		if (m_sizeMode == SizeMode::Capacity)
			m_size = ((inView->size() - 1) / m_size) + 1;


		std::vector<PointViewPtr> views;
		for (point_count_t i = 0; i < m_size; ++i)
		{
			PointViewPtr v(inView->makeNew());
			views.push_back(v);
			result.insert(v);
		}

		if (m_mode == Mode::Partition)
		{
			point_count_t limit = ((inView->size() - 1) / m_size) + 1;
			unsigned viewNum = 0;
			for (PointId i = 0; i < inView->size();)
			{
				views[viewNum]->appendPoint(*inView, i++);
				if (i % limit == 0)
					viewNum++;
			}
		}
		else // RoundRobin
		{
			unsigned viewNum = 0;
			for (PointId i = 0; i < inView->size(); ++i)
			{
				views[viewNum]->appendPoint(*inView, i);
				viewNum++;
				if (viewNum == m_size)
					viewNum = 0;
			}
		}
		return result;
	}
}
