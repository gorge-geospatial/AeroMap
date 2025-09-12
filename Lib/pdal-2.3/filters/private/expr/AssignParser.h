#ifndef ASSIGNPARSER_H
#define ASSIGNPARSER_H

#include "BaseParser.h"

namespace pdal
{
	namespace expr
	{
		class AssignStatement;

		class AssignParser : public BaseParser
		{
		public:
			AssignParser(Lexer& lexer) : BaseParser(lexer)
			{}

			bool statement(AssignStatement& expr);

		protected:
			bool assignment(AssignStatement& expr);
			bool where(AssignStatement& expr);
		};
	}
}

#endif // #ifndef ASSIGNPARSER_H
