#ifndef LOCATEFILTER_H
#define LOCATEFILTER_H

#include <pdal/Filter.h>

#include <map>
#include <string>

namespace pdal
{
	class PointView;
	class ProgramArgs;

	class LocateFilter : public Filter
	{
	public:
		LocateFilter() : Filter()
		{}

		std::string getName() const;

	private:
		std::string m_dimName;
		Dimension::Id m_dimId;
		std::string m_minmax;

		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual PointViewSet run(PointViewPtr view);

		LocateFilter& operator=(const LocateFilter&); // not implemented
		LocateFilter(const LocateFilter&); // not implemented
	};
}

#endif // #ifndef LOCATEFILTER_H
