#ifndef MONGOEXPRESSIONFILTER_H
#define MONGOEXPRESSIONFILTER_H

#include <pdal/Filter.h>
#include <pdal/JsonFwd.h>
#include <pdal/Streamable.h>

namespace pdal
{
	class Expression;

	class MongoExpressionFilter : public Filter, public Streamable
	{
	public:
		MongoExpressionFilter();
		~MongoExpressionFilter();

		std::string getName() const override;
		virtual bool processOne(PointRef& point) override;

	private:
		virtual void addArgs(ProgramArgs& args) override;
		virtual void prepared(PointTableRef table) override;
		virtual PointViewSet run(PointViewPtr view) override;

		NL::json m_json;
		std::unique_ptr<Expression> m_expression;
	};
}

#endif // #ifndef MONGOEXPRESSIONFILTER_H
