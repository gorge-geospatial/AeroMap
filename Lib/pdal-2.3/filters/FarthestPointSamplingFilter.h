#ifndef FARTHESTPOINTSAMPLINGFILTER_H
#define FARTHESTPOINTSAMPLINGFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	class FarthestPointSamplingFilter : public pdal::Filter
	{
	public:
		FarthestPointSamplingFilter();

		FarthestPointSamplingFilter&
			operator=(const FarthestPointSamplingFilter&) = delete;
		FarthestPointSamplingFilter(const FarthestPointSamplingFilter&) = delete;

		std::string getName() const;

	private:
		point_count_t m_count;

		virtual void addArgs(ProgramArgs& args);
		virtual PointViewSet run(PointViewPtr view);
	};
}

#endif // #ifndef FARTHESTPOINTSAMPLINGFILTER_H
