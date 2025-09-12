#ifndef READER_H
#define READER_H

#include <pdal/Stage.h>
#include <pdal/Options.h>

#include <functional>

namespace pdal
{
	class Reader;

	namespace expr
	{
		class ConditionalExpression;
	}

	class Reader : public virtual Stage
	{
	public:
		typedef std::function<void(PointView&, PointId)> PointReadFunc;

		Reader()
		{}

		void setReadCb(PointReadFunc cb)
		{
			m_cb = cb;
		}
		point_count_t count() const
		{
			return m_count;
		}

		using Stage::setSpatialReference;

	protected:
		std::string m_filename;
		point_count_t m_count;
		PointReadFunc m_cb;
		Arg* m_filenameArg;
		Arg* m_countArg;

		SpatialReference m_overrideSrs;
		SpatialReference m_defaultSrs;

		virtual void setSpatialReference(MetadataNode& m, const SpatialReference& srs);

	private:
		virtual PointViewSet run(PointViewPtr view)
		{
			PointViewSet viewSet;

			view->clearTemps();
			read(view, m_count);
			viewSet.insert(view);
			return viewSet;
		}
		virtual void l_initialize(PointTableRef table) final;
		virtual void l_addArgs(ProgramArgs& args) final;
		virtual void l_prepared(PointTableRef table) final;

		virtual const expr::ConditionalExpression* whereExpr() const;
		virtual WhereMergeMode mergeMode() const;
		virtual point_count_t read(PointViewPtr /*view*/, point_count_t /*num*/)
		{
			return 0;
		}
	};
}

#endif // #ifndef READER_H

