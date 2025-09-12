#ifndef CONDITIONALEXPRESSION_H
#define CONDITIONALEXPRESSION_H

#include "Expression.h"
#include "Lexer.h"
#include "ConditionalParser.h"

namespace pdal
{
	namespace expr
	{
		class ConditionalExpression : public Expression
		{
		public:
			Utils::StatusWithReason prepare(PointLayoutPtr layout);
			bool eval(PointRef& p) const;
		};
	}

	namespace Utils
	{
		template<>
		inline StatusWithReason fromString(const std::string& from,
			pdal::expr::ConditionalExpression& expr)
		{
			expr::Lexer lexer(from);
			expr::ConditionalParser parser(lexer);
			bool ok = parser.expression(expr) && parser.checkEnd();
			return { ok ? 0 : -1, expr.error() };
		}
	}
}

#endif // #ifndef CONDITIONALEXPRESSION_H
