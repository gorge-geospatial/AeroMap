#ifndef MADFILTER_H
#define MADFILTER_H

#include <pdal/Filter.h>

#include <string>

namespace pdal
{
	class ProgramArgs;
	class PointView;

	class MADFilter : public Filter
	{
	public:
		MADFilter() : Filter()
		{}

		std::string getName() const;

	private:
		double m_multiplier;
		std::string m_dimName;
		Dimension::Id m_dimId;
		double m_madMultiplier;

		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual PointViewSet run(PointViewPtr view);

		MADFilter& operator=(const MADFilter&); // not implemented
		MADFilter(const MADFilter&); // not implemented
	};
}

#endif // #ifndef MADFILTER_H
