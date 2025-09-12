#ifndef MERGEFILTER_H
#define MERGEFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

namespace pdal
{
	class MergeFilter : public Filter, 
						public Streamable
	{
	public:
		MergeFilter()
		{}

		std::string getName() const;

	private:
		PointViewPtr m_view;

		virtual void ready(PointTableRef table);
		virtual bool processOne(PointRef& point)
		{
			return true;
		}
		virtual PointViewSet run(PointViewPtr in);

		MergeFilter& operator=(const MergeFilter&); // not implemented
		MergeFilter(const MergeFilter&); // not implemented
	};
}

#endif // #ifndef MERGEFILTER_H
