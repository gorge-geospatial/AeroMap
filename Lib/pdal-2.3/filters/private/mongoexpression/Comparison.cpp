/******************************************************************************
 * Copyright (c) 2018, Connor Manning (connor@hobu.co)
 ****************************************************************************/

#include "Comparison.h"

namespace pdal
{
	std::unique_ptr<Comparison> Comparison::create(const PointLayout& layout,
		const std::string dimName, const NL::json& json)
	{
		if (!json.is_object())
		{
			// If it's a value specified without the $eq operator, convert it.
			NL::json converted;
			converted["$eq"] = json;
			return create(layout, dimName, converted);
		}

		if (json.size() != 1)
		{
			throw pdal_error("Invalid comparison object: " +
				json.get<std::string>());
		}

		auto it = json.begin();

		//const auto key(json.getMemberNames().at(0));
		const ComparisonType co(toComparisonType(it.key()));
		const NL::json& val(it.value());

		const Dimension::Id dimId(layout.findDim(dimName));
		if (dimId == pdal::Dimension::Id::Unknown)
		{
			throw pdal_error("Unknown dimension: " + dimName);
		}

		if (isSingle(co))
		{
			Operand op(layout, val);
			switch (co)
			{
			case ComparisonType::eq:
				return makeUnique<ComparisonEqual>(dimId, op);
			case ComparisonType::gt:
				return makeUnique<ComparisonGreater>(dimId, op);
			case ComparisonType::gte:
				return makeUnique<ComparisonGreaterEqual>(dimId, op);
			case ComparisonType::lt:
				return makeUnique<ComparisonLess>(dimId, op);
			case ComparisonType::lte:
				return makeUnique<ComparisonLessEqual>(dimId, op);
			case ComparisonType::ne:
				return makeUnique<ComparisonNotEqual>(dimId, op);
			default:
				throw pdal_error("Invalid single comparison operator");
			}
		}
		else
		{
			if (!val.is_array())
				throw pdal_error("Invalid comparisons: " + val.dump());

			Operands ops;
			for (auto& op : val)
			{
				ops.emplace_back(layout, op);
			}
			switch (co)
			{
			case ComparisonType::in:
				return makeUnique<ComparisonAny>(dimId, ops);
			case ComparisonType::nin:
				return makeUnique<ComparisonNone>(dimId, ops);
			default:
				throw pdal_error("Invalid multi comparison operator");
			}
		}
	}
}
