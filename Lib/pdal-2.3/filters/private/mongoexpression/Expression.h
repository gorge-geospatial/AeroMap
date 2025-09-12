/******************************************************************************
 * Copyright (c) 2018, Connor Manning (connor@hobu.co)
 ****************************************************************************/

#pragma once

#include "Comparison.h"
#include "LogicGate.h"

namespace pdal
{
	class Expression
	{
	public:
		Expression(const PointLayout& layout, const NL::json& json)
			: m_layout(layout)
		{
			build(m_root, json);
		}

		bool check(const pdal::PointRef& pr) const
		{
			return m_root(pr);
		}

		std::string toString() const
		{
			return m_root.toString("");
		}

	private:
		void build(LogicGate& gate, const NL::json& json);

		const PointLayout& m_layout;
		LogicalAnd m_root;
	};

	inline std::ostream& operator<<(std::ostream& os, const Expression& expression)
	{
		os << expression.toString() << std::endl;
		return os;
	}
}
