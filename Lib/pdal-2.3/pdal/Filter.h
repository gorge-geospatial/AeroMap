#ifndef FILTER_H
#define FILTER_H

#include <memory>

#include <pdal/Stage.h>

namespace pdal
{
	class Filter;

	class FilterWrapper;

	class Filter : public virtual Stage
	{
		friend class FilterWrapper;

		struct Args;

	public:
		Filter();
		~Filter();

		Filter& operator=(const Filter&) = delete;
		Filter(const Filter&) = delete;

	private:
		virtual void l_initialize(PointTableRef table) final;
		virtual void l_addArgs(ProgramArgs& args) final;
		virtual void l_prepared(PointTableRef table) final;
		virtual const expr::ConditionalExpression* whereExpr() const;
		virtual WhereMergeMode mergeMode() const;
		virtual PointViewSet run(PointViewPtr view);
		virtual void filter(PointView& /*view*/)
		{
		}

		friend std::istream& operator>>(std::istream& in, Filter::WhereMergeMode& mode);
		friend std::ostream& operator<<(std::ostream& out, const Filter::WhereMergeMode& mode);

		std::unique_ptr<Args> m_args;
	};
}

#endif // #ifndef FILTER_H
