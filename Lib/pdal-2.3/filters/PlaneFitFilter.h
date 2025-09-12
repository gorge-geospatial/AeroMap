#ifndef PLANEFITFILTER_H
#define PLANEFITFILTER_H

#include <pdal/Filter.h>

#include <Eigen/Dense>

#include <string>

namespace pdal
{
	using namespace Eigen;

	class PlaneFitFilter : public Filter
	{
	public:
		PlaneFitFilter() : Filter()
		{}
		PlaneFitFilter& operator=(const PlaneFitFilter&) = delete;
		PlaneFitFilter(const PlaneFitFilter&) = delete;

		std::string getName() const;

	private:
		int m_knn;
		int m_threads;

		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void filter(PointView& view);

		void setPlaneFit(PointView& view, const PointId& i);
		double absDistance(PointView& view, const PointId& i, Vector3d& centroid, Vector3d& normal);
	};
}

#endif // #ifndef PLANEFITFILTER_H
