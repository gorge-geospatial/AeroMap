#ifndef HAGDELAUNAYFILTER_H
#define HAGDELAUNAYFILTER_H

#include <pdal/Filter.h>

#include <cstdint>
#include <memory>
#include <string>

namespace pdal
{
	class Options;
	class PointLayout;
	class PointView;

	class HagDelaunayFilter : public Filter
	{
	public:
		HagDelaunayFilter();
		HagDelaunayFilter& operator=(const HagDelaunayFilter&) = delete;
		HagDelaunayFilter(const HagDelaunayFilter&) = delete;

		std::string getName() const;

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void prepared(PointTableRef table);
		virtual void filter(PointView& view);

		bool m_allowExtrapolation;
		point_count_t m_count;
	};
}

#endif // #ifndef HAGDELAUNAYFILTER_H
