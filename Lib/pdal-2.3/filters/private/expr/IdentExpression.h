#pragma once

#include "Expression.h"

namespace pdal
{
	namespace expr
	{
		class IdentExpression : public Expression
		{
		public:
			Utils::StatusWithReason prepare(PointLayoutPtr layout);
			Dimension::Id eval() const;
		};
	}
}
