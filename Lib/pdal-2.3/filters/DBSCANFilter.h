#ifndef DBSCANFILTER_H
#define DBSCANFILTER_H

#include <pdal/Filter.h>

#include <string>

namespace pdal
{
	class ProgramArgs;

	class DBSCANFilter : public Filter
	{
	public:
		DBSCANFilter();

		DBSCANFilter& operator=(const DBSCANFilter&) = delete;
		DBSCANFilter(const DBSCANFilter&) = delete;

		std::string getName() const;

	private:
		uint64_t m_minPoints;
		double m_eps;
		StringList m_dimStringList;
		Dimension::IdList m_dimIdList;

		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void prepared(PointTableRef table);
		virtual void filter(PointView& view);
	};
}

#endif // #ifndef DBSCANFILTER_H
