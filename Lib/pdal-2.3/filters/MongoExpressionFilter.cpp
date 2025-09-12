/******************************************************************************
 * Copyright (c) 2018, Connor Manning (connor@hobu.co)
 ****************************************************************************/

#include <nlohmann/json.h>

#include "MongoExpressionFilter.h"

#include "private/mongoexpression/Expression.h"

namespace pdal
{
	static const StaticPluginInfo s_info
	{
		"filters.mongo",
		"Pass only points that pass a logic filter.",
		"http://pdal.io/stages/filters.mongo.html"
	};

	CREATE_STATIC_STAGE(MongoExpressionFilter, s_info);

	std::string MongoExpressionFilter::getName() const
	{
		return s_info.name;
	}

	MongoExpressionFilter::MongoExpressionFilter()
	{}

	MongoExpressionFilter::~MongoExpressionFilter()
	{}

	void MongoExpressionFilter::addArgs(ProgramArgs& args)
	{
		args.add("expression", "Logical query expression", m_json).setPositional();
	}

	void MongoExpressionFilter::prepared(PointTableRef table)
	{
		log()->get(LogLevel::Debug) << "Building expression from: " << m_json <<
			std::endl;

		m_expression = makeUnique<Expression>(*table.layout(), m_json);

		log()->get(LogLevel::Debug) << "Built expression: " << *m_expression <<
			std::endl;
	}

	PointViewSet MongoExpressionFilter::run(PointViewPtr inView)
	{
		PointViewSet views;
		PointViewPtr view(inView->makeNew());

		for (PointId i(0); i < inView->size(); ++i)
		{
			PointRef pr(inView->point(i));
			if (processOne(pr))
			{
				view->appendPoint(*inView, i);
			}
		}

		views.insert(view);
		return views;
	}

	bool MongoExpressionFilter::processOne(PointRef& pr)
	{
		return m_expression->check(pr);
	}
}
