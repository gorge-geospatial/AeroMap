#ifndef CONDITIONALPARSER_H
#define CONDITIONALPARSER_H

#include "BaseParser.h"

namespace pdal
{
	namespace expr
	{
		class ConditionalParser : public BaseParser
		{
		public:
			ConditionalParser(Lexer& lexer) : BaseParser(lexer)
			{}
			bool expression(Expression& expr);

		protected:
			bool notexpr(Expression& expr);
			bool orexpr(Expression& expr);
			bool andexpr(Expression& expr);
			bool compareexpr(Expression& expr);
			bool addexpr(Expression& expr);
			bool multexpr(Expression& expr);
			bool uminus(Expression& expr);
			bool primary(Expression& expr);
			bool parexpr(Expression& expr);
		};
	}
}

#endif // #ifndef CONDITIONALPARSER_H
