/******************************************************************************
 * Copyright (c) 2014, Hobu Inc. (hobu@hobu.co)
 * Copyright (c) 2015, Bradley J Chambers, brad.chambers@gmail.com
 ****************************************************************************/

#include "SortFilter.h"

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.sort",
		"Sort data based on a given dimension.",
		"http://pdal.io/stages/filters.sort.html"
	};

	CREATE_STATIC_STAGE(SortFilter, s_info)

	std::string SortFilter::getName() const { return s_info.name; }

	void SortFilter::addArgs(ProgramArgs& args)
	{
		args.add("dimension", "Dimension on which to sort", m_dimName).setPositional();
		args.add("order", "Sort order ASC(ending) or DESC(ending)", m_order, SortOrder::ASC);
	}

	void SortFilter::prepared(PointTableRef table)
	{
		m_dim = table.layout()->findDim(m_dimName);
		if (m_dim == Dimension::Id::Unknown)
			throwError("Dimension '" + m_dimName + "' not found.");
	}

	void SortFilter::filter(PointView& view)
	{
		auto cmp = [this](const PointRef& p1, const PointRef& p2)
		{
			bool result = p1.compare(m_dim, p2);
			return (m_order == SortOrder::ASC) ? result : !result;
		};

		std::stable_sort(view.begin(), view.end(), cmp);
	}

	std::istream& operator >> (std::istream& in, SortOrder& order)
	{
		std::string s;

		in >> s;
		s = Utils::toupper(s);
		if (s == "ASC")
			order = SortOrder::ASC;
		else if (s == "DESC")
			order = SortOrder::DESC;
		else
			in.setstate(std::ios::failbit);
		return in;
	}

	std::ostream& operator<<(std::ostream& out, const SortOrder& order)
	{
		switch (order)
		{
		case SortOrder::ASC:
			out << "ASC";
		case SortOrder::DESC:
			out << "DESC";
		}
		return out;
	}
}
