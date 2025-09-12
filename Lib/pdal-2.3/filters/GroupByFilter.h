#ifndef GROUPBYFILTER_H
#define GROUPBYFILTER_H

#include <pdal/Filter.h>

#include <map>
#include <string>

namespace pdal
{
	class PointView;
	class ProgramArgs;

	class GroupByFilter : public Filter
	{
	public:
		GroupByFilter();

		std::string getName() const;

	private:
		std::map<uint64_t, PointViewPtr> m_viewMap;
		std::string m_dimName;
		Dimension::Id m_dimId;

		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual PointViewSet run(PointViewPtr view);

		GroupByFilter& operator=(const GroupByFilter&); // not implemented
		GroupByFilter(const GroupByFilter&); // not implemented
	};
}

#endif // #ifndef GROUPBYFILTER_H
