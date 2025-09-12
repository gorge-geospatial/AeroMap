#ifndef HAGNNFILTER_H
#define HAGNNFILTER_H

#include <pdal/Filter.h>

#include <cstdint>
#include <memory>
#include <string>

namespace pdal
{
	class Options;
	class PointLayout;
	class PointView;

	class HagNnFilter : public Filter
	{
	public:
		HagNnFilter();
		HagNnFilter& operator=(const HagNnFilter&) = delete;
		HagNnFilter(const HagNnFilter&) = delete;

		std::string getName() const;

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void prepared(PointTableRef table);
		virtual void filter(PointView& view);

		bool m_allowExtrapolation;
		double m_maxDistance;
		point_count_t m_count;
	};
}

#endif // #ifndef HAGNNFILTER_H
