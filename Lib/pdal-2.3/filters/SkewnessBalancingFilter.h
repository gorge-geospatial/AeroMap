#ifndef SKEWNESSBALANCINGFILTER_H
#define SKEWNESSBALANCINGFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	class SkewnessBalancingFilter : public Filter
	{
	public:
		SkewnessBalancingFilter() : Filter()
		{
		}

		std::string getName() const;

	private:
		virtual void addDimensions(PointLayoutPtr layout);
		void processGround(PointViewPtr view);
		virtual PointViewSet run(PointViewPtr view);

		SkewnessBalancingFilter& operator=(const SkewnessBalancingFilter&);     // not implemented
		SkewnessBalancingFilter(const SkewnessBalancingFilter&);				// not implemented
	};
}

#endif // #ifndef SKEWNESSBALANCINGFILTER_H
