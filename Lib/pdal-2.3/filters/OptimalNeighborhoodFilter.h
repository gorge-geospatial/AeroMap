#ifndef OPTIMALNEIGHBORHOODFILTER_H
#define OPTIMALNEIGHBORHOODFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	class OptimalNeighborhood : public Filter
	{
	public:
		OptimalNeighborhood();

		OptimalNeighborhood& operator=(const OptimalNeighborhood&) = delete;
		OptimalNeighborhood(const OptimalNeighborhood&) = delete;

		std::string getName() const;

	private:
		point_count_t m_kMin, m_kMax;

		virtual void addDimensions(PointLayoutPtr layout);
		virtual void addArgs(ProgramArgs& args);
		virtual void filter(PointView& view);
	};
}

#endif // #ifndef OPTIMALNEIGHBORHOODFILTER_H
