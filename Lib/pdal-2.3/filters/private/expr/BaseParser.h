#ifndef BASEPARSER_H
#define BASEPARSER_H

#include "Lexer.h"
#include "Expression.h"

namespace pdal
{
	namespace expr
	{
		class BaseParser
		{
		public:
			BaseParser(Lexer& lexer) : m_lexer(lexer)
			{}

			bool checkEnd();
			std::string error() const
			{
				return m_error;
			}

		protected:
			bool match(TokenType type);
			Token peekToken();
			Token curToken() const;
			Token lastToken() const;
			void setError(const std::string& err);
			void clearError();
			Lexer& lexer()
			{
				return m_lexer;
			}

		private:
			Lexer& m_lexer;
			Token m_curTok;
			std::string m_error;
		};
	}
}

#endif // #ifndef BASEPARSER_H
