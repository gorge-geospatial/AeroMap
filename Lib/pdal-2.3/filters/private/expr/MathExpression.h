#pragma once

#include "Expression.h"

namespace pdal
{
	namespace expr
	{
		class MathExpression : public Expression
		{
		public:
			Utils::StatusWithReason prepare(PointLayoutPtr layout);
			double eval(PointRef& p) const;
		};
	}
}
