#ifndef DECIMATIONFILTER_H
#define DECIMATIONFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

namespace pdal
{
	// we keep only 1 out of every step points; if step=100, we get 1% of the file
	class DecimationFilter : public Filter, public Streamable
	{
	public:
		DecimationFilter()
		{}

		std::string getName() const;

	private:
		uint32_t m_step;
		uint32_t m_offset;
		point_count_t m_limit;
		PointId m_index;

		virtual void addArgs(ProgramArgs& args);
		void ready(PointTableRef table)
		{
			m_index = 0;
		}
		bool processOne(PointRef& point);
		PointViewSet run(PointViewPtr view);
		void decimate(PointView& input, PointView& output);

		DecimationFilter& operator=(const DecimationFilter&); // not implemented
		DecimationFilter(const DecimationFilter&); // not implemented
	};
}

#endif // #ifndef DECIMATIONFILTER_H
