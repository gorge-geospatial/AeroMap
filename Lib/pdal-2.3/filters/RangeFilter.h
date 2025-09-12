#ifndef RANGEFILTER_H
#define RANGEFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

#include <memory>
#include <map>
#include <string>

namespace pdal
{
	struct DimRange;

	class RangeFilter : public Filter, public Streamable
	{
	public:
		RangeFilter();
		~RangeFilter();

		std::string getName() const;

	private:
		std::vector<DimRange> m_ranges;

		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual bool processOne(PointRef& point);
		virtual PointViewSet run(PointViewPtr view);

		RangeFilter& operator=(const RangeFilter&) = delete;
		RangeFilter(const RangeFilter&) = delete;
	};

}

#endif // #ifndef RANGEFILTER_H
