/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
****************************************************************************/

#include <pdal/Reader.h>
#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	const expr::ConditionalExpression* Reader::whereExpr() const
	{
		return nullptr;
	}

	Stage::WhereMergeMode Reader::mergeMode() const
	{
		return WhereMergeMode::True;
	}

	void Reader::l_addArgs(ProgramArgs& args)
	{
		Stage::l_addArgs(args);
		m_filenameArg = &args.add("filename", "Name of file to read", m_filename);
		m_countArg = &args.add("count", "Maximum number of points read", m_count,
			(std::numeric_limits<point_count_t>::max)());

		args.add("override_srs", "Spatial reference to apply to data",
			m_overrideSrs);
		args.addSynonym("override_srs", "spatialreference");

		args.add("default_srs",
			"Spatial reference to apply to data if one cannot be inferred",
			m_defaultSrs);
	}

	void Reader::setSpatialReference(MetadataNode& m, const SpatialReference& srs)
	{
		if (srs.empty() && !m_defaultSrs.empty())
		{
			// If an attempt comes in to clear the SRS but we have a default,
			// revert to the default rather than clearing.
			Stage::setSpatialReference(m, m_defaultSrs);
			return;
		}

		if (getSpatialReference().empty() || m_overrideSrs.empty())
		{
			Stage::setSpatialReference(m, srs);
		}
		else
		{
			log()->get(LogLevel::Debug) <<
				"Ignoring setSpatialReference attempt: an override was set";
		}
	}

	void Reader::l_initialize(PointTableRef table)
	{
		Stage::l_initialize(table);

		if (m_overrideSrs.valid() && m_defaultSrs.valid())
			throwError("Cannot specify both 'override_srs' and 'default_srs'");

		if (m_overrideSrs.valid())
			setSpatialReference(m_overrideSrs);
		else if (m_defaultSrs.valid())
			setSpatialReference(m_defaultSrs);
	}

	void Reader::l_prepared(PointTableRef table)
	{
		Stage::l_prepared(table);
	}
}
