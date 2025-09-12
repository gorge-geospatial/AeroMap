#ifndef HEADFILTER_H
#define HEADFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

namespace pdal
{
	class HeadFilter : public Filter, public Streamable
	{
	public:
		HeadFilter()
		{}
		HeadFilter& operator=(const HeadFilter&) = delete;
		HeadFilter(const HeadFilter&) = delete;

		std::string getName() const;

	private:
		point_count_t m_count;
		point_count_t m_index;
		bool m_invert;

		virtual void addArgs(ProgramArgs& args);
		virtual bool processOne(PointRef& point);
		virtual PointViewSet run(PointViewPtr view);
		virtual void ready(PointTableRef table);

	};
}

#endif // #ifndef HEADFILTER_H
