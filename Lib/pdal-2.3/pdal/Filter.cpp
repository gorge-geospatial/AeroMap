/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
* Copyright (c) 2020, Hobu Inc.
****************************************************************************/

#include "Filter.h"
#include "../filters/private/expr/ConditionalExpression.h"

namespace pdal
{
	struct Filter::Args
	{
		expr::ConditionalExpression m_where;
		Arg* m_whereArg;
		Filter::WhereMergeMode m_whereMerge;
		Arg* m_whereMergeArg;
	};

	Filter::Filter() : m_args(new Args)
	{}

	Filter::~Filter()
	{}

	PointViewSet Filter::run(PointViewPtr view)
	{
		PointViewSet viewSet;
		filter(*view);
		viewSet.insert(view);
		return viewSet;
	}

	void Filter::l_initialize(PointTableRef table)
	{
		Stage::l_initialize(table);
	}

	void Filter::l_addArgs(ProgramArgs& args)
	{
		Stage::l_addArgs(args);
		m_args->m_whereArg = &args.add("where",
			"Expression describing points to be passed to this "
			"filter", m_args->m_where);
		m_args->m_whereMergeArg = &args.add("where_merge", "If 'where' option is set, describes "
			"how skipped points should be merged with kept points in standard mode.",
			m_args->m_whereMerge, WhereMergeMode::Auto);
	}

	void Filter::l_prepared(PointTableRef table)
	{
		Stage::l_prepared(table);
		auto status = m_args->m_where.prepare(table.layout());
		if (!status)
			throwError("Invalid 'where': " + status.what());
		if (m_args->m_whereMergeArg->set() && !m_args->m_whereArg->set())
			throwError("Can't set 'where_merge' options without also setting 'where' option.");
	}

	const expr::ConditionalExpression* Filter::whereExpr() const
	{
		if (!m_args->m_where.valid())
			return nullptr;
		return &(m_args->m_where);
	}

	Stage::WhereMergeMode Filter::mergeMode() const
	{
		return m_args->m_whereMerge;
	}
}
