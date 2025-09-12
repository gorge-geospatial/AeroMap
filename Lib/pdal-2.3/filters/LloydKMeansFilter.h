#ifndef LLOYDKMEANSFILTER_H
#define LLOYDKMEANSFILTER_H

#include <pdal/Filter.h>

#include <string>

namespace pdal
{
	class ProgramArgs;

	class LloydKMeansFilter : public Filter
	{
	public:
		LloydKMeansFilter();

		LloydKMeansFilter& operator=(const LloydKMeansFilter&) = delete;
		LloydKMeansFilter(const LloydKMeansFilter&) = delete;

		std::string getName() const;

	private:
		uint16_t m_k;
		uint16_t m_maxiters;
		StringList m_dimStringList;
		Dimension::IdList m_dimIdList;

		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void prepared(PointTableRef table);
		virtual void filter(PointView& view);
	};
}

#endif // #ifndef LLOYDKMEANSFILTER_H
