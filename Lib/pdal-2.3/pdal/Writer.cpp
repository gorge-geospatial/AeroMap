/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
****************************************************************************/

#include <pdal/Writer.h>
#include <pdal/Stage.h>

#include <pdal/util/ProgramArgs.h>
#include "../filters/private/expr/ConditionalExpression.h"

#pragma warning(disable: 4127)  // conditional expression is constant

namespace pdal
{
	struct Writer::Args
	{
		expr::ConditionalExpression m_where;
		Arg* m_whereArg;
		Stage::WhereMergeMode m_whereMerge;
		Arg* m_whereMergeArg;
	};

	Writer::Writer() : m_args(new Args)
	{}

	Writer::~Writer()
	{}

	const expr::ConditionalExpression* Writer::whereExpr() const
	{
		if (!m_args->m_where.valid())
			return nullptr;
		return &(m_args->m_where);
	}

	Stage::WhereMergeMode Writer::mergeMode() const
	{
		return m_args->m_whereMerge;
	}

	void Writer::l_initialize(PointTableRef table)
	{
		Stage::l_initialize(table);
	}

	// This is here so that we can make the function final and make sure it
	// it isn't used by any subclasses.
	void Writer::l_addArgs(ProgramArgs& args)
	{
		Stage::l_addArgs(args);
		m_args->m_whereArg = &args.add("where",
			"Expression describing points to be passed to this "
			"filter", m_args->m_where);
		m_args->m_whereMergeArg = &args.add("where_merge", "If 'where' option is set, describes "
			"how skipped points should be merged with kept points in standard mode.",
			m_args->m_whereMerge, WhereMergeMode::Auto);
	}

	void Writer::l_prepared(PointTableRef table)
	{
		Stage::l_prepared(table);
		auto status = m_args->m_where.prepare(table.layout());
		if (!status)
			throwError("Invalid 'where': " + status.what());
		if (m_args->m_whereMergeArg->set() && !m_args->m_whereArg->set())
			throwError("Can't set 'where_merge' options without also setting 'where' option.");
	}

	std::string::size_type Writer::handleFilenameTemplate(const std::string& filename)
	{
		std::string::size_type suffixPos = filename.find_last_of('.');
		std::string::size_type hashPos = filename.find_first_of('#');
		if (hashPos == std::string::npos)
			return hashPos;

		if (hashPos > suffixPos)
		{
			std::ostringstream oss;
			oss << "Filename template placeholder ('#') is not "
				"allowed in filename suffix.";
			throw pdal_error(oss.str());
		}
		if (filename.find_first_of('#', hashPos + 1) != std::string::npos)
		{
			std::ostringstream oss;
			oss << "Filename specification can only contain "
				"a single '#' template placeholder.";
			throw pdal_error(oss.str());
		}
		return hashPos;
	}
}
