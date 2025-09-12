#ifndef OUTLIERFILTER_H
#define OUTLIERFILTER_H

#include <pdal/Filter.h>

#include <map>
#include <memory>
#include <string>

namespace pdal
{
	class Options;

	struct Indices
	{
		PointIdList inliers;
		PointIdList outliers;
	};

	class OutlierFilter : public pdal::Filter
	{
	public:
		OutlierFilter() : Filter()
		{
		}

		std::string getName() const;

	private:
		std::string m_method;
		int m_minK;
		double m_radius;
		int m_meanK;
		double m_multiplier;
		uint8_t m_class;

		virtual void addDimensions(PointLayoutPtr layout);
		virtual void addArgs(ProgramArgs& args);
		Indices processRadius(PointViewPtr inView);
		Indices processStatistical(PointViewPtr inView);
		virtual PointViewSet run(PointViewPtr view);

		OutlierFilter& operator=(const OutlierFilter&); // not implemented
		OutlierFilter(const OutlierFilter&);            // not implemented
	};
}

#endif // #ifndef OUTLIERFILTER_H
