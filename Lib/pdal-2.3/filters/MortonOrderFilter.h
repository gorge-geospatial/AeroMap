#ifndef MORTONORDERFILTER_H
#define MORTONORDERFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	class MortonOrderFilter : public pdal::Filter
	{
	public:
		MortonOrderFilter()
		{}
		MortonOrderFilter& operator=(const MortonOrderFilter&) = delete;
		MortonOrderFilter(const MortonOrderFilter&) = delete;

		std::string getName() const;

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual PointViewSet run(PointViewPtr view);

		PointViewSet reverseMorton(PointViewPtr view);
		PointViewSet morton(PointViewPtr view);

		bool m_reverse = false;
	};
}

#endif // #ifndef MORTONORDERFILTER_H
