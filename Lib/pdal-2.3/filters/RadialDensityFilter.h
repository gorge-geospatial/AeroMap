#ifndef RADIALDENSITYFILTER_H
#define RADIALDENSITYFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	class PointLayout;
	class PointView;
	class ProgramArgs;

	class RadialDensityFilter : public Filter
	{
	public:
		RadialDensityFilter() : Filter()
		{}
		RadialDensityFilter& operator=(const RadialDensityFilter&) = delete;
		RadialDensityFilter(const RadialDensityFilter&) = delete;

		std::string getName() const;

	private:
		double m_rad;

		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void filter(PointView& view);
	};
}

#endif // #ifndef RADIALDENSITYFILTER_H
