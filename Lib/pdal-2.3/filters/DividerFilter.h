#ifndef DIVIDERFILTER_H
#define DIVIDERFILTER_H

#include <pdal/Filter.h>
#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	class DividerFilter : public Filter
	{
	public:
		DividerFilter()
		{}

		std::string getName() const;

	private:
		enum class Mode
		{
			Partition,
			RoundRobin
		};

		enum class SizeMode
		{
			Count,
			Capacity
		};

		Arg* m_cntArg;
		Arg* m_capArg;
		Mode m_mode;
		SizeMode m_sizeMode;
		point_count_t m_size;

		virtual void addArgs(ProgramArgs& args);
		virtual void initialize();
		virtual PointViewSet run(PointViewPtr view);

		DividerFilter& operator=(const DividerFilter&); // not implemented
		DividerFilter(const DividerFilter&); // not implemented

		friend std::istream& operator>>(std::istream& in, DividerFilter::Mode& mode);
		friend std::ostream& operator<<(std::ostream& in, const DividerFilter::Mode& mode);
	};
}

#endif // #ifndef DIVIDERFILTER_H
