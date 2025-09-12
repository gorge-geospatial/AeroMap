/******************************************************************************
 * Copyright (c) 2018, Connor Manning (connor@hobu.co)
 ****************************************************************************/

#include "Expression.h"

namespace pdal
{
	void Expression::build(LogicGate& gate, const NL::json& json)
	{
		if (json.is_array())
		{
			for (auto& val : json)
				build(gate, val);
			return;
		}

		if (!json.is_object())
		{
			throw pdal_error("Unexpected expression: " + json.get<std::string>());
		}

		LogicGate* active(&gate);

		std::unique_ptr<LogicGate> outer;

		if (json.size() > 1)
		{
			outer = LogicGate::create(LogicalOperator::lAnd);
			active = outer.get();
		}

		for (auto& it : json.items())
		{
			const NL::json& val(it.value());
			const std::string& key(it.key());

			if (isLogicalOperator(key))
			{
				auto inner(LogicGate::create(key));
				if (inner->type() != LogicalOperator::lNot && !val.is_array())
				{
					throw pdal_error("Logical operator expressions must be arrays");
				}

				build(*inner, val);
				active->push(std::move(inner));
			}
			else if (!val.is_object() || val.size() == 1)
			{
				// A comparison object.
				active->push(Comparison::create(m_layout, key, val));
			}
			else
			{
				// key is the name of a dimension, val is an object of
				// multiple comparison key/val pairs, for example:
				//
				// key: "Red"
				// val: { "$gt": 100, "$lt": 200 }
				//
				// There cannot be any further nested logical operators
				// within val, since we've already selected a dimension.
				for (auto inner : val.items())
				{
					NL::json nest;
					nest[inner.key()] = inner.value();
					active->push(Comparison::create(m_layout, key, nest));
				}
			}
		}

		if (outer) gate.push(std::move(outer));
	}
}

