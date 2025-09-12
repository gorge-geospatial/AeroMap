#ifndef ESTIMATERANKFILTER_H
#define ESTIMATERANKFILTER_H

#include <pdal/Filter.h>

#include <string>

namespace pdal
{
	class PointLayout;
	class PointView;

	class EstimateRankFilter : public Filter
	{
	public:
		EstimateRankFilter() : Filter()
		{}
		EstimateRankFilter& operator=(const EstimateRankFilter&) = delete;
		EstimateRankFilter(const EstimateRankFilter&) = delete;

		std::string getName() const;

	private:
		int m_knn;
		double m_thresh;

		virtual void addDimensions(PointLayoutPtr layout);
		virtual void addArgs(ProgramArgs& args);
		virtual void filter(PointView& view);
	};
}

#endif // #ifndef ESTIMATERANKFILTER_H
