/******************************************************************************
 * Copyright (c) 2018, Connor Manning (connor@hobu.co)
 ****************************************************************************/

#include "LogicGate.h"

namespace pdal
{
	std::unique_ptr<LogicGate> LogicGate::create(const LogicalOperator type)
	{
		if (type == LogicalOperator::lAnd) return makeUnique<LogicalAnd>();
		if (type == LogicalOperator::lNot) return makeUnique<LogicalNot>();
		if (type == LogicalOperator::lOr) return makeUnique<LogicalOr>();
		if (type == LogicalOperator::lNor) return makeUnique<LogicalNor>();
		throw pdal_error("Invalid logic gate type");
	}
}
