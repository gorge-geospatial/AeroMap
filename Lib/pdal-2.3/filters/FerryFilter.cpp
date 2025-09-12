/******************************************************************************
* Copyright (c) 2014, Howard Butler, howard@hobu.co
****************************************************************************/

#include "FerryFilter.h"

#include <pdal/util/Algorithm.h>
#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.ferry",
		"Copy data from one dimension to another.",
		"http://pdal.io/stages/filters.ferry.html"
	};

	CREATE_STATIC_STAGE(FerryFilter, s_info)

	std::string FerryFilter::getName() const { return s_info.name; }

	void FerryFilter::addArgs(ProgramArgs& args)
	{
		args.add("dimensions", "List of dimensions to ferry", m_dimSpec).setPositional();
	}

	void FerryFilter::initialize()
	{
		std::vector<std::string> toNames;
		for (auto& dim : m_dimSpec)
		{
			StringList s = Utils::split(dim, '=');
			if (s.size() != 2)
				throwError("Invalid dimension specified '" + dim + "'.  Need "
					"<from dimension>=><to dimension>.  See documentation for "
					"details.");
			// Allow new '=>' syntax
			if (s[1][0] == '>')
				s[1].erase(s[1].begin());

			Utils::trim(s[0]);
			Utils::trim(s[1]);
			if (s[0] == s[1])
				throwError("Can't ferry dimension '" + s[0] + "' to itself.");
			if (Utils::contains(toNames, s[1]))
				throwError("Can't ferry two source dimensions to the same "
					"destination dimension.");
			toNames.push_back(s[1]);
			m_dims.emplace_back(s[0], s[1]);
		}
	}

	void FerryFilter::addDimensions(PointLayoutPtr layout)
	{
		for (auto& info : m_dims)
		{
			const Dimension::Id fromId = layout->findDim(info.m_fromName);
			// Dimensions being created with the "=>Dim" syntax won't
			// be in the layout, so we have to assign a default type.
			Dimension::Type fromType = layout->dimType(fromId);
			if (fromType == Dimension::Type::None)
				fromType = Dimension::Type::Double;

			info.m_toId = layout->registerOrAssignDim(info.m_toName, fromType);
		}
	}

	void FerryFilter::prepared(PointTableRef table)
	{
		for (auto& info : m_dims)
		{
			info.m_fromId = table.layout()->findDim(info.m_fromName);
			if (info.m_fromId == Dimension::Id::Unknown && info.m_fromName.size())
				throwError("Can't ferry dimension '" + info.m_fromName + "'. "
					"Dimension doesn't exist.");
		}
	}

	bool FerryFilter::processOne(PointRef& point)
	{
		for (const auto& info : m_dims)
		{
			if (info.m_fromId != Dimension::Id::Unknown)
			{
				double v = point.getFieldAs<double>(info.m_fromId);
				point.setField(info.m_toId, v);
			}
		}
		return true;
	}

	void FerryFilter::filter(PointView& view)
	{
		PointRef point(view, 0);
		for (PointId id = 0; id < view.size(); ++id)
		{
			point.setPointId(id);
			processOne(point);
		}
	}
}
