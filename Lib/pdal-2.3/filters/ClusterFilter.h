#ifndef CLUSTERFILTER_H
#define CLUSTERFILTER_H

#include <pdal/Filter.h>

#include <string>

namespace pdal
{
	class ProgramArgs;

	class ClusterFilter : public Filter
	{
	public:
		ClusterFilter() : Filter()
		{}
		ClusterFilter& operator=(const ClusterFilter&) = delete;
		ClusterFilter(const ClusterFilter&) = delete;

		std::string getName() const;

	private:
		uint64_t m_minPoints;
		uint64_t m_maxPoints;
		double m_tolerance;
		bool m_is3d;

		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void filter(PointView& view);
	};
}

#endif // #ifndef CLUSTERFILTER_H
