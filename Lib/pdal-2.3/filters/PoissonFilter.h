#ifndef POISSONFILTER_H
#define POISSONFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	class PoissonFilter : public Filter
	{
	public:
		PoissonFilter() : Filter(), m_normalsProvided(false)
		{}
		PoissonFilter& operator=(const PoissonFilter&) = delete;
		PoissonFilter(const PoissonFilter&) = delete;

		std::string getName() const;

	private:
		bool m_density;
		int m_depth;
		bool m_normalsProvided;
		bool m_doColor;

		virtual void addDimensions(PointLayoutPtr layout);
		virtual PointViewSet run(PointViewPtr view);
		virtual void addArgs(ProgramArgs& args);
	};
}

#endif // #ifndef POISSONFILTER_H
